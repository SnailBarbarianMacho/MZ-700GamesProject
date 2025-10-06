<?php
/** Z80 代数表記(algebraic notation)アセンブリ言語 簡易フィルタ パーサー(構文解析)
 * @author Snail Barbarian Macho (NWK) 2024.06.06
 */

declare(strict_types = 1);
namespace nwk\z80ana;
require_once(__DIR__ . '/../utils/error.class.php');


// MARK: Param
/** 命令のパラメータと種類 */
Class Param
{
    public const TYPE_VAL   = 'val';
    public const TYPE_VAL_C = 'val+c';
    public const TYPE_PORT  = 'port';
    public const TYPE_MEM   = 'mem';
    public const TYPE_MEM_C = 'mem+c';
    public const TYPE_REG   = 'reg';
    public const TYPE_REG_C = 'reg+c';
    public const TYPE_FLAG  = 'flag';

    public string $value; // 値
    public string $type;  // パラメータの型

    public function __construct(string $value, string $type)
    {
        $this->value = $value;
        $this->type  = $type;
    }

    public function isTypeMemRegVal(): bool {
        return $this->type === Param::TYPE_MEM || $this->type === Param::TYPE_REG || $this->type === Param::TYPE_VAL;
    }
    public function isTypeMemRegValC(): bool {
        return $this->type === Param::TYPE_MEM_C || $this->type === Param::TYPE_REG_C || $this->type === Param::TYPE_VAL_C;
    }

    /** TYPE_VAL の場合, Z80 ニーモニックのメモリ間接と判定しないように, 0 + を足します */
    public function adjustVal(): void
    {
        if ($this->type === Param::TYPE_VAL) {
            $this->value = '0 + '.$this->value;
        }
    }
}


