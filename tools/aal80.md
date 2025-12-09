# Z80 代数アセンブリ言語フィルタ aal80
- [Z80 代数アセンブリ言語フィルタ aal80](#z80-代数アセンブリ言語フィルタ-aal80)
  - [特徴](#特徴)
  - [前提となる知識](#前提となる知識)
- [ソースファイル \& コマンドライン](#ソースファイル--コマンドライン)
    - [ソース ファイル](#ソース-ファイル)
    - [コマンド ライン](#コマンド-ライン)
- [ソースの書き方](#ソースの書き方)
    - [C の関数](#c-の関数)
    - [マクロ定義](#マクロ定義)
- [プログラミング モデル](#プログラミング-モデル)
    - [レジスタ](#レジスタ)
    - [メモリ, ポート](#メモリ-ポート)
- [Z80 基本命令](#z80-基本命令)
    - [ロード命令, 入出力命令](#ロード命令-入出力命令)
    - [算術演算](#算術演算)
    - [比較, サーチ](#比較-サーチ)
    - [論理演算](#論理演算)
    - [ビット操作](#ビット操作)
    - [シフト・ローテート](#シフトローテート)
    - [ジャンプ](#ジャンプ)
    - [CPU 制御, キャリーフラグ制御](#cpu-制御-キャリーフラグ制御)
- [Z80 構造化命令](#z80-構造化命令)
    - [if (cc) \<文1\>\[ else \<文2\>\]](#if-cc-文1-else-文2)
    - [do \<文\> while (cc);](#do-文-while-cc)
    - [while (cc) \<文\>](#while-cc-文)
- [疑似命令](#疑似命令)

## 特徴
- ソースの可読性が高い代数アセンブリ言語 (Algebraic Assembly Language 以下 **AAL**) です
- C と関数単位で混在できます
- VS Code の Microsoft C 文法チェッカをパスします
- 1行に複数命令を書けます
- if や while 構文があるので, ラベルが少なくなります
- マクロも書けます

## 前提となる知識
- Z80 のニーモニックをだいたい知ってる
- Z80 アセンブラの疑似命令をだいたい知ってる
- SDCC の C 呼び出し規約を知ってると便利です



# ソースファイル & コマンドライン

### ソース ファイル
- AAL のソースは, C ソースのインライン アセンブリ言語として扱います
- 拡張子は **.aal.c** を推奨します
- ヘッダには AAL ソースは書きません. 従来のインライン アセンブラを使用してください

### コマンド ライン
- aal80 でコンバートして, インラインアセンブラ込みの C ソースとして出力します
```
php aal80.php foo.aal.c foo.c
```
- SDCC の旧インライン アセンブラ形式 (__asm ～ __endasm;) で出力します<br>
  そのうち一般的な書式 (asm("～"))にするかも

# ソースの書き方
- 関数単位で C と混在できます
- ヘッダにはコードは書きません
- 最初に aal80.h を include します
```
#include "aal80.h"
```
- そのあとは2通りの書き方があります(混在できます)

### C の関数
- 関数単位で書きます. 関数内で C とインライン アセンブラを混在するような書き方はできません
- 関数定義の前後に, 必要に応じて「引数未使用警告」「戻値未設定警告」を抑止する pragma を入れておきます
- 関数定義に「__aal」修飾子を追加します
- 関数内最初に「AAL_DEF_VARS」を記述します
```
pragma save
#pragma disable_warning 85          // 引数未使用警告抑止
#pragma disable_warning 59          // 戻値未設定警告抑止
char add1(const char x) __aal __z88dk_fastcall
{
  AAL_DEF_VARS;                     // L = 引数 (SDCC 呼び出し規約 version 0 の __z88dk_fastcall)
  L++;                              // L = 戻値 (SDCC 呼び出し規約 version 0)
  // ret 命令は SDCC が自動で追加します
}
#pragma restore
```

- naked 関数の場合は最後に必ず「AAL_NO_RETURN」か「AAL_FALL_THROUGH」を記述しないとエラーになります (naked 関数であることを強調するため)
```c
void foo(void) __aal __naked
{
  AAL_DEF_VARS;   // 先頭に必ず AAL_DEF_VARS を付ける
  jp(bar);
  AAL_NO_RETURN;  // naked 関数なら末尾に必ずこれか AAL_FALL_THROUGH を付ける
}
```

### マクロ定義
- 関数のように書きます
- 関数名は必ず UPPER_SNAKE_CASE でなければなりません (でないとエラー)
- 関数定義に「__aal_macro」修飾子を追加します
- 関数引数がある場合は, すべて int 型にしてください (パーサの手抜きのため)
- 引数の名前に 'reg_' プレフィックスつけると, レジスタとみなされます
- 最初に「AAL_DEF_VARS」を記述します
- 最後に必ず「AAL_ENDM」を記述しないとエラーになります (マクロの末端であることを強調するため)
```c
char FOO(int reg_x, int val) __aal_macro
{
  AAL_DEF_VARS; // 先頭に必ず AAL_DEF_VARS を付ける
  reg_x = val;  // reg_ プレフィックスがあるので, reg_x はレジスタでエラーにならない
  AAL_ENDM;     // 末尾に必ず AAL_ENDM を付ける
}
```

- マクロの呼び出しは C 関数のように書きます. もちろんマクロからも呼び出せます:
```
  FOO(A, 123);
```

# プログラミング モデル

### レジスタ
- すべて大文字です. 小文字は許されません
- アンダースコアが付いてるレジスタは, 付いてないのと同じです. <br>
  裏レジスタであることをプログラマに示すなどの使い方ができます
```c
    A, B, C, D, E, H, L, IXH, IXL, IYH, IYL, I, R,
    AF, BC, DE, HL, PC, SP, IX, IY,
    A_, B_, C_, D_, E_, H_, L_,
    AF_, BC_, DE_, HL_
```

### メモリ, ポート
- メモリやポートは, C の配列として扱います
```c
    mem[～]
    port[～]
```

# Z80 基本命令

### ロード命令, 入出力命令
- 大半は, **代入演算子** (=) を使います
```c
    A = 1;
    DE = HL;                    // D = H, E = L に展開されます
    mem[0x1234] = A;
    HL = (FOO + BAR + BAZ);     // メモリ アクセスではなく, 即値です
    HL = mem[FOO + BAR + BAZ];  // メモリ アクセスは mem[～]
    B = mem[HL];
    port[C] = A;                // I/O ポートは port[～]
```
- **ブロック転送** は, 関数です
```c
    ldir(); lddr();
    otir(); otdr();
```
- **交換** 手抜きなのでちょっと制限があります
```c
    // OK
    ex(AF, AF); ex(AF, AF_); ex(AF_, AF_); ex(AF_, AF);
    ex(HL, DE); ex(mem[SP], HL);
    exx();

    // BAD エラー
    ex(DE, HL); ex(HL, mem[SP]);
```
- **スタック** は, レジスタをまとめて PUSH/POP できます
```c
    push(HL); push(DE);
    pop(DE, HL);
```

### 算術演算
- ADD, ADC, SUB, SBC, INC, DEC 命令は C の様に書けます
- 単項演算子 - には対応してません. neg を使ってください
```c
    add(A, B); A += B;          add(HL, DE); HL += DE;
    adc(A, B); A += B + c;      adc(HL, DE); HL += DE + c;
    sub(A, B); A -= B;
    sbc(A, B); A -= B + c;      sbc(HL, DE); HL -= DE + c;
    inc(A); A++; ++A;           inc(HL); HL++;
    dec(A); A--; --A;           dec(HL); HL--;
    neg(A); daa(A);
```

### 比較, サーチ
```c
    cp(A, B);
    cpi(); cpir(); cpd(); cpdr();
```

### 論理演算
```c
    and(A, B); A &= B;
    or(A, B);  A |= B;
    xor(A, B); A ^= B;
    cpl(A); not(A);   // この 2 つは同じ命令です
```

### ビット操作
- 未定義命令の bit3, set3, res3 命令も対応してます
```c
    bit(1, A); set(1, A); res(1, A);
    bit_3(1, mem[IX + 2], B); set_3(1, mem[IX + 2], B); res_3(1, mem[IX + 2], B);
```

### シフト・ローテート
- 論理(符号なし)シフトのみ C の用に書けます
- 複数回シフト・ローテート命令もあります (命令の終わりに "_n" が付きます)
```c
  sla(A); sla_n(A, 2); A <<= 2;
  sll(A); sll_n(A, 2);
  sra(A); sra_n(A, 2);
  srl(A); srl_n(A, 2); A >>= 2;
  rlc(A); rlc_n(A, 2); rlca(A); rlca_n(A, 2);
  rrc(A); rrc_n(A, 2); rrca(A); rrca(A, 2);
  rl(A);  rl_n(A, 2);  rla(A);  rla_n(A, 2);
  rr(A);  rr_n(A, 2);  rra(A);  rra(A, 2);
  rra();
```

### ジャンプ
- **絶対ジャンプ:**
  jp_*cc* の *cc* (コンディション コード) は,
  **z, eq, nz, ne, c, lt, nc, ge, p, m, v, nv, pe, po** が使えます<br>
  eq は z, ne は nz, lt は c, ge は nc と同じ意味です
```c
  goto addr; jp(addr);    // この 2 つは同じ命令
  jp(HL);
  jp_lt(addr);
```
- **相対ジャンプ:**
  jr_*cc* の *cc* (コンディション コード) は,
  **z, eq, nz, ne, c, lt, nc, ge** が使えます.
```c
  jr(addr);
  jr_z(addr);
  djnz(B, addr);
```
- **コール:**
  call_*cc* の *cc* (コンディション コード) は, jp_*cc* と同じです
```c
  call(addr);
  call_v(addr);
  rst(0x38);
```
- **リターン:**
  ret_*cc* の *cc* (コンディション コード) は, jp_*cc* と同じです
```c
  return; ret();          // この 2 つは同じ命令
  ret_m();
  reti(); retn();
```

### CPU 制御, キャリーフラグ制御
- 未定義の NOP_NOP には非対応です
```c
  nop(); halt();
  di(); ei(); im0(); im1(); im2();
  scf(); ccf();
```

# Z80 構造化命令
- 基本, ジャンプ命令に変換されます

### if (cc) <文1>[ else <文2>]
- *cc* は次のいずれかです:<br>
  - if も else も絶対ジャンプ: **z,    eq,    nz,    ne,    c,    lt,    nc,    ge,   p, m, v, nv, pe, po**
  - if は相対, else は絶対:    **z_jr, eq_jr, nz_jr, ne_jr, c_jr, lt_jr, nc_jr, ge_jr**
  - if は絶対, else は相対:    **z_else_jr, eq_else_jr, nz_else_jr, ne_else_jr, c_else_jr, lt_else_jr, nc_else_jr, ge_else_jr, p_else_jr, m_else_jr, v_else_jr, nv_else_jr, pe_else_jr, po_else_jr**
  - if も else も相対ジャンプ: **z_jr_else_jr, eq_jr_else_jr, nz_jr_else_jr, ne_jr_else_jr, c_jr_else_jr, lt_jr_else_jr, nc_jr_else_jr, ge_jr_else_jr**
```
  if (z_jr_else_jr) {
    A = 1;
  } else A = 2;
```
- ネストする場合は, else 前のジャンプ命令は最適化されます
- <文1> が単文無条件ジャンプの場合は最適化されます:
```
  if (eq) jp(addr);          // jp_z(addr) と同じ
```

### do <文> while (cc);
- *cc* は次のいずれかです:<br>
  - 絶対ジャンプ: **z,    eq,    nz,    ne,    c,    lt,    nc,    ge,   p, m, v, nv, pe, po**
  - 相対ジャンプ: **z_jr, eq_jr, nz_jr, ne_jr, c_jr, lt_jr, nc_jr, ge_jr**
  - 無限ループ: **true, true_jr**
  - 1回で抜ける **false**
  - djnz 命令: **B--**
-  break はループから抜けます. continue はループのトップにジャンプします
```c
  do {
    A++;
    if (c_jr) { continue; } // 「jr c, ループ先頭」と同じ
    if (c) {
      B = 1;
      break;                // ループを強制的に抜ける(この場合は, JP 命令になります)
    }
  } while (z);
```

### while (cc) <文>
- *cc* は次のいずれかです:<br>
  - 絶対ジャンプ: **z,    eq,    nz,    ne,    c,    lt,    nc,    ge,   p, m, v, nv, pe, po**
  - 相対ジャンプ: **z_jr, eq_jr, nz_jr, ne_jr, c_jr, lt_jr, nc_jr, ge_jr**
  - 無限ループ: **true**
- break はループから抜けます. continue はループのトップにジャンプします
```c
  while (c_jr) {
    A++;
  }                         // ループは, 常に JP 命令が使われます
```

# 疑似命令
```c
    AAL_DEF_VARS;
    AAL_LOCAL(expr, ...);
    AAL_DB(expr, ...);
    AAL_DW(expr, ...);
    AAL_IF(expr); ... AAL_ELIF(expr); ... AAL_ELSE; ... AAL_ENDIF;
    AAL_REPT(expr); AAL_REPTI(var, expr); AAL_ENDR;
    AAL_GLOBAL(...);
```
- extern 宣言は C のをそのまま使います
```c
    extern foo;
```
