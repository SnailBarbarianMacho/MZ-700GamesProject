/**
 * ハードウェア以外の objWork やテーブル等の固定アドレス
 *
 * @author Snail Barbarian Macho (NWK)
 */

#ifndef ADDR_H_INCLUDED
#define ADDR_H_INCLUDED

// ---------------------------------------------------------------- 0x0000 - 0x21ff までの領域のアドレス
#define ADDR_VVRAM              0xfc00  /// 仮想 VRAM のアドレス
#define ADDR_SD3_ATT_TAB        0x1d00  /// #0x0100 sound.c 減衰テーブル

// ---------------------------------------------------------------- 0xd000 - 0xefff までの領域のアドレス
#define ADDR_OBJ                0xd000  /// #0x1000 obj.c   Obj
#define ADDR_DIV256_SIGN_TAB    0xe000  /// #0x0100 math.c  [0, 256) -> [-1, 1] に変換するテーブル
#define ADDR_DIV256_7_TAB       0xe100  /// #0x0100 math.c  [0, 256) -> [0, 7) に変換するテーブル
#define ADDR_DIV256_25_TAB      0xe200  /// #0x0100 math.c  [0, 256) -> [0, 25) に変換するテーブル
#define ADDR_DIV256_40_TAB      0xe300  /// #0x0100 math.c  [0, 256) -> [0, 40) に変換するテーブル
#define ADDR_ATAN2_TAB          0xe400  /// #0x0100 math.c  atan2 テーブル
#define ADDR_SIN_TAB            0xe500  /// #0x0100 math.c  sin テーブル
#define ADDR_COS_TAB            0xe600  /// #0x0100 math.c  cos テーブル
#define ADDR_STARS              0xe700  /// #0x0100 stars.c 背景の星ワーク (3 bytes なら 85個 = 255bytes)
#define ADDR_SUB_LEVEL          0xe800  /// #0x0100 score.c サブレ ベルの表示((TEXT + ATB) * 128 = 256 bytes)
#define ADDR_ITEM_TAB           0xe900  /// #0x0100 item.c  アイテムの表示キャラ テーブル
#define ADDR_SD_SCALE_TAB       0xea00  /// #0x0100 sound.c 音階-音程変換テーブル

//#define ADDR_STACK              0xfc00  /// 実際は 0x80 バイトくらいしか彫ってないので, 0x100 だけ確保しておけばいい. crt0.asm 参照
#endif // ADDR_H_INCLUDED
