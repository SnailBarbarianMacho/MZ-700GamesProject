/**
 * ハードウェア以外の objWork やテーブル等の固定アドレス
 *
 * @author Snail Barbarian Macho (NWK)
 */

#ifndef ADDR_H_INCLUDED
#define ADDR_H_INCLUDED

#define SZ_SD4_VTAB     0x0040
#define SZ_SD4_TAB      0x0010
#define SZ_SD4_REP      0x000c
#define SZ_SD4_NOT_USED 0x00a4
#define SZ_SD4_DRUM     0x0300

#define ADDR_SD4_VTAB   0x0100                              /// 音量変換テーブル. 0x100 単位
#define ADDR_SD4_TAB        (ADDR_SD4_VTAB + SZ_SD4_VTAB)   /// その他テーブル
#define ADDR_SD4_REP        (ADDR_SD4_TAB  + SZ_SD4_TAB)    /// リピート用スタック (3 x 4 = 12 bytes)
#define ADDR_SD4_NOT_USED   (ADDR_SD4_REP  + SZ_SD4_REP)    /// 未使用
#define ADDR_SD4_DRUM       (ADDR_SD4_NOT_USED + SZ_SD4_NOT_USED)  /// ドラムデータ x 3
#define SZ_SD4_REP_STACK    (3 * 4)

#endif // ADDR_H_INCLUDED
