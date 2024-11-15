// 簡単なビルド例(310 bytes)
// zcc +mz -o test1 -create-app test1.c
//
// 高速化例
// zcc +mz -mz80 -O3 --opt-code-speed=all -zorg=8192 -o test1 -create-app test1.c
//
// SDCC で高速で容量を減らすビルド例(164 bytes)
// zcc +mz -mz80 -OS3 -zorg=8192 -compiler=sdcc -lndos -pragma-define:CRT_ENABLE_STDIO=0 -pragma-define:CRT_INITIALIZE_BSS=0 -pragma-define:CLIB_EXIT_STACK_SIZE=0 -o test1 -create-app test1.c
//
// 独自の crt0 を使う場合
// zcc +mz -mz80 -OS3 -compiler=sdcc -lndos -crt0mycrt0 -o test1 -create-app test1.c

typedef unsigned char  u8;
typedef unsigned short u16;

#if 0
inline u8 poke_c(volatile u8* const addr, const u8 data)
{
    *addr = data;
    return data;
}

#pragma save
#pragma disable_warning 85
u8 poke_asm(volatile u8* const addr, const u8 data)
{
#if 0 //SCCZ80
#asm
        ld ix,2
        add ix, sp

        ld h, (ix+3)
        ld l, (ix+2)
        ld b, (ix)
        ld (hl), b

        ld l, b
#endasm
// return 不要
#else //SDCC
__asm;
    ld ix,2
    add ix, sp
    ld l, (ix)
    ld h, (ix+1)
    ld b, (ix+2)
    ld (hl), b
__endasm;
    return data; // return 必要
#endif
}
#pragma restore

u8 poke_asm_fastcall(const u8 data)  __z88dk_fastcall __naked
{
#if 0 //SCCZ80
#asm
    ld d, l
    ld hl, 0d1f4h
    ld (hl), d
    ld l, d
    ret
#endasm
#else
__asm;
    ld d, l
    ld hl, 0d1f4h
    ld (hl), d
    ld l, d
    ret
__endasm;
#endif
}
#endif

void main(void)
{
    // 画面書き込みと音のテスト
    u8 c = 0;
    static u8* const VRAM_START = (u8* const)(0xd000);
    static u8* const VRAM_END   = (u8* const)(0xd000+1000);
    *(u8* const)(0xe002) = 0x36;    // 8253 設定
    *(u8* const)(0xe008) = 0x01;    // サウンドマスク
    static u8* const SOUND      = (u8* const)(0xe004);

    do {
        // 音を出す
        *SOUND = c; // high
        *SOUND = c; // low

        // VRAM 書き込み
        for (u8* addr = (u8*)VRAM_START; addr < VRAM_END; addr ++) {
            *addr = c;
        }
        c++;

//        c = poke_c((volatile u8* const)(0xd000+500), c);
//        c = poke_asm((volatile u8* const)(0xd000+500), c);
//        c = poke_asm_fastcall(c);
    } while (c != 0);

    *(u8* const)(0xe008) = 0x00;    // サウンドマスク
}
