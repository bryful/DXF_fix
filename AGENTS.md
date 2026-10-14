# Visual Studio作業方針

- ユーザーの指定により、Visual Studio関連作業は必ずVisual Studio標準に準拠すること。
- ソリューションと既存のvcxprojを使用してビルドする。コンパイラーの直接実行で独自のビルド経路を作らない。
- 出力ディレクトリ・中間ディレクトリはVisual Studioの既定値を使用し、独自のOutDir/IntDirや/Fo、/Feを指定しない。
- ビルド環境の問題は、まずVisual Studioの開発環境を初期化して確認し、プロジェクトの標準設定を迂回しない。
