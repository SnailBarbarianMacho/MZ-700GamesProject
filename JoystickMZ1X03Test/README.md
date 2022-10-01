# Joystick MZ-1X03 test
画面の左上に「上下左右BA」の表示をします.
検出出来なかった場合は, 'X' を表示します.<br>
The upper left corner of the screen displays 'up, down, left, right, BA'.
If the detection fails, 'X' will be displayed.

## 非接続の判定方法 / How to determine disconnection

![pic](MZ-1X03_TimingChart.png)

MZ-1X03 の軸は, /VBLK の立下がり後, 少しの遅延の後に '0' になり, ((10 + n) * 28) T states 後に '1' (n=0～255) になるようです.<br>
本プログラムは, /VBLK の立下がり後, 約 150 T states 後に '1' を検出したら非接続と判定します.<br>
The axis of MZ-1X03 seems to be '0' after a short delay after the falling edge of /VBLK, and '1' after ((10 + n) * 28) T states (n=0-255).<br>
This program judges the connection to be disconnected when it detects '1' about 150 T states after the falling edge of /VBLK.<br>
