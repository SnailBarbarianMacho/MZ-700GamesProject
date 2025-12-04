<?php
/** Z80 代数表記(algebraic notation)アセンブリ言語 簡易フィルタ 命令のパラメータと種類
 * @author Snail Barbarian Macho (NWK) 2024.06.06
 */

declare(strict_types = 1);
namespace nwk\z80ana;
require_once(__DIR__ . '/../utils/error.class.php');
require_once('z80ana-parser.class.php');

// MARK: Param
/** 命令のパラメータの種類と値 */
Class Param
{
    public const TYPE_VAL   = 'val';    // 値
    public const TYPE_VAL_C = 'val+c';  // 値 + c
    public const TYPE_PORT  = 'port';   // ポート
    public const TYPE_MEM   = 'mem';    // メモリ
    public const TYPE_MEM_C = 'mem+c';  // メモリ + c
    public const TYPE_REG   = 'reg';    // レジスタ
    public const TYPE_REG_C = 'reg+c';  // レジスタ + c
    public const TYPE_FLAG  = 'flag';   // フラグ
    public const TYPE_OTHER = 'other';  // それ以外 (命令のパラメータとして使えないが, マクロ引数にはOKかも)

    public string $value; // 値
    public string $type;  // パラメータの型

    /**
     * @param $param パラメータ
     * @param $mode モード Parser::MODE_FUNC または, Parser::MODE_MACRO
     */
    public function __construct(string $param, string $mode)
    {
        // 改行コードを取る, 両端の空白文字を取る
        $param = str_replace("\n", ' ', $param);
        $param = trim($param);
        $type = Param::TYPE_OTHER;

        if (preg_match('/^mem\s*\[(' . Parser::EXPR_NMEM . ')\]$/x', $param, $match)) { // メモリ "mem[123]"
            $type = Param::TYPE_MEM;
            $out = "($match[1])";
        } else if (preg_match('/^port\s*\[(' . Parser::EXPR_NMEM . ')\]$/x', $param, $match)) { // ポート "port[123]"
            $type = Param::TYPE_PORT;
            $out = "($match[1])";
        } else if (preg_match('/^(?:' . Parser::REG . ')$/x', $param)) {// レジスタ "A"
            $type = Param::TYPE_REG;
            //if (substr($param, -1) === '_') { echo("$param\n"); }
            //$out = trim($param, '_');   // 裏レジスタをカット
            $out = $param;
        } else if (preg_match('/^(:?reg_\w*)$/x', $param)) { // マクロ定義/呼び出し用のレジスタ指定 "reg_A"
            if ($mode === Parser::MODE_MACRO) {
                $type = Param::TYPE_REG;
            } else {
                $type = Param::TYPE_OTHER;
            }
            $out = $param;
        } else if (preg_match('/^(?:' . Parser::FLAG_IF . ')$/x', $param)) { // フラグ
            $type = Param::TYPE_FLAG;
            $out = $param;
        } else if (preg_match('/^mem\s*\[(' . Parser::EXPR_NMEM . ')\]\s*\+\s*c$/x', $param, $match)) { // adc/sbc "mem[HL] + c"
            $type = Param::TYPE_MEM_C;
            $out = "($match[1])";
        } else if (preg_match('/^(' . Parser::REG . ')\s*\+\s*c$/x', $param, $match)) { // adc/sbc "HL + c"
            $type = Param::TYPE_REG_C;
            $out = $match[1];
        } else if (preg_match('/^(reg_\w*)\s*\+\s*c$/x', $param, $match)) { // マクロ定義/呼び出し用レジスタ指定 adc/sbc "reg_A + c"
            if ($mode === Parser::MODE_MACRO) {
                $type = Param::TYPE_REG;
            } else {
                $type = Param::TYPE_OTHER;
            }
        } else if (preg_match('/^(' . Parser::EXPR_NMEM . ')\+\s*c$/x', $param, $match)) { // adc/sbc "123 + c"
            $type = Param::TYPE_VAL_C;
            $out = $match[1];
        } else if (preg_match('/^' . Parser::SYMBOL . '$/x', $param)) { // シンボル名 "foo"
            $type = Param::TYPE_VAL;
            $out = $param;
        } else if (preg_match('/^' . Parser::EXPR_NMEM . '$/x', $param)) {  // 一般的な式 "123+X"
            $type = Param::TYPE_VAL;
            //$out = "0 + ($param)";
            $out = $param;
        } else {
            $type = Param::TYPE_OTHER;
            $out = $param;
        }

        $this->value = $out;
        $this->type  = $type;
    }

    public function isTypeMemRegVal(): bool {
        return $this->type === Param::TYPE_MEM || $this->type === Param::TYPE_REG || $this->type === Param::TYPE_VAL;
    }
    public function isTypeMemRegValC(): bool {
        return $this->type === Param::TYPE_MEM_C || $this->type === Param::TYPE_REG_C || $this->type === Param::TYPE_VAL_C;
    }

    /** TYPE_VAL だったら, 内容を調整します
     * - Z80 ニーモニックのメモリ間接と判定しないように, '0 +' を前足します
     */
    public function adjustVal(): void
    {
        if ($this->type === Param::TYPE_VAL) {
            $this->value = '0 + ' . $this->value;
        }
    }
}
