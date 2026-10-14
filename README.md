# DXF_fix

SolidCraftが出力する最小構成のASCII DXFを、Illustrator向けのAutoCAD R12 (AC1009) DXFへ書き直すC++コンソールアプリです。外部ライブラリは不要です。

## 調査結果

提供されたSketchTest_app.dxfとSketchTest.dxfは、それぞれLINE 4本とCIRCLE 1個です。矩形の寸法は前者37 × 36、後者45 × 36、円の半径はそれぞれ31.531730051、36.830693721です。単位指定はmmです。両ファイルは同一ではありません。

両方ともHEADERには$INSUNITS=4だけがあり、$ACADVER、TABLES、BLOCKS、各図形のZ座標がありません。グループコードと値の対、ENDSEC、EOFは揃っています。この最小構成をIllustratorの読み込み処理が受け付けない可能性があります。省略項目すべてが必須という意味ではなく、どの項目が直接の原因かはIllustratorでの比較試験が必要です。

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

## ビルドと使用方法

Visual StudioでDXF_fix.slnxを開き、Release / x64をビルドします。生成物は通常 x64/Release/DXF_fix.exe です。

`build_release.cmd` はVisual Studioの開発環境を初期化してから、ソリューションをMSBuildでビルドします。コンパイラーの直接実行や独自の出力先指定は行いません。出力先・中間ディレクトリはVisual Studioの既定値を使用します。x64 / Releaseでは実行ファイルはソリューション配下の `x64/Release/`、中間ファイルはプロジェクト配下の `DXF_fix/x64/Release/` です。

```powershell
.\x64\Release\DXF_fix.exe "D:\Downloads\SketchTest_app.dxf"
.\x64\Release\DXF_fix.exe "D:\Downloads\SketchTest.dxf" ".\SketchTest_fixed.dxf"
```

出力先を省略すると入力と同じフォルダーの `元の名前_fixed.dxf` になります。日本語を含むWindowsのパスにも対応します。成功時の終了コードは0、失敗時は1です。

Illustratorでは「ファイル > 開く」から修正ファイルを選び、「元のサイズ」「1単位 = 1mm」で読み込んでください。矩形の寸法と円の半径を確認してください。Illustratorによる実際の読み込みは未検証です。

## 自動確認

```powershell
python tests/test_fix.py
```

Release版を先にビルドしてください。提供ファイルの図形・座標維持、R12ヘッダー、Z座標、CRLF、不正入力と上書き拒否を検査します。
