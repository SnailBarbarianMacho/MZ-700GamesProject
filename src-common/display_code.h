/**
 * ディスプレイ コードの文字
 * - 名前空間 DC_
 *
 * @author Snail Barbarian Macho (NWK)
 */

#ifndef DISPLAY_CODE_H_INCLUDED
#define DISPLAY_CODE_H_INCLUDED

// ---------------------------------------------------------------- 文字のディスプレイ コード
#define DC_A            0x01
#define DC_B            0x02
#define DC_C            0x03
#define DC_D            0x04
#define DC_E            0x05
#define DC_F            0x06
#define DC_G            0x07
#define DC_H            0x08
#define DC_I            0x09
#define DC_J            0x0a
#define DC_K            0x0b
#define DC_L            0x0c
#define DC_M            0x0d
#define DC_N            0x0e
#define DC_O            0x0f
#define DC_P            0x10
#define DC_Q            0x11
#define DC_R            0x12
#define DC_S            0x13
#define DC_T            0x14
#define DC_U            0x15
#define DC_V            0x16
#define DC_W            0x17
#define DC_X            0x18
#define DC_Y            0x19
#define DC_Z            0x1a

#define DC_0            0x20
#define DC_1            0x21
#define DC_2            0x22
#define DC_3            0x23
#define DC_4            0x24
#define DC_5            0x25
#define DC_6            0x26
#define DC_7            0x27
#define DC_8            0x28
#define DC_9            0x29

#define DC_MINUS        0x2a    // -
#define DC_EQUAL        0x2b    // =
#define DC_SEMICOLON    0x2c    // ;
#define DC_SLASH        0x2d    // /
#define DC_PERIOD       0x2e    // . 唯一の ASCII / Display Code 一致キャラ
#define DC_COMMA        0x2f    // ,

#define DC_SQUARE       0x43    // ■

#define DC_R_ARROW      0x40    // ←
#define DC_QUESTION     0x49    // ?
#define DC_COLON        0x4f    // :

#define DC_SPADE        0x41    // ♠
#define DC_DIAMOND      0x44    // ♦
#define DC_CLUB         0x46    // ♣
#define DC_HEART        0x53    // ♥

#define DC_U_ARROW      0x50    // ↑
#define DC_LT           0x51    // <
#define DC_L_SQ_BLACKET 0x52    // [
#define DC_R_SQ_BLACKET 0x54    // ]
#define DC_AT           0x55    // @
#define DC_GT           0x57    // >
#define DC_BACK_SLASH   0x59    // /

#define DC_PI           0x60    // π
#define DC_EXCLAM       0x61    // !
#define DC_QUOT         0x62    // "
#define DC_NUMBER       0x63    // #
#define DC_DOLLER       0x64    // $
#define DC_PERCENT      0x65    // %
#define DC_AMP          0x66    // &
#define DC_APOS         0x67    // '
#define DC_L_BLACKET    0x68    // (
#define DC_R_BLACKET    0x69    // )
#define DC_PLUS         0x6a    // +
#define DC_STAR         0x6b    // *

#define DC_KANA_CHI     0x81    // ち
#define DC_KANA_KO      0x82    // こ
#define DC_KANA_SO      0x83    // そ
#define DC_KANA_SHI     0x84    // し
#define DC_KANA_I       0x85    // い
#define DC_KANA_HA      0x86    // は
#define DC_KANA_KI      0x87    // き
#define DC_KANA_KU      0x88    // く
#define DC_KANA_NI      0x89    // な
#define DC_KANA_MA      0x8a    // ま
#define DC_KANA_NO      0x8b    // の
#define DC_KANA_RI      0x8c    // り
#define DC_KANA_MO      0x8d    // も
#define DC_KANA_MI      0x8e    // み
#define DC_KANA_RA      0x8f    // ら

#define DC_KANA_SE      0x90    // せ
#define DC_KANA_TA      0x91    // た
#define DC_KANA_SU      0x92    // す
#define DC_KANA_TO      0x93    // と
#define DC_KANA_KA      0x94    // か
#define DC_KANA_NA      0x95    // な
#define DC_KANA_HI      0x96    // ひ
#define DC_KANA_TE      0x97    // て
#define DC_KANA_SA      0x98    // さ
#define DC_KANA_N       0x99    // ん
#define DC_KANA_TSU     0x9a    // つ
#define DC_KANA_RO      0x9b    // ろ
#define DC_KANA_KE      0x9c    // け
#define DC_L_BRACKET    0x9d    // 「
#define DC_KANA_XA      0x9e    // ぁ
#define DC_KANA_XYA     0x9f    // ゃ

