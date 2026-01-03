/**
 * ハードウェア以外の objWork やテーブル等の固定アドレス
 *
 * @author Snail Barbarian Macho (NWK)
 */

#ifndef ADDR_H_INCLUDED
#define ADDR_H_INCLUDED

#define SZ_SD6_DRUM     0x0100
#define SZ_SD6_TAB      0x0100
#define SZ_SD6_REP      0x000c

#define ADDR_SD6_DRUM   0x0100                          /// ドラム データ (SD6用テーブルから, 上位3bitを抽出し, bit1～3にシフト). 0x0100 単位
#define ADDR_SD6_TAB    (ADDR_SD6_DRUM + SZ_SD6_DRUM)   /// 波長->パルス幅変換テーブル (SD6用テーブルから, 下位5bitを抽出したもの)
#define ADDR_SD6_REP    (ADDR_SD6_TAB + SZ_SD6_TAB)     /// リピート用スタック (3 x 4 = 12 bytes)

#endif // ADDR_H_INCLUDED
