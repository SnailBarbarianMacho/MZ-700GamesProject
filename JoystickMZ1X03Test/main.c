/**
 * 三重和音サウンド テスト
 *
 * @author Snail Barbarian Macho (NWK)
 */
#include "../src-common/common.h"
#include "../src-common/hard.h"


#define INPUT_MASK_A 0x01   // A ボタン
#define INPUT_MASK_B 0x02   // B ボタン
#define INPUT_MASK_L 0x04   // 左
#define INPUT_MASK_R 0x08   // 右
#define INPUT_MASK_D 0x10   // 下
#define INPUT_MASK_U 0x20   // 上
#define INPUT_MASK_NC   0x80  // 未検出フラグ
#define INPUT_SHIFT_NC  0x7   // 未検出フラグ


/** シャープ純正ジョイスティックを入力します. デバッグ用
 * - VBLANK を待ち, 更に 63 ライン分 (約4msec) かかります
 * @return 00UD_RLBA. 未検出なら INPUT_MASK_NC が立ちます
 */
static u8 inputMZ1X03() __z88dk_fastcall __naked
{
__asm
    // -------- /VBLK の立下がりを待つ
    xor     A
    ld      HL, #MMIO_8255_PORTC
INPUT_MZ1X03_VBLK_SYNC00:
    or      A, (HL)                 // /VBLK = H になるまで待つ
    jp      p,  INPUT_MZ1X03_VBLK_SYNC00
INPUT_MZ1X03_VBLK_SYNC01:
    and     A, (HL)                 // /VBLK = L になるまで待つ
    jp      m,  INPUT_MZ1X03_VBLK_SYNC01

    // -------- /VBLK 直後 150 くらいで '0' が読めなかったら未検出扱い
    ld      L,  #(MMIO_ETC & 0xff)  // 7

    ld      B,  #10                 // 7
INPUT_MZ1X03_WAIT:
    djnz    B,  INPUT_MZ1X03_WAIT   // 13 * 10 - 5
    ld      A, (HL)                 // 7
    and     A,  #(MMIO_ETC_JA1_MASK | MMIO_ETC_JA2_MASK)// 7
    jp      nz, INPUT_MZ1X03_NOT_DETECT                 // 10 接続されてないなら '1' が読める筈
    // 小計 7+7+(13*10-5)+7+7+10 = 163

    // -------- /VBLK 直後 300 で '1' ならば左 or 上
    ld      B,  #11                 // 7
INPUT_MZ1X03_WAIT1:
    djnz    B,  INPUT_MZ1X03_WAIT1  // 13 * 11 - 5
    ld      A, (HL)                 // 7    ****_*YX*
    // 小計 7+(13*11-5)+7 = 157

    // -------- /VBLK ここから更に 7000 くらいで '0' ならば右 or 下
    ld      E,  #7                  // 7
INPUT_MZ1X03_WAIT2:
    ld      B,  #76                 // 7
INPUT_MZ1X03_WAIT3:
    djnz    B,  INPUT_MZ1X03_WAIT3  // 13 * 76 - 5
    dec     E                       // 4
    jp      NZ, INPUT_MZ1X03_WAIT2  // 10
    ld      D, (HL)                 // 7    ****_*yx*
    // 小計 7+((7+13*76-5)+4+10)*4 = 7035

    // -------- 上下左右判定
INPUT_MZ1X03:
    and     A,  #(MMIO_ETC_JA1_MASK | MMIO_ETC_JA2_MASK)
    rrca                            //      0000_00YX
    ld      E, A
    ld      A, D                    //      ****_*yx*
    and     A,  #(MMIO_ETC_JA1_MASK | MMIO_ETC_JA2_MASK)
    add     A, A                    //      0000_yx00
    or      A, E                    //      0000_yxYX
section rodata_compiler
INPUT_MZ1X03_TAB:
    db      INPUT_MASK_R | INPUT_MASK_D, INPUT_MASK_NC,               INPUT_MASK_NC              , INPUT_MASK_NC               // 0000, 0001, 0010, 0011
    db                     INPUT_MASK_D, INPUT_MASK_L | INPUT_MASK_D, INPUT_MASK_NC              , INPUT_MASK_NC               // 0100, 0101, 0110, 0111
    db      INPUT_MASK_R,                INPUT_MASK_NC,               INPUT_MASK_R | INPUT_MASK_U, INPUT_MASK_NC               // 1000, 1001, 1010, 1011
    db      0,                           INPUT_MASK_L,                INPUT_MASK_U,                INPUT_MASK_L | INPUT_MASK_U // 1100, 1101, 1110, 1111
section code_compiler
    ld      HL, INPUT_MZ1X03_TAB
    ld      D,  #0x00
    ld      E,  A
    add     HL, DE
    ld      E,  (HL)                //      00UD_RL00
    bit     #INPUT_SHIFT_NC, E
    jp      nz, INPUT_MZ1X03_NOT_DETECT // 未検出 (違うプロトコルのジョイスティックが繋がってる可能性)

    // -------- /VBLK 外になるまで待つ
    ld      HL, #MMIO_8255_PORTC
INPUT_MZ1X03_VBLK_SYNC2:
    bit     #MMIO_8255_PORTC_VBLK_SHIFT, (HL)// /VBLK = H になるまで待つ
    jp      Z,  INPUT_MZ1X03_VBLK_SYNC2

    // -------- 少し待ってからボタンを読む
    ld      B,  200
INPUT_MZ1X03_WAIT4:
    djnz    B,  INPUT_MZ1X03_WAIT4

    ld      L,  #(MMIO_ETC & 0xff)
    ld      A,  (HL)                //      ****_**BA*
    cpl     A                       // ビット反転
    and     A,  #(MMIO_ETC_JA1_MASK | MMIO_ETC_JA2_MASK)
    rrca                            //      0000_00BA
    or      A,  E                   //      00UD_RLBA
    ld      L,  A
    ret

    // -------- 未検出
INPUT_MZ1X03_NOT_DETECT:
    ld      L, #INPUT_MASK_NC
    ret

__endasm;
}


void main() __naked
{
    // 1 行だけ消去
    {
        volatile u8* vram = (volatile u8*)(VRAM_TEXT);
        for (int i = 40; i != 0; i--) {
            *vram++ = 0x00;
        }
    }

    // メインループ
    while (1) {
        volatile u8* vram = (volatile u8*)(VRAM_TEXT);
        u8           joy  = inputMZ1X03();

        if (joy == INPUT_MASK_NC) {
            for (u8 i = 6; i != 0; i--) {
                *vram++ = 0x6d; // 'X'
            }
        } else {
            vram[0] = (joy & INPUT_MASK_U) ? 0x15 : 0x2a; //'-'
            vram[1] = (joy & INPUT_MASK_D) ? 0x04 : 0x2a;
            vram[2] = (joy & INPUT_MASK_R) ? 0x12 : 0x2a;
            vram[3] = (joy & INPUT_MASK_L) ? 0x0c : 0x2a;
            vram[4] = (joy & INPUT_MASK_B) ? 0x02 : 0x2a;
            vram[5] = (joy & INPUT_MASK_A) ? 0x01 : 0x2a;
        }
    }
}
