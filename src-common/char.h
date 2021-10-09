/**
 * ディスプレイ コードの文字
 * - 名前空間 CHAR_
 *
 * @author Snail Barbarian Macho (NWK)
 */

#ifndef CHAR_H_INCLUDED
#define CHAR_H_INCLUDED

// ---------------------------------------------------------------- 文字のディスプレイ コード
#define CHAR_A              0x01
#define CHAR_B              0x02
#define CHAR_C              0x03
#define CHAR_D              0x04
#define CHAR_E              0x05
#define CHAR_F              0x06
#define CHAR_G              0x07
#define CHAR_H              0x08
#define CHAR_I              0x09
#define CHAR_J              0x0a
#define CHAR_K              0x0b
#define CHAR_L              0x0c
#define CHAR_M              0x0d
#define CHAR_N              0x0e
#define CHAR_O              0x0f
#define CHAR_P              0x10
#define CHAR_Q              0x11
#define CHAR_R              0x12
#define CHAR_S              0x13
#define CHAR_T              0x14
#define CHAR_U              0x15
#define CHAR_V              0x16
#define CHAR_W              0x17
#define CHAR_X              0x18
#define CHAR_Y              0x19
#define CHAR_Z              0x1a

#define CHAR_0              0x20
#define CHAR_1              0x21
#define CHAR_2              0x22
#define CHAR_3              0x23
#define CHAR_4              0x24
#define CHAR_5              0x25
#define CHAR_6              0x26
#define CHAR_7              0x27
#define CHAR_8              0x28
#define CHAR_9              0x29

#define CHAR_HYPHEN         0x2a    // -
#define CHAR_EQUALS         0x2b    // =
#define CHAR_SEMICOLON      0x2c    // ;
#define CHAR_SLASH          0x2d    // /
#define CHAR_PERIOD         0x2e    // .
#define CHAR_COMMA          0x2f    // ,

#define CHAR_SQUARE         0x43    // ■

#define CHAR_R_ARROW        0x40    // ←
#define CHAR_QUESTION       0x49    // ?
#define CHAR_COLON          0x4f    // :

#define CHAR_SPADE          0x41    // ♠
#define CHAR_DIAMOND        0x44    // ♦
#define CHAR_CLUB           0x46    // ♣
#define CHAR_HEART          0x53    // ♥

#define CHAR_U_ARROW        0x50    // ↑
#define CHAR_LT             0x51    // <
#define CHAR_L_SQ_BLACKET   0x52    // [
#define CHAR_R_SQ_BLACKET   0x54    // ]
#define CHAR_AT             0x55    // @
#define CHAR_GT             0x57    // >
#define CHAR_BACK_SLASH     0x59    // /

#define CHAR_PI             0x60    // π
#define CHAR_EXCLAMATION    0x61    // !
#define CHAR_QUOTATION      0x62    // "
#define CHAR_NUMBER         0x63    // #
#define CHAR_DOLLER         0x64    // $
#define CHAR_PERCENT        0x65    // %
#define CHAR_AMPERSAND      0x66    // &
#define CHAR_APOSTROPHE     0x67    // '
#define CHAR_L_PARENTHESIS  0x68    // (
#define CHAR_R_PARENTHESIS  0x69    // )
#define CHAR_PLUS           0x6a    // +
#define CHAR_ASTERISK       0x6b    // *

#define CHAR_KANA_CHI       0x81    // ち
#define CHAR_KANA_KO        0x82    // こ
#define CHAR_KANA_SO        0x83    // そ
#define CHAR_KANA_SHI       0x84    // し
#define CHAR_KANA_I         0x85    // い
#define CHAR_KANA_HA        0x86    // は
#define CHAR_KANA_KI        0x87    // き
#define CHAR_KANA_KU        0x88    // く
#define CHAR_KANA_NI        0x89    // な
#define CHAR_KANA_MA        0x8a    // ま
#define CHAR_KANA_NO        0x8b    // の
#define CHAR_KANA_RI        0x8c    // り
#define CHAR_KANA_MO        0x8d    // も
#define CHAR_KANA_MI        0x8e    // み
#define CHAR_KANA_RA        0x8f    // ら

