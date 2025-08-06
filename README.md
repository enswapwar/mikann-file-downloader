# Mikann File Downloader for 3DS

3DS向けの自作ファイルダウンローダー。  
CFW環境で動作し、URLを入力して任意ファイル（最大16GB）をSDカードに保存可能。  
4GBを超えるファイルでは警告が表示される。

## 🔧 必要環境

- devkitPro / devkitARM（https://devkitpro.org/wiki/Getting_Started）
- ctrulib（libctru）
- makerom（.ciaビルドに使用）

## 🛠 ビルド方法

```bash
git clone https://github.com/enswapwar/mikann-file-downloader.git
cd mikann-file-downloader
make

ここではgithubactionで自動的にビルドされる。