// MARK: Parser
/** パーサー(構文解析) */
Class Parser
{
    private string  $mode_;         // 'macro' or 'func'
    private bool    $is_naked_;     // __naked が付いてるか
    private int     $line_nr_;      // ソースの行
    private string  $funcname_;     // 関数またはマクロ名
    private bool    $is_ended_;     // 末端に Z80ANA_ENDM, Z80ANA_NO_RETURN, Z80ANA_FALL_THROUGH を指定したかのフラグ

    private int     $label_ct_;
    private array   $labels_;

    private int     $indent_lv_;

    private \nwk\utils\Error $error_;

    private const EXPR0_ = '[\w\+\-\*\/\%\&\|\^\~\(\)\<\>\,\.\ \n\[\]]*';
    private const EXPR_  = '[\w\+\-\*\/\%\&\|\^\~\(\)\<\>\,\.\ \n\[\]]+'; // A-Za-z0-9_+-*%/()<>,.[] くらい?
    private const EXPR1_ = '[\w\+\-\*\/\%\&\|\^\~\(\)\<\>\,\.\ \n]+';     // A-Za-z0-9_+-*%/()<>,.   くらい?
    private const SYMBOL_= '[A-Za-z_]\w*';
    private const REG_   = 'IXH|IXL|IYH|IYL|HL|DE|BC|AF|PC|SP|IX|IY|XH|XL|YH|YL|A|B|C|D|E|F|H|L|I|R';
    private const FLAG_      = 'z|eq|nz|ne|c|lt|nc|ge|p|m|v|nv|pe|po|z_else_jr|eq_else_jr|nz_else_jr|ne_else_jr|c_else_jr|lt_else_jr|nc_else_jr|ge_else_jr|p_else_jr|m_else_jr|v_else_jr|nv_else_jr|pe_else_jr|po_else_jr';
    private const FLAG_REL_  = 'z_jr|eq_jr|nz_jr|ne_jr|c_jr|lt_jr|nc_jr|ge_jr|z_jr_else_jr|eq_jr_else_jr|nz_jr_else_jr|ne_jr_else_jr|c_jr_else_jr|lt_jr_else_jr|nc_jr_else_jr|ge_jr_else_jr';
    private const FLAG_NEG_TAB_ = array(
        ''   => 'false',
        'z'  => 'nz',
        'eq' => 'nz',
        'nz' => 'z',
        'ne' => 'z',
        'c'  => 'nc',
        'lt' => 'nc',
        'nc' => 'c',
        'ge' => 'c',
        'p'  => 'm',
        'm'  => 'p',
        'v'  => 'nv',
        'nv' => 'v',
        'pe' => 'po',
        'po' => 'pe',
        'z_else_jr'  => 'nz',
        'eq_else_jr' => 'nz',
        'nz_else_jr' => 'z',
        'ne_else_jr' => 'z',
        'c_else_jr'  => 'nc',
        'lt_else_jr' => 'nc',
        'nc_else_jr' => 'c',
        'ge_else_jr' => 'c',
        'p_else_jr'  => 'm',
        'm_else_jr'  => 'p',
        'v_else_jr'  => 'nv',
        'nv_else_jr' => 'v',
        'pe_else_jr' => 'po',
        'po_else_jr' => 'pe',
        'z_jr'  => 'nz',
        'eq_jr' => 'nz',
        'nz_jr' => 'z',
        'ne_jr' => 'z',
        'c_jr'  => 'nc',
        'lt_jr' => 'nc',
        'nc_jr' => 'c',
        'ge_jr' => 'c',
        'z_jr_else_jr'  => 'nz',
        'eq_jr_else_jr' => 'nz',
        'nz_jr_else_jr' => 'z',
        'ne_jr_else_jr' => 'z',
        'c_jr_else_jr'  => 'nc',
        'lt_jr_else_jr' => 'nc',
        'nc_jr_else_jr' => 'c',
        'ge_jr_else_jr' => 'c',
    );
    public const REGS_G16_SPLIT8_TAB_ = [ // AF, SP, PC を除いた汎用16bitレジスタと 8bit に分解したテーブル
        'HL' => ['L', 'H'],
        'DE' => ['E', 'D'],
        'BC' => ['C', 'B'],
        'IX' => ['IXL', 'IXH'],
        'IY' => ['IYL', 'IYH'],
    ];

    // MARK: __construct()
    /** コンストラクタ */
    public function __construct(\nwk\utils\Error $error) {
        $this->error_ = $error;
    }


    // -------------------------------- パース
    // MARK: parse()
    /** パースして変換後の文字列を返します
     * @param $mode      モード 'macro', 'func' のどちらか
     * @param $is_naked  __naked 関数か
     * @param $r_str     解析したい文字列
     * @param $line_nr   $str のある行(0～, エラー表示用)
     * @param $funcname  関数名(ラベル表示用)
     */
    public function parse(string $mode, bool $is_naked, string &$r_str, int $line_nr, string $funcname): string
    {
        $this->line_nr_   = $line_nr;

        if ($mode == 'macro' && !preg_match('/^[A-Z_][A-Z_0-9]+$/', $funcname)) {
            $this->errorLine_("マクロ名は, [A-Z_0-9] で書いてください", $funcname);
        }

        $this->mode_      = $mode;
        $this->is_naked_  = $is_naked;
        $this->funcname_  = $funcname;
        $this->is_ended_  = false;
        $this->initLabels_();
        $this->initIndent_();

        $ret = $this->parseSentences_($r_str);

        if (!$this->is_ended_) {
            if ($this->mode_ === 'macro') {
                $this->errorLine_("マクロの末端には, Z80ANA_ENDM; を追加してくだい");
            } else if ($this->is_naked_) {
                $this->errorLine_("__naked 関数の末端には, Z80ANA_NO_RETURN; または Z80ANA_FALL_THROUGH; を追加してくだい");
            }
        }

        return $ret;
    }


    // -------------------------------- パース サブ関数
    // MARK: parseSentences_()
    /** 複文の解析 */
    private function parseSentences_(string &$r_str)
    {
        $offset = 0;            // $r_str の位置
        $len = strlen($r_str);
        $out = '';
        $old_line_nr = -1;

        $this->incIndent_();
        while ($offset < $len) {
            $line_str = '';
            if ($old_line_nr !== $this->line_nr_) {
                $old_line_nr = $this->line_nr_;
                $line_str = "// line " . ($this->line_nr_ + 1) . "\n";
            }

            $str = $this->parseSentence_($r_str, $offset);
            if ($str !== '') {
                $out .= $line_str;
            }
            $out .= $str;
        }
        $this->decIndent_();

        return $out;
    }


    // MARK: parseSentence_()
    /** 単文の解析 */
    private function parseSentence_(string &$r_str, int &$r_offset): string
    {
        // "\n", ' ', ';'
        if (preg_match('/([\s\;]+)/Ax', $r_str, $matches, 0, $r_offset)) {
            return $this->parseWhitespaces_($r_str, $r_offset, $matches[1]);
        }

        // 'Z80ANA_' で始まる指示語
        if (preg_match('/(Z80ANA_[A-Z_]+)/Ax', $r_str, $matches, 0, $r_offset)) {
            return $this->parseZ80anaDirective_($r_str, $r_offset, $matches[1]);
        }

        // '#' で始まるプリプロセス
        if (preg_match('/(\#\s*\w+)\s+/Ax', $r_str, $matches, 0, $r_offset)) {
            return $this->parsePreprocess_($r_str, $r_offset, $matches[1]);
        }

        // { <文>* }
        if ($r_str[$r_offset] == '{') {
            return $this->parseSubSentences_($r_str, $r_offset);
        }

        // goto <式>;
        if (preg_match('/(goto) \s* (\w+) \s* \;/Ax', $r_str, $matches, 0, $r_offset)) {
            return $this->parseGoto_($r_str, $r_offset, $matches[0], $matches[1], $matches[2]);
        }

        // extern <式>[, ...];
        if (preg_match('/(extern) \s* ([\w\s\,]+) \s* \;/Ax', $r_str, $matches, 0, $r_offset)) {
            return $this->parseExtern_($r_str, $r_offset, $matches[0], $matches[1], $matches[2]);
        }

        // if (<式>) <文> else <文>
        if (preg_match('/if\s*\(\s*(' . Parser::FLAG_ .  '|'. Parser::FLAG_REL_ . ')\s*\)/Ax', $r_str, $matches, 0, $r_offset)) {
            return $this->parseIf_($r_str, $r_offset, $matches[0], $matches[1]);
        }

        // return;  ※式なし
        if (preg_match('/(return) \s*(\w*)\s*\;/Ax', $r_str, $matches, 0, $r_offset)) {
            if (!$matches[2]) {
                return $this->parseReturn_($r_str, $r_offset, $matches[0]);
            }
        }

        // シンボル名:
        if (preg_match('/(' . Parser::SYMBOL_ . '\:)/Ax', $r_str, $matches, 0, $r_offset)) {
            return $this->parseLabel_($r_str, $r_offset, $matches[1]);
        }

        // <式> <代入演算子> <式>;
        if (preg_match('/(' . Parser::EXPR_ . ') \= (' . Parser::EXPR_ . ')\;/Ax', $r_str, $matches, 0, $r_offset)) {
            return $this->parseAssignmentOp_($r_str, $r_offset, $matches[0], $matches[1], $matches[2]);
        }

        // <単項演算子> <reg>;
        if (preg_match('/(\+\+|\-\-) \s* (' . Parser::EXPR_ . ') \s*;/Ax', $r_str, $matches, 0, $r_offset)) {
            return $this->parseUnaryOp_($r_str, $r_offset, $matches[0], $matches[1], $matches[2]);
        }

        // <reg> <単項演算子>;
        if (preg_match('/(' . Parser::EXPR_ . ') (\+\+|\-\-) \s*\;/Ax', $r_str, $matches, 0, $r_offset)) {
            return $this->parseUnaryOp_($r_str, $r_offset, $matches[0], $matches[2], $matches[1]);
        }

        // シンボル名(<式リスト>);
        if (preg_match('/(\w+)\s*\((' . Parser::EXPR0_ . ')\)\s*;/Ax', $r_str, $matches, 0, $r_offset)) {
            return $this->functionCall_($r_str, $r_offset, $matches[0], $matches[1], $matches[2]);
        }

        // 解析できなかった時は, ';' までスキップ
        $pos = strpos($r_str, ';', $r_offset);
        $len = (($pos !== false) ? $pos : strlen($r_str)) - $r_offset;
        $str = substr($r_str, $r_offset, $len);
        $this->errorLine_("解析失敗", $str);

        $r_offset += $len;
        $this->line_nr_ += substr_count($str, "\n");

        return $r_str;
    }


    // -------------------------------- パース ディスパッチャ
    // MARK: parseWhitespaces_()
    /** "\n", ' ', ';' のパース */
    private function parseWhitespaces_(string &$r_str, int &$r_offset, string $whitespaces): string
    {
        //echo("whitespace1[$r_offset $this->line_nr_][$whitespaces]\n");
        $r_offset  += strlen($whitespaces);
        $this->line_nr_ += substr_count($whitespaces, "\n");
        //echo("whitespace2[$r_offset $this->line_nr_]\n");
        return '';
    }


    // MARK: parseZ80anaDirective_()
    /** 'Z80ANA_' で始まる指示語. インデントなし */
    private function parseZ80anaDirective_(string &$r_str, int &$r_offset, string $directive): string
    {
        // echo("directive1[$r_offset $this->line_nr_][$directive]\n");
        // 括弧の無いディレクティブの処理を先にやる
        $offset_old = $r_offset;
        $r_offset += strlen($directive);
        switch ($directive) {
            case 'Z80ANA_ELSE':
                $this->decIndent_();
                $ret = "else\n";
                $this->incIndent_();
                return $ret;
            case 'Z80ANA_ENDIF':
                $this->decIndent_();
                return "endif\n";
            case 'Z80ANA_ENDR':
                $this->decIndent_();
                return "endr\n";
            case 'Z80ANA_DEF_VARS':
                $this->defVarsDirective_($r_str, $offset_old);
                return '';
            case 'Z80ANA_ENDM':
                $this->endmDirective_($r_str, $r_offset);
                return '';
            case 'Z80ANA_NO_RETURN':
                $this->noReturnDirective_($r_str, $r_offset);
                return '';
            case 'Z80ANA_FALL_THROUGH':
                $this->fallThroughDirective_($r_str, $r_offset);
                return '';
        }

        // 括弧のあるディレクティブの処理
        $args = $this->extractParentheses_($r_str, $r_offset);
        $f = $directive . '_';
        if (method_exists($this, $f)) {
            return $this->{$f}($args) . "\n";
        } else {
            $this->errorLine_("この Z80ANA ディレクティブは存在しません", $directive);
            return '';
        }
    }


    // MARK: parsePreprocess_()
    /** 行頭, '#' で始まるプリプロセッサ ディレクティブ */
    private function parsePreprocess_(string &$r_str, int &$r_offset, string $directive): string
    {
        //echo("preprocess1[$r_offset $this->line_nr_][$directive]\n");
        if ($this->isBeginningOfLine_($r_str, $r_offset) === false) {
            $this->toNextBeginningOfLine_($r_str, $r_offset);
            return '';
        }
        $r_offset += strlen($directive);
        $off_start = $r_offset;
        // 改行までを $args にコピー
        $this->toNextBeginningOfLine_($r_str, $r_offset);
        $args = substr($r_str, $off_start, $r_offset - $off_start);
        //echo("preprocess2[$r_offset $this->line_nr_][$directive][$args]\n");
        return "$directive $args"; // $args には改行を含まん
    }


    // MARK: parseSubSentences_()
    /** { ... } で囲んだ複文 */
    private function parseSubSentences_(string &$r_str, int &$r_offset): string
    {
        //echo("subSentences1[$r_offset $this->line_nr_] $r_str[$r_offset]\n");
        // { ... } を検出
        $line_nr = $this->line_nr_;
        $str = $this->extractCurlyBracket_($r_str, $r_offset);
        $this->line_nr_ = $line_nr;
        if ($str == false) {
            return '';
        }

        $out = $this->parseSentences_($str);
        //echo("subSentences2[$r_offset $this->line_nr_]\n");
        return $out;
    }


    // MARK: parseGoto_()
    /** goto <式>; */
    private function parseGoto_(string &$r_str, int &$r_offset, $match, $directive, $label): string
    {
        $r_offset  += strlen($match);
        $this->line_nr_ += substr_count($match, "\n");
        return $this->getIndentStr_() . "jp $label\n";
    }


    // MARK: parseExtern_()
    /** extern <式>[, ...]; */
    private function parseExtern_(string &$r_str, int &$r_offset, $match, $directive, $labels): string
    {
        $r_offset  += strlen($match);
        $this->line_nr_ += substr_count($match, "\n");
        $params = $this->explodeByComma_($labels);

        $ret = $this->getIndentStr_() . "extern ";
        $err = false;

        foreach($params as $i => $param) {
            $p = $this->checkExpr_($param);
            if ($p === null || $p->type !== Param::TYPE_VAL) {
                $this->errorLine_("$opcode 引数 ". ($i + 1) . " のエラー", $param);
                $err = true;
            } else {
                if ($i !== 0) { $ret .= ', '; }
                $ret .= $p->value;
            }
        }
        if ($err) {
            return '';
        }
        return "$ret\n";
    }


    // MARK: parseIf_()
    /** if ( ... ) 文 else 文 */
    private function parseIf_(string &$r_str, int &$r_offset, $match, $flag): string
    {
        $r_offset  += strlen($match);
        $this->line_nr_ += substr_count($match, "\n");

        // フラグとジャンプ命令の設定
        $flag_neg = Parser::FLAG_NEG_TAB_[$flag];
        $opcode_if   = 'jp';
        $opcode_else = 'jp';
        $label1 = $this->genRegistLabel_();
        if (str_ends_with($flag, '_jr_else_jr') ||
            (str_ends_with($flag, '_jr') && !str_ends_with($flag, '_else_jr'))) {
            $opcode_if = 'jr';
        }
        if (str_ends_with($flag, '_else_jr')) {
            $opcode_else = 'jr';
        }

        if ($flag_neg !== 'false') { $flag_neg .= ','; }

        $out = $this->getIndentStr_() . "$opcode_if $flag_neg $label1\n";
        $this->incIndent_();

        $this->skipSpaces_($r_str, $r_offset);
        $out .= $this->parseSentence_($r_str, $r_offset);
        $this->skipSpaces_($r_str, $r_offset);

        // else を見つけた!
        //$str = substr($r_str, $r_offset); echo("[[$str]]\n");
        if (preg_match('/else/Ax', $r_str, $matches, 0, $r_offset)) {
            $r_offset  += strlen('else');

            $label2 = $this->genRegistLabel_();
            $out .= $this->getIndentStr_() . "$opcode_else $label2\n";
            $out .= "$label1:\n";

            $this->skipSpaces_($r_str, $r_offset);
            $out .= $this->parseSentence_($r_str, $r_offset);
            $this->skipSpaces_($r_str, $r_offset);

            $out .= "$label2:\n";
        } else {
            $out .= "$label1:\n";
        }
        $this->decIndent_();

        return $out;
    }

    // MARK: parseReturn_()
    /** return;※<式なし>; */
    private function parseReturn_(string &$r_str, int &$r_offset, $match): string
    {
        $r_offset  += strlen($match);
        $this->line_nr_ += substr_count($match, "\n");
        return $this->getIndentStr_() . "ret\n";
    }


    // MARK: parseLabel_()
    /** ':' で終るシンボル名 */
    private function parseLabel_(string &$r_str, int &$r_offset, string $label): string
    {
        //echo("label1[$r_offset $this->line_nr_][$label]\n");
        $r_offset += strlen($label);
        //$this->registLabel_(substr($label, 0, strlen($label) - 1));
        return "$label\n";  // 改行します
    }


    // MARK: parseAssignmentOp_()
    /** 代入演算子 */
    private function parseAssignmentOp_(string &$r_str, int &$r_offset, string $match, string $l_value, string $r_value): string
    {
        // 演算子の確定. $l_value の右端から1～2文字借りてみて, 有効な演算子ならば, $l_value は借りた文字を失う
        $op = $l_value[-1] . '=';
        if ($op === '+=' || $op === '-=' | $op === '&=' | $op === '|=' | $op === '^=') {
            $l_value = substr($l_value, 0, -1);
        } else if (2 <= strlen($l_value)) {
            $op = $l_value[-2] . $op;
            if ($op === '<<=' || $op === '>>=') {
                $l_value = substr($l_value, 0, -2);
            } else {
                $op = '=';
            }
        } else {
            $op = '=';
        }
        //echo("assignOp1[$r_offset $this->line_nr_][$l_value][$op][$r_value]\n");

        // 型によって命令が変わるので, 右辺値, 左辺値の型を調べる
        $b_err = false;
        $l_param = $this->checkExpr_($l_value);
        $r_param = $this->checkExpr_($r_value);
        if ($l_param === null) {
            $this->errorLine_("左辺値の異常", $match);
            $b_err = true;
        }
        if ($r_param === null) {
            $this->errorLine_("右辺値の異常", $match);
            $b_err = true;
        }
        $ret = '';
        if ($b_err !== true) {
            $expr = $l_value . "," . $this->cutOffCarryExpr_($r_value);

            // 入出力
            if ($l_param->type === Param::TYPE_PORT || $r_param->type === Param::TYPE_PORT) {
                if ($op !== '=') {
                    $this->errorLine_("入出力で使える演算子は '=' のみです", $match);
                } else {
                    if ($r_param->type === Param::TYPE_PORT) {
                        $ret = $this->in_($expr, $this->line_nr_);
                    } else {
                        $ret = $this->out_($expr, $this->line_nr_);
                    }
                }
            } else {
                // $op によって命令を決める
                switch ($op) {
                    case '=': $ret = $this->ld_($expr, $this->line_nr_); break;
                    case '+=':
                        if ($r_param->isTypeMemRegValC()) {
                            $ret = $this->adc_($expr, $this->line_nr_);
                        } else {
                            $ret = $this->add_($expr, $this->line_nr_);
                        }
                        break;
                    case '-=':
                        if ($r_param->isTypeMemRegValC()) {
                            $ret = $this->sbc_($expr, $this->line_nr_);
                        } else {
                            $ret = $this->sub_($expr, $this->line_nr_);
                        }
                        break;
                    case '&=': $ret = $this->and_($expr, $this->line_nr_); break;
                    case '|=': $ret = $this->or_($expr, $this->line_nr_); break;
                    case '^=': $ret = $this->xor_($expr, $this->line_nr_); break;
                    case '>>=': $ret = $this->srl_n_($expr, $this->line_nr_); break;
                    case '<<=': $ret = $this->sla_n_($expr, $this->line_nr_); break;
                }
            }
        }

        $r_offset  += strlen($match);
        $this->line_nr_ += substr_count($match, "\n");
        //echo("assignOp2[$r_offset $this->line_nr_][$l_value][$op][$r_value]\n");

        return ($b_err) ? '' : ($this->getIndentStr_() . "$ret\n");
    }


    // MARK: parseUnaryOp_()
    /** 単項演算子 */
    private function parseUnaryOp_(string &$r_str, int &$r_offset, string $match, string $op, string $expr): string
    {
        //echo("unaryOp1[$r_offset $this->line_nr_][$op][$expr]\n");

        // $op によって命令を決める
        $ret = '';
        switch ($op) {
            case '++': $ret = $this->inc_($expr, $this->line_nr_); break;
            case '--': $ret = $this->dec_($expr, $this->line_nr_); break;
            //case '-': $ret = $this->neg_($expr, $this->line_nr_); break;
            //case '~': $ret = $this->cpl_($expr, $this->line_nr_); break;
        }

        $r_offset  += strlen($match);
        $this->line_nr_ += substr_count($match, "\n");

        //echo("unaryOp2[$r_offset $this->line_nr_][$opcode][$expr]\n");
        return $this->getIndentStr_() . "$ret\n";
    }


    // MARK: functionCall_()
    /** 関数呼び出し, またはマクロ呼び出し */
    private function functionCall_(string &$r_str, int &$r_offset, string $match, string $funcname, string $expr): string
    {
        //echo("func1[$r_offset $this->line_nr_][$funcname($expr)]\n");

        if (preg_match('/^[A-Z_][A-Z_0-9]*$/', $funcname)) {
            // 全て大文字ならばマクロとみなす
            $expr = str_replace("\n", ' ', $expr);// マクロ呼び出しは1行で書かないといけないので改行を削除します
            $ret = $funcname . ' ' . $expr;
        } else if (preg_match('/^[a-z_][a-z_0-9]*$/', $funcname) &&
            method_exists($this, $funcname . '_')) {
            // 全て小文字ならば関数のようだ
            $ret = $this->{$funcname . '_'}($expr, $this->line_nr_);
        } else {
            $this->errorLine_("この関数はありません. マクロとして呼び出す場合は, 大文字を使います", $funcname);
            $ret = '';
        }

        $r_offset  += strlen($match);
        $this->line_nr_ += substr_count($match, "\n");

        //echo("func2[$r_offset $this->line_nr_][$ret]\n");
        return $this->getIndentStr_() . "$ret\n";
    }



    // -------------------------------- ラベル
    // MARK: initLabels_()
    /** ラベルを初期化します */
    private function initLabels_(): void
    {
        $this->label_ct_ = 0;
        $this->labels_ = [];
    }


    // MARK: genRegistLabel_()
    /** ラベル名を生成し, 登録もしますます */
    private function genRegistLabel_(): string
    {
        $ret = $this->funcname_ . '__' . $this->label_ct_;
        $this->label_ct_++;
        $this->registLabel_($ret);
        return $ret;
    }


    // MARK: registLabel_()
    /** ラベル名を登録します */
    private function registLabel_(string $label): void
    {
        $this->labels_[] = $label;
    }


    // MARK: getLabels()
    /** 登録したラベル一覧を出力します */
    public function getLabels(): array
    {
        return $this->labels_;
    }

    // -------------------------------- インデント

    // MARK: initIndent_()
    /** インデントを初期化します */
    private function initIndent_(): void { $this->indent_lv_ = 0; }


    // MARK: incIndent_()
    /** インデント レベルを進めます */
    private function incIndent_(): void { $this->indent_lv_++; }


    // MARK: decIndent_()
    /** インデント レベルを戻します */
    private function decIndent_(): void { $this->indent_lv_--; }


    // MARK: getIndentStr_()
    /** インデント文字列を生成します */
    private function getIndentStr_(): string
    {
        $ret = '';
        for ($i = 0; $i < $this->indent_lv_; $i++) {
            $ret .= '  ';
        }
        return $ret;
    }

    // -------------------------------- 関数やマクロの最後のディレクティブ
    // MARK: DEF_VARS
    private function defVarsDirective_(string &$r_str, int $offset): void
    {
        // $offset より前の文字列が全て空白文字でなければエラー
        if ($offset !== 0 && ctype_space(substr($r_str, 0, $offset)) === false) {
            $this->errorLine_("Z80ANA_DEF_VARS は関数かマクロの先頭に書いてください");
        }
    }

    // MARK: ENDM
    private function endmDirective_(string &$r_str, int $offset): void
    {
        //echo('noReturnDirective(): [' . substr($r_str, $offset) . "]\n");
        if ($this->mode_ !== 'macro') {
            $this->errorLine_("Z80ANA_ENDM は, マクロ用です");
        } else {
            // $offset から後の文字列が全て空白文字かセミコロンでなければエラー
            $str = substr($r_str, $offset);
            if (preg_match('/^\s*;\s*$/', $str) === false) {
                $this->errorLine_("Z80ANA_ENDM は, マクロの一番最後に記述してください");
            }
            $this->is_ended_ = true;
        }
    }

    // MARK: NO_RETURN
    private function noReturnDirective_(string &$r_str, int $offset): void
    {
        //echo('noReturnDirective(): [' . substr($r_str, $offset) . "]\n");
        if ($this->mode_ !== 'func') {
            $this->errorLine_("Z80ANA_NO_RETURN は, 関数用です");
        } else if (!$this->is_naked_) {
            $this->errorLine_("Z80ANA_NO_RETURN は, __naked でない関数にはつけてはいけなせん");
        } else {
            // $offset から後の文字列が全て空白文字かセミコロンでなければエラー
            $str = substr($r_str, $offset);
            if (preg_match('/^\s*;\s*$/', $str) === false) {
                $this->errorLine_("Z80ANA_NO_RETURN は, 関数の一番最後に記述してください");
            }
            $this->is_ended_ = true;
        }
    }

    // MARK: FALL_THROUGH
    private function fallThroughDirective_(string &$r_str, int $offset): void
    {
        if ($this->mode_ !== 'func') {
            $this->errorLine_("Z80ANA_FALL_THROUGH は, 関数用です");
        } else if (!$this->is_naked_) {
            $this->errorLine_("Z80ANA_FALL_THROUGH は, __naked でない関数にはつけてはいけなせん");
        } else {
            // $offset から後の文字列が全て空白文字かセミコロンでなければエラー
            $str = substr($r_str, $offset);
            if (preg_match('/^\s*;\s*$/', $str) === false) {
                $this->errorLine_("Z80ANA_FALL_THROUGH は, 関数の一番最後に記述してください");
            }
            $this->is_ended_ = true;
        }
    }

    // -------------------------------- 文の解析ユーティリティ
    // ---------------- 空白文字(半角スペースと改行)
    // MARK: skipSpaces_()
    /** 空白文字をスキップします.
     * @param $r_offset $r_str の文字位置(0～)
     * - [in]  開始時
     * - [out] スキップした空白文字の次の位置. スキップしなかったら変化なし
     */
    private function skipSpaces_(string &$r_str, int &$r_offset): void
    {
        if (preg_match('/[\ \n]+/Ax', $r_str, $matches, 0, $r_offset)) {
            $r_offset  += strlen($matches[0]);
            $this->line_nr_ += substr_count($matches[0], "\n");
        }
    }


    // MARK: isBeginningOfLine_()
    /** $offset が行頭にあるかをチェック
     * - $offset より前を改行(or行頭)までサーチして, 空白文字以外の文字があればエラー
     */
    private function isBeginningOfLine_(string &$r_str, int $offset): bool
    {
        for ($off = $offset - 1; 0 <= $off; $off--) {
            switch ($r_str[$off]) {
                case ' ': break;
                case "\n":
                    return true;
                default:
                    $this->errorLine_("行頭にありません", substr($r_str, $offset));
                    return false;
            }
        }
        return true;
    }


    // MARK: toNextBeginningOfLine_()
    /** 次の行頭まで移動します.
     * - 但し 「'\' + 改行」は除く
     */
    private function toNextBeginningOfLine_(string &$r_str, int &$r_offset): void
    {
        while (true) {
            $pos = strpos($r_str, "\n", $r_offset);
            if ($pos === false) {// $r_str の末端まで行った
                $r_offset = strlen($r_str);
                return;
            }
            $r_offset = $pos + 1;
            $this->line_nr_++;
            if (1 <= $pos && $r_str[$pos - 1] == '\\') {
                continue;
            }
            break;
        }
    }

    // ---------------- 括弧内の検出
    // MARK: extractParentheses_()
    /** 空白文字に続くカッコ (...) で囲まれた中の文字列を抽出します
     * @param $r_offset $r_str の文字位置(0～)
     * - [in]  '(' の位置
     * - [out] ')' の次の位置. 抽出失敗時は strlen($r_str)
     * @param $this->line_nr_ $r_str の行番号(0～).
     * - [in]  '(' の位置
     * - [out] ')' の次の位置. 抽出失敗時は strlen($r_str)
     * @return 抽出した文字列. 抽出失敗時は false
     */
    private function extractParentheses_(string &$r_str, int &$r_offset): string|false
    {
        return $this->extractParenthesesSub_($r_str, $r_offset, '(', ')');
    }


    // MARK: extractCurlyBracket_()
    /** 空白文字に続く中カッコ {...} で囲まれた中の文字列を抽出します
     * - 引数などは, 上の extractParentheses_() を参照してください
     */
    private function extractCurlyBracket_(string &$r_str, int &$r_offset): string|false
    {
        return $this->extractParenthesesSub_($r_str, $r_offset, '{', '}');
    }


    // MARK: extractParenthesesSub_()
    /** カッコ $c0...$c1 で囲まれた中の文字列を抽出します */
    private function extractParenthesesSub_(string &$r_str, int &$r_offset, string $c0, string $c1): string|false
    {
        $lv = 1;
        $len = strlen($r_str);

        if (preg_match('/[\s\n]*\\' . $c0 . '/Ax', $r_str, $matches, 0, $r_offset)) {
            $r_offset  += strlen($matches[0]);
            $this->line_nr_ += substr_count($matches[0], "\n");
        } else {
            $this->errorLine_("'$c0' が見つかりません", substr($r_str, $r_offset));
            $this->line_nr_ += substr_count($r_str, "\n", $r_offset);
            $r_offset   = $len;
            return false;
        }

        $start = $r_offset;
        $start_line_nr = $this->line_nr_;

        for ($off = $r_offset; $off < $len; $off++) {
            switch ($r_str[$off]) {
                case "\n":
                    $this->line_nr_++;
                    break;

                case $c0:
                    $lv++;
                    break;

                case $c1:
                    $lv--;
                    if ($lv == 0) {
                        $str = substr($r_str, $r_offset, $off - $start);
                        $r_offset  = $off + 1;
                        return substr($r_str, $start, $off - $start);
                    }
            }
        }

        $this->error_->errorLine($start_line_nr, "$c0...$c1 が閉じてません", substr($r_str, $r_offset));
        $r_offset   = $len;
        return false;
    }


    // ---------------- その他文字処理
    // MARK: explodeByComma_()
    /** $expr を ',' で分割します. */
    private function explodeByComma_(string $expr): array
    {
        $ret = [];
        $lv = 0;
        $len = strlen($expr);
        $i = 0;
        $j = 0;

        for (; $i < $len; $i++) {
            switch ($expr[$i]) {
                case ',':
                    if ($lv === 0) {
                        $out[] = substr($expr, $j, $i - $j);
                        $j = $i + 1;
                    }
                    break;
                case '[':
                case '(':
                    $lv ++;
                    break;
                case ']':
                case ')':
                    $lv --;
                    break;
            }
        }

        if ($j < $i) {
            $out[] = substr($expr, $j);
        }

        //echo("[$expr] --> "); print_r($out);
        //return explode(',', $expr);
        return $out;
    }


    // MARK: cutOffCarryExpr_()
    /**  文字列の末端に ' + c' があれば, それを削除します */
    private function cutOffCarryExpr_(string $expr): string
    {
        if (preg_match('/(\+\s*c)$/x', $expr, $matches, PREG_OFFSET_CAPTURE)) {
            //echo("[$expr] --> "); print_r($matches);
            $expr = substr($expr, 0, $matches[1][1]);
            //echo("-->[$expr]\n");
        }
        return $expr;
    }


    // MARK: str2int_()
    /** 文字列から整数へ. 10/16進数対応 */
    private function str2int_($str): int
    {
        if (str_starts_with($str, '0x')) {
            return hexdec(substr($str, 2));
        }
        return (int)$str;
    }


    // ---------------- 式
    // MARK: checkExpr_()
    /** 式のチェックと改変
     * <expr> := mem[<expr1>]
     *         | mem[<expr1>] + c
     *         | port[<expr1>]
     *         | <reg> + c
     *         | <reg>
     *         | 値
     *         | 値 + c
     *         | <expr1>
     * <expr1> := 面倒なので, Parser::EXPR1_ で指定した文字種
     * @return エラーなら null
     */
    private function checkExpr_(string $expr): Param|null
    {
        // 改行コードを取る, 両端の空白文字を取る
        $expr = str_replace("\n", ' ', $expr);
        $expr = trim($expr);
        $type = Param::TYPE_VAL;

        if (preg_match('/^mem\s*\[(' . Parser::EXPR1_ . ')\]$/x', $expr, $match)) {
            $type = Param::TYPE_MEM;
            $out = "($match[1])";
        } else if (preg_match('/^port\s*\[(' . Parser::EXPR1_ . ')\]$/x', $expr, $match)) {
            $type = Param::TYPE_PORT;
            $out = "($match[1])";
        } else if (preg_match('/^(?:' . Parser::REG_ . ')$/x', $expr)) {
            $type = Param::TYPE_REG;
            $out = $expr;
        } else if (preg_match('/^(:?reg_\w*)$/x', $expr)) { // マクロ モードならば reg_ で始まる名でもレジスタ扱い
            if ($this->mode_ === 'macro') {
                $type = Param::TYPE_REG;
                $out = $expr;
            } else {
                $this->errorLine_("この名前はマクロモードでしか使えません", $expr);
                return null;
            }
        } else if (preg_match('/^(?:' . Parser::FLAG_ . ')$/x', $expr)) {
            $type = Param::TYPE_FLAG;
            $out = $expr;
        } else if (preg_match('/^mem\s*\[(' . Parser::EXPR1_ . ')\]\s*\+\s*c$/x', $expr, $match)) {
            $type = Param::TYPE_MEM_C;
            $out = "($match[1])";
        } else if (preg_match('/^(' . Parser::REG_ . ')\s*\+\s*c$/x', $expr, $match)) {
            $type = Param::TYPE_REG_C;
            $out = $match[1];
        } else if (preg_match('/^(reg_\w*)\s*\+\s*c$/x', $expr, $match)) { // マクロ モードならば reg_ で始まる名でもレジスタ扱い
            if ($this->mode_ === 'macro') {
                $type = Param::TYPE_REG_C;
                $out = $expr;
                //echo($expr . " " . $type . "\n");
            } else {
                $this->errorLine_("この名前はマクロモードでしか使えません", $expr);
                return null;
            }
        } else if (preg_match('/^(' . Parser::EXPR1_ . ')\+\s*c$/x', $expr, $match)) {
            $type = Param::TYPE_VAL_C;
            $out = $match[1];
        } else if (preg_match('/^' . Parser::SYMBOL_ . '$/x', $expr)) {
            $type = Param::TYPE_VAL;
            $out = $expr;
        } else if (preg_match('/^' . Parser::EXPR1_ . '$/x', $expr)) {
            $type = Param::TYPE_VAL;
            //$out = "0 + ($expr)";
            $out = $expr;
        } else {
            $this->errorLine_("未知の式です", $expr);
            return null;
        }
        return new Param($out, $type);
    }


    // ---------------- その他
    // MARK: swap_()
    private function swap_(&$a, &$b): void
    {
        $c = $a;
        $a = $b;
        $b = $c;
    }

    // MARK: errorLine_()
    private function errorLine_(string $msg, string $src = ''): void
    {
        $this->error_->errorLine($this->line_nr_, $msg, $src);
    }


    // MARK: command dispatches
    // -------------------------------- z80anaDirective->疑似命令ディスパッチ
    private function Z80ANA_DEF_DUMMY_VARS_(string $expr): string { return ''; }
    private function Z80ANA_LOCAL_(string $expr): string { return $this->checkParamNN_S_('local', $expr); }
    private function Z80ANA_DB_(string $expr): string { return $this->checkParamNN_V_('db', $expr); }
    private function Z80ANA_DW_(string $expr): string { return $this->checkParamNN_V_('dw', $expr); }
    private function Z80ANA_IF_(string $expr): string { $this->incIndent_(); return $this->checkParam11_('if', $expr); }
    private function Z80ANA_ELIF_(string $expr): string { return $this->checkParam11_('elif', $expr); }
    private function Z80ANA_REPT_(string $expr): string { $this->incIndent_(); return $this->checkParam11_('rept', $expr); }
    private function Z80ANA_REPTC_(string $expr): string { $this->incIndent_(); return $this->checkParam22_('reptc', $expr); }
    private function Z80ANA_REPTI_(string $expr): string { $this->incIndent_(); return $this->checkParamNN_('repti', $expr); }

    // -------------------------------- 関数->命令ディスパッチ
    private function ld_(string $expr): string { return $this->checkParam22_RM_RMV_('ld', $expr); }
    private function ldi_(string $expr): string { return $this->checkParam00_('ldi', $expr); }
    private function ldd_(string $expr): string { return $this->checkParam00_('ldd', $expr); }
    private function ldir_(string $expr): string { return $this->checkParam00_('ldir', $expr); }
    private function lddr_(string $expr): string { return $this->checkParam00_('lddr', $expr); }
    private function ex_(string $expr): string { return $this->checkParam22_RM_R_('ex', $expr); }
    private function exx_(string $expr): string { return 'exx'; }// 引数無し
    private function push_(string $expr): string { return $this->checkParam1N_R_('push', $expr); }
    private function pop_(string $expr): string { return $this->checkParam1N_R_('pop', $expr); }
    private function and_(string $expr): string { return $this->checkParam22_A_RM_('and', $expr); }
    private function or_( string $expr): string { return $this->checkParam22_A_RM_('or', $expr); }
    private function xor_(string $expr): string { return $this->checkParam22_A_RM_('xor', $expr); }
    private function cpl_(string $expr): string { return $this->checkParam01_A_('cpl', $expr); }
    private function not_(string $expr): string { return $this->cpl_($expr); }
    private function cp_(string $expr): string { return $this->checkParam22_A_RM_('cp', $expr); }
    private function cpi_(string $expr): string { return $this->checkParam00_('cpi', $expr); }
    private function cpd_(string $expr): string { return $this->checkParam00_('cpd', $expr); }
    private function cpir_(string $expr): string { return $this->checkParam00_('cpir', $expr); }
    private function cpdr_(string $expr): string { return $this->checkParam00_('cpdr', $expr); }
    private function add_(string $expr): string { return $this->checkParam22_R_RMV_('add', $expr); }
    private function adc_(string $expr): string { return $this->checkParam22_R_RMV_('adc', $expr); }
    private function sub_(string $expr): string { return $this->checkParam22_A_RM_('sub', $expr); }
    private function sbc_(string $expr): string { return $this->checkParam22_R_RMV_('sbc', $expr); }
    private function inc_(string $expr): string { return $this->checkParam11_RM_('inc', $expr); }
    private function dec_(string $expr): string { return $this->checkParam11_RM_('dec', $expr); }
    private function neg_(string $expr): string { return $this->checkParam01_A_('neg', $expr); }
    private function daa_(string $expr): string { return $this->checkParam01_A_('daa', $expr); }
    private function bit_(string $expr): string { return $this->checkParam22_V_RM_('bit', $expr); }
    private function set_(string $expr): string { return $this->checkParam22_V_RM_('set', $expr); }
    private function res_(string $expr): string { return $this->checkParam22_V_RM_('res', $expr); }
    private function bit_3_(string $expr): string { return $this->checkParam33_V_M_R_('bit', $expr); }
    private function set_3_(string $expr): string { return $this->checkParam33_V_M_R_('set', $expr); }
    private function res_3_(string $expr): string { return $this->checkParam33_V_M_R_('res', $expr); }
    private function sla_(string $expr): string { return $this->checkParam11_RM_('sla', $expr); }
    private function sll_(string $expr): string { return $this->checkParam11_RM_('sll', $expr); }
    private function sra_(string $expr): string { return $this->checkParam11_RM_('sra', $expr); }
    private function srl_(string $expr): string { return $this->checkParam11_RM_('srl', $expr); }
    private function rl_( string $expr): string { return $this->checkParam11_RM_('rl', $expr); }
    private function rr_( string $expr): string { return $this->checkParam11_RM_('rr', $expr); }
    private function rlc_(string $expr): string { return $this->checkParam11_RM_('rlc', $expr); }
    private function rrc_(string $expr): string { return $this->checkParam11_RM_('rrc', $expr); }
    private function sla_n_(string $expr): string { return $this->checkParam12_RM_V_('sla', $expr); }
    private function sll_n_(string $expr): string { return $this->checkParam12_RM_V_('sll', $expr); }
    private function sra_n_(string $expr): string { return $this->checkParam12_RM_V_('sra', $expr); }
    private function srl_n_(string $expr): string { return $this->checkParam12_RM_V_('srl', $expr); }
    private function rl_n_( string $expr): string { return $this->checkParam12_RM_V_('rl', $expr); }
    private function rr_n_( string $expr): string { return $this->checkParam12_RM_V_('rr', $expr); }
    private function rlc_n_(string $expr): string { return $this->checkParam12_RM_V_('rlc', $expr); }
    private function rrc_n_(string $expr): string { return $this->checkParam12_RM_V_('rrc', $expr); }
    private function sla_2_(string $expr): string { return $this->checkParam22_M_R_('sla', $expr); }
    private function sll_2_(string $expr): string { return $this->checkParam22_M_R_('sll', $expr); }
    private function sra_2_(string $expr): string { return $this->checkParam22_M_R_('sra', $expr); }
    private function srl_2_(string $expr): string { return $this->checkParam22_M_R_('srl', $expr); }
    private function rl_2_( string $expr): string { return $this->checkParam22_M_R_('rl', $expr); }
    private function rr_2_( string $expr): string { return $this->checkParam22_M_R_('rr', $expr); }
    private function rlc_2_(string $expr): string { return $this->checkParam22_M_R_('rlc', $expr); }
    private function rrc_2_(string $expr): string { return $this->checkParam22_M_R_('rrc', $expr); }
    private function rla_ (string $expr): string { return $this->checkParam01_A_('rla', $expr); }
    private function rra_ (string $expr): string { return $this->checkParam01_A_('rra', $expr); }
    private function rlca_(string $expr): string { return $this->checkParam01_A_('rlca', $expr); }
    private function rrca_(string $expr): string { return $this->checkParam01_A_('rrca', $expr); }
    private function rla_n_ (string $expr): string { return $this->checkParam02_AV_('rla', $expr); }
    private function rra_n_ (string $expr): string { return $this->checkParam02_AV_('rra', $expr); }
    private function rlca_n_(string $expr): string { return $this->checkParam02_AV_('rlca', $expr); }
    private function rrca_n_(string $expr): string { return $this->checkParam02_AV_('rrca', $expr); }
    private function rld_(string $expr): string { return $this->checkParam20_A_M_('rld', $expr); }
    private function rrd_(string $expr): string { return $this->checkParam20_A_M_('rrd', $expr); }
    private function jp_(string $expr): string { return $this->checkParam11_V_HL_('jp', $expr); }
    private function jp_z_(string $expr): string { return $this->checkParam11_V_HL_('jp z,', $expr); }
    private function jp_eq_(string $expr): string { return $this->jp_z_($expr); }
    private function jp_nz_(string $expr): string { return $this->checkParam11_V_HL_('jp nz,', $expr); }
    private function jp_ne_(string $expr): string { return $this->jp_nz_($expr); }
    private function jp_c_(string $expr): string { return $this->checkParam11_V_HL_('jp c,', $expr); }
    private function jp_lt_(string $expr): string { return $this->jp_c_($expr); }
    private function jp_nc_(string $expr): string { return $this->checkParam11_V_HL_('jp nc,', $expr); }
    private function jp_ge_(string $expr): string { return $this->jp_nc_($expr); }
    private function jp_p_(string $expr): string { return $this->checkParam11_V_HL_('jp p,', $expr); }
    private function jp_m_(string $expr): string { return $this->checkParam11_V_HL_('jp m,', $expr); }
    private function jp_v_(string $expr): string { return $this->checkParam11_V_HL_('jp v,', $expr); }
    private function jp_nv_(string $expr): string { return $this->checkParam11_V_HL_('jp nv,', $expr); }
    private function jp_pe_(string $expr): string { return $this->checkParam11_V_HL_('jp pe,', $expr); }
    private function jp_po_(string $expr): string { return $this->checkParam11_V_HL_('jp po,', $expr); }
    private function jr_(string $expr): string { return $this->checkParam11_V_('jr', $expr); }
    private function jr_z_(string $expr): string { return $this->checkParam11_V_('jr z,', $expr); }
    private function jr_eq_(string $expr): string { return $this->jr_z_($expr); }
    private function jr_nz_(string $expr): string { return $this->checkParam11_V_('jr nz,', $expr); }
    private function jr_ne_(string $expr): string { return $this->jr_nz_($expr); }
    private function jr_c_(string $expr): string { return $this->checkParam11_V_('jr c,', $expr); }
    private function jr_lt_(string $expr): string { return $this->jr_c_($expr); }
    private function jr_nc_(string $expr): string { return $this->checkParam11_V_('jr nc,', $expr); }
    private function jr_ge_(string $expr): string { return $this->jr_nc_($expr); }
    private function djnz_(string $expr): string { return $this->checkParam22_B_V_('djnz', $expr); }
    private function call_(string $expr): string { return $this->checkParam11_V_('call', $expr); }
    private function call_z_(string $expr): string { return $this->checkParam11_V_('call z,', $expr); }
    private function call_eq_(string $expr): string { return $this->call_z_($expr); }
    private function call_nz_(string $expr): string { return $this->checkParam11_V_('call nz,', $expr); }
    private function call_ne_(string $expr): string { return $this->call_nz_($expr); }
    private function call_c_(string $expr): string { return $this->checkParam11_V_('call c,', $expr); }
    private function call_lt_(string $expr): string { return $this->call_c_($expr); }
    private function call_nc_(string $expr): string { return $this->checkParam11_V_('call nc,', $expr); }
    private function call_ge_(string $expr): string { return $this->call_nc_($expr); }
    private function call_p_(string $expr): string { return $this->checkParam11_V_('call p,', $expr); }
    private function call_m_(string $expr): string { return $this->checkParam11_V_('call m,', $expr); }
    private function call_v_(string $expr): string { return $this->checkParam11_V_('call v,', $expr); }
    private function call_nv_(string $expr): string { return $this->checkParam11_V_('call nv,', $expr); }
    private function call_pe_(string $expr): string { return $this->checkParam11_V_('call pe,', $expr); }
    private function call_po_(string $expr): string { return $this->checkParam11_V_('call po,', $expr); }
    private function rst_(string $expr): string { return $this->checkParam11_V_('rst', $expr); }
    private function ret_(string $expr): string { return $this->checkParam00_('ret', $expr); }
    private function ret_z_(string $expr): string { return $this->checkParam00_('ret z', $expr); }
    private function ret_eq_(string $expr): string { return $this->ret_z_($expr); }
    private function ret_nz_(string $expr): string { return $this->checkParam00_('ret nz', $expr); }
    private function ret_ne_(string $expr): string { return $this->ret_nz_($expr); }
    private function ret_c_(string $expr): string { return $this->checkParam00_('ret c', $expr); }
    private function ret_lt_(string $expr): string { return $this->ret_c_($expr); }
    private function ret_nc_(string $expr): string { return $this->checkParam00_('ret nc', $expr); }
    private function ret_ge_(string $expr): string { return $this->ret_ge_($expr); }
    private function ret_p_(string $expr): string { return $this->checkParam00_('ret p', $expr); }
    private function ret_m_(string $expr): string { return $this->checkParam00_('ret m', $expr); }
    private function ret_v_(string $expr): string { return $this->checkParam00_('ret v', $expr); }
    private function ret_nv_(string $expr): string { return $this->checkParam00_('ret nv', $expr); }
    private function ret_pe_(string $expr): string { return $this->checkParam00_('ret pe', $expr); }
    private function ret_po_(string $expr): string { return $this->checkParam00_('ret po', $expr); }
    private function reti_(string $expr): string { return $this->checkParam00_('reti', $expr); }
    private function retn_(string $expr): string { return $this->checkParam00_('retn', $expr); }
    private function nop_(string $expr): string { return $this->checkParam00_('nop', $expr); }
    private function halt_(string $expr): string { return $this->checkParam00_('halt', $expr); }
    private function di_(string $expr): string { return $this->checkParam00_('di', $expr); }
    private function ei_(string $expr): string { return $this->checkParam00_('ei', $expr); }
    private function im0_(string $expr): string { return $this->checkParam00_('im 0', $expr); }
    private function im1_(string $expr): string { return $this->checkParam00_('im 1', $expr); }
    private function im2_(string $expr): string { return $this->checkParam00_('im 2', $expr); }
    private function scf_(string $expr): string { return $this->checkParam01_c_('scf', $expr); }
    private function ccf_(string $expr): string { return $this->checkParam01_c_('ccf', $expr); }
    private function in_(string $expr): string { return $this->checkParam22_R_P_('in', $expr); }
    private function out_(string $expr): string { return $this->checkParam22_P_RV_('out', $expr); }
    private function ini_(string $expr): string { return $this->checkParam00_('ini', $expr); }
    private function ind_(string $expr): string { return $this->checkParam00_('ind', $expr); }
    private function inir_(string $expr): string { return $this->checkParam00_('inir', $expr); }
    private function indr_(string $expr): string { return $this->checkParam00_('indr', $expr); }
    private function outi_(string $expr): string { return $this->checkParam00_('outi', $expr); }
    private function outd_(string $expr): string { return $this->checkParam00_('outd', $expr); }
    private function otir_(string $expr): string { return $this->checkParam00_('otir', $expr); }
    private function otdr_(string $expr): string { return $this->checkParam00_('otdr', $expr); }


    // MARK: checkParamSub_()
    /** expr を n 分割し, それぞれを更にパラメータとタイプに分割します */
    private function checkParamSub_(string $opcode, string $expr, int $n): array|null
    {
        $params = $this->explodeByComma_($expr);
        if (count($params) != $n) {
            //echo("------"); print_r($expr); echo("------>"); print_r($params);
            $this->errorLine_("$opcode の引数は $n 個です", $expr);
            return null;
        }

        $ret = [];
        foreach($params as $i => $param) {
            $p = $this->checkExpr_($param);
            if ($p === null) {
                $this->errorLine_("$opcode 引数 ". ($i + 1) . " のエラー", $param);
            } else {
                $ret[] = $p;
            }
        }
        if (count($ret) !== $n) { return null; }
        return $ret;
    }


    // MARK: checkParam00_()
    /** 命令, 関数共に引数ありません */
    private function checkParam00_(string $opcode, string $expr): string
    {
        if ($expr !== '') {
            $this->errorLine_("$opcode は引数不要です", $expr);
        }
        return $opcode;
    }


    // MARK: checkParam01_A_()
    /** 命令は引数無しですが, 関数は 1 つの引数 ('A') を持ちます. 例: rra(A) -> rra  */
    private function checkParam01_A_(string $opcode, string $expr): string
    {
        if ($expr !== 'A') {
            $this->errorLine_("$opcode で使用できる引数は, A のみです", $expr);
        }
        return $opcode;
    }


    // MARK: checkParam01_c_()
    /** 命令は引数無しですが, 関数は 1つのの引数 ('c') を持ちます  */
    private function checkParam01_c_(string $opcode, string $expr): string
    {
        if ($expr !== 'c') {
            $this->errorLine_("$opcode で使用できる引数は, c のみです", $expr);
        }
        return $opcode;
    }


    // MARK: checkParam02_AV_()
    /** 命令は引数無しですが, 関数は 2 つの引数 ('A', 値) を持ちます. 例: rrca(a, n) -> rrca  */
    private function checkParam02_AV_(string $opcode, string $expr): string
    {
        $p = $this->checkParamSub_($opcode, $expr, 2);
        if ($p === null) { return ''; }

        if ($p[0]->value !== 'A') {
            $this->errorLine_("$opcode の第1引数で使用できるのは, A のみです", $expr);
            return '';
        }
        if ($p[1]->type !== Param::TYPE_VAL) {
            $this->errorLine_("$opcode の第2引数で使用できるのは, 値 のみです", $expr);
            return '';
        }

        // 値が数値ならば直接展開(4まで), そうでなければ rept
        $out = '';
        //echo($p[1]->value . "\n");
        if (preg_match('/^0\ \+\ \(((:?0x[0-9a-fA-F]+)|(:?[0-9]+))\)$/x', $p[1]->value, $matches)) {
            //print_r($matches);
            if (4 < $matches[1]) {
                $this->errorLine_("$opcode の第2引数値が大きすぎます", $expr);
            } else {
                $n = $this->str2int_($matches[1]);
                for ($i = 0; $i < $n; $i++) {
                    if ($i != 0) { $out .= $this->getIndentStr_(); }
                    $out .= $opcode;
                    if ($i != $n - 1) { $out .= "\n"; }
                }
            }
        } else {
            $out .= "rept " . $p[1]->value . "\n";
            $this->incIndent_();
            $out .= $this->getIndentStr_() . "$opcode\n";
            $this->decIndent_();
            $out .= $this->getIndentStr_() . "endr";
        }
        return $out;
    }


    // MARK: checkParam11_()
    /** 命令, 関数共に 1つの引数 (タイプはなんでもいい) を持ちます  */
    private function checkParam11_(string $opcode, string $expr): string
    {
        $params = $this->explodeByComma_($expr);
        if (count($params) != 1) {
            $this->errorLine_("$opcode の引数は 1 個です", $params[0]);
            return '';
        }

        return "$opcode " . $params[0];
    }


    // MARK: checkParam11_RM_()
    /** 命令, 関数共に 1つの引数 (レジスタ|メモリ) を持ちます  */
    private function checkParam11_RM_(string $opcode, string $expr): string
    {
        $p = $this->checkParamSub_($opcode, $expr, 1);
        if ($p === null) { return ''; }

        if ($p[0]->type !== Param::TYPE_REG && $p[0]->type !== Param::TYPE_MEM) {
            $this->errorLine_("$opcode の引数は, レジスタ|メモリ のみです", $expr);
            return '';
        }

        if ($p[0]->type !== Param::TYPE_REG || array_key_exists($p[0]->value, Parser::REGS_G16_SPLIT8_TAB_) === false) {
            return "$opcode " . $p[0]->value;
        }
        // 例外:次の命令は, レジスタが16bitの時(HL,DE,BC,IX,IY)は, 複数の命令に分解されます:
        // sla(HL) -> sla L, rl H
        // sll(HL) -> sll L, rl H
        // sra(HL) -> sra H, rr L
        // srl(HL) -> srl H, rr L
        // rla(HL) -> rla H, rla L
        // rra(HL) -> rra H, rra L
        $reg_l = Parser::REGS_G16_SPLIT8_TAB_[$p[0]->value][0];
        $reg_h = Parser::REGS_G16_SPLIT8_TAB_[$p[0]->value][1];
        $out = '';
        switch ($opcode) {
            default:
                $out = "$opcode " . $p[0]->value;
                break;
            case 'sla':
            case 'sll':
                $out = "$opcode " . $reg_l . "\n";
                $this->incIndent_();
                $out .= "rl " . $reg_h;
                break;
            case 'sra':
            case 'srl':
                $out = "$opcode " . $reg_h . "\n";
                $this->incIndent_();
                $out .= "rr " . $reg_l;
                break;
            case 'rla':
                $out = "$opcode " . $reg_l . "\n";
                $this->incIndent_();
                $out .= "$opcode " . $reg_h . "\n";
                break;
            case 'rra':
                $out = "$opcode " . $reg_h . "\n";
                $this->incIndent_();
                $out .= "$opcode " . $reg_l . "\n";
                break;
        }
        return $out;
    }


    // MARK: checkParam11_V_HL_()
    /** 命令, 関数共に 1つの引数 (値|HL|IX|IY) を持ちます  */
    private function checkParam11_V_HL_(string $opcode, string $expr): string
    {
        $p = $this->checkParamSub_($opcode, $expr, 1);
        if ($p === null) { return ''; }
        if (($p[0]->type !== Param::TYPE_VAL && $p[0]->type !== Param::TYPE_REG) ||
            ($p[0]->type === Param::TYPE_REG &&
                ($p[0]->value !== 'HL' && $p[0]->value !== 'IX' && $p[0]->value !== 'IY'))) {
            $this->errorLine_("$opcode の引数は, 値|HL|IX|IY のみです", $expr);
            return '';
        }

        $p[0]->adjustVal();

        if ($p[0]->type === Param::TYPE_REG) {
            $p[0]->value = '(' . $p[0]->value . ')';
        }

        return "$opcode " . $p[0]->value;
    }


    // MARK: checkParam11_V_()
    /** 命令, 関数共に 1つの引数 (値) を持ちます  */
    private function checkParam11_V_(string $opcode, string $expr): string
    {
        $p = $this->checkParamSub_($opcode, $expr, 1);
        if ($p === null) { return ''; }

        if ($p[0]->type !== Param::TYPE_VAL) {
            $this->errorLine_("$opcode の引数は, 値 のみです", $expr);
            return '';
        }
        $p[0]->adjustVal();

        return "$opcode " . $p[0]->value;
    }


    // MARK: checkParam11_F_()
    /** 命令, 関数共に 1つの引数 (フラグ) を持ちます  */
    private function checkParam11_F_(string $opcode, string $expr): string
    {
        $p = $this->checkParamSub_($opcode, $expr, 1);
        if ($p === null) { return ''; }

        if ($p[0]->type !== Param::TYPE_FLAG) {
            $this->errorLine_("$opcode の引数は, フラグ のみです", $expr);
        }

        return "$opcode " . $p[0]->value;
    }


    // MARK: checkParam12_RM_V_()
    /** 命令は1つ(レジスタ|メモリ), 関数は2つの引数 (値) を持ちます */
    private function checkParam12_RM_V_(string $opcode, string $expr): string
    {
        $p = $this->checkParamSub_($opcode, $expr, 2);
        if ($p === null) { return ''; }

        if ($p[0]->type !== Param::TYPE_REG && $p[0]->type !== Param::TYPE_MEM) {
            $this->errorLine_("$opcode の第1引数で使用できるのは, レジスタ|メモリ のみです", $expr);
        }
        if ($p[1]->type !== Param::TYPE_VAL) {
            $this->errorLine_("$opcode の第2引数で使用できるのは, 値 のみです", $expr);
        }

        // 値が数値ならば直接展開(4まで), そうでなければ rept
        $out = '';
        //echo($p[1]->value . "\n");
        if (preg_match('/^0\ \+\ \(((:?0x[0-9a-fA-F]+)|(:?[0-9]+))\)$/x', $p[1]->value, $matches)) {
            //print_r($matches);
            if (4 < $matches[1]) {
                $this->errorLine_("$opcode の第2引数値が大きすぎます", $expr);
            } else {
                $n = $this->str2int_($matches[1]);
                for ($i = 0; $i < $n; $i++) {
                    if ($i != 0) { $out .= $this->getIndentStr_(); }
                    $out .= "$opcode " . $p[0]->value;
                    if ($i != $n - 1) { $out .= "\n"; }
                }
            }
        } else {
            $out .= "rept " . $p[1]->value . "\n";
            $this->incIndent_();
            $out .= $this->getIndentStr_() . "$opcode " . $p[0]->value . "\n";
            $this->decIndent_();
            $out .= $this->getIndentStr_() . "endr";
        }
        return $out;
    }

    // MARK: checkParam22_()
    /** 命令, 関数共に 2つの引数 (タイプはなんでもいい) を持ちます  */
    private function checkParam22_(string $opcode, string $expr): string
    {
        $params = $this->explodeByComma_($expr);
        if (count($params) != 2) {
            $this->errorLine_("$opcode の引数は 2 個です", $expr);
            return '';
        }

        return "$opcode " . $expr;
    }

    // MARK: checkParam22_RM_RMV_()
    /** 命令, 関数共に 2つの引数 (レジスタ|メモリ, レジスタ|メモリ|値) を持ちます */
    private function checkParam22_RM_RMV_(string $opcode, string $expr): string
    {
        $p = $this->checkParamSub_($opcode, $expr, 2);
        if ($p === null) { return ''; }
        if ($p[0]->type !== Param::TYPE_REG && $p[0]->type !== Param::TYPE_MEM) {
            $this->errorLine_("$opcode の第1引数で使用できるのは, レジスタ|メモリ のみです", $p[0]->value);
        }
        if ($p[1]->type !== Param::TYPE_REG && $p[1]->type !== Param::TYPE_MEM && $p[1]->type !== Param::TYPE_VAL) {
            $this->errorLine_("$opcode の第2引数で使用できるのは, レジスタ|メモリ|値 のみです", $p[1]->value);
        }
        $p[1]->adjustVal();

        return "$opcode " . $p[0]->value . ', ' .  $p[1]->value;
    }


    // MARK: checkParam22_R_P_()
    /** 命令, 関数共に 2つの引数 (レジスタ, ポート) を持ちます */
    private function checkParam22_R_P_(string $opcode, string $expr): string
    {
        $p = $this->checkParamSub_($opcode, $expr, 2);
        if ($p === null) { return ''; }

        if ($p[0]->type !== Param::TYPE_REG) {
            $this->errorLine_("$opcode の第1引数で使用できるのは, レジスタ のみです", $p[0]->value);
        }
        if ($p[1]->type !== Param::TYPE_PORT) {
            $this->errorLine_("$opcode の第2引数で使用できるのは, ポート のみです", $p[1]->value);
        }

        return "$opcode " . $p[0]->value . ', ' .  $p[1]->value;
    }


    // MARK: checkParam22_P_RV_()
    /** 命令, 関数共に 2つの引数 (ポート, レジスタ|値) を持ちます */
    private function checkParam22_P_RV_(string $opcode, string $expr): string
    {
        $p = $this->checkParamSub_($opcode, $expr, 2);
        if ($p === null) { return ''; }

        if ($p[0]->type !== Param::TYPE_PORT) {
            $this->errorLine_("$opcode の第1引数で使用できるのは, ポート のみです", $p[0]->value);
        }
        if ($p[1]->type !== Param::TYPE_REG && $p[1]->type !== Param::TYPE_VAL) {
            $this->errorLine_("$opcode の第2引数で使用できるのは, レジスタ|値 のみです", $p[1]->value);
        }

        $p[1]->adjustVal();

        return "$opcode " . $p[0]->value . ', ' .  $p[1]->value;
    }

    // MARK: checkParam22_A_RM_()
    /** 命令, 関数共に 2つの引数 ('A', 二番目はレジスタ|メモリ|値) を持ちます */
    private function checkParam22_A_RM_(string $opcode, string $expr): string
    {
        $p = $this->checkParamSub_($opcode, $expr, 2);
        if ($p === null) { return ''; }

        if ($p[0]->value !== 'A') {
            $this->errorLine_("$opcode の第1引数で使用できるのは, 'A' のみです", $p[0]->value);
        }
        if ($p[1]->type !== Param::TYPE_REG && $p[1]->type !== Param::TYPE_MEM && $p[1]->type !== Param::TYPE_VAL) {
            $this->errorLine_("$opcode の第2引数で使用できるのは, レジスタ|メモリ|値 のみです", $p[1]->value);
        }

        $p[1]->adjustVal();

        return "$opcode " . $p[0]->value . ', ' .  $p[1]->value;
    }


    // MARK: checkParam22_R_RMV_()
    /** 命令, 関数共に 2つの引数 (レジスタ, レジスタ|メモリ|値) を持ちます */
    private function checkParam22_R_RMV_(string $opcode, string $expr): string
    {
        $p = $this->checkParamSub_($opcode, $expr, 2);
        if ($p === null) { return ''; }

        if ($p[0]->type !== 'reg') {
            $this->errorLine_("$opcode の第1引数で使用できるのは, レジスタのみです", $p[0]->value);
        }
        if ($p[1]->type !== Param::TYPE_REG && $p[1]->type !== Param::TYPE_MEM && $p[1]->type !== Param::TYPE_VAL) {
            $this->errorLine_("$opcode の第2引数で使用できるのは, レジスタ|メモリ|値 のみです", $p[1]->value);
        }

        $p[1]->adjustVal();

        return "$opcode " . $p[0]->value . ', ' .  $p[1]->value;
    }


    // MARK: checkParam22_V_RM_()
    /** 命令, 関数共に 2つの引数 (値, レジスタ|メモリ) を持ちます */
    private function checkParam22_V_RM_(string $opcode, string $expr): string
    {
        $p = $this->checkParamSub_($opcode, $expr, 2);
        if ($p === null) { return ''; }

        if ($p[0]->type !== Param::TYPE_VAL) {
            $this->errorLine_("$opcode の第1引数で使用できるのは, 値 のみです", $p[0]->value);
        }
        if ($p[1]->type !== Param::TYPE_REG && $p[1]->type !== Param::TYPE_MEM) {
            $this->errorLine_("$opcode の第2引数で使用できるのは, レジスタ|メモリ のみです", $p[1]->value);
        }

        return "$opcode " . $p[0]->value . ', ' .  $p[1]->value;
    }


    // MARK: checkParam22_RM_R_()
    /** 命令, 関数共に 2つの引数 (レジスタ|メモリ, レジスタ) を持ちます */
    private function checkParam22_RM_R_(string $opcode, string $expr): string
    {
        $p = $this->checkParamSub_($opcode, $expr, 2);
        if ($p === null) { return ''; }

        if ($p[0]->type !== Param::TYPE_REG && $p[0]->type !== Param::TYPE_MEM) {
            $this->errorLine_("$opcode の第1引数で使用できるのは, レジスタ|メモリ のみです", $p[0]->value);
        }
        if ($p[1]->type !== Param::TYPE_REG ) {
            $this->errorLine_("$opcode の第2引数で使用できるのは, レジスタ のみです", $p[1]->value);
        }

        return "$opcode " . $p[0]->value . ', ' .  $p[1]->value;
    }


    // MARK: checkParam20_A_M_()
    /** 命令, 関数共に 2つの引数 ('A', メモリ) を持ちます */
    private function checkParam20_A_M_(string $opcode, string $expr): string
    {
        $p = $this->checkParamSub_($opcode, $expr, 2);
        if ($p === null) { return ''; }

        if ($p[0]->value !== 'A') {
            $this->errorLine_("$opcode の第1引数で使用できるのは, 'A' のみです", $p[0]->value);
        }
        if ($p[1]->type !== Param::TYPE_MEM ) {
            $this->errorLine_("$opcode の第2引数で使用できるのは, メモリ のみです", $p[1]->value);
        }

        return "$opcode";
    }


    // MARK: checkParam22_B_V_()
    /** 命令, 関数共に 2つの引数 ('B', 値) を持ちます */
    private function checkParam22_B_V_(string $opcode, string $expr): string
    {
        $p = $this->checkParamSub_($opcode, $expr, 2);
        if ($p === null) { return ''; }

        if ($p[0]->value !== 'B') {
            $this->errorLine_("$opcode の第1引数で使用できるのは, 'B' のみです", $p[0]->value);
        }
        if ($p[1]->type !== Param::TYPE_VAL) {
            $this->errorLine_("$opcode の第2引数で使用できるのは, 値 のみです", $p[1]->value);
        }

        $p[1]->adjustVal();

        return "$opcode " . $p[0]->value . ', ' .  $p[1]->value;
    }


    // MARK: checkParam22_F_V_()
    /** 命令, 関数共に 2つの引数 (フラグ, 値) を持ちます */
    private function checkParam22_F_V_(string $opcode, string $expr): string
    {
        $p = $this->checkParamSub_($opcode, $expr, 2);
        if ($p === null) { return ''; }

        if ($p[0]->type !== Param::TYPE_FLAG) {
            $this->errorLine_("$opcode の第1引数で使用できるのは, フラグ のみです", $p[0]->value);
        }
        if ($p[1]->type !== Param::TYPE_VAL) {
            $this->errorLine_("$opcode の第2引数で使用できるのは, 値 のみです", $p[1]->value);
        }

        $p[1]->adjustVal();

        return "$opcode " . $p[0]->value . ', ' .  $p[1]->value;
    }


    // MARK: checkParam22_M_R_()
    /** 命令, 関数共に 2つの引数 (メモリ, レジスタ) を持ちます */
    private function checkParam22_M_R_(string $opcode, string $expr): string
    {
        $p = $this->checkParamSub_($opcode, $expr, 2);
        if ($p === null) { return ''; }

        if ($p[0]->type !== Param::TYPE_MEM) {
            $this->errorLine_("$opcode の第2引数で使用できるのは, メモリ のみです", $p[0]->value);
        }
        if ($p[1]->type !== Param::TYPE_REG) {
            $this->errorLine_("$opcode の第3引数で使用できるのは, レジスタ のみです", $p[1]->value);
        }

        return "$opcode " . $p[0]->value . ', ' .  $p[1]->value;
    }


    // MARK: checkParam33_V_M_R_()
    /** 命令, 関数共に 3つの引数 (値, メモリ, レジスタ) を持ちます */
    private function checkParam33_V_M_R_(string $opcode, string $expr): string
    {
        $p = $this->checkParamSub_($opcode, $expr, 3);
        if ($p === null) { return ''; }

        if ($p[0]->type !== Param::TYPE_VAL) {
            $this->errorLine_("$opcode の第1引数で使用できるのは, 値 のみです", $p[0]->value);
        }
        if ($p[1]->type !== Param::TYPE_MEM) {
            $this->errorLine_("$opcode の第2引数で使用できるのは, メモリ のみです", $p[1]->value);
        }
        if ($p[2]->type !== Param::TYPE_REG) {
            $this->errorLine_("$opcode の第3引数で使用できるのは, レジスタ のみです", $p[2]->value);
        }

        $p[0]->adjustVal();

        return "$opcode " . $p[0]->value . ', ' .  $p[1]->value . ', ' .  $p[2]->value;
    }

    // MARK: checkParam1N_R_()
    /** 命令は引数1. 関数は1～n個の引数 (全てレジスタ) を持ちます  */
    private function checkParam1N_R_(string $opcode, string $expr): string
    {
        $params = $this->explodeByComma_($expr);
        $b_err = false;
        $out = '';

        foreach($params as $i => $param) {
            $p = $this->checkExpr_($param);
            if ($p === null || $p->type !== Param::TYPE_REG) {
                $this->errorLine_("$opcode で使用できる引数は, レジスタ のみです", $p->value);
                $b_err = true;
            } else {
                if ($i !== 0) { $out .= $this->getIndentStr_(); }
                $out .= "$opcode " . $p->value;
                if ($i !== count($params) - 1) { $out .= "\n"; }
            }
        }
        return $b_err ? '' : $out;
    }


    // MARK: checkParamNN_()
    /** 命令は引数n. 関数もn個の引数 (タイプはなんでもいい) を持ちます. nは2以上  */
    private function checkParamNN_(string $opcode, string $expr): string
    {
        $params = $this->explodeByComma_($expr);
        if (count($params) < 2) {
            $this->errorLine_("$opcode の引数は 2 個以上です", $expr);
            return '';
        }

        return "$opcode " . $expr;
    }


    // MARK: checkParamNN_S_()
    /** 命令は引数n. 関数もn個の引数 (全てシンボル値) を持ちます  */
    private function checkParamNN_S_(string $opcode, string $expr): string
    {
        $params = $this->explodeByComma_($expr);
        $b_err = false;
        $out = $opcode . ' ';

        foreach($params as $i => $param) {
            $p = $this->checkExpr_($param);
            if ($p === null || !preg_match('/^' . Parser::SYMBOL_ . '$/x', $p->value)) {
                $this->errorLine_("$opcode で使用できる引数は, シンボル値 のみです", $p->value);
                $b_err = true;
            } else {
                $out .= $p->value;
                if ($i !== count($params) - 1) { $out .= ", "; }
            }
        }
        return $b_err ? '' : $out;
    }


    // MARK: checkParamNN_V_()
    /** 命令は引数n. 関数もn個の引数 (全て値) を持ちます  */
    private function checkParamNN_V_(string $opcode, string $expr): string
    {
        $params = $this->explodeByComma_($expr);
        $b_err = false;
        $out = $opcode . ' ';

        foreach($params as $i => $param) {
            $p = $this->checkExpr_($param);
            if ($p === null || $p->type !== Param::TYPE_VAL) {
                $this->errorLine_("$opcode で使用できる引数は, 値 のみです", $p->value ?? '');
                $b_err = true;
            } else {
                $out .= $p->value;
                if ($i !== count($params) - 1) { $out .= ", "; }
            }
        }
        return $b_err ? '' : $out;
    }
}
