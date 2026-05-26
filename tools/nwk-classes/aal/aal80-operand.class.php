<?php
/** Z80 代数アセンブリ言語フィルタ (Algebraic Assembly Language) - オペランドを表現するクラスです
 * @author Snail Barbarian Macho (NWK) 2024.06.06
 */

declare(strict_types = 1);
namespace nwk\aal;
require_once(__DIR__ . '/../utils/error.class.php');
require_once('aal80-parser.class.php');

// MARK: Operand
/** 命令のパラメータを分類します
 */
Class Operand
{
    // ---------------------------------------------------------------- 定数, メンバ変数
    public const TYPE_A         = 'a';          // A
    public const TYPE_A_C       = 'a+c';        // A+c
    public const TYPE_F         = 'f';          // F
    public const TYPE_B         = 'b';          // B
    public const TYPE_B_C       = 'b+c';        // B+c
    public const TYPE_CDE8      = 'cde8';       // C, D, E
    public const TYPE_CDE8_C    = 'cde8+c';     // C+c, D+c, E+c
    public const TYPE_HL8       = 'hl8';        // H, L
    public const TYPE_HL8_C     = 'hl8+c';      // H+c, L+c
    public const TYPE_IX8       = 'ix8';        // IXH, IXL等
    public const TYPE_IX8_C     = 'ix8+c';      // IXH+c, IXL+c等
    public const TYPE_IR        = 'ir';         // I,R
    public const TYPE_AF        = 'af';         // AF
    public const TYPE_HL        = 'hl';         // HL
    public const TYPE_HL_C      = 'hl+c';       // HL+c
    public const TYPE_BC        = 'bc';         // BC
    public const TYPE_BC_C      = 'bc+c';       // BC+c
    public const TYPE_DE        = 'de';         // DE
    public const TYPE_DE_C      = 'de+c';       // DE+c
    public const TYPE_IX        = 'ix';         // IX等
    public const TYPE_IX_C      = 'ix+c';       // IX+c等
    public const TYPE_SP        = 'sp';         // SP
    public const TYPE_SP_C      = 'sp+c';       // SP+c
    public const TYPE_MEM       = 'mem';        // mem[addr]
    public const TYPE_MEM_HL    = 'mem_hl';     // mem[HL]
    public const TYPE_MEM_HL_C  = 'mem_hl+c';   // mem[HL]+c
    public const TYPE_MEM_IX    = 'mem_ix';     // mem[IX+d]等
    public const TYPE_MEM_IX_C  = 'mem_ix+c';   // mem[IX+d]+c等
    public const TYPE_MEM_DEBC  = 'mem_debc';   // mem[DE], mem[BC]
    public const TYPE_MEM_SP    = 'mem_sp';     // mem[SP]
    public const TYPE_PORT      = 'port';       // port[addr]
    public const TYPE_PORT_C    = 'port_c';     // port[C], port[BC]
    public const TYPE_NUM       = 'num';        // 即値(整数) 123, 0x123, -12等
    public const TYPE_NUM_C     = 'num+c';      // 即値(整数)+c
    public const TYPE_TRUE      = 'true';       // true
    public const TYPE_FALSE     = 'false';      // false
    public const TYPE_SYM       = 'sym';        // シンボル AAA aaa_ABC 等
    public const TYPE_SYM_C     = 'sym+c';      // シンボル+c
    public const TYPE_EXPR      = 'expr';       // 定数式 AAA, (AAA+123), 123+456 等
    public const TYPE_EXPR_C    = 'expr+c';     // 定数式+c
    public const TYPE_STR       = 'str';        // 文字列 "ABC"
    public const TYPE_FLAG_COMMON   = 'flag';          // すべてのフラグに共通
    public const TYPE_FLAG_IF       = 'flag_if';       // 共通に加えて, if    にあるフラグ
    public const TYPE_FLAG_WHILE    = 'flag_while';    // 共通に加えて, while にあるフラグ
    public const TYPE_FLAG_DO_WHILE = 'flag_do_while'; // 共通に加えて, do - while にあるフラグ
    public const TYPE_OTHER     = 'other';      // それ以外 (命令のパラメータとして使えないが, マクロ引数にはOKかも)

    // 凡例文字列テーブル. エラー時に使います
    public const LEGEND_TAB = array(
        Operand::TYPE_A         => 'A',
        Operand::TYPE_A_C       => 'A+c',
        Operand::TYPE_F         => 'F',
        Operand::TYPE_B         => 'B',
        Operand::TYPE_B_C       => 'B+c',
        Operand::TYPE_CDE8      => 'C|D|E',
        Operand::TYPE_CDE8_C    => 'C+c|D+c|E+c',
        Operand::TYPE_HL8       => 'H|L',
        Operand::TYPE_HL8_C     => 'H+c|L+c',
        Operand::TYPE_IX8       => 'IXH等',
        Operand::TYPE_IX8_C     => 'IXH+c等',
        Operand::TYPE_IR        => 'I|R',
        Operand::TYPE_AF        => 'AF',
        Operand::TYPE_HL        => 'HL',
        Operand::TYPE_HL_C      => 'HL+c',
        Operand::TYPE_BC        => 'BC',
        Operand::TYPE_BC_C      => 'BC+c',
        Operand::TYPE_DE        => 'DE',
        Operand::TYPE_DE_C      => 'DE+c',
        Operand::TYPE_IX        => 'IX等',
        Operand::TYPE_IX_C      => 'IX+c等',
        Operand::TYPE_SP        => 'SP',
        Operand::TYPE_SP_C      => 'SP+c',
        Operand::TYPE_MEM       => 'mem[addr]',
        Operand::TYPE_MEM_HL    => 'mem[HL]',
        Operand::TYPE_MEM_HL_C  => 'mem[HL]+c',
        Operand::TYPE_MEM_IX    => 'mem[IX+d]',
        Operand::TYPE_MEM_IX_C  => 'mem[IX+d]+c',
        Operand::TYPE_MEM_DEBC  => 'mem[DE]|mem[BC]',
        Operand::TYPE_MEM_SP    => 'mem[SP]',
        Operand::TYPE_PORT      => 'port[addr]',
        Operand::TYPE_PORT_C    => 'port[C]',
        Operand::TYPE_NUM       => '数値',
        Operand::TYPE_NUM_C     => '数値+c',
        Operand::TYPE_SYM       => 'シンボル',
        Operand::TYPE_SYM_C     => 'シンボル+c',
        Operand::TYPE_EXPR      => '数式',
        Operand::TYPE_EXPR_C    => '数式+c',
        Operand::TYPE_TRUE      => 'true',
        Operand::TYPE_FALSE     => 'talse',
        Operand::TYPE_STR       => '文字列',
        Operand::TYPE_FLAG_COMMON   => 'z等',
        Operand::TYPE_FLAG_IF       => 'z_else_jr等',
        Operand::TYPE_FLAG_WHILE    => '',
        Operand::TYPE_FLAG_DO_WHILE => 'B--等',
        Operand::TYPE_OTHER     => ''
    );

    private const BACK_REGS8_  = 'A_|B_|C_|D_|E_|H_|L_';
    private const BACK_REGS16_ = 'AF_|BC_|DE_|HL_';
    private const BACK_REGS_   = Operand::BACK_REGS8_ . '|' . Operand::BACK_REGS16_;
    public const REGS8     = 'A|B|C|D|E|H|L|I|R|IXH|IXL|IYH|IYL|XH|XL|YH|YL|' . Operand::BACK_REGS8_;
    public const REGS16    = 'AF|BC|DE|HL|PC|SP|IX|IY|' . Operand::BACK_REGS16_;
    public const REGS      = Operand::REGS8 . '|' . Operand::REGS16;
    public const SYMBOL    = '[A-Za-z_][A-Za-z0-9_]*';                // シンボル('fooBar2', 'foo_bar_2' など)

    //                       説明                     例
    // -------------------------------------------------------------------------------------------------
    public string $org;   // 元の表記                 A         mem[reg_HL_]    0x10        FooBar      (123+ABC)
    public string $type;  // パラメータの型           TYPE_A    TYPE_MEM_HL     TYPE_NUM    TYPE_SYM    TYPE_EXPR
    public string $value; // パーサーで使用する表記   A         (HL)            16          FooBar      (123+ABC)
    public string $out;   // アセンブラに出力する表記 A         (reg_HL_)       0x10        0+FooBar    0+(123+ABC)

    // ---------------------------------------------------------------- コンストラクタ
    /** 文字列からオペランドを作成します
     * - 種類もここで分類されます
     * - "HL_" 等の裏レジスタは表レジスタに変換されます
     * - "reg_X" 等のマクロ引数はそのまま. "reg_X_" のような裏レジスタ指定があってもそのまま
     * @param $operandStr オペランド文字列
     * @param $mode    モード Parser::MODE_FUNC または, Parser::MODE_MACRO
     */
    public function __construct(string $operandStr, string $mode)
    {
        // $operandStr 内の空白文字をすべて削除します
        // 但し, \w に挟まれた空白文字(連続含む)は維持します.
        // (?<! ... ) は, 否定後読み「直前は...が無い」
        // (?! ... ) は, 否定先読み「直後...が無い」
        $operandStr = preg_replace('/(?<!\w)\s+|\s+(?!\w)/u', '', $operandStr);
        $type  = Operand::TYPE_OTHER;
        $this->org = $operandStr;
        $value  = $operandStr;
        $out    = $operandStr;

        do {
            // 8bit レジスタ
            if (preg_match('/^(I|R)(_\w*)?$/',  $operandStr, $matches)) { $type = Operand::TYPE_IR;  $value = $matches[1]; $out = Operand::genRegOut_($mode, $matches); break; }
            if (preg_match('/^(A)(_\w*)?$/',    $operandStr, $matches)) { $type = Operand::TYPE_A;   $value = $matches[1]; $out = Operand::genRegOut_($mode, $matches); break; }
            if (preg_match('/^(A)(_\w*)?\+c$/', $operandStr, $matches)) { $type = Operand::TYPE_A_C; $value = $matches[1]; $out = Operand::genRegOut_($mode, $matches); break; }
            if (preg_match('/^(B)(_\w*)?$/',    $operandStr, $matches)) { $type = Operand::TYPE_B;   $value = $matches[1]; $out = Operand::genRegOut_($mode, $matches); break; }
            if (preg_match('/^(B)(_\w*)?\+c$/', $operandStr, $matches)) { $type = Operand::TYPE_B_C; $value = $matches[1]; $out = Operand::genRegOut_($mode, $matches); break; }
            if (preg_match('/^(F)(_\w*)?$/',    $operandStr, $matches)) { $type = Operand::TYPE_F;   $value = $matches[1]; $out = Operand::genRegOut_($mode, $matches); break; }
            if (preg_match('/^(C|D|E)(_\w*)?$/',    $operandStr, $matches)) { $type = Operand::TYPE_CDE8;   $value = $matches[1]; $out = Operand::genRegOut_($mode, $matches); break; }
            if (preg_match('/^(C|D|E)(_\w*)?\+c$/', $operandStr, $matches)) { $type = Operand::TYPE_CDE8_C; $value = $matches[1]; $out = Operand::genRegOut_($mode, $matches); break; }
            if (preg_match('/^(H|L)(_\w*)?$/',      $operandStr, $matches)) { $type = Operand::TYPE_HL8;    $value = $matches[1]; $out = Operand::genRegOut_($mode, $matches); break; }
            if (preg_match('/^(H|L)(_\w*)?\+c$/',   $operandStr, $matches)) { $type = Operand::TYPE_HL8_C;  $value = $matches[1]; $out = Operand::genRegOut_($mode, $matches); break; }
            if (preg_match('/^(IXH|IXL|IYH|IYL)(_\w*)?$/',    $operandStr, $matches)) { $type = Operand::TYPE_IX8;    $value = $matches[1]; $out = Operand::genRegOut_($mode, $matches); break; }
            if (preg_match('/^(IXH|IXL|IYH|IYL)(_\w*)?\+c$/', $operandStr, $matches)) { $type = Operand::TYPE_IX8_C;  $value = $matches[1]; $out = Operand::genRegOut_($mode, $matches); break; }
            // 16bit レジスタ
            if (preg_match('/^(AF)(_\w*)?$/',       $operandStr, $matches)) { $type = Operand::TYPE_AF;   $value = $matches[1]; $out = Operand::genRegOut_($mode, $matches); break; }
            if (preg_match('/^(HL)(_\w*)?$/',       $operandStr, $matches)) { $type = Operand::TYPE_HL;   $value = $matches[1]; $out = Operand::genRegOut_($mode, $matches); break; }
            if (preg_match('/^(HL)(_\w*)?\+c$/',    $operandStr, $matches)) { $type = Operand::TYPE_HL_C; $value = $matches[1]; $out = Operand::genRegOut_($mode, $matches); break; }
            if (preg_match('/^(BC)(_\w*)?$/',       $operandStr, $matches)) { $type = Operand::TYPE_BC;   $value = $matches[1]; $out = Operand::genRegOut_($mode, $matches); break; }
            if (preg_match('/^(BC)(_\w*)?\+c$/',    $operandStr, $matches)) { $type = Operand::TYPE_BC_C; $value = $matches[1]; $out = Operand::genRegOut_($mode, $matches); break; }
            if (preg_match('/^(DE)(_\w*)?$/',       $operandStr, $matches)) { $type = Operand::TYPE_DE;   $value = $matches[1]; $out = Operand::genRegOut_($mode, $matches); break; }
            if (preg_match('/^(DE)(_\w*)?\+c$/',    $operandStr, $matches)) { $type = Operand::TYPE_DE_C; $value = $matches[1]; $out = Operand::genRegOut_($mode, $matches); break; }
            if (preg_match('/^(IX|IY)(_\w*)?$/',    $operandStr, $matches)) { $type = Operand::TYPE_IX;   $value = $matches[1]; $out = Operand::genRegOut_($mode, $matches); break; }
            if (preg_match('/^(IX|IY)(_\w*)?\+c$/', $operandStr, $matches)) { $type = Operand::TYPE_IX_C; $value = $matches[1]; $out = Operand::genRegOut_($mode, $matches); break; }
            if (preg_match('/^(SP)(_\w*)?$/',       $operandStr, $matches)) { $type = Operand::TYPE_SP;   $value = $matches[1]; $out = Operand::genRegOut_($mode, $matches); break; }
            if (preg_match('/^(SP)(_\w*)?\+c$/',    $operandStr, $matches)) { $type = Operand::TYPE_SP_C; $value = $matches[1]; $out = Operand::genRegOut_($mode, $matches); break; }
            // メモリ
            if (preg_match('/^mem\[(BC|DE)(_\w*)?\]$/', $operandStr, $matches)) { $type = Operand::TYPE_MEM_DEBC; $value = '(' . $matches[1] . ')'; $out = '(' . Operand::genRegOut_($mode, $matches) . ')'; break; }
            if (preg_match('/^mem\[(HL)(_\w*)?\]$/',    $operandStr, $matches)) { $type = Operand::TYPE_MEM_HL;   $value = '(' . $matches[1] . ')'; $out = '(' . Operand::genRegOut_($mode, $matches) . ')'; break; }
            if (preg_match('/^mem\[(HL)(_\w*)?\]\+c$/', $operandStr, $matches)) { $type = Operand::TYPE_MEM_HL_C; $value = '(' . $matches[1] . ')'; $out = '(' . Operand::genRegOut_($mode, $matches) . ')'; break; }
            if (preg_match('/^mem\[(SP)(_\w*)?\]$/',    $operandStr, $matches)) { $type = Operand::TYPE_MEM_SP;   $value = '(' . $matches[1] . ')'; $out = '(' . Operand::genRegOut_($mode, $matches) . ')'; break; }
            if (preg_match('/^mem\[(IX|IY)(_\w*)?\+(' . Parser::EXPR_NMEM . ')\]$/',    $operandStr, $matches)) { $type = Operand::TYPE_MEM_IX;   $value = '(' . $matches[1] . ' + '. $matches[2]. ')'; $out = Operand::genMemIxOut_($mode, $matches); break; }
            if (preg_match('/^mem\[(IX|IY)(_\w*)?\+(' . Parser::EXPR_NMEM . ')\]\+c$/', $operandStr, $matches)) { $type = Operand::TYPE_MEM_IX_C; $value = '(' . $matches[1] . ' + '. $matches[2]. ')';  $out = Operand::genMemIxOut_($mode, $matches); break; }
            if (preg_match('/^mem\[(' .                 Parser::EXPR_NMEM . ')\]$/',    $operandStr, $matches)) { $type = Operand::TYPE_MEM;      $value = $out = '(' . $matches[1] . ')'; break; }
            // ポート
            if (preg_match('/^port\[(C|BC)(_\w*)?\]$/',              $operandStr, $matches)) { $type = Operand::TYPE_PORT_C; $value = '(C)'; $out = '(' . Operand::genRegOut_($mode, $matches) . ')'; break; }
            if (preg_match('/^port\[(' . Parser::EXPR_NMEM .')\]$/', $operandStr, $matches)) { $type = Operand::TYPE_PORT;   $value = $out = '(' . $matches[1] . ')'; break; }
            // 10/16進即値
            if (preg_match('/^([+-]?\d+)$/',                    $operandStr, $matches)) { $type = Operand::TYPE_NUM;   $value = $out = $matches[1]; break; }
            if (preg_match('/^([+-]?)(0[xX][0-9a-fA-F]+)$/',    $operandStr, $matches)) { $type = Operand::TYPE_NUM;   $value = $out = $matches[1] . strval(hexdec($matches[2])); break; }
            if (preg_match('/^([+-]?\d+)\+c$/',                 $operandStr, $matches)) { $type = Operand::TYPE_NUM_C; $value = $out = $matches[1]; break; }
            if (preg_match('/^([+-]?)(0[xX][0-9a-fA-F]+)\+c$/', $operandStr, $matches)) { $type = Operand::TYPE_NUM_C; $value = $out = $matches[1] . strval(hexdec($matches[2])); break; }
            // true/false
            if (preg_match('/^true$/',  $operandStr, $matches)) { $type = Operand::TYPE_TRUE;  $value = $out = 'true';   break; }
            if (preg_match('/^false$/', $operandStr, $matches)) { $type = Operand::TYPE_FALSE; $value = $out = 'false';  break; }
            // フラグ
            if (preg_match('/^(?:z|eq|nz|ne|c|lt|nc|ge|p|m|v|nv|pe|po|z_jr|eq_jr|nz_jr|ne_jr|c_jr|lt_jr|nc_jr|ge_jr)$/',
                $operandStr, $matches)) { $type = Operand::TYPE_FLAG_COMMON;   break; }
            if (preg_match('/^(?:z_else_jr|eq_else_jr|nz_else_jr|ne_else_jr|c_else_jr|lt_else_jr|nc_else_jr|ge_else_jr|p_else_jr|m_else_jr|v_else_jr|nv_else_jr|pe_else_jr|po_else_jr|z_jr_else_jr|eq_jr_else_jr|nz_jr_else_jr|ne_jr_else_jr|c_jr_else_jr|lt_jr_else_jr|nc_jr_else_jr|ge_jr_else_jr)$/',
                $operandStr, $matches)) { $type = Operand::TYPE_FLAG_IF;       break; }
            //if (preg_match('/^(?:true_jr)$/',           $operandStr, $matches)) { $type = Operand::TYPE_FLAG_WHILE;    break; }
            if (preg_match('/^(?:B--|--B|B_--|--B_)$/', $operandStr, $matches)) { $type = Operand::TYPE_FLAG_DO_WHILE; break; }
            // シンボル
            if (preg_match('/^(' . Operand::SYMBOL . ')\+c$/',   $operandStr, $matches)) { $type = Operand::TYPE_SYM_C; $out = '0 + ' . $matches[1]; break; }
            if (preg_match('/^(' . Operand::SYMBOL . ')$/',      $operandStr, $matches)) { $type = Operand::TYPE_SYM;   $out = '0 + ' . $matches[1]; break; }
            // 文字列
            if (preg_match('/^"((?:[^\\"]+|\\.)*)"$/', $operandStr, $matches)) { $type = Operand::TYPE_STR; $out = '"' . $matches[1] . '"'; break; }
            // 定数式
            if (preg_match('/^(' . Parser::EXPR_NMEM .')\+c$/', $operandStr, $matches)) { $type = Operand::TYPE_EXPR_C; $out = '0 + ' . $matches[1]; break; }
            if (preg_match('/^(' . Parser::EXPR_NMEM .')$/',    $operandStr, $matches)) { $type = Operand::TYPE_EXPR;   $out = '0 + ' . $matches[1]; break; }
        } while (false);

        $this->type   = $type;
        $this->value  = $value;
        $this->out    = $out;
    }

    /** レジスタの出力文字列を作成します
     * - マクロモードで, $matches[2] があれば, $matches[1] . $matches[2] を返します
     *   但し, $matches[2] が '_' だけの場合は, $matches[1] のみを返します
     * - そうでなければ $matches[1] を返します
     */
    static private function genRegOut_(string $mode, array $matches): string
    {
        if ($mode == Parser::MODE_MACRO && isset($matches[2])) {
            if ($matches[2] !== '_') {
                return $matches[1] . $matches[2];
            }
        }
        return $matches[1];
    }

    /** インデックス レジスタの出力文字列を作成します
     * - マクロモードで, $matches[2] があれば, $matches[1] . $matches[2] を返します
     * - そうでなければ $matches[1] を返します
     */
    static private function genMemIxOut_(string $mode, array $matches): string
    {
        if ($mode == Parser::MODE_MACRO) {
            if (count($matches) === 4) {
                return '(' . $matches[1] . $matches[2] . '+' . $matches[3] . ')';
            }
            return '(' . $matches[1] . '+' . $matches[2] . ')';
        }
        if (count($matches) === 4) {
            return '(' . $matches[1] . '+' . $matches[3] . ')';
        }
        return '(' . $matches[1] . '+' . $matches[2] . ')';
    }


    /** マクロで使われる reg_RR_xxxx (XXはレジスタ名, xxxx は説明) を生成するサブルーチン
     * param $matches [1] にマッチしたレジスタ名, [2] にマッチした説明 が入ります. [2] は無いこともあります
     */
/*
    static private function genRegOut_(array $matches): string
    {
        if (isset($matches[2])) {
            return 'reg_' . $matches[1] . $matches[2];
        }
        return 'reg_' . $matches[1];
    }
*/

    // ---------------------------------------------------------------- 型チェック ユーティリティ
    /** この Operand オブジェクトが, 配列でしめした型のいずれかかどうかを返します
     *
     */
    public function isTypeOneOf(array $types): bool
    {
        foreach ($types as $type) {
            if ($this->type === $type) { return true; }
        }
        return false;
    }


    /**
     * キャリーがあるかどうかを判定します
     */
    public function isWithCarry(): bool {
        return $this->isTypeOneOf(
            [
                Operand::TYPE_A_C, Operand::TYPE_B_C, Operand::TYPE_CDE8_C, Operand::TYPE_HL8_C,
                Operand::TYPE_IX8_C, Operand::TYPE_MEM_HL_C, Operand::TYPE_MEM_IX_C, Operand::TYPE_NUM_C, Operand::TYPE_SYM_C, Operand::TYPE_EXPR_C,
                Operand::TYPE_HL_C, Operand::TYPE_BC_C, Operand::TYPE_DE_C, Operand::TYPE_IX_C, Operand::TYPE_SP_C,
            ]
        );
    }


    // ---------------------------------------------------------------- 事前処理ユーティリティ
    /** 裏レジスタならば, '_' をカット. それ以外はそのまま */
    public static function stripBackReg(string $value): string
    {
        if (preg_match('/^(?:' . Operand::BACK_REGS_ . ')$/x', $value)) {
            //echo("stripBackReg_: [$value] --> " . trim($value, '_') . "\n");
            return trim($value, '_'); // 最後の '_' を削る
        }
        return $value;
    }



}