#define CHAR_KANA_SE        0x90    // せ
#define CHAR_KANA_TA        0x91    // た
#define CHAR_KANA_SU        0x92    // す
#define CHAR_KANA_TO        0x93    // と
#define CHAR_KANA_KA        0x94    // か
#define CHAR_KANA_NA        0x95    // な
#define CHAR_KANA_HI        0x96    // ひ
#define CHAR_KANA_TE        0x97    // て
#define CHAR_KANA_SA        0x98    // さ
#define CHAR_KANA_N         0x99    // ん
#define CHAR_KANA_TSU       0x9a    // つ
#define CHAR_KANA_RO        0x9b    // ろ
#define CHAR_KANA_KE        0x9c    // け
#define CHAR_L_BRACKET      0x9d    // 「
#define CHAR_KANA_XA        0x9e    // ぁ
#define CHAR_KANA_XYA       0x9f    // ゃ

#define CHAR_KANA_WA        0xa0    // わ
#define CHAR_KANA_NU        0xa1    // ぬ
#define CHAR_KANA_FU        0xa2    // ふ
#define CHAR_KANA_A         0xa3    // あ
#define CHAR_KANA_U         0xa4    // う
#define CHAR_KANA_E         0xa5    // え
#define CHAR_KANA_O         0xa6    // お
#define CHAR_KANA_YA        0xa7    // や
#define CHAR_KANA_YU        0xa8    // ゆ
#define CHAR_KANA_YO        0xa9    // よ
#define CHAR_KANA_HO        0xaa    // ほ
#define CHAR_KANA_HE        0xab    // へ
#define CHAR_KANA_RE        0xac    // れ
#define CHAR_KANA_ME        0xad    // め
#define CHAR_KANA_RU        0xae    // る
#define CHAR_KANA_NE        0xaf    // ね

#define CHAR_KANA_MU        0xb0    // む
#define CHAR_R_BRACKET      0xb1    // 」
#define CHAR_KANA_XI        0xb2    // ぃ
#define CHAR_KANA_XYU       0xb3    // ゅ
#define CHAR_KANA_WO        0xb4    // を
#define CHAR_KUTEN          0xb5    // 、
#define CHAR_KANA_XU        0xb6    // ぅ
#define CHAR_KANA_XYO       0xb7    // ょ
#define CHAR_HANDAKUTEN     0xb8    // ゜
#define CHAR_1DOT           0xb9    // .
#define CHAR_KANA_XE        0xba    // ぇ
#define CHAR_KANA_XTSU      0xbb    // っ
#define CHAR_DAKUTEN        0xbc    // ゛
#define CHAR_TOUTEN         0xbd    // 。
#define CHAR_KANA_XO        0xbe    // ぉ
#define CHAR_SMALL_HYPHEN   0xbf    // -

#define CHAR_KANA_VU    CHAR_KANA_U,  CHAR_DAKUTEN      // ヴ
#define CHAR_KANA_GA    CHAR_KANA_KA, CHAR_DAKUTEN      // が
#define CHAR_KANA_GI    CHAR_KANA_KI, CHAR_DAKUTEN      // ぎ
#define CHAR_KANA_GU    CHAR_KANA_KU, CHAR_DAKUTEN      // ぐ
#define CHAR_KANA_GE    CHAR_KANA_KE, CHAR_DAKUTEN      // げ
#define CHAR_KANA_GO    CHAR_KANA_KO, CHAR_DAKUTEN      // ご
#define CHAR_KANA_ZA    CHAR_KANA_SA, CHAR_DAKUTEN      // ざ
#define CHAR_KANA_ZI    CHAR_KANA_SHI, CHAR_DAKUTEN     // じ
#define CHAR_KANA_ZU    CHAR_KANA_SU, CHAR_DAKUTEN      // ず
#define CHAR_KANA_ZE    CHAR_KANA_SE, CHAR_DAKUTEN      // ぜ
#define CHAR_KANA_ZO    CHAR_KANA_SO, CHAR_DAKUTEN      // ぞ
#define CHAR_KANA_DA    CHAR_KANA_TA, CHAR_DAKUTEN      // だ
#define CHAR_KANA_DI    CHAR_KANA_CHI, CHAR_DAKUTEN     // ぢ
#define CHAR_KANA_DU    CHAR_KANA_TSU, CHAR_DAKUTEN     // づ
#define CHAR_KANA_DE    CHAR_KANA_TE, CHAR_DAKUTEN      // で
#define CHAR_KANA_DO    CHAR_KANA_TO, CHAR_DAKUTEN      // ど
#define CHAR_KANA_BA    CHAR_KANA_HA, CHAR_DAKUTEN      // ば
#define CHAR_KANA_BI    CHAR_KANA_HI, CHAR_DAKUTEN      // び
#define CHAR_KANA_BU    CHAR_KANA_FU, CHAR_DAKUTEN      // ぶ
#define CHAR_KANA_BE    CHAR_KANA_HE, CHAR_DAKUTEN      // べ
#define CHAR_KANA_BO    CHAR_KANA_HO, CHAR_DAKUTEN      // ぼ
#define CHAR_KANA_PA    CHAR_KANA_HA, CHAR_HANDAKUTEN   // ぱ
#define CHAR_KANA_PI    CHAR_KANA_HI, CHAR_HANDAKUTEN   // ぴ
#define CHAR_KANA_PU    CHAR_KANA_FU, CHAR_HANDAKUTEN   // ぷ
#define CHAR_KANA_PE    CHAR_KANA_HE, CHAR_HANDAKUTEN   // ぺ
#define CHAR_KANA_PO    CHAR_KANA_HO, CHAR_HANDAKUTEN   // ぽ

