/**
 * Exomizer 3 depacker for SDCC inline asm
 *
 * アセンブラ マクロを覚えるのが面倒なので C プリプロセッサを活用してみた版
 *
 * @author Snail Barbarian Macho (NWK)
 * original code: Copyright (c) 2019-2021 uniabis
 *
 */
#ifndef DEEXO3_H_INCLUDED
#define DEEXO3_H_INCLUDED
/**
 * Exomizer 3.1 形式で圧縮したデータを伸長します
 */
void deexo3(const u8* const src, u8* const dest) __naked;


#endif //DEEXO3_H_INCLUDED