#define DC_KANA_WA      0xa0    // わ
#define DC_KANA_NU      0xa1    // ぬ
#define DC_KANA_FU      0xa2    // ふ
#define DC_KANA_A       0xa3    // あ
#define DC_KANA_U       0xa4    // う
#define DC_KANA_E       0xa5    // え
#define DC_KANA_O       0xa6    // お
#define DC_KANA_YA      0xa7    // や
#define DC_KANA_YU      0xa8    // ゆ
#define DC_KANA_YO      0xa9    // よ
#define DC_KANA_HO      0xaa    // ほ
#define DC_KANA_HE      0xab    // へ
#define DC_KANA_RE      0xac    // れ
#define DC_KANA_ME      0xad    // め
#define DC_KANA_RU      0xae    // る
#define DC_KANA_NE      0xaf    // ね

#define DC_KANA_MU      0xb0    // む
#define DC_R_BRACKET    0xb1    // 」
#define DC_KANA_XI      0xb2    // ぃ
#define DC_KANA_XYU     0xb3    // ゅ
#define DC_KANA_WO      0xb4    // を
#define DC_KUTEN        0xb5    // 、
#define DC_KANA_XU      0xb6    // ぅ
#define DC_KANA_XYO     0xb7    // ょ
#define DC_HANDAKUTEN   0xb8    // ゜
#define DC_1DOT         0xb9    // .
#define DC_KANA_XE      0xba    // ぇ
#define DC_KANA_XTSU    0xbb    // っ
#define DC_DAKUTEN      0xbc    // ゛
#define DC_TOUTEN       0xbd    // 。
#define DC_KANA_XO      0xbe    // ぉ
#define DC_KANA_HYPHEN  0xbf    // ー

#define DC_KANA_VU      DC_KANA_U,  DC_DAKUTEN      // ヴ
#define DC_KANA_GA      DC_KANA_KA, DC_DAKUTEN      // が
#define DC_KANA_GI      DC_KANA_KI, DC_DAKUTEN      // ぎ
#define DC_KANA_GU      DC_KANA_KU, DC_DAKUTEN      // ぐ
#define DC_KANA_GE      DC_KANA_KE, DC_DAKUTEN      // げ
#define DC_KANA_GO      DC_KANA_KO, DC_DAKUTEN      // ご
#define DC_KANA_ZA      DC_KANA_SA, DC_DAKUTEN      // ざ
#define DC_KANA_ZI      DC_KANA_SHI, DC_DAKUTEN     // じ
#define DC_KANA_ZU      DC_KANA_SU, DC_DAKUTEN      // ず
#define DC_KANA_ZE      DC_KANA_SE, DC_DAKUTEN      // ぜ
#define DC_KANA_ZO      DC_KANA_SO, DC_DAKUTEN      // ぞ
#define DC_KANA_DA      DC_KANA_TA, DC_DAKUTEN      // だ
#define DC_KANA_DI      DC_KANA_CHI, DC_DAKUTEN     // ぢ
#define DC_KANA_DU      DC_KANA_TSU, DC_DAKUTEN     // づ
#define DC_KANA_DE      DC_KANA_TE, DC_DAKUTEN      // で
#define DC_KANA_DO      DC_KANA_TO, DC_DAKUTEN      // ど
#define DC_KANA_BA      DC_KANA_HA, DC_DAKUTEN      // ば
#define DC_KANA_BI      DC_KANA_HI, DC_DAKUTEN      // び
#define DC_KANA_BU      DC_KANA_FU, DC_DAKUTEN      // ぶ
#define DC_KANA_BE      DC_KANA_HE, DC_DAKUTEN      // べ
#define DC_KANA_BO      DC_KANA_HO, DC_DAKUTEN      // ぼ
#define DC_KANA_PA      DC_KANA_HA, DC_HANDAKUTEN   // ぱ
#define DC_KANA_PI      DC_KANA_HI, DC_HANDAKUTEN   // ぴ
#define DC_KANA_PU      DC_KANA_FU, DC_HANDAKUTEN   // ぷ
#define DC_KANA_PE      DC_KANA_HE, DC_HANDAKUTEN   // ぺ
#define DC_KANA_PO      DC_KANA_HO, DC_HANDAKUTEN   // ぽ

