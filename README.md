# DXF_fix

[SolidCraft](https://bherbruck.github.io/solidcraft/app/)で書き出したDXFを、Adobe Illustratorで読み込める形式に整えるWindows用ツールです。元の図形の座標や円の半径を維持し、修正したファイルを別名で保存します。

提供されたサンプルでは、修正後にIllustratorで問題なく読み込めることを確認しています。

## 使い方

`DXF_fix.exe` が入っているフォルダーでターミナル（PowerShell）を開き、次を実行します。入力ファイルのパスは、お使いのファイルに置き換えてください。

```powershell
.\DXF_fix.exe "D:\Downloads\SketchTest_app.dxf"
```

入力と同じフォルダーに `SketchTest_app_fixed.dxf` が作成されます。元のファイルは変更しません。`OK:` と表示されれば変換完了です。

保存先や名前を指定する場合は、2つ目の引数に出力ファイルのパスを指定します。保存先のフォルダーはあらかじめ作成してください。

```powershell
.\DXF_fix.exe "D:\Downloads\SketchTest.dxf" "D:\Downloads\SketchTest_for_Illustrator.dxf"
```

スペースや日本語を含むパスにも対応しています。パスは上の例のように引用符で囲んでください。

## Illustratorで開く

1. Illustratorの「ファイル → 開く」で修正したDXFを選択します。
2. 読み込みオプションで「元のサイズ」を選び、単位を **1単位＝1mm** に設定します。
3. 読み込んだ図形の寸法を確認してください。

## 対応するファイル

SolidCraftが出力する、直線・円・円弧だけを含む簡単なDXFが対象です。単位はmmです。単位情報のないファイルもmmとして扱います。

その他の図形や複雑なDXFは変換できない場合があります。修正済みのファイルを再変換する必要はありません。

## エラーが出た場合

| 表示・状況 | 対処方法 |
| --- | --- |
| `Output exists` | 同名の出力ファイルが存在します。別の出力名を指定してください。既存ファイルは上書きしません。 |
| `Cannot open input` | 入力ファイルの場所と名前を確認してください。 |
| `Cannot create output` | 保存先フォルダーが存在するか、書き込みできるかを確認してください。 |
| `Unsupported` / `Only minimal SolidCraft files supported` | 対応範囲外の図形やデータが含まれています。SolidCraftから簡単な図形のDXFとして書き出し直してください。 |
| `Only millimetre input is supported` | mm以外の単位が指定されています。mmで書き出し直してください。 |
| 読み込んだ図形の大きさが違う | Illustratorで「元のサイズ」「1単位＝1mm」を指定して開き直してください。 |

調査結果、修正の仕様、ビルド・テスト手順は [開発者向け情報](DEVELOPMENT.md) を参照してください。
