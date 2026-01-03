/**
 * 8253 をいろいろいじるテスト
 *
 * @author Snail Barbarian Macho (NWK)
 */
#include "../src-common/aal80.h"
#include "../src-common/common.h"
#include "../src-common/hard.h"
#include "../src-common/display-code.h"

// VS Code をだますためのダミー宣言
extern int _vvramVSyncAndWait_;

// ----------------------------------------------------------------
#define VVRAM_TEXT  0x8000      // 仮想 VRAM(TEXT)
#define VVRAM_ATB   0x8400      // 仮想 VRAM(ATB)
#define VVRAM_SIZE  1000        // 仮想 VRAM の TEXT/ATB それぞれの大きさ
#define V_CT        262         // 1画面の垂直ライン数

// ----------------------------------------------------------------

/** A のうち, 1 ニブルを HL で示すアドレスに表示 */
#pragma save
#pragma disable_warning 85          // 引数未使用警告抑止
static void DISP_NIBBLE(int A, int HL) __aal_macro __naked
{
    AAL_DEF_VARS;

    A &= 0x0f; cp(A, 10); if (ge_jr) { A += DC_A - 10 - DC_0; } A += DC_0;
    mem[HL] = A;
    HL++;

    AAL_ENDM
}
#pragma restore


void main(void) __aal __naked
{
    AAL_DEF_VARS;

    // -------- 仮想 VRAM 初期化
    HL = VVRAM_TEXT; DE = VVRAM_TEXT + 1; BC = VVRAM_SIZE; mem[HL] = 0x00;          ldir();
    HL = VVRAM_ATB;  DE = VVRAM_ATB  + 1; BC = VVRAM_SIZE; mem[HL] = VATB(7, 0, 0); ldir();

    // -------- 8255 の初期化
    // カウンタ1は, 16bit, 周期262で, 垂直ライン番号が読めます
    // カウンタ2は, 8bit, 周期256で, ゲーム処理に1/60秒以上かかる場合の補助に使います
    HL = MMIO_8253_CTRL;
    mem[HL] = MMIO_8253_CTRL_CT1(MMIO_8253_CTRL_RL_LH_MASK, MMIO_8253_CTRL_MODE2_MASK);
    mem[HL] = MMIO_8253_CTRL_CT2(MMIO_8253_CTRL_RL_L_MASK, MMIO_8253_CTRL_MODE2_MASK);
    L--;                                                // L = CT2
    mem[HL] = 0x00;                                     // カウンタ2L
    mem[HL] = 0x00;                                     // カウンタ2H
    L--;
    DE = V_CT;

    push(HL); {
        BC = 2712 + 100 + 15;
        call(_vvramVSyncAndWait_);
    } pop(HL);

    mem[HL] = E;                                        // カウンタ1L
    mem[HL] = D;                                        // カウンタ1H

    // -------- 画面表示
    while (true) {
        BC = 250;                                       // 2桁なら500, 4桁なら250
        HL = VVRAM_TEXT; DE = VVRAM_ATB;

        // -------- VBLK 待ち
        exx(); {
            HL = MMIO_8255_PORTC;
            A ^= A;
            do { A |= mem[HL]; } while (p);             // /VBLK = H になるまで待つ
            do { A &= mem[HL]; } while (m);             // /VBLK = L になるまで待つ
        } exx();

        // -------- 仮想VRAMに描画
        do {
            // カウンタ値を表示
            A = mem[MMIO_8253_CT1];                     // カウンタ 1(L)
            ex(AF, AF); {
                A = mem[MMIO_8253_CT1];                 // カウンタ 1(H)
#if 0           // カウンタ=0の検出テスト
                A |= A; if (z) {
                    ex(AF,AF); cp(A, 0x12); if (lt) {
aaaa:                   nop();
                    }
                    ex(AF,AF);
                }
#endif
                // 上 4bit 値表示
                push(AF); {
                    rrca_n(A, 4);
                    DISP_NIBBLE(A, HL);
                } pop(AF);
                // 下 4bit 値表示
                DISP_NIBBLE(A, HL);
            } ex(AF, AF);
            // 上 4bit 値表示
            push(AF); {
                rrca_n(A, 4);
                DISP_NIBBLE(A, HL);
            } pop(AF);
            // 下 4bit 値表示
            DISP_NIBBLE(A, HL);

            // VBLANK が '0' ならば, 背景アミカケ
            A = mem[MMIO_8255_PORTC]; A |= A;
            A = VATB(CYAN, 0, 0); if (p) { A = VATB(CYAN, RED, 0); }

            // カウンタが奇数が偶数かで文字色変更委
            bit(0, C); if (z_jr) { A |= VATB(WHITE, 0, 0); }
            mem[DE] = A; DE++; mem[DE] = A; DE++;       // 2, 4桁
            mem[DE] = A; DE++; mem[DE] = A; DE++;       // 4桁

            BC--; A = B; A |= C;
        } while (nz);

        // -------- 転送
        // Emuz-700 ではVRAMへのLDIやLDIR転送ではコケるので, やむを得ず手動コピー
        HL = VVRAM_TEXT; DE = VRAM_TEXT; BC = VVRAM_SIZE;// ldir();
        do { A = mem[HL]; mem[DE] = A; HL++; DE++; BC--; A = B; A |= C; } while(nz);
        HL = VVRAM_ATB;  DE = VRAM_ATB;  BC = VVRAM_SIZE;// ldir();
        do { A = mem[HL]; mem[DE] = A; HL++; DE++; BC--; A = B; A |= C; } while(nz);
    }

    AAL_NO_RETURN;
}



/**
 * /VBLK の立下がりを待って, その後規定時間だけ待ちます
 * @param BANK MMIO に切り替わってること
 * @param BC: カウンタ 17～27+(6+4+4+12)*BC-5+10 だけ待ちます
 * @broken A, BC, HL
 */
static void vvramVSyncAndWait_(void) __aal
{
    AAL_DEF_VARS;

    HL = MMIO_8255_PORTC;
    A ^= A;
    do { A |= mem[HL]; } while (p);                     //          /VBLK = H になるまで待つ
    do { A &= mem[HL]; } while (m);                     //  7+10    /VBLK = L になるまで待つ
    do { BC--; A = B; A |= C; } while (nz_jr);          //  6+4+4+12/7
}