#define DC_CURSOR_DOWN  0xc1    // ↓
#define DC_CURSOR_UP    0xc2    // ↑
#define DC_CURSOR_RIGHT 0xc3    // →
#define DC_CURSOR_LEFT  0xc4    // ←
#define DC_UFO          0xc7    // UFO
#define DC_MAN_UP       0xca
#define DC_MAN_LEFT     0xcb
#define DC_MAN_RIGHT    0xcc
#define DC_MAN_DOWN     0xcd
#define DC_NICOCHAN_1   0xce
#define DC_NICOCHAN_0   0xcf

#define DC_KNAJI_SUN    0xd0    // 日
#define DC_KNAJI_MON    0xd1    // 月
#define DC_KNAJI_TUE    0xd2    // 火
#define DC_KNAJI_WED    0xd3    // 水
#define DC_KNAJI_THU    0xd4    // 木
#define DC_KNAJI_FRI    0xd5    // 金
#define DC_KNAJI_SAT    0xd6    // 土
#define DC_KNAJI_LIVE   0xd7    // 生
#define DC_KNAJI_YEAR   0xd8    // 年
#define DC_KNAJI_HOUR   0xd9    // 時
#define DC_KNAJI_MIN    0xda    // 分
#define DC_KNAJI_SEC    0xdb    // 秒
#define DC_KNAJI_YEN    0xdc    // 円
#define DC_YEN          0xdd    // ￥
#define DC_SNAKE        0xdf

// セミ グラフィック  +------ 左上
//                    |+----- 右上
//                    ||+---- 左下
//                    |||+--- 右下
#define DC_SEMI_GRAPH_0000   0xf0
#define DC_SEMI_GRAPH_1000   0xf1    // 左上
#define DC_SEMI_GRAPH_0100   0xf2    // 右上
#define DC_SEMI_GRAPH_1100   0xf3
#define DC_SEMI_GRAPH_0010   0xf4    // 左下
#define DC_SEMI_GRAPH_1010   0xf5
#define DC_SEMI_GRAPH_0110   0xf6
#define DC_SEMI_GRAPH_1110   0xf7
#define DC_SEMI_GRAPH_0001   0xf8    // 右下
#define DC_SEMI_GRAPH_1001   0xf9
#define DC_SEMI_GRAPH_0101   0xfa
#define DC_SEMI_GRAPH_1101   0xfb
#define DC_SEMI_GRAPH_0011   0xfc
#define DC_SEMI_GRAPH_1011   0xfd
#define DC_SEMI_GRAPH_0111   0xfe
#define DC_SEMI_GRAPH_1111   0xff

// 0xf0 以降は print での制御コード
#define DC_CONTROL      0xf0    // ここから後は制御コード
#define DC_SP           0xf0    // DC_MOVE_RIGHT, 1 と同じ
#define DC_MOVE_RIGHT   0xf1    // 位置を n 文字右へ移動(2 bytes 目で指定) 改行には影響なし
#define DC_MOVE_DOWN    0xf2    // 位置を n 文字下へ移動(2 bytes 目で指定) 改行には影響します
// 0xf3 eserved
#define DC_COL4         0xf4    // DC_ATB, 0x40 と同じ
#define DC_COL5         0xf5    // DC_ATB, 0x50 と同じ
#define DC_COL6         0xf6    // DC_ATB, 0x60 と同じ
#define DC_COL7         0xf7    // DC_ATB, 0x70 と同じ
#define DC_CAPS         0xf8    // 大文字 / 小文字 切替.                      DC_ATB,  DC_COLn で設定が上書きされます
#define DC_ATB          0xf9    // ATB 変更 (2 byte 目を VATB() マクロで指定. DC_CAPS, DC_COLn で設定が上書きされます
#define DC_LF           0xfa    // 改行
#define DC_LF2          0xfb    // 改行 * 2

#endif // DISPLAY_CODE_H_INCLUDED