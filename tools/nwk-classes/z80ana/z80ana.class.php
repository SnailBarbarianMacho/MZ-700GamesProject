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
    private string $mode_;
    private int    $line_nr_;
    private string $funcname_;

    private int    $label_ct_;
    private array  $labels_;

    private int    $indent_lv_;

    private \nwk\utils\Error $error_;

    private const EXPR0_ = '[\w\+\-\*\/\%\&\|\^\~\(\)\<\>\,\.\ \n\[\]]*';
    private const EXPR_  = '[\w\+\-\*\/\%\&\|\^\~\(\)\<\>\,\.\ \n\[\]]+'; // A-Za-z0-9_+-*%/()<>,.[] くらい?
    private const EXPR1_ = '[\w\+\-\*\/\%\&\|\^\~\(\)\<\>\,\.\ \n]+';     // A-Za-z0-9_+-*%/()<>,.   くらい?
    private const SYMBOL_= '[A-Za-z_]\w*';
    private const REG_   = 'IXH|IXL|IYH|IYL|HL|DE|BC|AF|PC|SP|IX|IY|XH|XL|YH|YL|A|B|C|D|E|F|H|L|I|R';
    private const FLAG_      = 'nz|po|pe|nv|nc|z|p|m|v|c';
    private const FLAG_REL_  = 'nz_r|nc_r|z_r|c_r';
    private const FLAG_NEG_TAB_ = array(
        ''   => 'false',
        'nz' => 'z',
        'nc' => 'c',
        'nv' => 'v',
        'z'  => 'nz',
        'c'  => 'nc',
        'v'  => 'nv',
        'p'  => 'm',
        'm'  => 'p',
        'po' => 'pe',
        'pe' => 'po',
        'nz_r' => 'z_r',
        'nc_r' => 'c_r',
        'z_r'  => 'nz_r',
        'c_r'  => 'nc_r',
    );


    // MARK: __construct()
    /** コンストラクタ */
    public function __construct(\nwk\utils\Error $error) {
        $this->error_ = $error;
    }


    // -------------------------------- パース
    // MARK: parse()
    /** パースして変換後の文字列を返します
     * @param $mode     モード 'macro', 'func' のどちらか
     * @param $r_str    解析したい文字列
     * @param $line_nr  $str のある行(0～, エラー表示用)
     * @param $funcname 関数名(ラベル表示用)
     */
    public function parse(string $mode, string &$r_str, int $line_nr, string $funcname): string
    {
        if ($mode == 'macro' && !preg_match('/^[A-Z_][A-Z_0-9]+$/', $funcname)) {
            $this->error_->errorLine($line_nr, "マクロ名は, [A-Z_0-9]です", $funcname);
        }

        $this->mode_     = $mode;
        $this->funcname_ = $funcname;
        $this->initLabels_();
        $this->initIndent_();
        return $this->parseSentences_($r_str, $line_nr);
    }


    // -------------------------------- パース サブ関数
    // MARK: parseSentences_()
    /** 複文の解析 */
    private function parseSentences_(string &$r_str, int &$r_line_nr)
    {
        $offset = 0;            // $r_str の位置
        $len = strlen($r_str);
        $out = '';
        $old_line_nr = -1;

        $this->incIndent_();
        while ($offset < $len) {
            $line_str = '';
            if ($old_line_nr !== $r_line_nr) {
                $old_line_nr = $r_line_nr;
                $line_str = "// line " . ($r_line_nr + 1) . "\n";
            }

            $str = $this->parseSentence_($r_str, $offset, $r_line_nr);
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
    private function parseSentence_(string &$r_str, int &$r_offset, int &$r_line_nr): string
    {
        // "\n", ' ', ';'
        if (preg_match('/([\s\;]+)/Ax', $r_str, $matches, 0, $r_offset)) {
            return $this->parseWhitespaces_($r_str, $r_offset, $r_line_nr, $matches[1]);
        }

        // 'Z80ANA_' で始まる指示語
        if (preg_match('/(Z80ANA_[A-Z_]+)/Ax', $r_str, $matches, 0, $r_offset)) {
            return $this->parseZ80anaDirective_($r_str, $r_offset, $r_line_nr, $matches[1]);
        }

        // '#' で始まるプリプロセス
        if (preg_match('/(\#\s*\w+)\s+/Ax', $r_str, $matches, 0, $r_offset)) {
            return $this->parsePreprocess_($r_str, $r_offset, $r_line_nr, $matches[1]);
        }

        // { <文>* }
        if ($r_str[$r_offset] == '{') {
            return $this->parseSubSentences_($r_str, $r_offset, $r_line_nr);
        }

        // goto <式>;
        if (preg_match('/(goto) \s* (\w+) \s* \;/Ax', $r_str, $matches, 0, $r_offset)) {
            return $this->parseGoto_($r_str, $r_offset, $r_line_nr, $matches[0], $matches[1], $matches[2]);
        }

        // extern <式>[, ...];
        if (preg_match('/(extern) \s* ([\w\s\,]+) \s* \;/Ax', $r_str, $matches, 0, $r_offset)) {
            return $this->parseExtern_($r_str, $r_offset, $r_line_nr, $matches[0], $matches[1], $matches[2]);
        }

        // if (<式>) <文> else <文>
        if (preg_match('/if\s*\(\s*(' . Parser::FLAG_ .  '|'. Parser::FLAG_REL_ . ')\s*\)/Ax', $r_str, $matches, 0, $r_offset)) {
            return $this->parseIf_($r_str, $r_offset, $r_line_nr, $matches[0], $matches[1]);
        }

        // シンボル名:
        if (preg_match('/(' . Parser::SYMBOL_ . '\:)/Ax', $r_str, $matches, 0, $r_offset)) {
            return $this->parseLabel_($r_str, $r_offset, $r_line_nr, $matches[1]);
        }

        // <式> <代入演算子> <式>;
        if (preg_match('/(' . Parser::EXPR_ . ') \= (' . Parser::EXPR_ . ')\;/Ax', $r_str, $matches, 0, $r_offset)) {
            return $this->parseAssignmentOp_($r_str, $r_offset, $r_line_nr, $matches[0], $matches[1], $matches[2]);
        }

        // <単項演算子> <reg>;
        if (preg_match('/(\+\+|\-\-|\-|\~) \s* (' . Parser::EXPR_ . ') \s*;/Ax', $r_str, $matches, 0, $r_offset)) {
            return $this->parseUnaryOp_($r_str, $r_offset, $r_line_nr, $matches[0], $matches[1], $matches[2]);
        }

        // <reg> <単項演算子>;
        if (preg_match('/(' . Parser::EXPR_ . ') (\+\+|\-\-) \s*\;/Ax', $r_str, $matches, 0, $r_offset)) {
            return $this->parseUnaryOp_($r_str, $r_offset, $r_line_nr, $matches[0], $matches[2], $matches[1]);
        }

        // シンボル名(<式リスト>);
        if (preg_match('/(\w+)\s*\((' . Parser::EXPR0_ . ')\)\s*;/Ax', $r_str, $matches, 0, $r_offset)) {
            return $this->functionCall_($r_str, $r_offset, $r_line_nr, $matches[0], $matches[1], $matches[2]);
        }

        // 解析できなかった時は, ';' までスキップ
        $pos = strpos($r_str, ';', $r_offset);
        $len = (($pos !== false) ? $pos : strlen($r_str)) - $r_offset;
        $str = substr($r_str, $r_offset, $len);
        $this->error_->errorLine($r_line_nr, "解析失敗", $str);

        $r_offset += $len;
        $r_line_nr += substr_count($str, "\n");

        return $r_str;
    }


    // -------------------------------- パース ディスパッチャ
    // MARK: parseWhitespaces_()
    /** "\n", ' ', ';' のパース */
    private function parseWhitespaces_(string &$r_str, int &$r_offset, int &$r_line_nr, string $whitespaces): string
    {
        //echo("whitespace1[$r_offset $r_line_nr][$whitespaces]\n");
        $r_offset  += strlen($whitespaces);
        $r_line_nr += substr_count($whitespaces, "\n");
        //echo("whitespace2[$r_offset $r_line_nr]\n");
        return '';
    }


    // MARK: parseZ80anaDirective_()
    /** 'Z80ANA_' で始まる指示語. インデントなし */
    private function parseZ80anaDirective_(string &$r_str, int &$r_offset, int &$r_line_nr, string $directive): string
    {
        // echo("directive1[$r_offset $r_line_nr][$directive]\n");
        // 括弧の無いディレクティブの処理を先にやる
        $r_offset += strlen($directive);
        switch ($directive) {
            case 'Z80ANA_DEF_VARS':
                return '';
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
        }

        // 括弧のあるディレクティブの処理
        $args = $this->extractParentheses_($r_str, $r_offset, $r_line_nr);
        $f = $directive . '_';
        if (method_exists($this, $f)) {
            return $this->{$f}($args, $r_line_nr) . "\n";
        } else {
            $this->error_->errorLine($r_line_nr, "この Z80ANAディレクティブはありません", $directive);
            return '';
        }
    }


    // MARK: parsePreprocess_()
    /** 行頭, '#' で始まるプリプロセッサ ディレクティブ */
    private function parsePreprocess_(string &$r_str, int &$r_offset, int &$r_line_nr, string $directive): string
    {
        //echo("preprocess1[$r_offset $r_line_nr][$directive]\n");
        if ($this->isBeginningOfLine_($r_str, $r_offset, $r_line_nr) === false) {
            $this->toNextBeginningOfLine_($r_str, $r_offset, $r_line_nr);
            return '';
        }
        $r_offset += strlen($directive);
        $off_start = $r_offset;
        // 改行までを $args にコピー
        $this->toNextBeginningOfLine_($r_str, $r_offset, $r_line_nr);
        $args = substr($r_str, $off_start, $r_offset - $off_start);
        //echo("preprocess2[$r_offset $r_line_nr][$directive][$args]\n");
        return "$directive $args"; // $args には改行を含まん
    }


    // MARK: parseSubSentences_()
    /** { ... } で囲んだ複文 */
    private function parseSubSentences_(string &$r_str, int &$r_offset, int &$r_line_nr): string
    {
        //echo("subSentences1[$r_offset $r_line_nr] $r_str[$r_offset]\n");
        // { ... } を検出
        $line_nr = $r_line_nr;
        $str = $this->extractCurlyBracket_($r_str, $r_offset, $line_nr);
        if ($str == false) {
            return '';
        }

        $out = $this->parseSentences_($str, $r_line_nr);
        //echo("subSentences2[$r_offset $r_line_nr]\n");
        return $out;
    }


    // MARK: parseGoto_()
    /** goto <式>; */
    private function parseGoto_(string &$r_str, int &$r_offset, int &$r_line_nr, $match, $directive, $label): string
    {
        $r_offset  += strlen($match);
        $r_line_nr += substr_count($match, "\n");
        return $this->getIndentStr_() . "jp $label\n";
    }


    // MARK: parseExtern_()
    /** extern <式>[, ...]; */
    private function parseExtern_(string &$r_str, int &$r_offset, int &$r_line_nr, $match, $directive, $labels): string
    {
        $r_offset  += strlen($match);
        $r_line_nr += substr_count($match, "\n");
        $params = $this->explodeByComma_($labels);

        $ret = $this->getIndentStr_() . "extern ";
        $err = false;

        foreach($params as $i => $param) {
            $p = $this->checkExpr_($param, $r_line_nr);
            if ($p === null || $p->type !== Param::TYPE_VAL) {
                $this->error_->errorLine($r_line_nr, "$opcode 引数 ". ($i + 1) . " のエラー", $param);
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
    private function parseIf_(string &$r_str, int &$r_offset, int &$r_line_nr, $match, $flag): string
    {
        $r_offset  += strlen($match);
        $r_line_nr += substr_count($match, "\n");
        // フラグとジャンプ命令の設定
        $flag = Parser::FLAG_NEG_TAB_[$flag];
        $opcode = 'jp';
        $label1 = $this->genRegistLabel_();
        if (str_ends_with($flag, '_r')) {
            $flag = substr($flag, 0, -2);
            $opcode = 'jr';
        }
        if ($flag !== 'false') { $flag .= ','; }

        $out = $this->getIndentStr_() . "$opcode $flag $label1\n";
        $this->incIndent_();

        $this->skipSpaces_($r_str, $r_offset, $r_line_nr);
        $out .= $this->parseSentence_($r_str, $r_offset, $r_line_nr);
        $this->skipSpaces_($r_str, $r_offset, $r_line_nr);

        // else を見つけた!
        //$str = substr($r_str, $r_offset); echo("[[$str]]\n");
        if (preg_match('/else/Ax', $r_str, $matches, 0, $r_offset)) {
            $r_offset  += strlen('else');

            $label2 = $this->genRegistLabel_();
            $out .= $this->getIndentStr_() . "jp $label2\n";
            $out .= "$label1:\n";

            $this->skipSpaces_($r_str, $r_offset, $r_line_nr);
            $out .= $this->parseSentence_($r_str, $r_offset, $r_line_nr);
            $this->skipSpaces_($r_str, $r_offset, $r_line_nr);

            $out .= "$label2:\n";
        } else {
            $out .= "$label1:\n";
        }
        $this->decIndent_();

        return $out;
    }


    // MARK: parseLabel_()
    /** ':' で終るシンボル名 */
    private function parseLabel_(string &$r_str, int &$r_offset, int &$r_line_nr, string $label): string
    {
        //echo("label1[$r_offset $r_line_nr][$label]\n");
        $r_offset += strlen($label);
        //$this->registLabel_(substr($label, 0, strlen($label) - 1));
        return "$label\n";  // 改行します
    }


    // MARK: parseAssignmentOp_()
    /** 代入演算子 */
    private function parseAssignmentOp_(string &$r_str, int &$r_offset, int &$r_line_nr, string $match, string $l_value, string $r_value): string
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
        //echo("assignOp1[$r_offset $r_line_nr][$l_value][$op][$r_value]\n");

        // 型によって命令が変わるので, 右辺値, 左辺値の型を調べる
        $b_err = false;
        $l_param = $this->checkExpr_($l_value, $r_line_nr);
        $r_param = $this->checkExpr_($r_value, $r_line_nr);
        if ($l_param === null) {
            $this->error_->errorLine($r_line_nr, "左辺値の異常", $match);
            $b_err = true;
        }
        if ($r_param === null) {
            $this->error_->errorLine($r_line_nr, "右辺値の異常", $match);
            $b_err = true;
        }
        $ret = '';
        if ($b_err !== true) {
            $expr = $l_value . "," . $this->cutOffCarryExpr_($r_value);

            // 入出力
            if ($l_param->type === Param::TYPE_PORT || $r_param->type === Param::TYPE_PORT) {
                if ($op !== '=') {
                    $this->error_->errorLine($r_line_nr, "入出力で使える演算子は '=' のみです", $match);
                } else {
                    if ($r_param->type === Param::TYPE_PORT) {
                        $ret = $this->in_($expr, $r_line_nr);
                    } else {
                        $ret = $this->out_($expr, $r_line_nr);
                    }
                }
            } else {
                // $op によって命令を決める
                switch ($op) {
                    case '=': $ret = $this->ld_($expr, $r_line_nr); break;
                    case '+=':
                        if ($r_param->isTypeMemRegValC()) {
                            $ret = $this->adc_($expr, $r_line_nr);
                        } else {
                            $ret = $this->add_($expr, $r_line_nr);
                        }
                        break;
                    case '-=':
                        if ($r_param->isTypeMemRegValC()) {
                            $ret = $this->sbc_($expr, $r_line_nr);
                        } else {
                            $ret = $this->sub_($expr, $r_line_nr);
                        }
                        break;
                    case '&=': $ret = $this->and_($expr, $r_line_nr); break;
                    case '|=': $ret = $this->or_($expr, $r_line_nr); break;
                    case '^=': $ret = $this->xor_($expr, $r_line_nr); break;
                    case '>>=': $ret = $this->srl_n_($expr, $r_line_nr); break;
                    case '<<=': $ret = $this->sla_n_($expr, $r_line_nr); break;
                }
            }
        }

        $r_offset  += strlen($match);
        $r_line_nr += substr_count($match, "\n");
        //echo("assignOp2[$r_offset $r_line_nr][$l_value][$op][$r_value]\n");

        return ($b_err) ? '' : ($this->getIndentStr_() . "$ret\n");
    }


    // MARK: parseUnaryOp_()
    /** 単項演算子 */
    private function parseUnaryOp_(string &$r_str, int &$r_offset, int &$r_line_nr, string $match, string $op, string $expr): string
    {
        //echo("unaryOp1[$r_offset $r_line_nr][$op][$expr]\n");

        // $op によって命令を決める
        $ret = '';
        switch ($op) {
            case '++': $ret = $this->inc_($expr, $r_line_nr); break;
            case '--': $ret = $this->dec_($expr, $r_line_nr); break;
            case '-': $ret = $this->neg_($expr, $r_line_nr); break;
            case '~': $ret = $this->cpl_($expr, $r_line_nr); break;
        }

        $r_offset  += strlen($match);
        $r_line_nr += substr_count($match, "\n");

        //echo("unaryOp2[$r_offset $r_line_nr][$opcode][$expr]\n");
        return $this->getIndentStr_() . "$ret\n";
    }


    // MARK: functionCall_()
    /** 関数呼び出し, またはマクロ呼び出し */
    private function functionCall_(string &$r_str, int &$r_offset, int &$r_line_nr, string $match, string $funcname, string $expr): string
    {
        //echo("func1[$r_offset $r_line_nr][$funcname($expr)]\n");

        $f = $funcname . '_';
        if (method_exists($this, $f)) {
            $ret = $this->{$f}($expr, $r_line_nr);
        } else {
            // マクロとみなす(但し大文字英数のみ)
            if (preg_match('/^[A-Z_][A-Z_0-9]*$/', $funcname)) {
                $expr = str_replace("\n", ' ', $expr);// マクロ呼び出しは1行で書かないといけないので改行を削除します
                $ret = $funcname . ' ' . $expr;
            } else {
                $this->error_->errorLine($r_line_nr, "この関数はありません. マクロとして呼び出す場合は, 大文字を使います", $funcname);
                $ret = '';
            }
        }

        $r_offset  += strlen($match);
        $r_line_nr += substr_count($match, "\n");

        //echo("func2[$r_offset $r_line_nr][$ret]\n");
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


    // -------------------------------- 文の解析ユーティリティ
    // ---------------- 空白文字(半角スペースと改行)
    // MARK: skipSpaces_()
    /** 空白文字をスキップします.
     * @param $r_offset $r_str の文字位置(0～)
     * - [in]  開始時
     * - [out] スキップした空白文字の次の位置. スキップしなかったら変化なし
     * @param $r_line_nr $r_str の行番号(0～)
     * - [in]  開始時
     * - [out] 終了時. スキップしなかったら変化なし
     */
    private function skipSpaces_(string &$r_str, int &$r_offset, int &$r_line_nr): void
    {
        if (preg_match('/[\ \n]+/Ax', $r_str, $matches, 0, $r_offset)) {
            $r_offset  += strlen($matches[0]);
            $r_line_nr += substr_count($matches[0], "\n");
        }
    }


    // MARK: isBeginningOfLine_()
    /** $offset が行頭にあるかをチェック
     * - $offset より前を改行(or行頭)までサーチして, 空白文字以外の文字があればエラー
     */
    private function isBeginningOfLine_(string &$r_str, int $offset, int $line_nr): bool
    {
        for ($off = $offset - 1; 0 <= $off; $off--) {
            switch ($r_str[$off]) {
                case ' ': break;
                case "\n":
                    return true;
                default:
                    $this->error_->errorLine($line_nr, "行頭にありません", substr($r_str, $offset));
                    return false;
            }
        }
        return true;
    }


    // MARK: toNextBeginningOfLine_()
    /** 次の行頭まで移動します.
     * - 但し 「'\' + 改行」は除く
     */
    private function toNextBeginningOfLine_(string &$r_str, int &$r_offset, int &$r_line_nr): void
    {
        while (true) {
            $pos = strpos($r_str, "\n", $r_offset);
            if ($pos === false) {// $r_str の末端まで行った
                $r_offset = strlen($r_str);
                return;
            }
            $r_offset = $pos + 1;
            $r_line_nr++;
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
     * @param $r_line_nr $r_str の行番号(0～).
     * - [in]  '(' の位置
     * - [out] ')' の次の位置. 抽出失敗時は strlen($r_str)
     * @return 抽出した文字列. 抽出失敗時は false
     */
    private function extractParentheses_(string &$r_str, int &$r_offset, int &$r_line_nr): string|false
    {
        return $this->extractParenthesesSub_($r_str, $r_offset, $r_line_nr, '(', ')');
    }


    // MARK: extractCurlyBracket_()
    /** 空白文字に続く中カッコ {...} で囲まれた中の文字列を抽出します
     * - 引数などは, 上の extractParentheses_() を参照してください
     */
    private function extractCurlyBracket_(string &$r_str, int &$r_offset, int &$r_line_nr): string|false
    {
        return $this->extractParenthesesSub_($r_str, $r_offset, $r_line_nr, '{', '}');
    }


    // MARK: extractParenthesesSub_()
    /** カッコ $c0...$c1 で囲まれた中の文字列を抽出します */
    private function extractParenthesesSub_(string &$r_str, int &$r_offset, int &$r_line_nr, string $c0, string $c1): string|false
    {
        $lv = 1;
        $len = strlen($r_str);

        if (preg_match('/[\s\n]*\\' . $c0 . '/Ax', $r_str, $matches, 0, $r_offset)) {
            $r_offset  += strlen($matches[0]);
            $r_line_nr += substr_count($matches[0], "\n");
        } else {
            $this->error_->errorLine($r_line_nr, "'$c0' が見つかりません", substr($r_str, $r_offset));
            $r_line_nr += substr_count($r_str, "\n", $r_offset);
            $r_offset   = $len;
            return false;
        }

        $start = $r_offset;
        $start_line_nr = $r_line_nr;

        for ($off = $r_offset; $off < $len; $off++) {
            switch ($r_str[$off]) {
                case "\n":
                    $r_line_nr++;
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
                case '}':
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
    private function checkExpr_(string $expr, int $line_nr): Param|null
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
                $this->error_->errorLine($line_nr, "この名前はマクロモードでしか使えません", $expr);
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
                $this->error_->errorLine($line_nr, "この名前はマクロモードでしか使えません", $expr);
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
            $this->error_->errorLine($line_nr, "未知の式です", $expr);
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


    // MARK: command dispatches
    // -------------------------------- z80anaDirective->疑似命令ディスパッチ
    private function Z80ANA_LOCAL_(string $expr, int $line_nr): string { return $this->checkParamNN_S_('local', $expr, $line_nr); }
    private function Z80ANA_DB_(string $expr, int $line_nr): string { return $this->checkParamNN_V_('db', $expr, $line_nr); }
    private function Z80ANA_DW_(string $expr, int $line_nr): string { return $this->checkParamNN_V_('dw', $expr, $line_nr); }
    private function Z80ANA_IF_(string $expr, int $line_nr): string { $this->incIndent_(); return $this->checkParam11_('if', $expr, $line_nr); }
    private function Z80ANA_ELIF_(string $expr, int $line_nr): string { return $this->checkParam11_('elif', $expr, $line_nr); }
    private function Z80ANA_REPT_(string $expr, int $line_nr): string { $this->incIndent_(); return $this->checkParam11_('rept', $expr, $line_nr); }

    // -------------------------------- 関数->命令ディスパッチ
    private function ld_(string $expr, int $line_nr): string { return $this->checkParam22_RM_RMV_('ld', $expr, $line_nr); }
    private function ldi_(string $expr, int $line_nr): string { return $this->checkParam00_('ldi', $expr, $line_nr); }
    private function ldd_(string $expr, int $line_nr): string { return $this->checkParam00_('ldd', $expr, $line_nr); }
    private function ldir_(string $expr, int $line_nr): string { return $this->checkParam00_('ldir', $expr, $line_nr); }
    private function lddr_(string $expr, int $line_nr): string { return $this->checkParam00_('lddr', $expr, $line_nr); }
    private function ex_(string $expr, int $line_nr): string { return $this->checkParam22_RM_R_('ex', $expr, $line_nr); }
    private function exx_(string $expr, int $line_nr): string { return 'exx'; }// 引数無し
    private function push_(string $expr, int $line_nr): string { return $this->checkParam1N_R_('push', $expr, $line_nr); }
    private function pop_(string $expr, int $line_nr): string { return $this->checkParam1N_R_('pop', $expr, $line_nr); }
    private function and_(string $expr, int $line_nr): string { return $this->checkParam22_A_RM_('and', $expr, $line_nr); }
    private function or_( string $expr, int $line_nr): string { return $this->checkParam22_A_RM_('or', $expr, $line_nr); }
    private function xor_(string $expr, int $line_nr): string { return $this->checkParam22_A_RM_('xor', $expr, $line_nr); }
    private function cpl_(string $expr, int $line_nr): string { return $this->checkParam01_A_('cpl', $expr, $line_nr); }
    private function not_(string $expr, int $line_nr): string { return $this->cpl_($expr, $line_nr); }
    private function cp_(string $expr, int $line_nr): string { return $this->checkParam22_A_RM_('cp', $expr, $line_nr); }
    private function cpi_(string $expr, int $line_nr): string { return $this->checkParam00_('cpi', $expr, $line_nr); }
    private function cpd_(string $expr, int $line_nr): string { return $this->checkParam00_('cpd', $expr, $line_nr); }
    private function cpir_(string $expr, int $line_nr): string { return $this->checkParam00_('cpir', $expr, $line_nr); }
    private function cpdr_(string $expr, int $line_nr): string { return $this->checkParam00_('cpdr', $expr, $line_nr); }
    private function add_(string $expr, int $line_nr): string { return $this->checkParam22_R_RMV_('add', $expr, $line_nr); }
    private function adc_(string $expr, int $line_nr): string { return $this->checkParam22_R_RMV_('adc', $expr, $line_nr); }
    private function sub_(string $expr, int $line_nr): string { return $this->checkParam22_A_RM_('sub', $expr, $line_nr); }
    private function sbc_(string $expr, int $line_nr): string { return $this->checkParam22_R_RMV_('sbc', $expr, $line_nr); }
    private function inc_(string $expr, int $line_nr): string { return $this->checkParam11_RM_('inc', $expr, $line_nr); }
    private function dec_(string $expr, int $line_nr): string { return $this->checkParam11_RM_('dec', $expr, $line_nr); }
    private function neg_(string $expr, int $line_nr): string { return $this->checkParam01_A_('neg', $expr, $line_nr); }
    private function daa_(string $expr, int $line_nr): string { return $this->checkParam01_A_('daa', $expr, $line_nr); }
    private function bit_(string $expr, int $line_nr): string { return $this->checkParam22_V_RM_('bit', $expr, $line_nr); }
    private function set_(string $expr, int $line_nr): string { return $this->checkParam22_V_RM_('set', $expr, $line_nr); }
    private function res_(string $expr, int $line_nr): string { return $this->checkParam22_V_RM_('res', $expr, $line_nr); }
    private function bit_3_(string $expr, int $line_nr): string { return $this->checkParam33_V_M_R_('bit', $expr, $line_nr); }
    private function set_3_(string $expr, int $line_nr): string { return $this->checkParam33_V_M_R_('set', $expr, $line_nr); }
    private function res_3_(string $expr, int $line_nr): string { return $this->checkParam33_V_M_R_('res', $expr, $line_nr); }
    private function sla_(string $expr, int $line_nr): string { return $this->checkParam11_RM_('sla', $expr, $line_nr); }
    private function sll_(string $expr, int $line_nr): string { return $this->checkParam11_RM_('sll', $expr, $line_nr); }
    private function sra_(string $expr, int $line_nr): string { return $this->checkParam11_RM_('sra', $expr, $line_nr); }
    private function srl_(string $expr, int $line_nr): string { return $this->checkParam11_RM_('srl', $expr, $line_nr); }
    private function rl_( string $expr, int $line_nr): string { return $this->checkParam11_RM_('rl', $expr, $line_nr); }
    private function rr_( string $expr, int $line_nr): string { return $this->checkParam11_RM_('rr', $expr, $line_nr); }
    private function rlc_(string $expr, int $line_nr): string { return $this->checkParam11_RM_('rlc', $expr, $line_nr); }
    private function rrc_(string $expr, int $line_nr): string { return $this->checkParam11_RM_('rrc', $expr, $line_nr); }
    private function sla_n_(string $expr, int $line_nr): string { return $this->checkParam12_RM_V_('sla', $expr, $line_nr); }
    private function sll_n_(string $expr, int $line_nr): string { return $this->checkParam12_RM_V_('sll', $expr, $line_nr); }
    private function sra_n_(string $expr, int $line_nr): string { return $this->checkParam12_RM_V_('sra', $expr, $line_nr); }
    private function srl_n_(string $expr, int $line_nr): string { return $this->checkParam12_RM_V_('srl', $expr, $line_nr); }
    private function rl_n_( string $expr, int $line_nr): string { return $this->checkParam12_RM_V_('rl', $expr, $line_nr); }
    private function rr_n_( string $expr, int $line_nr): string { return $this->checkParam12_RM_V_('rr', $expr, $line_nr); }
    private function rlc_n_(string $expr, int $line_nr): string { return $this->checkParam12_RM_V_('rlc', $expr, $line_nr); }
    private function rrc_n_(string $expr, int $line_nr): string { return $this->checkParam12_RM_V_('rrc', $expr, $line_nr); }
    private function sla_2_(string $expr, int $line_nr): string { return $this->checkParam22_M_R_('sla', $expr, $line_nr); }
    private function sll_2_(string $expr, int $line_nr): string { return $this->checkParam22_M_R_('sll', $expr, $line_nr); }
    private function sra_2_(string $expr, int $line_nr): string { return $this->checkParam22_M_R_('sra', $expr, $line_nr); }
    private function srl_2_(string $expr, int $line_nr): string { return $this->checkParam22_M_R_('srl', $expr, $line_nr); }
    private function rl_2_( string $expr, int $line_nr): string { return $this->checkParam22_M_R_('rl', $expr, $line_nr); }
    private function rr_2_( string $expr, int $line_nr): string { return $this->checkParam22_M_R_('rr', $expr, $line_nr); }
    private function rlc_2_(string $expr, int $line_nr): string { return $this->checkParam22_M_R_('rlc', $expr, $line_nr); }
    private function rrc_2_(string $expr, int $line_nr): string { return $this->checkParam22_M_R_('rrc', $expr, $line_nr); }
    private function rla_ (string $expr, int $line_nr): string { return $this->checkParam01_A_('rla', $expr, $line_nr); }
    private function rra_ (string $expr, int $line_nr): string { return $this->checkParam01_A_('rra', $expr, $line_nr); }
    private function rlca_(string $expr, int $line_nr): string { return $this->checkParam01_A_('rlca', $expr, $line_nr); }
    private function rrca_(string $expr, int $line_nr): string { return $this->checkParam01_A_('rrca', $expr, $line_nr); }
    private function rla_n_ (string $expr, int $line_nr): string { return $this->checkParam02_AV_('rla', $expr, $line_nr); }
    private function rra_n_ (string $expr, int $line_nr): string { return $this->checkParam02_AV_('rra', $expr, $line_nr); }
    private function rlca_n_(string $expr, int $line_nr): string { return $this->checkParam02_AV_('rlca', $expr, $line_nr); }
    private function rrca_n_(string $expr, int $line_nr): string { return $this->checkParam02_AV_('rrca', $expr, $line_nr); }
    private function rld_(string $expr, int $line_nr): string { return $this->checkParam20_A_M_('rld', $expr, $line_nr); }
    private function rrd_(string $expr, int $line_nr): string { return $this->checkParam20_A_M_('rrd', $expr, $line_nr); }
    private function jp_(string $expr, int $line_nr): string { return $this->checkParam11_V_HL_('jp', $expr, $line_nr); }
    private function jp_z_(string $expr, int $line_nr): string { return $this->checkParam11_V_HL_('jp z,', $expr, $line_nr); }
    private function jp_nz_(string $expr, int $line_nr): string { return $this->checkParam11_V_HL_('jp nz,', $expr, $line_nr); }
    private function jp_c_(string $expr, int $line_nr): string { return $this->checkParam11_V_HL_('jp c,', $expr, $line_nr); }
    private function jp_nc_(string $expr, int $line_nr): string { return $this->checkParam11_V_HL_('jp nc,', $expr, $line_nr); }
    private function jp_p_(string $expr, int $line_nr): string { return $this->checkParam11_V_HL_('jp p,', $expr, $line_nr); }
    private function jp_m_(string $expr, int $line_nr): string { return $this->checkParam11_V_HL_('jp m,', $expr, $line_nr); }
    private function jp_pe_(string $expr, int $line_nr): string { return $this->checkParam11_V_HL_('jp pe,', $expr, $line_nr); }
    private function jp_po_(string $expr, int $line_nr): string { return $this->checkParam11_V_HL_('jp po,', $expr, $line_nr); }
    private function jp_v_(string $expr, int $line_nr): string { return $this->checkParam11_V_HL_('jp v,', $expr, $line_nr); }
    private function jp_nv_(string $expr, int $line_nr): string { return $this->checkParam11_V_HL_('jp nv,', $expr, $line_nr); }
    private function jr_(string $expr, int $line_nr): string { return $this->checkParam11_V_('jr', $expr, $line_nr); }
    private function jr_z_(string $expr, int $line_nr): string { return $this->checkParam11_V_('jr z,', $expr, $line_nr); }
    private function jr_nz_(string $expr, int $line_nr): string { return $this->checkParam11_V_('jr nz,', $expr, $line_nr); }
    private function jr_c_(string $expr, int $line_nr): string { return $this->checkParam11_V_('jr c,', $expr, $line_nr); }
    private function jr_nc_(string $expr, int $line_nr): string { return $this->checkParam11_V_('jr nc,', $expr, $line_nr); }
    private function djnz_(string $expr, int $line_nr): string { return $this->checkParam22_B_V_('djnz', $expr, $line_nr); }
    private function call_(string $expr, int $line_nr): string { return $this->checkParam11_V_('call', $expr, $line_nr); }
    private function call_z_(string $expr, int $line_nr): string { return $this->checkParam11_V_('call z,', $expr, $line_nr); }
    private function call_nz_(string $expr, int $line_nr): string { return $this->checkParam11_V_('call nz,', $expr, $line_nr); }
    private function call_c_(string $expr, int $line_nr): string { return $this->checkParam11_V_('call c,', $expr, $line_nr); }
    private function call_nc_(string $expr, int $line_nr): string { return $this->checkParam11_V_('call nc,', $expr, $line_nr); }
    private function call_p_(string $expr, int $line_nr): string { return $this->checkParam11_V_('call p,', $expr, $line_nr); }
    private function call_m_(string $expr, int $line_nr): string { return $this->checkParam11_V_('call m,', $expr, $line_nr); }
    private function call_pe_(string $expr, int $line_nr): string { return $this->checkParam11_V_('call pe,', $expr, $line_nr); }
    private function call_po_(string $expr, int $line_nr): string { return $this->checkParam11_V_('call po,', $expr, $line_nr); }
    private function call_v_(string $expr, int $line_nr): string { return $this->checkParam11_V_('call v,', $expr, $line_nr); }
    private function call_nv_(string $expr, int $line_nr): string { return $this->checkParam11_V_('call nv,', $expr, $line_nr); }
    private function rst_(string $expr, int $line_nr): string { return $this->checkParam11_V_('rst', $expr, $line_nr); }
    private function ret_(string $expr, int $line_nr): string { return $this->checkParam00_('ret', $expr, $line_nr); }
    private function ret_z_(string $expr, int $line_nr): string { return $this->checkParam00_('ret z,', $expr, $line_nr); }
    private function ret_nz_(string $expr, int $line_nr): string { return $this->checkParam00_('ret nz,', $expr, $line_nr); }
    private function ret_c_(string $expr, int $line_nr): string { return $this->checkParam00_('ret c,', $expr, $line_nr); }
    private function ret_nc_(string $expr, int $line_nr): string { return $this->checkParam00_('ret nc,', $expr, $line_nr); }
    private function ret_p_(string $expr, int $line_nr): string { return $this->checkParam00_('ret p,', $expr, $line_nr); }
    private function ret_m_(string $expr, int $line_nr): string { return $this->checkParam00_('ret m,', $expr, $line_nr); }
    private function ret_pe_(string $expr, int $line_nr): string { return $this->checkParam00_('ret pe,', $expr, $line_nr); }
    private function ret_po_(string $expr, int $line_nr): string { return $this->checkParam00_('ret po,', $expr, $line_nr); }
    private function ret_v_(string $expr, int $line_nr): string { return $this->checkParam00_('ret v,', $expr, $line_nr); }
    private function ret_nv_(string $expr, int $line_nr): string { return $this->checkParam00_('ret nv,', $expr, $line_nr); }
    private function reti_(string $expr, int $line_nr): string { return $this->checkParam00_('reti', $expr, $line_nr); }
    private function retn_(string $expr, int $line_nr): string { return $this->checkParam00_('retn', $expr, $line_nr); }
    private function nop_(string $expr, int $line_nr): string { return $this->checkParam00_('nop', $expr, $line_nr); }
    private function halt_(string $expr, int $line_nr): string { return $this->checkParam00_('halt', $expr, $line_nr); }
    private function di_(string $expr, int $line_nr): string { return $this->checkParam00_('di', $expr, $line_nr); }
    private function ei_(string $expr, int $line_nr): string { return $this->checkParam00_('ei', $expr, $line_nr); }
    private function im0_(string $expr, int $line_nr): string { return $this->checkParam00_('im 0', $expr, $line_nr); }
    private function im1_(string $expr, int $line_nr): string { return $this->checkParam00_('im 1', $expr, $line_nr); }
    private function im2_(string $expr, int $line_nr): string { return $this->checkParam00_('im 2', $expr, $line_nr); }
    private function scf_(string $expr, int $line_nr): string { return $this->checkParam01_c_('scf', $expr, $line_nr); }
    private function ccf_(string $expr, int $line_nr): string { return $this->checkParam01_c_('ccf', $expr, $line_nr); }
    private function in_(string $expr, int $line_nr): string { return $this->checkParam22_R_P_('in', $expr, $line_nr); }
    private function out_(string $expr, int $line_nr): string { return $this->checkParam22_P_RV_('out', $expr, $line_nr); }
    private function ini_(string $expr, int $line_nr): string { return $this->checkParam00_('ini', $expr, $line_nr); }
    private function ind_(string $expr, int $line_nr): string { return $this->checkParam00_('ind', $expr, $line_nr); }
    private function inir_(string $expr, int $line_nr): string { return $this->checkParam00_('inir', $expr, $line_nr); }
    private function indr_(string $expr, int $line_nr): string { return $this->checkParam00_('indr', $expr, $line_nr); }
    private function outi_(string $expr, int $line_nr): string { return $this->checkParam00_('outi', $expr, $line_nr); }
    private function outd_(string $expr, int $line_nr): string { return $this->checkParam00_('outd', $expr, $line_nr); }
    private function otir_(string $expr, int $line_nr): string { return $this->checkParam00_('otir', $expr, $line_nr); }
    private function otdr_(string $expr, int $line_nr): string { return $this->checkParam00_('otdr', $expr, $line_nr); }


    // MARK: checkParamSub_()
    /** expr を n 分割し, それぞれを更にパラメータとタイプに分割します */
    private function checkParamSub_(string $opcode, string $expr, int $n, int $line_nr): array|null
    {
        $params = $this->explodeByComma_($expr);
        if (count($params) != $n) {
            $this->error_->errorLine($line_nr, "$opcode の引数は $n 個です", $expr);
            return null;
        }

        $ret = [];
        foreach($params as $i => $param) {
            $p = $this->checkExpr_($param, $line_nr);
            if ($p === null) {
                $this->error_->errorLine($line_nr, "$opcode 引数 ". ($i + 1) . " のエラー", $param);
            } else {
                $ret[] = $p;
            }
        }
        if (count($ret) !== $n) { return null; }
        return $ret;
    }


    // MARK: checkParam00_()
    /** 命令, 関数共に引数ありません */
    private function checkParam00_(string $opcode, string $expr, int $line_nr): string
    {
        if ($expr !== '') {
            $this->error_->errorLine($line_nr, "$opcode は引数不要です", $expr);
        }
        return $opcode;
    }


    // MARK: checkParam01_A_()
    /** 命令は引数無しですが, 関数は 1 つの引数 ('A') を持ちます  */
    private function checkParam01_A_(string $opcode, string $expr, int $line_nr): string
    {
        if ($expr !== 'A') {
            $this->error_->errorLine($line_nr, "$opcode で使用できる引数は, A のみです", $expr);
        }
        return $opcode;
    }


    // MARK: checkParam01_c_()
    /** 命令は引数無しですが, 関数は 1つのの引数 ('c') を持ちます  */
    private function checkParam01_c_(string $opcode, string $expr, int $line_nr): string
    {
        if ($expr !== 'c') {
            $this->error_->errorLine($line_nr, "$opcode で使用できる引数は, c のみです", $expr);
        }
        return $opcode;
    }


    // MARK: checkParam02_AV_()
    /** 命令は引数無しですが, 関数は 2 つの引数 ('A', 値) を持ちます  */
    private function checkParam02_AV_(string $opcode, string $expr, int $line_nr): string
    {
        $p = $this->checkParamSub_($opcode, $expr, 2, $line_nr);
        if ($p === null) { return ''; }

        if ($p[0]->value !== 'A') {
            $this->error_->errorLine($line_nr, "$opcode の第1引数で使用できるのは, A のみです", $expr);
            return '';
        }
        if ($p[1]->type !== Param::TYPE_VAL) {
            $this->error_->errorLine($line_nr, "$opcode の第2引数で使用できるのは, 値 のみです", $expr);
            return '';
        }

        // 値が数値ならば直接展開(4まで), そうでなければ rept
        $out = '';
        //echo($p[1]->value . "\n");
        if (preg_match('/^0\ \+\ \(((:?0x[0-9a-fA-F]+)|(:?[0-9]+))\)$/x', $p[1]->value, $matches)) {
            //print_r($matches);
            if (4 < $matches[1]) {
                $this->error_->errorLine($line_nr, "$opcode の第2引数値が大きすぎます", $expr);
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
    private function checkParam11_(string $opcode, string $expr, int $line_nr): string
    {
        $params = $this->explodeByComma_($expr);
        if (count($params) != 1) {
            $this->error_->errorLine($line_nr, "$opcode の引数は 1 個です", $params[0]);
            return '';
        }

        return "$opcode " . $params[0];
    }


    // MARK: checkParam11_RM_()
    /** 命令, 関数共に 1つの引数 (レジスタ|メモリ) を持ちます  */
    private function checkParam11_RM_(string $opcode, string $expr, int $line_nr): string
    {
        $p = $this->checkParamSub_($opcode, $expr, 1, $line_nr);
        if ($p === null) { return ''; }

        if ($p[0]->type !== Param::TYPE_REG && $p[0]->type !== Param::TYPE_MEM) {
            $this->error_->errorLine($line_nr, "$opcode の引数は, レジスタ|メモリ のみです", $expr);
            return '';
        }

        return "$opcode " . $p[0]->value;
    }


    // MARK: checkParam11_V_HL_()
    /** 命令, 関数共に 1つの引数 (値|HL|IX|IY) を持ちます  */
    private function checkParam11_V_HL_(string $opcode, string $expr, int $line_nr): string
    {
        $p = $this->checkParamSub_($opcode, $expr, 1, $line_nr);
        if ($p === null) { return ''; }
        if (($p[0]->type !== Param::TYPE_VAL && $p[0]->type !== Param::TYPE_REG) ||
            ($p[0]->type === Param::TYPE_REG &&
                ($p[0]->value !== 'HL' && $p[0]->value !== 'IX' && $p[0]->value !== 'IY'))) {
            $this->error_->errorLine($line_nr, "$opcode の引数は, 値|HL|IX|IY のみです", $expr);
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
    private function checkParam11_V_(string $opcode, string $expr, int $line_nr): string
    {
        $p = $this->checkParamSub_($opcode, $expr, 1, $line_nr);
        if ($p === null) { return ''; }

        if ($p[0]->type !== Param::TYPE_VAL) {
            $this->error_->errorLine($line_nr, "$opcode の引数は, 値 のみです", $expr);
            return '';
        }
        $p[0]->adjustVal();

        return "$opcode " . $p[0]->value;
    }


    // MARK: checkParam11_F_()
    /** 命令, 関数共に 1つの引数 (フラグ) を持ちます  */
    private function checkParam11_F_(string $opcode, string $expr, int $line_nr): string
    {
        $p = $this->checkParamSub_($opcode, $expr, 1, $line_nr);
        if ($p === null) { return ''; }

        if ($p[0]->type !== Param::TYPE_FLAG) {
            $this->error_->errorLine($line_nr, "$opcode の引数は, フラグ のみです", $expr);
        }

        return "$opcode " . $p[0]->value;
    }


    // MARK: checkParam12_RM_V_()
    /** 命令は1つ(レジスタ|メモリ), 関数は2つの引数 (値) を持ちます */
    private function checkParam12_RM_V_(string $opcode, string $expr, int $line_nr): string
    {
        $p = $this->checkParamSub_($opcode, $expr, 2, $line_nr);
        if ($p === null) { return ''; }

        if ($p[0]->type !== Param::TYPE_REG && $p[0]->type !== Param::TYPE_MEM) {
            $this->error_->errorLine($line_nr, "$opcode の第1引数で使用できるのは, レジスタ|メモリ のみです", $expr);
        }
        if ($p[1]->type !== Param::TYPE_VAL) {
            $this->error_->errorLine($line_nr, "$opcode の第2引数で使用できるのは, 値 のみです", $expr);
        }

        // 値が数値ならば直接展開(4まで), そうでなければ rept
        $out = '';
        //echo($p[1]->value . "\n");
        if (preg_match('/^0\ \+\ \(((:?0x[0-9a-fA-F]+)|(:?[0-9]+))\)$/x', $p[1]->value, $matches)) {
            //print_r($matches);
            if (4 < $matches[1]) {
                $this->error_->errorLine($line_nr, "$opcode の第2引数値が大きすぎます", $expr);
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


    // MARK: checkParam22_RM_RMV_()
    /** 命令, 関数共に 2つの引数 (レジスタ|メモリ, レジスタ|メモリ|値) を持ちます */
    private function checkParam22_RM_RMV_(string $opcode, string $expr, int $line_nr): string
    {
        $p = $this->checkParamSub_($opcode, $expr, 2, $line_nr);
        if ($p === null) { return ''; }
        if ($p[0]->type !== Param::TYPE_REG && $p[0]->type !== Param::TYPE_MEM) {
            $this->error_->errorLine($line_nr, "$opcode の第1引数で使用できるのは, レジスタ|メモリ のみです", $p[0]->value);
        }
        if ($p[1]->type !== Param::TYPE_REG && $p[1]->type !== Param::TYPE_MEM && $p[1]->type !== Param::TYPE_VAL) {
            $this->error_->errorLine($line_nr, "$opcode の第2引数で使用できるのは, レジスタ|メモリ|値 のみです", $p[1]->value);
        }
        $p[1]->adjustVal();

        return "$opcode " . $p[0]->value . ', ' .  $p[1]->value;
    }


    // MARK: checkParam22_R_P_()
    /** 命令, 関数共に 2つの引数 (レジスタ, ポート) を持ちます */
    private function checkParam22_R_P_(string $opcode, string $expr, int $line_nr): string
    {
        $p = $this->checkParamSub_($opcode, $expr, 2, $line_nr);
        if ($p === null) { return ''; }

        if ($p[0]->type !== Param::TYPE_REG) {
            $this->error_->errorLine($line_nr, "$opcode の第1引数で使用できるのは, レジスタ のみです", $p[0]->value);
        }
        if ($p[1]->type !== Param::TYPE_PORT) {
            $this->error_->errorLine($line_nr, "$opcode の第2引数で使用できるのは, ポート のみです", $p[1]->value);
        }

        return "$opcode " . $p[0]->value . ', ' .  $p[1]->value;
    }


    // MARK: checkParam22_P_RV_()
    /** 命令, 関数共に 2つの引数 (ポート, レジスタ|値) を持ちます */
    private function checkParam22_P_RV_(string $opcode, string $expr, int $line_nr): string
    {
        $p = $this->checkParamSub_($opcode, $expr, 2, $line_nr);
        if ($p === null) { return ''; }

        if ($p[0]->type !== Param::TYPE_PORT) {
            $this->error_->errorLine($line_nr, "$opcode の第1引数で使用できるのは, ポート のみです", $p[0]->value);
        }
        if ($p[1]->type !== Param::TYPE_REG && $p[1]->type !== Param::TYPE_VAL) {
            $this->error_->errorLine($line_nr, "$opcode の第2引数で使用できるのは, レジスタ|値 のみです", $p[1]->value);
        }

        $p[1]->adjustVal();

        return "$opcode " . $p[0]->value . ', ' .  $p[1]->value;
    }

    // MARK: checkParam22_A_RM_()
    /** 命令, 関数共に 2つの引数 ('A', 二番目はレジスタ|メモリ|値) を持ちます */
    private function checkParam22_A_RM_(string $opcode, string $expr, int $line_nr): string
    {
        $p = $this->checkParamSub_($opcode, $expr, 2, $line_nr);
        if ($p === null) { return ''; }

        if ($p[0]->value !== 'A') {
            $this->error_->errorLine($line_nr, "$opcode の第1引数で使用できるのは, 'A' のみです", $p[0]->value);
        }
        if ($p[1]->type !== Param::TYPE_REG && $p[1]->type !== Param::TYPE_MEM && $p[1]->type !== Param::TYPE_VAL) {
            $this->error_->errorLine($line_nr, "$opcode の第2引数で使用できるのは, レジスタ|メモリ|値 のみです", $p[1]->value);
        }

        $p[1]->adjustVal();

        return "$opcode " . $p[0]->value . ', ' .  $p[1]->value;
    }


    // MARK: checkParam22_R_RMV_()
    /** 命令, 関数共に 2つの引数 (レジスタ, レジスタ|メモリ|値) を持ちます */
    private function checkParam22_R_RMV_(string $opcode, string $expr, int $line_nr): string
    {
        $p = $this->checkParamSub_($opcode, $expr, 2, $line_nr);
        if ($p === null) { return ''; }

        if ($p[0]->type !== 'reg') {
            $this->error_->errorLine($line_nr, "$opcode の第1引数で使用できるのは, レジスタのみです", $p[0]->value);
        }
        if ($p[1]->type !== Param::TYPE_REG && $p[1]->type !== Param::TYPE_MEM && $p[1]->type !== Param::TYPE_VAL) {
            $this->error_->errorLine($line_nr, "$opcode の第2引数で使用できるのは, レジスタ|メモリ|値 のみです", $p[1]->value);
        }

        $p[1]->adjustVal();

        return "$opcode " . $p[0]->value . ', ' .  $p[1]->value;
    }


    // MARK: checkParam22_V_RM_()
    /** 命令, 関数共に 2つの引数 (値, レジスタ|メモリ) を持ちます */
    private function checkParam22_V_RM_(string $opcode, string $expr, int $line_nr): string
    {
        $p = $this->checkParamSub_($opcode, $expr, 2, $line_nr);
        if ($p === null) { return ''; }

        if ($p[0]->type !== Param::TYPE_VAL) {
            $this->error_->errorLine($line_nr, "$opcode の第1引数で使用できるのは, 値 のみです", $p[0]->value);
        }
        if ($p[1]->type !== Param::TYPE_REG && $p[1]->type !== Param::TYPE_MEM) {
            $this->error_->errorLine($line_nr, "$opcode の第2引数で使用できるのは, レジスタ|メモリ のみです", $p[1]->value);
        }

        return "$opcode " . $p[0]->value . ', ' .  $p[1]->value;
    }


    // MARK: checkParam22_RM_R_()
    /** 命令, 関数共に 2つの引数 (レジスタ|メモリ, レジスタ) を持ちます */
    private function checkParam22_RM_R_(string $opcode, string $expr, int $line_nr): string
    {
        $p = $this->checkParamSub_($opcode, $expr, 2, $line_nr);
        if ($p === null) { return ''; }

        if ($p[0]->type !== Param::TYPE_REG && $p[0]->type !== Param::TYPE_MEM) {
            $this->error_->errorLine($line_nr, "$opcode の第1引数で使用できるのは, レジスタ|メモリ のみです", $p[0]->value);
        }
        if ($p[1]->type !== Param::TYPE_REG ) {
            $this->error_->errorLine($line_nr, "$opcode の第2引数で使用できるのは, レジスタ のみです", $p[1]->value);
        }

        return "$opcode " . $p[0]->value . ', ' .  $p[1]->value;
    }


    // MARK: checkParam20_A_M_()
    /** 命令, 関数共に 2つの引数 ('A', メモリ) を持ちます */
    private function checkParam20_A_M_(string $opcode, string $expr, int $line_nr): string
    {
        $p = $this->checkParamSub_($opcode, $expr, 2, $line_nr);
        if ($p === null) { return ''; }

        if ($p[0]->value !== 'A') {
            $this->error_->errorLine($line_nr, "$opcode の第1引数で使用できるのは, 'A' のみです", $p[0]->value);
        }
        if ($p[1]->type !== Param::TYPE_MEM ) {
            $this->error_->errorLine($line_nr, "$opcode の第2引数で使用できるのは, メモリ のみです", $p[1]->value);
        }

        return "$opcode";
    }


    // MARK: checkParam22_B_V_()
    /** 命令, 関数共に 2つの引数 ('B', 値) を持ちます */
    private function checkParam22_B_V_(string $opcode, string $expr, int $line_nr): string
    {
        $p = $this->checkParamSub_($opcode, $expr, 2, $line_nr);
        if ($p === null) { return ''; }

        if ($p[0]->value !== 'B') {
            $this->error_->errorLine($line_nr, "$opcode の第1引数で使用できるのは, 'B' のみです", $p[0]->value);
        }
        if ($p[1]->type !== Param::TYPE_VAL) {
            $this->error_->errorLine($line_nr, "$opcode の第2引数で使用できるのは, 値 のみです", $p[1]->value);
        }

        $p[1]->adjustVal();

        return "$opcode " . $p[0]->value . ', ' .  $p[1]->value;
    }


    // MARK: checkParam22_F_V_()
    /** 命令, 関数共に 2つの引数 (フラグ, 値) を持ちます */
    private function checkParam22_F_V_(string $opcode, string $expr, int $line_nr): string
    {
        $p = $this->checkParamSub_($opcode, $expr, 2, $line_nr);
        if ($p === null) { return ''; }

        if ($p[0]->type !== Param::TYPE_FLAG) {
            $this->error_->errorLine($line_nr, "$opcode の第1引数で使用できるのは, フラグ のみです", $p[0]->value);
        }
        if ($p[1]->type !== Param::TYPE_VAL) {
            $this->error_->errorLine($line_nr, "$opcode の第2引数で使用できるのは, 値 のみです", $p[1]->value);
        }

        $p[1]->adjustVal();

        return "$opcode " . $p[0]->value . ', ' .  $p[1]->value;
    }


    // MARK: checkParam22_M_R_()
    /** 命令, 関数共に 2つの引数 (メモリ, レジスタ) を持ちます */
    private function checkParam22_M_R_(string $opcode, string $expr, int $line_nr): string
    {
        $p = $this->checkParamSub_($opcode, $expr, 2, $line_nr);
        if ($p === null) { return ''; }

        if ($p[0]->type !== Param::TYPE_MEM) {
            $this->error_->errorLine($line_nr, "$opcode の第2引数で使用できるのは, メモリ のみです", $p[0]->value);
        }
        if ($p[1]->type !== Param::TYPE_REG) {
            $this->error_->errorLine($line_nr, "$opcode の第3引数で使用できるのは, レジスタ のみです", $p[1]->value);
        }

        return "$opcode " . $p[0]->value . ', ' .  $p[1]->value;
    }


    // MARK: checkParam33_V_M_R_()
    /** 命令, 関数共に 3つの引数 (値, メモリ, レジスタ) を持ちます */
    private function checkParam33_V_M_R_(string $opcode, string $expr, int $line_nr): string
    {
        $p = $this->checkParamSub_($opcode, $expr, 3, $line_nr);
        if ($p === null) { return ''; }

        if ($p[0]->type !== Param::TYPE_VAL) {
            $this->error_->errorLine($line_nr, "$opcode の第1引数で使用できるのは, 値 のみです", $p[0]->value);
        }
        if ($p[1]->type !== Param::TYPE_MEM) {
            $this->error_->errorLine($line_nr, "$opcode の第2引数で使用できるのは, メモリ のみです", $p[1]->value);
        }
        if ($p[2]->type !== Param::TYPE_REG) {
            $this->error_->errorLine($line_nr, "$opcode の第3引数で使用できるのは, レジスタ のみです", $p[2]->value);
        }

        $p[0]->adjustVal();

        return "$opcode " . $p[0]->value . ', ' .  $p[1]->value . ', ' .  $p[2]->value;
    }

    // MARK: checkParam1N_R_()
    /** 命令は引数1. 関数は1～n個の引数 (全てレジスタ) を持ちます  */
    private function checkParam1N_R_(string $opcode, string $expr, int $line_nr): string
    {
        $params = $this->explodeByComma_($expr);
        $b_err = false;
        $out = '';

        foreach($params as $i => $param) {
            $p = $this->checkExpr_($param, $line_nr);
            if ($p === null || $p->type !== Param::TYPE_REG) {
                $this->error_->errorLine($line_nr, "$opcode で使用できる引数は, レジスタ のみです", $p->value);
                $b_err = true;
            } else {
                if ($i !== 0) { $out .= $this->getIndentStr_(); }
                $out .= "$opcode " . $p->value;
                if ($i !== count($params) - 1) { $out .= "\n"; }
            }
        }
        return $b_err ? '' : $out;
    }


    // MARK: checkParamMN_S_()
    /** 命令は引数n. 関数もn個の引数 (全てシンボル値) を持ちます  */
    private function checkParamNN_S_(string $opcode, string $expr, int $line_nr): string
    {
        $params = $this->explodeByComma_($expr);
        $b_err = false;
        $out = $opcode . ' ';

        foreach($params as $i => $param) {
            $p = $this->checkExpr_($param, $line_nr);
            if ($p === null || !preg_match('/^' . Parser::SYMBOL_ . '$/x', $p->value)) {
                $this->error_->errorLine($line_nr, "$opcode で使用できる引数は, シンボル値 のみです", $p->value);
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
    private function checkParamNN_V_(string $opcode, string $expr, int $line_nr): string
    {
        $params = $this->explodeByComma_($expr);
        $b_err = false;
        $out = $opcode . ' ';

        foreach($params as $i => $param) {
            $p = $this->checkExpr_($param, $line_nr);
            if ($p === null || $p->type !== Param::TYPE_VAL) {
                $this->error_->errorLine($line_nr, "$opcode で使用できる引数は, 値 のみです", $p->value);
                $b_err = true;
            } else {
                $out .= $p->value;
                if ($i !== count($params) - 1) { $out .= ", "; }
            }
        }
        return $b_err ? '' : $out;
    }
}