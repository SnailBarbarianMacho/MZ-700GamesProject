# Z80 代数アセンブリ言語フィルタ aal80
目次:
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
- [Z80 命令](#z80-命令)
    - [ロード命令, 入出力命令](#ロード命令-入出力命令)
        - [8bit 転送](#8bit-転送)
        - [16bit 転送](#16bit-転送)
        - [入出力](#入出力)
        - [使用例](#使用例)
    - [ブロック転送](#ブロック転送)
    - [交換](#交換)
    - [スタック操作](#スタック操作)
    - [算術演算](#算術演算)
    - [比較, サーチ](#比較-サーチ)
    - [論理演算](#論理演算)
    - [ビット操作](#ビット操作)
    - [シフト・ローテート](#シフトローテート)
        - [8bit シフト](#8bit-シフト)
        - [16bit シフト](#16bit-シフト)
        - [ローテート](#ローテート)
    - [制御命令 (絶対ジャンプ, 相対ジャンプ, コール, リターン)](#制御命令-絶対ジャンプ-相対ジャンプ-コール-リターン)
    - [CPU 制御, キャリーフラグ制御](#cpu-制御-キャリーフラグ制御)
- [構造化制御文](#構造化制御文)
    - [if (cc) \<文1\>\[ else \<文2\>\]](#if-cc-文1-else-文2)
    - [do \<文\> while (cc);](#do-文-while-cc)
    - [while (cc) \<文\>](#while-cc-文)
- [疑似命令](#疑似命令)
    - [関数やマクロで必要な疑似命令](#関数やマクロで必要な疑似命令)
    - [z88dk アセンブラ疑似命令](#z88dk-アセンブラ疑似命令)
    - [extern](#extern)

## 特徴
- ソースの可読性が高い代数アセンブリ言語 (Algebraic Assembly Language, 以下 **AAL**) を作り, z88dk/SDCC のプリプロセッサとして PHP で軽く書きました
- C と関数単位で混在できます
- VS Code の Microsoft C 文法チェッカをパスします
- 1 行に複数命令を書けるので, 処理の小さなひとかたまりを 1 行で書けます
- if や while 構文があるので, ラベルが少なくなります
- マクロも書けます
- 一部の未定義命令にも対応
- 個人使用のツールなので, バグが沢山あります
  - 予約語のチェックはやってない
  - 式の評価はおざなり
  - etc!


## 前提となる知識
- Z80 のニーモニックや疑似命令をある程度は知っておいてください
- z88dk/SDCC の C 呼び出し規約を知っておくと便利です


# ソースファイル & コマンドライン

### ソース ファイル
- AAL のソースは, C ソースのインライン アセンブリ言語として扱います
- 拡張子は **.aal.c** を推奨します
- ヘッダには AAL ソースは書きません. 従来のインライン アセンブラを使用してください
- コメントは取り除かれます
- プリプロセッサ処理は行わないので, #if ～ #endif 等は無視してブロック内も処理します

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
- 関数の最初の行は「AAL_DEF_VARS」を記述します (無いとエラー)
```c
#pragma save
#pragma disable_warning 85          // 引数未使用警告抑止
#pragma disable_warning 59          // 戻値未設定警告抑止
char add1(const char x) __aal __z88dk_fastcall // __aal 修飾子をつけます
{
  AAL_DEF_VARS;                     // L = 引数 (SDCC 呼び出し規約 version 0 の __z88dk_fastcall)
  L++;                              // L = 戻値 (SDCC 呼び出し規約 version 0)
  // ret 命令は SDCC が自動で追加します
}
#pragma restore
```

- __naked 関数の場合は, 最後に必ず「AAL_NO_RETURN」か「AAL_FALL_THROUGH」を記述します (無いとエラー ... __naked 関数であることを強調するため)
```c
void fooBar(void) __aal __naked
{
  AAL_DEF_VARS;   // 先頭に必ず AAL_DEF_VARS を付けます
  extern bas;
  jp(baz);
  AAL_NO_RETURN;  // naked 関数なら末尾に必ずこれか AAL_FALL_THROUGH を付けます
}
```

### マクロ定義
- 関数のように書きます
- 関数名は必ず UPPER_SNAKE_CASE でなければなりません (でないとエラー)
- 関数定義に「__aal_macro」修飾子を追加します
- 関数引数がある場合は, すべて int 型にしてください (パーサの手抜きのため)
- レジスタ名で始まる引数名はレジスタとして解釈されます( [レジスタ](#レジスタ) 参照 )
- マクロの最初の行は「AAL_DEF_VARS」を記述します (無いとエラー)
- マクロの最後に必ず「AAL_ENDM」を記述します (無いとエラー ... マクロの末端であることを強調するため)
```c
#pragma save
#pragma disable_warning 85          // 引数未使用警告抑止
#pragma disable_warning 59          // 戻値未設定警告抑止
char FOO_BAR(int A_reg, int val, int HL_test) __aal_macro // 関数名は大文字, __aal_macro 修飾子を付ける, 引数はすべて int
{
  AAL_DEF_VARS; // 先頭に必ず AAL_DEF_VARS を付けます
  A_reg = val;
  HL_test = val;
  AAL_ENDM;     // 末尾に必ず AAL_ENDM を付けます
}
#pragma restore
```

- マクロの呼び出しは C 関数のように書きます. もちろんマクロからも呼び出せます:
```
  FOO(A, 123);
```

# プログラミング モデル

### レジスタ
- 次の書き方があります (※レジスタ名は大文字です):
  |種類|説明|例|
  |----|----|----|
  |a.レジスタ名                    |A B C D E H L IXH IXL IYH IYL I R AF BC DE HL PC SP IX IY のいずれか.マクロ引数には使わないでください||
  |b.レジスタ名にアンダースコア    |裏レジスタであることを強調するのに使えます. マクロ引数には使わないでください|A_ HL_|
  |c.レジスタ名にアンダースコアに\w|レジスタに意味付けるに使えます|A_counter HL_objPtr|
- AAL_DEF_VARS には a. と b. が C のローカル変数として事前定義されてます.<br>
  c. は定義されてないので int HL_fooBar_baz; のように定義しておいてください (aal80 はこの行は無視します)

### メモリ, ポート
- メモリやポートは, C の配列として扱います
```c
    A = mem[HL];    // ld A, (HL)
    A = port[C];    // in A, (C)
```

# Z80 命令
- 未=未定義命令

### ロード命令, 入出力命令
- 大半は, **代入演算子** (=) を使います
- ld(), in(), out() 関数はありません
##### 8bit 転送
```c
    r = r;
    r = n; mem[HL] = n;
    r = mem[HL]; mem[HL] = r; r = mem[IX+d]; mem[IX+d] = r;
    A = mem[rr]; mem[rr] = A; A = mem[nn];   mem[nn]   = A;
    I = A; A = I; R = A; A = R;
    IXH = n;未 IXH = r;未 r = IXH未
```
##### 16bit 転送
```c
    rr = nn; IX = nn;
    HL = mem[nn]; mem[nn] = HL; rr = mem[nn]; mem[nn] = rr;
    SP = HL;      SP = IX;      IX = mem[nn]; mem[nn] = IX;
```
##### 入出力
```c
    A = port[n]; r = port[C]; r = port[BC]; F = port[n];未※zcc はこの未定義命令を解釈できません
    port[n] = A; port[C] = r; port[BC] = r; port[n] = 0;未 port[n] = 255;未
```
##### 使用例
```c
    A = 1;
    DE = HL;                    // z88dk が, ld D, H と ld E, L に展開してくれます
    mem[0x1234] = A;
    HL = (FOO + BAR + BAZ);     // メモリ アクセスではなく, 即値です
    HL = mem[FOO + BAR + BAZ];  // メモリ アクセスは mem[～]
    B = mem[HL];
    port[C] = A;                // I/O ポートは port[～]
    port[123] = 0;
```
### ブロック転送
- 関数です
```c
    ldir(); lddr();
    otir(); otdr();
```
### 交換
- 手抜きなのでちょっと制限があります
```c
    // OK
    ex(AF, AF); ex(AF, AF_); ex(AF_, AF_); ex(AF_, AF);
    ex(HL, DE); ex(mem[SP], HL);
    exx();

    // BAD エラー
    ex(DE, HL); ex(HL, mem[SP]);
```
### スタック操作
- 可変長引数となっており, 複数のレジスタをまとめて push / pop できます
```c
    push(HL); push(DE);
    pop(DE, HL);
```

### 算術演算
- add(), adc(), sub(), sbc(), inc(), dec() 関数はありません
- 単項演算子 - には対応しません (A = -A; と書いても判りづらいため)
  neg() を使ってください
- neg(), daa() 命令は, 引数に「A」の明示が必要です
```c
    r += r;     r += mem[HL]; rr += rr;
    r += r + c;               rr += rr + c;
    r -= r;
    r -= r + c;               rr -= rr + c;
    r++; ++r; mem[HL]++; ++mem[HL]; rr++; ++rr;
    r--; --r; mem[HL]--; --mem[HL]; rr--; --rr;
    neg(A); daa(A);
```

### 比較, サーチ
- cp() 命令は, 引数に「A」の明示が必要です
```c
    cp(A, B);
    cpi(); cpir(); cpd(); cpdr();
```

### 論理演算
- and(), or(). xor() 関数はありません
- 単項演算子 ~ には対応しません (A = ~A; と書いても判りづらいため)<br>
  not() を使ってください
- cpl() はありません. not() を使ってください
- not() 命令は, 引数に「A」の明示が必要です
```c
    A &= B; A |= B; A ^= B;
    not(A);         // この 2 つは同じ命令です
```

### ビット操作
- 未定義命令の bit3, set3, res3 命令も対応してます
```c
    bit(n, r); bit(n, mem[HL]); bit3(n, mem[IX+d], r);未
    set(n, r); set(n, mem[HL]); set3(n, mem[IX+d], r);未
    res(n, r); res(n, mem[HL]); res3(b, mem[IX+d], r);未
```

### シフト・ローテート
##### 8bit シフト
- 8bit の右シフト演算子 (>>=) は, 符号無し (srl) 扱いです
- A レジスタの左シフトは add 命令のほうが高速ですが, ここでは最適化されません
- 複数回シフト命令もあります (sla_n() 等のように, 末尾に "_n" が付きます)<br>
  imm に数値を指定すると, 8以上の値はエラーになります
```c
  sla(r);   sla(mem[HL]);   sla_n(r, imm);   sla_n(mem[HL], imm);    r <<= imm; sla2(mem[IX+d], r);未
  sll(r);未 sll(mem[HL]);未 sll_n(r, imm);未 sll_n(mem[HL], imm);未             sll2(mem[IX+d], r);未
  sra(r);   sra(mem[HL]);   sra_n(r, imm);   sra_n(mem[HL], imm);               sra2(mem[IX+d], r);未
  srl(r);   srl(mem[HL]);   srl_n(r, imm);   srl_n(mem[HL], imm);    r >>= imm; srl2(mem[IX+d], r);未
```

##### 16bit シフト
- 16bit のシフトは 2 個の命令に分解されます<br>
  例えば, sla(HL) は, sla(L) と rl(H) になります
- 16bit の右シフト演算子 (>>=) は, 符号付き (sra) 扱いです
- HL レジスタの左シフトは add 命令のほうが高速ですが, ここでは最適化されません
- 複数回シフト命令もあります (sla_n() 等のように, 末尾に "_n" が付きます)<br>
  imm に数値を指定すると, 8以上の値はエラーになります
```c
  sla(rr);   sla_n(rr, imm);   rr <<= imm;
  sll(rr);未 sll_n(rr, imm);未
  sra(rr);   sra_n(rr, imm);   rr >>= imm;
  srl(rr);   srl_n(rr, imm);
```

##### ローテート
- 複数回ローテートに展開される命令もあります (rlc_n() 等の様に, 末尾に "_n" が付きます)<br>
  imm に数値を指定すると, 8以上の値はエラーになります
- rlca() 等の命令は, 引数に「A」の明示が必要です
- rlc(A) と書いても, rlca() 命令にはなりません
```c
  rlc(r);   rlc(mem[HL]);   rlc_n(r, imm);   rlc_n(mem[HL], imm); rlc2(mem[IX+d], r);未
  rrc(r);   rrc(mem[HL]);   rrc_n(r, imm);   rrc_n(mem[HL], imm); rrc2(mem[IX+d], r);未
  rl(r);    rl(mem[HL]);    rl_n(A, imm);    rl_n(mem[HL], imm);  rl2(mem[IX+d], r);未
  rr(r);    rr(mem[HL]);    rr_n(A, imm);    rr_n(mem[HL], imm);  rr2(mem[IX+d], r);未
  rlca(A);                  rlca_n(A, imm);
  rrca(A);                  rrca_n(A, imm);
  rla(A);                   rla_n(A, imm);
  rra(A);                   rra_n(A, imm);
  rld(A, mem[HL]);
  rrd(A, mem[HL]);
```
未=未定義命令

### 制御命令 (絶対ジャンプ, 相対ジャンプ, コール, リターン)
- *cc* (コンディションコード) は次のいずれかです:

 |種類|*cc*|
 |----|----|
 |jp_*cc* (絶対条件ジャンプ)<br>call_*cc* (条件コール)<br>ret_*cc* (条件リターン)|z, eq, nz, ne, c, lt, nc, ge, p, m, v, nv, pe, po<br>eq は z, ne は nz, lt は c, ge は nc と同じ|
 |jr_*cc* (相対条件ジャンプ)|z, eq, nz, ne, c, lt, nc, ge<br>eq は z, ne は nz, lt は c, ge は nc と同じ|


```c
  // 絶対ジャンプ
  jp(addr);         // goto は使用できません
  jp(HL);           // jp (HL)
  jp_lt(addr);

  // 相対ジャンプ
  jr(addr);
  jr_z(addr);
  djnz(B, addr);    // djnz() は, 引数に「B」を明示します

  // コール
  call(addr);
  call_v(addr);
  rst(0x38);

  // リターン
  return;           // ret() はありません
  ret_m();
  reti(); retn();
```
- djnz() は, 「do ... while (B--);」 という書き方もあります

### CPU 制御, キャリーフラグ制御
- 未定義の NOP_NOP 命令には非対応です
```c
  nop(); halt();
  di(); ei(); im0(); im1(); im2();
  scf(); ccf();
```

# 構造化制御文
- if, do-while, while があり, ジャンプ命令に変換されます
- 単文の場合は { ... } で囲む必要はありませんが, あったほうがコードが読みやすいです

### if (cc) <文1>[ else <文2>]
- *cc* は次のいずれかです:

 |ジャンプの種類|cc|
 |----|----|
 |if も else も絶対ジャンプ|z,    eq,    nz,    ne,    c,    lt,    nc,    ge,   p, m, v, nv, pe, po |
 |if は相対, else は絶対   |z_jr, eq_jr, nz_jr, ne_jr, c_jr, lt_jr, nc_jr, ge_jr |
 |if は絶対, else は相対   |z_else_jr, eq_else_jr, nz_else_jr, ne_else_jr, c_else_jr, lt_else_jr, nc_else_jr, ge_else_jr, p_else_jr, m_else_jr, v_else_jr, nv_else_jr, pe_else_jr, po_else_jr|
 |if も else も相対        |z_jr_else_jr, eq_jr_else_jr, nz_jr_else_jr, ne_jr_else_jr, c_jr_else_jr, lt_jr_else_jr, nc_jr_else_jr, ge_jr_else_jr|

```c
  if (z_jr_else_jr) {
    A = 1;
  } else A = 2;
```
- if がネストする場合は, else 前のジャンプ命令は最適化されます
```c
  if (c) {
    if (z) {      // nz の場合は, else 節の最後にジャンプします
       /* ... */
    }
  } else {
    /* ... */
  }
```
- <文1> が単文で無条件ジャンプ命令で else が無い場合は最適化されます:
```c
  if (z)    { jp(addr); }                   // jp_z(addr) と同じ
  if (z_jr) { jp(addr); }                   // jr_z(addr) と同じ
  if (c)    { call(addr); }                 // call_z(addr) と同じ
  if (c_jr) { call(addr); }                 // call_z(addr) と同じ (相対ジャンプは無視)
  if (z) { if (z) { jp(foo); }}             // 内側の if のみ最適化されます
  if (z) { jp(addr); A = 1; }               // 最適化されません (<文1> が単文でない)
  if (z) { jp_c(addr); } else { /* ... */ } // 最適化されません (無条件ジャンプでない)
  if (z) { jp(addr); } else { /* ... */ }   // 最適化されません (else がある)
```

### do <文> while (cc);
- *cc* は次のいずれかです:

 |種類|cc|
 |----|----|
 |絶対ジャンプ|z,    eq,    nz,    ne,    c,    lt,    nc,    ge,   p, m, v, nv, pe, po|
 |相対ジャンプ|z_jr, eq_jr, nz_jr, ne_jr, c_jr, lt_jr, nc_jr, ge_jr|
 |1回で抜ける |false|
 |djnz 命令   |B--, --B|

- break はループから抜けます. continue は<span style="color: red;">ループ末端</span>にジャンプします. <br>
- 無限ループは, <span style="color: red;">while(true) <文></span> を使用してください
```c
  do {
    A++;
    if (c_jr) { continue; } // 「jr c, ループ末端」と同じ
    if (c) {
      B = 1;
      break;                // ループを強制的に抜けます (この場合は, JP 命令になります)
    }
    if (c_jr) { break; }    // JR 命令による break
    // ループ末端
  } while (z);
```

### while (cc) <文>
- *cc* は次のいずれかです:

 |種類|cc|
 |----|----|
 |絶対ジャンプ|z,    eq,    nz,    ne,    c,    lt,    nc,    ge,   p, m, v, nv, pe, po|
 |相対ジャンプ|z_jr, eq_jr, nz_jr, ne_jr, c_jr, lt_jr, nc_jr, ge_jr|
 |無限ループ  |true|
- break はループから抜けます. continue は<span style="color: red;">ループ先頭</span>にジャンプします
- 1回しか実行しないループは, <span style="color: red;">do <文> while(false);</span> を使用してください
```c
  while (c_jr) {
    A++;
  }                         // ループ先頭へのジャンプは, 常に JP 命令が使われます
```

# 疑似命令

### 関数やマクロで必要な疑似命令
- [C の関数](#c-の関数), [マクロ定義](#マクロ定義) も参考にしてください
```c
    AAL_DEF_VARS;         // 関数やマクロの先頭で必要. 無いとエラー
    AAL_NO_RETURN;        // __naked 関数の一番最後に必要 (AAL_FALL_THROUGH のどちらか）. 無いとエラー
    AAL_FALL_THROUGH;     // __naked 関数の一番最後に必要 (AAL_NO_RETURN のどちらか). 無いとエラー
    AAL_ENDM;             // マクロの一番最後に必要. 無いとエラー
```

### z88dk アセンブラ疑似命令
```c
    AAL_LOCAL(sym, ...);    // マクロのローカル シンボル
    AAL_GLOBAL(sym, ...);   // グローバル シンボル
    AAL_DB(expr, ...);      // バイト列. ASCII 文字列("...") にも対応
    AAL_DW(expr, ...);      // ワード列
    AAL_DS(size, expr, ...);// スペースをあけて, バイト列(余りは 0)で埋める
    AAL_DS(size, string);   // スペースをあけて, ASCII 文字列("...") (余りは 0)で埋める
    AAL_IF(expr); ... AAL_ELIF(expr); ... AAL_ELSE; ... AAL_ENDIF;// 条件アセンブル. 恰好悪いですが最後のセミコロンを忘れずに
    AAL_REPT(expr); ... AAL_ENDR;                // 反復. 恰好悪いですが最後のセミコロンを忘れずに
    AAL_REPTC(var, string); ... AAL_ENDR;        // ASCII 文字列("...")を文字単位に反復して var に代入します
    AAL_REPTI(var, expr, ... ); ... AAL_ENDR;    // expr を反復して var に代入します
    AAL_DEF_DUMMY_VARS(...);                     // REPTC, REPTI で使用してる var をダミー定義して, VSCode でエラー扱いにならないようにします
```

### extern
- extern 宣言は C のをそのまま使います
```c
    extern foo;
```
