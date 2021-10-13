# MZ-700 Game Projects

* ある日唐突に MZ-700 で動くゲームを作ってみたくなったので作りました<br>
  One day I suddenly wanted to make a game that runs on the MZ-700.
  Nowadays, there are 8-bit cross-compilers and emulators. So I made it.
* 実行可能バイナリ (.mzt) は残してるので, ビルドしないで遊ぶことができます<br>
  I'm leaving the executable binaries (.mzt) so you can play without building.
* EmuZ-700 エミュレータでしか動作確認してません<br>
  I confirmed the operation only with the EmuZ-700 emulator.

# Features

* 複数のサブ プロジェクトが中に入ってます<br>
  There are multiple subprojects inside.
* **Test** <br>
  コンパイラの動作テスト. 今はもう動かないかもしれません<br>
  Compiler operation test. May not work anymore.
* **SoundTest** <br>
  三重和音サウンドテスト <br>
  3 chord sound test.<br>
  https://www.youtube.com/watch?v=Qs5GZ-hzjp4
* **ST-2** <br>
  習作 縦シュー<br>
  Study shooting game.<br>
  https://www.youtube.com/watch?v=PG0ijYIvdNw
* 以後, 妻が許せばゲームを増やす予定<br>
  I'm planning to increase the number of games, if my wife allows it.

# Do you want to play early? 手っ取り早く遊びたい人のガイド(for Windows)

1. Set up the EmuZ-700 emulator and image ROMs.
2. Download the .mzt file from this repository.

# Requirement

* **EmuZ-700** http://takeda-toshiya.my.coocan.jp/common/index.html<br>
  代表的な MZ-700 エミュレータ.<br>
  A typical MZ-700 emulator. <br>
  モニター ROM イメージと日本版フォントイメージが必要です.<br>
  Needs MZ-700 monitor ROM image and Japanese font image.<br>
  互換イメージがここから入手できます:<br>
  Compatible images are available here: http://mzakd.cool.coocan.jp/starthp/mz700win.html
* Windows 10 + Power Shell 7
* **z88dk** https://github.com/z88dk/z88dk/wiki <br>
  nightly build 20211001
* **php 8.0** https://windows.php.net/ <br>
  mbstring を有効化してください.<br>
  I'm using mbstring, so enable it.
* **gmake 3.81 for Windows** http://gnuwin32.sourceforge.net/packages/make.htm
* **exomizer 3.1.1** https://bitbucket.org/magli143/exomizer/wiki/Home<br>
  Exomizer は用意してないのでソースを入手してビルドしてください.<br>
  Needs ANSI-C compiler. Build exomizer.exe and put in tools/.

# Build

* ビルドはプロジェクト ルートで<br>
  Build at the project root:
```
>make
```
* 又は, 各サブ プロジェクト フォルダへ移動して<br>
  Or, in each subproject folder:
```
>cd SoundTest
>make
```
* それぞれ, make clean や meke release が用意されてます<br>
  'make clean' and 'meke release' are available respectively.

# EmuZ-700 あんちょこ Cheat Sheet

* エミュレータのおすすめ設定 Recommended emulator settings
  * Uncheck [Device]->[Sound]->[Play CMT Noise]
  * Uncheck [Device]->[Sound]->[Play CMT Sound]
  * Check [Device]->[Joystick]->[AM7J Adapter]
  * Use XInput Joystick.
* MZT ファイルのロードと実行 How to load and run the MZT program
  1. Reset once the CPU [Control]->[Reset]
  2. Set the MZT file [CMT]->[Play]
  3. Type 'L[CR]' on the monitor. The program is automatically loaded and executed.

# Author

* SnailBarbarianMacho

# License

* This project is under [MIT license]<br>
  except deexo3.c (original by uniabis) is under [zlib license]

# Thanks to

* MZ-700 ハードウェア資料を提供し, ビット マップのアルゴリズムを考え, EGC7 を作ってくれた 紅茶羊羹氏<br>
  KochaYokan, who provided the MZ-700 hardware materials, considered the bitmap algorithm, and created EGC7.
* Exomizer を作ってくれた Magnus Lind 氏らと, その depacker を作ってくれた uniabis 氏<br>
  Magnus Lind and others who made the Exomizer and uniabis who made the depacker
* 再現性の高いエミュレータ EmuZ-700 を作ってくれた TAKEDA, toshiya 氏<br>
  Mr. TAKEDA, toshiya who made the highly reproducible emulator EmuZ-700
