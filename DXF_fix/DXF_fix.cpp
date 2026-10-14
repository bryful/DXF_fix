// DXF_fix.cpp : このファイルには 'main' 関数が含まれています。プログラム実行の開始と終了がそこで行われます。
//

#include <iostream>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <vector>
#include <set>
#include <cmath>
#include <cstdio>
#include <locale>
#include <stdexcept>

namespace fs = std::filesystem;
struct Pair { int code; std::string value; };
using Record = std::vector<Pair>;
std::string trim(const std::string& s) {
    auto a = s.find_first_not_of(" \t\r\n");
    return a == s.npos ? "" : s.substr(a, s.find_last_not_of(" \t\r\n") - a + 1);
}
int integer(const std::string& s) {
    size_t end; int n = std::stoi(s, &end);
    if (end != s.size()) throw std::runtime_error("Invalid integer: " + s);
    return n;
}
double number(const std::string& s) {
    std::istringstream in(s); in.imbue(std::locale::classic());
    double n;
    if (!(in >> n) || !std::isfinite(n)) throw std::runtime_error("Invalid number: " + s);
    in >> std::ws;
    if (!in.eof()) throw std::runtime_error("Invalid number: " + s);
    return n;
}
std::string convert(const fs::path& path, size_t& count) {
    std::ifstream in(path, std::ios::binary);
    if (!in) throw std::runtime_error("Cannot open input.");
    std::vector<Pair> pairs;
    std::string c, v;
    while (std::getline(in, c)) {
        if (pairs.empty() && c.substr(0, 3) == "\xef\xbb\xbf") c.erase(0, 3);
        if (!std::getline(in, v)) throw std::runtime_error("Missing group value.");
        int code = integer(trim(c));
        if (code < 0 || code > 1071) throw std::runtime_error("Invalid group code.");
        pairs.push_back({code, trim(v)});
    }
    if (in.bad()) throw std::runtime_error("Input read failed.");
    std::vector<Record> records;
    std::set<std::string> sections;
    size_t i = 0; bool eof = false;
    while (i < pairs.size()) {
        const auto p = pairs[i++];
        if (p.code == 999) continue;
        if (p.code == 0 && p.value == "EOF") { eof = true; break; }
        if (p.code != 0 || p.value != "SECTION") throw std::runtime_error("Expected SECTION.");
        if (i == pairs.size() || pairs[i].code != 2) throw std::runtime_error("Missing section name.");
        const auto section = pairs[i++].value;
        if (!sections.insert(section).second) throw std::runtime_error("Duplicate section.");
        // Reject richer drawings rather than silently losing blocks or unsupported objects.
        if (section != "HEADER" && section != "ENTITIES") throw std::runtime_error("Only minimal SolidCraft files supported; section: " + section);
        while (i < pairs.size() && !(pairs[i].code == 0 && pairs[i].value == "ENDSEC")) {
            if (section == "HEADER") {
                if (pairs[i].code == 0) throw std::runtime_error("Malformed HEADER.");
                if (pairs[i].code == 9 && pairs[i].value == "$INSUNITS") {
                    if (i + 1 >= pairs.size() || pairs[i+1].code != 70 || integer(pairs[i+1].value) != 4)
                        throw std::runtime_error("Only millimetre input is supported.");
                }
                ++i; continue;
            }
            if (pairs[i].code == 999) { ++i; continue; }
            if (pairs[i].code != 0) throw std::runtime_error("Missing entity type.");
            Record r{pairs[i++]};
            while (i < pairs.size() && pairs[i].code != 0) r.push_back(pairs[i++]);
            records.push_back(r);
        }
        if (i == pairs.size()) throw std::runtime_error("Missing ENDSEC.");
        ++i;
    }
    if (!eof || i != pairs.size() || records.empty()) throw std::runtime_error("Missing EOF, trailing data or no geometry.");
    std::set<std::string> layers{"0"};
    for (auto& r : records) {
        const auto type = r[0].value;
        std::set<int> allowed{8,6,62,39,210,220,230};
        std::vector<int> required{10,20};
        if (type == "LINE") { allowed.insert({10,20,30,11,21,31}); required.insert(required.end(), {11,21}); }
        else if (type == "CIRCLE" || type == "ARC") {
            allowed.insert({10,20,30,40}); required.push_back(40);
            if (type == "ARC") { allowed.insert({50,51}); required.insert(required.end(), {50,51}); }
        } else throw std::runtime_error("Unsupported entity: " + type);
        std::set<int> seen;
        for (size_t j = 1; j < r.size(); ++j) {
            const auto& p = r[j];
            if (!allowed.count(p.code) || !seen.insert(p.code).second) throw std::runtime_error("Unsupported or duplicate group in " + type);
            if (p.code == 8) {
                if (p.value.empty() || p.value.size() > 31 || p.value.find_first_of("<>/\\\":;?*|=,") != std::string::npos)
                    throw std::runtime_error("Unsupported R12 layer name.");
                for (unsigned char ch : p.value) if (ch < 32 || ch >= 127) throw std::runtime_error("R12 layer names must be ASCII.");
                layers.insert(p.value);
            } else if (p.code == 6) {
                if (p.value != "CONTINUOUS" && p.value != "BYLAYER" && p.value != "BYBLOCK") throw std::runtime_error("Unsupported linetype.");
            } else if (p.code == 62) {
                int n = integer(p.value); if (n < -255 || n > 256) throw std::runtime_error("Invalid color.");
            } else {
                double n = number(p.value);
                if (p.code == 40 && n <= 0) throw std::runtime_error("Radius must be positive.");
            }
        }
        for (int code : required) if (!seen.count(code)) throw std::runtime_error("Missing coordinate/radius/angle in " + type);
        if (!seen.count(8)) r.push_back({8,"0"});
        if (!seen.count(30)) r.push_back({30,"0"});
        if (type == "LINE" && !seen.count(31)) r.push_back({31,"0"});
    }
    std::ostringstream out;
    auto put = [&](int code, const std::string& value) { out << code << "\r\n" << value << "\r\n"; };
    put(999,"DXF_fix R12: coordinates in millimetres");
    put(0,"SECTION"); put(2,"HEADER"); put(9,"$ACADVER"); put(1,"AC1009");
    put(9,"$LUNITS"); put(70,"2"); put(9,"$LUPREC"); put(70,"9"); put(0,"ENDSEC");
    put(0,"SECTION"); put(2,"TABLES");
    put(0,"TABLE"); put(2,"LTYPE"); put(70,"1");
    put(0,"LTYPE"); put(2,"CONTINUOUS"); put(70,"0"); put(3,"Solid line");
    put(72,"65"); put(73,"0"); put(40,"0"); put(0,"ENDTAB");
    put(0,"TABLE"); put(2,"LAYER"); put(70,std::to_string(layers.size()));
    for (const auto& layer : layers) { put(0,"LAYER"); put(2,layer); put(70,"0"); put(62,"7"); put(6,"CONTINUOUS"); }
    put(0,"ENDTAB"); put(0,"ENDSEC");
    put(0,"SECTION"); put(2,"BLOCKS"); put(0,"ENDSEC");
    put(0,"SECTION"); put(2,"ENTITIES");
    for (const auto& r : records) for (const auto& p : r) put(p.code,p.value);
    put(0,"ENDSEC"); put(0,"EOF"); count = records.size(); return out.str();
}
#ifdef _WIN32
int wmain(int argc, wchar_t** argv) {
#else
int main(int argc, char** argv) {
#endif
    if (argc != 2 && argc != 3) {
        std::cout << "Usage: DXF_fix input.dxf [output.dxf]\nDefault: input_fixed.dxf; never overwrite existing files.\nIllustrator: Original Size, 1 unit = 1 mm.\n";
        return argc == 1 ? 0 : 1;
    }
    try {
        fs::path input(argv[1]);
        fs::path output = argc == 3 ? fs::path(argv[2]) : input.parent_path() / (input.stem().wstring() + L"_fixed.dxf");
        if (fs::exists(output)) throw std::runtime_error("Output exists; choose a new name.");
        size_t count = 0; const auto data = convert(input,count);
        FILE* file = nullptr;
#ifdef _WIN32
        if (_wfopen_s(&file,output.c_str(),L"wbx") != 0 || !file) throw std::runtime_error("Cannot create output.");
#else
        file = fopen(output.c_str(),"wbx");
        if (!file) throw std::runtime_error("Cannot create output.");
#endif
        bool written = fwrite(data.data(),1,data.size(),file) == data.size();
        bool closed = fclose(file) == 0;
        if (!written || !closed) { std::error_code ec; fs::remove(output,ec); throw std::runtime_error("Output write failed."); }
        std::cout << "OK: " << count << " entities preserved; R12 written.\nIllustrator: Original Size, 1 unit = 1 mm.\n";
        return 0;
    } catch (const std::exception& e) { std::cerr << "Error: " << e.what() << '\n'; return 1; }
}

// プログラムの実行: Ctrl + F5 または [デバッグ] > [デバッグなしで開始] メニュー
// プログラムのデバッグ: F5 または [デバッグ] > [デバッグの開始] メニュー

// 作業を開始するためのヒント: 
//    1. ソリューション エクスプローラー ウィンドウを使用してファイルを追加/管理します 
//   2. チーム エクスプローラー ウィンドウを使用してソース管理に接続します
//   3. 出力ウィンドウを使用して、ビルド出力とその他のメッセージを表示します
//   4. エラー一覧ウィンドウを使用してエラーを表示します
//   5. [プロジェクト] > [新しい項目の追加] と移動して新しいコード ファイルを作成するか、[プロジェクト] > [既存の項目の追加] と移動して既存のコード ファイルをプロジェクトに追加します
//   6. 後ほどこのプロジェクトを再び開く場合、[ファイル] > [開く] > [プロジェクト] と移動して .sln ファイルを選択します
