# 開発者向け情報

利用方法は [README.md](README.md) を参照してください。

## 調査結果


提供されたSketchTest_app.dxfとSketchTest.dxfは、それぞれLINE 4本とCIRCLE 1個です。矩形の寸法は前者37 × 36、後者45 × 36、円の半径はそれぞれ31.531730051、36.830693721です。単位指定はmmです。両ファイルは同一ではありません。

両方ともHEADERには$INSUNITS=4だけがあり、$ACADVER、TABLES、BLOCKS、各図形のZ座標がありません。グループコードと値の対、ENDSEC、EOFは揃っています。この最小構成をIllustratorの読み込み処理が受け付けない可能性があります。省略項目すべてが必須という意味ではなく、どの項目が直接の原因かは、項目ごとの比較試験を行っていないため特定していません。

Adobeの公式案内ではAutoCAD 2.5〜2007の読み込みに対応しています。R12はこの範囲内です。

- [Adobe: IllustratorでAutoCADファイルを読み込む](https://helpx.adobe.com/jp/illustrator/desktop/add-and-import-files/import-other-file-types/import-autocad-files.html)
- [Autodesk: HEADERのバージョン番号（AC1009 = R11/R12）](https://help.autodesk.com/cloudhelp/2024/ENU/AutoCAD-DXF/files/GUID-A85E8E67-27CD-4C59-BE61-4DC9FADBE74A.htm)
- [SolidCraft](https://bherbruck.github.io/solidcraft/app/)

## 修正内容

- $ACADVER=AC1009、CONTINUOUS線種、使用レイヤーの定義、空のBLOCKSセクションを生成。
- 未指定のZ座標に0を追加。既存の座標、半径、角度は文字列表現も維持。
- R12に合わせて$INSUNITSを除去。座標値はmmのままで、Illustratorの読み込み時に単位をmmに指定する必要があります。
- ASCII、BOMなし、CRLFで出力。元ファイルや既存の出力ファイルは上書きしません。

対応範囲はHEADER/ENTITIESだけの最小DXF、およびLINE/CIRCLE/ARCです。非mmの単位指定、未対応の図形・属性・セクション、不正な数値や構造はエラーにします。一般のDXF変換器ではありません。修正済みファイルの再入力もTABLESがあるため拒否します。単位指定がない入力はmmと解釈します。元のHEADERのその他の設定は引き継ぎません。

## Illustratorでの確認結果

2026年10月14日、ユーザーから修正済みDXFをIllustratorで問題なく読み込めたとの報告がありました。この結果は今回の提供ファイルに対するもので、すべてのDXFやIllustratorのバージョンでの動作を保証するものではありません。

## ビルド

Visual Studioで `DXF_fix.slnx` を開き、構成を `Release`、プラットフォームを `x64` にしてビルドします。既存プロジェクトはv145ツールセット、Windows SDK、C++20を使用します。Visual Studioの「C++によるデスクトップ開発」が必要です。外部ライブラリは不要です。

コマンドからビルドする場合は、ソリューションのフォルダーで次を実行します。

```powershell
.\build_release.cmd
```

このスクリプトはVisual Studioの開発環境を初期化し、ソリューションをMSBuildでビルドします。コンパイラーの直接実行や独自の出力先指定は行いません。出力先と中間ディレクトリはVisual Studio標準に準拠します。

| 種類 | x64 / Releaseでの場所（ソリューション基準） |
| --- | --- |
| 実行ファイル | `x64/Release/DXF_fix.exe` |
| 中間ファイル | `DXF_fix/x64/Release/` |

この環境では、開発環境の初期化なしで実行したビルドで `LNK1101: MSPDB140.DLL バージョンが正しくありません` が発生しました。開発環境を初期化してから標準設定でビルドすることで成功しています。

## 自動確認

Release / x64を先にビルドし、ソリューションのフォルダーで次を実行します。

```powershell
python tests/test_fix.py
```

Pythonが必要です。提供ファイルの `D:/Downloads/SketchTest_app.dxf` と `D:/Downloads/SketchTest.dxf` を参照するため、別の環境ではテスト内の入力パスを変更してください。

図形・座標の維持、R12ヘッダー、Z座標、CRLF、不正入力と上書きの拒否、円弧、日本語パスを確認します。

## 終了コード

- `0`: 変換成功、または引数なしで使用方法を表示。
- `1`: 引数、入力ファイル、変換処理、出力などのエラー。