#define CHAR_CURSOR_DOWN    0xc1    // ↓
#define CHAR_CURSOR_UP      0xc2    // ↑
#define CHAR_CURSOR_RIGHT   0xc3    // →
#define CHAR_CURSOR_LEFT    0xc4    // ←
#define CHAR_UFO            0xc7    // UFO
#define CHAR_MAN_UP         0xca
#define CHAR_MAN_LEFT       0xcb
#define CHAR_MAN_RIGHT      0xcc
#define CHAR_MAN_DOWN       0xcd
#define CHAR_NICOCHAN_1     0xce
#define CHAR_NICOCHAN_0     0xcf

#define CHAR_KNAJI_SUN      0xd0    // 日
#define CHAR_KNAJI_MON      0xd1    // 月
#define CHAR_KNAJI_TUE      0xd2    // 火
#define CHAR_KNAJI_WED      0xd3    // 水
#define CHAR_KNAJI_THU      0xd4    // 木
#define CHAR_KNAJI_FRI      0xd5    // 金
#define CHAR_KNAJI_SAT      0xd6    // 土
#define CHAR_KNAJI_LIVE     0xd7    // 生
#define CHAR_KNAJI_YEAR     0xd8    // 年
#define CHAR_KNAJI_HOUR     0xd9    // 時
#define CHAR_KNAJI_MIN      0xda    // 分
#define CHAR_KNAJI_SEC      0xdb    // 秒
#define CHAR_KNAJI_YEN      0xdc    // 円
#define CHAR_YEN            0xdd    // ￥
#define CHAR_SNAKE          0xdf

// 0xf0 以降は制御コード
#define CHAR_SP             0xf0    // CHAR_MOVE_RIGHT, 1 と同じ
#define CHAR_MOVE_RIGHT     0xf1    // 位置を n 文字右へ移動(2 bytes 目で指定) 改行には影響なし
#define CHAR_MOVE_DOWN      0xf2    // 位置を n 文字下へ移動(2 bytes 目で指定) 改行には影響します
// 0xf3 eserved
#define CHAR_COL4           0xf4    // CHAR_ATB, 0x40 と同じ
#define CHAR_COL5           0xf5    // CHAR_ATB, 0x50 と同じ
#define CHAR_COL6           0xf6    // CHAR_ATB, 0x60 と同じ
#define CHAR_COL7           0xf7    // CHAR_ATB, 0x70 と同じ
#define CHAR_CAPS           0xf8    // 大文字 / 小文字 切替. CHAR_ATB, CHAR_COL7 で上書きされます
#define CHAR_ATB            0xf9    // ATB 変更 (2 byte 目を VATB() マクロで指定. CHAR_CAPS, CHAR_COL7 で設定が上書きされます
#define CHAR_LF             0xfa    // 改行
#define CHAR_LF2            0xfb    // 改行 * 2

#endif // CHAR_H_INCLUDED