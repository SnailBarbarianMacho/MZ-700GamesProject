<?php
/** Z80 代数表記(algebraic notation)アセンブリ言語 簡易フィルタ パーサー(構文解析)
 * @author Snail Barbarian Macho (NWK) 2024.06.06
 */

declare(strict_types = 1);
namespace nwk\z80ana;
require_once(__DIR__ . '/../utils/error.class.php');
require_once('z80ana-param.class.php');


// MARK: Parser
/** パーサー(構文解析) */
Class Parser
{
    private string  $mode_;         // モード(関数 or マクロ定義)
    private bool    $is_naked_;     // __naked が付いてるか
    private int     $line_nr_;      // ソースの行
    private string  $funcname_;     // 関数またはマクロ名
    private bool    $is_ended_;     // 末端に Z80ANA_ENDM, Z80ANA_NO_RETURN, Z80ANA_FALL_THROUGH を指定したかのフラグ

    private int     $label_ct_;
    private array   $labels_;

    private int     $indent_lv_;

    private \nwk\utils\Error $error_;

    // モードの値
    public  const MODE_FUNC  = 'func';  // 関数モード
    public  const MODE_MACRO = 'macro'; // マクロ定義モード

    private const EXPR_CHAR_ = '\w\+\-\*\/\%\&\|\^\~\(\)\<\>\,\.\ \t';  // 式を構成する文字 A-Za-z0-9_+-*%/()<>,. くらい?
    private const EXPR_LIST_ = '[' . Parser::EXPR_CHAR_ . '\n\[\]]*';   // 式リスト     (EXPR_CHAR_ に \n[] を追加. 0文字以上)
    private const EXPR_MEM_  = '[' . Parser::EXPR_CHAR_ . '\n\[\]]+';   // メモリ式     (EXPR_CHAR_ に \n[] を追加. 1文字以上)
    public  const EXPR_NMEM  = '[' . Parser::EXPR_CHAR_ . '\n]+';       // メモリ無し式 (EXPR_CHAR_ に \n   を追加. 1文字以上)
    private const EXPR_NLB_  = '[' . Parser::EXPR_CHAR_ . ']+';         // 改行無し式   (EXPR_CHAR_.                1文字以上)
    public  const SYMBOL    = '[A-Za-z_][A-Za-z0-9_]*';                // シンボル('fooBar2', 'foo_bar_2' など)
    private const BACK_REG_  = 'A_|B_|C_|D_|E_|H_|L_|AF_|BC_|DE_|HL_';
    public  const REG        = 'A|B|C|D|E|H|L|I|R|IXH|IXL|IYH|IYL|AF|BC|DE|HL|PC|SP|IX|IY|XH|XL|YH|YL|' . Parser::BACK_REG_;
    public  const FLAG_IF           = // if (<式>) の式の内容
        'z|eq|nz|ne|c|lt|nc|ge|p|m|v|nv|pe|po|' .
        'z_else_jr|eq_else_jr|nz_else_jr|ne_else_jr|c_else_jr|lt_else_jr|nc_else_jr|ge_else_jr|p_else_jr|m_else_jr|v_else_jr|nv_else_jr|pe_else_jr|po_else_jr|' .
        'z_jr|eq_jr|nz_jr|ne_jr|c_jr|lt_jr|nc_jr|ge_jr|' .
        'z_jr_else_jr|eq_jr_else_jr|nz_jr_else_jr|ne_jr_else_jr|c_jr_else_jr|lt_jr_else_jr|nc_jr_else_jr|ge_jr_else_jr';
    private const FLAG_IF1_         = // if (<式>) <文1> で, <文1> が「単文の goto/jp()/jr()/call()/ret()」の場合に使える <式> の内容
        'z|nz|c|nc|p|m|v|nv|pe|po';
    private const FLAG_DO_WHILE_    = // do <文> while(<式>) での式の内容
        'z|eq|nz|ne|c|lt|nc|ge|p|m|v|nv|pe|po|' .
        'z_jr|eq_jr|nz_jr|ne_jr|c_jr|lt_jr|nc_jr|ge_jr|' .
        'true|true_jr|false|B--';
    private const FLAG_WHILE_       = // while(<式>) <文> での式の内容
        'z|eq|nz|ne|c|lt|nc|ge|p|m|v|nv|pe|po|' .
        'z_jr|eq_jr|nz_jr|ne_jr|c_jr|lt_jr|nc_jr|ge_jr|' .
        'true';
    private const NOT_FLAG_         =  '(?!z,)(?!nz,)(?!c,)(?!nc,)(?!p,)(?!m,)(?!v,)(?!nv,)(?!pe,)(?!po,)(?!eq,)(?!ne,)(?!lt,)(?!ge,)';
    private const FLAG_CONV_TAB_ = array( // フラグを一般的なcc形式に
        'z'  => 'z',
        'eq' => 'z',
        'nz' => 'nz',
        'ne' => 'nz',
        'c'  => 'c',
        'lt' => 'c',
        'nc' => 'nc',
        'ge' => 'nc',
        'p'  => 'p',
        'm'  => 'm',
        'v'  => 'v',
        'nv' => 'nv',
        'pe' => 'pe',
        'po' => 'po',
        'z_else_jr'  => 'z',
        'eq_else_jr' => 'z',
        'nz_else_jr' => 'nz',
        'ne_else_jr' => 'nz',
        'c_else_jr'  => 'c',
        'lt_else_jr' => 'c',
        'nc_else_jr' => 'nc',
        'ge_else_jr' => 'nc',
        'p_else_jr'  => 'p',
        'm_else_jr'  => 'm',
        'v_else_jr'  => 'n',
        'nv_else_jr' => 'nv',
        'pe_else_jr' => 'pe',
        'po_else_jr' => 'po',
        'z_jr'  => 'z',
        'eq_jr' => 'z',
        'nz_jr' => 'nz',
        'ne_jr' => 'nz',
        'c_jr'  => 'c',
        'lt_jr' => 'c',
        'nc_jr' => 'nc',
        'ge_jr' => 'nc',
        'z_jr_else_jr'  => 'z',
        'eq_jr_else_jr' => 'z',
        'nz_jr_else_jr' => 'nz',
        'ne_jr_else_jr' => 'nz',
        'c_jr_else_jr'  => 'c',
        'lt_jr_else_jr' => 'c',
        'nc_jr_else_jr' => 'nc',
        'ge_jr_else_jr' => 'nc',
    );
    private const FLAG_NEG_TAB_ = array( // フラグを反転
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
    private const DO_WHILE_OP_TAB_ = // do <文> while (<式>) の式からループ末のジャンプ命令を決めます
        array(
            'z'     => 'jp z,',  'eq'    => 'jp z,',
            'nz'    => 'jp nz,', 'ne'    => 'jp nz,',
            'c'     => 'jp c,',  'lt'    => 'jp c,',
            'nc'    => 'jp nc,', 'ge'    => 'jp nc,',
            'p'     => 'jp p,',  'm'     => 'jp m,',
            'pe'    => 'jp pe,', 'po'    => 'jp po,',
            'z_jr'  => 'jr z,',  'eq_jr' => 'jr z',
            'nz_jr' => 'jr nz,', 'ne_jr' => 'jr nz',
            'c_jr'  => 'jr c,',  'lt_jr' => 'jr c,',
            'nc_jr' => 'jr nc,', 'ge_jr' => 'jr nc,',
            'true'  => 'jp',
            'true_jr' => 'jr',
            'false' => '',
            'B--'   => 'djnz B,',
        );
    private const WHILE_OP_TAB_ = // while (<式>) <文> の式からループ先頭のジャンプ命令を決めます
        array(
            'z'     => 'jp nz,', 'eq'    => 'jp nz,',
            'nz'    => 'jp z,',  'ne'    => 'jp z,',
            'c'     => 'jp nc,', 'lt'    => 'jp nc,',
            'nc'    => 'jp c,',  'ge'    => 'jp c,',
            'p'     => 'jp m,',  'm'     => 'jp p,',
            'pe'    => 'jp po,', 'po'    => 'jp pe,',
            'z_jr'  => 'jr nz,', 'eq_jr' => 'jr nz',
            'nz_jr' => 'jr z,',  'ne_jr' => 'jr z',
            'c_jr'  => 'jr nc,', 'lt_jr' => 'jr nc,',
            'nc_jr' => 'jr c,',  'ge_jr' => 'jr c,',
            'true'  => 'jp',
        );
    public const REGS_G16_SPLIT8_TAB_ = [ // AF, SP, PC を除いた汎用16bitレジスタと 8bit に分解したテーブル
        'HL' => ['L', 'H'],
        'DE' => ['E', 'D'],
        'BC' => ['C', 'B'],
        'IX' => ['IXL', 'IXH'],
        'IY' => ['IYL', 'IYH'],
        'HL_' => ['L', 'H'],    // 裏レジスタに分解する必要はないでしょう
        'DE_' => ['E', 'D'],
        'BC_' => ['C', 'B'],
        'IX_' => ['IXL', 'IXH'],
        'IY_' => ['IYL', 'IYH'],
    ];

    // MARK: __construct()
    /** コンストラクタ */
    public function __construct(\nwk\utils\Error $error) {
        $this->error_ = $error;
    }


    // -------------------------------- パース
    // MARK: parse()
    /** パースして変換後の文字列を返します
     * @param $mode      モード
     * @param $is_naked  __naked 関数か
     * @param $r_str     解析したい文字列
     * @param $line_nr   $str のある行(0～, エラー表示用)
     * @param $funcname  関数名(ラベル表示用)
     */
    public function parse(string $mode, bool $is_naked, string &$r_str, int $line_nr, string $funcname): string
    {
        $this->line_nr_   = $line_nr;

        if ($mode == Parser::MODE_MACRO && !preg_match('/^[A-Z_][A-Z_0-9]+$/', $funcname)) {
            $this->errorLine_("マクロ名は, [A-Z_0-9] で書いてください", $funcname);
        }

        $this->mode_      = $mode;
        $this->is_naked_  = $is_naked;
        $this->funcname_  = $funcname;
        $this->is_ended_  = false;
        $this->initLabels_();
        $this->initIndent_();

        $out = $this->parseSentences_($r_str, null, null);

        if (!$this->is_ended_) {
            if ($this->mode_ === Parser::MODE_MACRO) {
                $this->errorLine_("マクロ定義の末端には, Z80ANA_ENDM; を追加してくだい");
            } else if ($this->is_naked_) {
                $this->errorLine_("__naked 関数の末端には, Z80ANA_NO_RETURN; または Z80ANA_FALL_THROUGH; を追加してくだい");
            }
        }

        return $out;
    }


    // -------------------------------- パース サブ関数
    // MARK: parseSentences_()
    /** 複文の解析 */
    private function parseSentences_(string &$r_str, ?string $continue, ?string $break)
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
                $line_str = $this->getIndentStr_() . "// line " . ($this->line_nr_ + 1) . "\n";
            }

            $str = $this->parseSentence_($r_str, $offset, $continue, $break);
            if ($str !== '') {
                $out .= $line_str;
            }
            $out .= $str;
        }
        $this->decIndent_();

        return $out;
    }


    // MARK: parseSentence_()
    /** 単文の解析
     * @param $r_str    [in]ソース文字列
     * @param $r_offset [in][out]ソース文字列の位置
     * @param $continue continue 時のジャンプ先 (null可)
     * @param $break    break 時のジャンプ先 (null可)
     * @returns 解析出力
     */
    private function parseSentence_(string &$r_str, int &$r_offset, ?string $continue, ?string $break): string
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
            return $this->parseSubSentences_($r_str, $r_offset, $continue, $break);
        }

        // goto <式>;
        if (preg_match('/(goto) \s* (\w+) \s* \;/Ax', $r_str, $matches, 0, $r_offset)) {
            return $this->parseGoto_($r_str, $r_offset, $matches[0], $matches[1], $matches[2]);
        }

        // extern <式>[, ...];
        if (preg_match('/(extern) \s* ([\w\s\,]+) \s* \;/Ax', $r_str, $matches, 0, $r_offset)) {
            return $this->parseExtern_($r_str, $r_offset, $matches[0], $matches[1], $matches[2]);
        }

        // if (<式>) <文> else <文> ※式はフラグのみ
        if (preg_match('/if \s* \( \s* (' . Parser::FLAG_IF . ') \s* \) /Ax', $r_str, $matches, 0, $r_offset)) {
            return $this->parseIf_($r_str, $r_offset, $matches[0], $matches[1], $continue, $break);
        }

        // do <文> while (<式>); ※式はフラグ, 'true', 'B--'のみ
        if (preg_match('/do [\w\s,]+ /Ax', $r_str, $matches, 0, $r_offset)) {
            return $this->parseDoWhile_($r_str, $r_offset);
        }

        // while(<式>) <文> ※式はフラグ, 'true' のみ
        if (preg_match('/while \s* \( \s* (' . Parser::FLAG_WHILE_ . ') \s* \)/Ax', $r_str, $matches, 0, $r_offset)) {
            return $this->parseWhile_($r_str, $r_offset, $matches[0], $matches[1]);
        }

        // break;
        if (preg_match('/break \s* ;/Ax', $r_str, $matches, 0, $r_offset)) {
            return $this->parseBreak_($r_str, $r_offset, $matches[0], $break);
        }

        // continue;
        if (preg_match('/continue \s* ;/Ax', $r_str, $matches, 0, $r_offset)) {
            return $this->parseContinue_($r_str, $r_offset, $matches[0], $continue);
        }

        // return;  ※式なし
        if (preg_match('/(return) \s*(\w*)\s*\;/Ax', $r_str, $matches, 0, $r_offset)) {
            if (!$matches[2]) {
                return $this->parseReturn_($r_str, $r_offset, $matches[0]);
            }
        }

        // シンボル名:
        if (preg_match('/(' . Parser::SYMBOL . '\:)/Ax', $r_str, $matches, 0, $r_offset)) {
            return $this->parseLabel_($r_str, $r_offset, $matches[1]);
        }

        // <式> <代入演算子> <式>;
        if (preg_match('/(' . Parser::EXPR_MEM_ . ') \= (' . Parser::EXPR_MEM_ . ')\;/Ax', $r_str, $matches, 0, $r_offset)) {
            return $this->parseAssignmentOp_($r_str, $r_offset, $matches[0], $matches[1], $matches[2]);
        }

        // <単項演算子> <reg>;
        if (preg_match('/(\+\+|\-\-) \s* (' . Parser::EXPR_MEM_ . ') \s*;/Ax', $r_str, $matches, 0, $r_offset)) {
            return $this->parseUnaryOp_($r_str, $r_offset, $matches[0], $matches[1], $matches[2]);
        }

        // <reg> <単項演算子>;
        if (preg_match('/(' . Parser::EXPR_MEM_ . ') (\+\+|\-\-) \s*\;/Ax', $r_str, $matches, 0, $r_offset)) {
            return $this->parseUnaryOp_($r_str, $r_offset, $matches[0], $matches[2], $matches[1]);
        }

        // 関数呼び出し シンボル名(<式リスト>);
        if (preg_match('/(\w+)\s*\((' . Parser::EXPR_LIST_ . ')\)\s*;/Ax', $r_str, $matches, 0, $r_offset)) {
            return $this->parseFunctionCall_($r_str, $r_offset, $matches[0], $matches[1], $matches[2]);
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
    private function parseSubSentences_(string &$r_str, int &$r_offset, ?string $continue, ?string $break): string
    {
        //echo("subSentences1[$r_offset $this->line_nr_] $r_str[$r_offset]\n");
        // { ... } を検出
        $line_nr = $this->line_nr_;
        $str = $this->extractCurlyBracket_($r_str, $r_offset);
        $this->line_nr_ = $line_nr;
        if ($str == false) {
            return '';
        }

        $out = $this->parseSentences_($str, $continue, $break);
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
            $p = $this->createParamObject_($param);
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
    /** if (flag) <文1> else <文2> */
    private function parseIf_(string &$r_str, int &$r_offset, $match, $flag, ?string $continue, ?string $break): string
    {
        $r_offset  += strlen($match);
        $this->line_nr_ += substr_count($match, "\n");

        // flag から, ジャンプ命令とラベルの設定
        $flag_neg = Parser::FLAG_NEG_TAB_[$flag];
        $opcode_if   = 'jp';
        $opcode_else = 'jp';

        if (str_ends_with($flag, '_jr_else_jr') ||
            (str_ends_with($flag, '_jr') && !str_ends_with($flag, '_else_jr'))) {
            $opcode_if = 'jr';
        }
        if (str_ends_with($flag, '_else_jr')) {
            $opcode_else = 'jr';
        }

        if ($flag_neg !== 'false') { $flag_neg .= ','; }

        $label_else = '';
        $label_endif = $this->genRegistLabel_();
        $this->skipSpaces_($r_str, $r_offset);

        // <文1> の処理
        $this->incIndent_();
        $out_sentence1 = $this->parseSentence_($r_str, $r_offset, $continue, $break);
        $this->decIndent_();

        // <文1> が「単文の jp/jr/call/ret 命令」になってる場合, 1命令に最適化されます.
        // ※if (cc) break; 等も最適化されます
        if (preg_match('/^([ \t]*)(jp|jr|call)([ \t]+)(' . Parser::EXPR_NLB_ . ')(\r?\n)$/Ax', $out_sentence1, $matches) &&
            !preg_match('/^(' . Parser::FLAG_IF1_ . '),/', $matches[4])) { // フラグのない jp/jr/call 命令であること
            $opcode_1 = $matches[2];   // <文1> の命令 (jp|jr|call)
            $addr     = $matches[4];   // <文1> ジャンプ/コール先
            if ($opcode_if === 'jr') {
                if ($opcode_1 === 'call') {
                    $this->errorLine_("if 節の 1命令 call 文では, if (式) に jr は使えません");
                }
                $opcode_1 = 'jr';
            }
            $out = $this->getIndentStr_() . $opcode_1 . ' ' . Parser::FLAG_CONV_TAB_[$flag] . ', ' . $matches[4] . "\n";
            $opcode_else = '';  // 最適化したフラグ兼用
        } else if (preg_match('/^([ \t]*)(ret)([ \t]*)(\r?\n)$/Ax', $out_sentence1, $matches)) {
            if ($opcode_if === 'jr') {
                $this->errorLine_("if 節の 1命令 ret 文では, if (式) に jr は使えません");
            }
            $out = $this->getIndentStr_() . 'ret ' . Parser::FLAG_CONV_TAB_[$flag] . "\n";
            $opcode_else = '';  // 最適化したフラグ兼用
        }
        $this->skipSpaces_($r_str, $r_offset);

        if (!preg_match('/else/Ax', $r_str, $matches, 0, $r_offset)) { // if のみ
            // if () <文1> 節の後のラベル. if が最適化された場合はラベル無し
            if ($opcode_else !== '') {
                $out = $this->getIndentStr_() . "$opcode_if $flag_neg $label_endif // if ($flag) {\n";  // if <!flag> goto endif;
                $out .= $out_sentence1;                                                 // <文1>;
                $out .= $this->getIndentStr_() . "$label_endif: // } endif \n";
            }
        } else { // else <文2> 処理
            $r_offset  += strlen('else');

            if ($opcode_else !== '') { // if 節が最適化された場合は, else 前の jp/jr は不要
                $label_else = $this->genRegistLabel_();
                $out = $this->getIndentStr_() . "$opcode_if $flag_neg $label_else // if ($flag) {\n";   // if <!flag> goto else;
                $out .= $out_sentence1;                                                 // <文1>;
                $out .= $this->getIndentStr_() . "$opcode_else $label_endif\n";         // goto endif;
                $out .= $this->getIndentStr_() . "$label_else: // } else {\n";
            }

            // <文2> の処理
            $this->skipSpaces_($r_str, $r_offset);
            $this->incIndent_();
            $out .= $this->parseSentence_($r_str, $r_offset, $continue, $break);
            $this->decIndent_();
            $this->skipSpaces_($r_str, $r_offset);

            // else <文2> 節の後のラベル. if が最適化された場合はラベル無し
            if ($opcode_else !== '') {
                $out .= "$label_endif: // } endif\n";
            }
        }
        // 後処理の最適化
        //    xxxxx:\n  jp label_endif
        // となってるコードを探し, xxxxx にジャンプしてるコードを label_endif に書き換えます
        if (preg_match_all('/\n[ \t]*(' . Parser::SYMBOL . '):\n[ \t]*(jp|jr)[ \t]*' . $label_endif . '/', $out, $matches)) {
            if ($matches) {
                //echo("else:$label_else endif:$label_endif\n");
                for ($i = 0; $i < count($matches[0]); $i++) {
                    $label_org = $matches[1][$i]; // 元のジャンプ先
                    //echo("最適化[". $matches[0][$i] . "] $label_org -> $label_endif\n");
                    $out = preg_replace('/('. $label_org . ')([^:])/', $label_endif . ' // optimized from ' . $label_org . '$2' , $out, -1, $count);
                    if ($count !== 0) {
                        echo("note: ネストする if - else 節の多重ジャンプが最適化されました: $label_org -> $label_endif, $count 箇所\n");
                    }
                }
            }
        }

        return $out;
    }

    // MARK: parseDoWhile_()
    /** do <文> while (<式>); */
    private function parseDoWhile_(string &$r_str, int &$r_offset): string
    {
        $r_offset += strlen('do');

        $label_loop_top = $this->genRegistLabel_(); // continue のジャンプ先
        $label_loop_end = $this->genRegistLabel_(); // break のジャンプ先

        // <文>の処理
        $this->skipSpaces_($r_str, $r_offset);
        $this->incIndent_();
        $out_sentence = $this->parseSentence_($r_str, $r_offset, $label_loop_top, $label_loop_end);
        $this->decIndent_();
        $this->skipSpaces_($r_str, $r_offset);

        // while() の処理
        if (!preg_match('/while \s* \( \s* (' . Parser::FLAG_DO_WHILE_ . ') \s* \) \s* ;/Ax', $r_str, $matches, 0, $r_offset)) {
            $this->errorLine_("do に対する while 句が無いか, あってもカッコの中が間違ってます");
            return '';
        }

        $flag = $matches[1];
        $loop_end_inst = '';    // while (false) の場合, ループ末端のジャンプ命令は省く
        $loop_end_op = Parser::DO_WHILE_OP_TAB_[$flag];
        if ($loop_end_op !== '') {
            $loop_end_inst = $loop_end_op . ' ' . $label_loop_top;
        }

        $out = $this->getIndentStr_() . $label_loop_top . ": // do {\n" .
            $out_sentence .
            $this->getIndentStr_() . $loop_end_inst . " // } while ($flag)\n" .
                $this->getIndentStr_() . $label_loop_end . ": // loop end\n";

            $r_offset += strlen($matches[0]);

        return $out;
    }


    /** while(<式>) <文> ※式はフラグ, 'true' のみ */
    private function parseWhile_(string &$r_str, int &$r_offset, string $match, string $flag): string
    {
        $r_offset += strlen($match);

        $label_loop_top = $this->genRegistLabel_(); // continue のジャンプ先
        $label_loop_end = $this->genRegistLabel_(); // break のジャンプ先

        $this->skipSpaces_($r_str, $r_offset);
        $this->incIndent_();
        $out_sentence = $this->parseSentence_($r_str, $r_offset, $label_loop_top, $label_loop_end);
        $this->decIndent_();
        $this->skipSpaces_($r_str, $r_offset);

        $loop_top_op = '';
        if ($flag !== 'true') {
            $loop_top_op = Parser::WHILE_OP_TAB_[$flag] . ' ' . $label_loop_end;
        }

        $out = $label_loop_top . ": // while ($flag) {\n" .
            $loop_top_op .
            $out_sentence .
            $this->getIndentStr_() . 'jp ' . $label_loop_top . " // }\n" .
            $label_loop_end . ": // loop end\n";

        return $out;
    }


    // MARK: parseBreak_()
    private function parseBreak_(string &$r_str, int &$r_offset, string $match, ?string $break): string
    {
        $out = '';

        if (gettype($break) !== 'string') {
            $this->errorLine_("ループの外では break できません", '');
        } else {
            $out = $this->getIndentStr_() . "jp $break // break\n";
        }

        $r_offset += strlen($match);
        $this->skipSpaces_($r_str, $r_offset);

        return $out;
    }


    // MARK: parseContinue_()
    private function parseContinue_(string &$r_str, int &$r_offset, string $match, ?string $continue): string
    {
        $out = '';

        if (gettype($continue) !== 'string') {
            $this->errorLine_("ループの外では continue できません", '');
        } else {
            $out = $this->getIndentStr_() . "jp $continue // continue\n";
        }

        $r_offset += strlen($match);
        $this->skipSpaces_($r_str, $r_offset);

        return $out;
    }


    // MARK: parseReturn_()
    /** return;※<式なし>; */
    private function parseReturn_(string &$r_str, int &$r_offset, string $match): string
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
        $l_param = $this->createParamObject_($l_value);
        $r_param = $this->createParamObject_($r_value);
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
    private function parseFunctionCall_(string &$r_str, int &$r_offset, string $match, string $funcname, string $expr): string
    {
        //echo("func1[$r_offset $this->line_nr_][$funcname($expr)]\n");

        if (preg_match('/^[A-Z_][A-Z_0-9]*$/', $funcname)) {
            // 全て大文字ならばマクロの展開とみなす(例 ABC() )
            $expr = str_replace("\n", ' ', $expr);// マクロ呼び出しは1行で書かないといけないので改行を削除
            $params = $this->explodeByComma_($expr);
            // ヘタに加工するとマクロ展開がおかしくなるので, パラメータはそのまま渡す
            $ret = $funcname . ' ';
            foreach($params as $i => $param) {
                $ret .= $this->stripBackReg_($param);
                //echo("{$ret} [{$param}]\n");
                if ($i !== count($params) - 1) { $ret .= ", "; }
            }
        } else if (preg_match('/^[a-z_][a-z_0-9]*$/', $funcname) &&
            method_exists($this, $funcname . '_')) {
            // 全て小文字のアセンブラの命令ならば(例: ldir() ), 同名の関数 (例: ldir_()) を呼び出し
            $ret = $this->{$funcname . '_'}($expr, $this->line_nr_);
        } else { // それ以外
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
        if ($this->mode_ !== Parser::MODE_MACRO) {
            $this->errorLine_("Z80ANA_ENDM は, マクロ用定義用です");
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
        if ($this->mode_ !== Parser::MODE_FUNC) {
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
        if ($this->mode_ !== Parser::MODE_FUNC) {
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
                        $ret[] = trim(substr($expr, $j, $i - $j));
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
            $ret[] = trim(substr($expr, $j));
        }

        //echo("[$expr] --> "); print_r($ret);
        //return explode(',', $expr);
        return $ret;
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
    /** 文字列から整数へ. 10/16進数対応. 失敗したら false */
    private function str2int_($str): int | bool
    {
        if (str_starts_with($str, '0x')) {
            $str = substr($str, 2);
            if (ctype_xdigit($str)) { return hexdec($str); }
        } else {
            if (ctype_digit($str)) { return (int)$str;}
        }

        return false;
    }


    // ---------------- 式
    // MARK: createParamObject_()
    /** 式から, パラメータ オブジェクトを生成します
     * <expr> := mem[<expr1>]
     *         | mem[<expr1>] + c
     *         | port[<expr1>]
     *         | <reg>
     *         | <reg> + c
     *         | reg_<reg>
     *         | reg_<reg> + c
     *         | 値
     *         | 値 + c
     *         | <expr1>
     * <expr1> := 面倒なので, Parser::EXPR_NMEM で指定した文字種
     * モードによって挙動が異なります
     * @return エラーなら null
     */
    private function createParamObject_(string $expr): Param|null
    {
        $param = new Param($expr, $this->mode_);
        if ($param->type === Param::TYPE_OTHER) {
            if ($this->mode_ === Parser::MODE_MACRO) {
                $this->errorLine_("未知の式です. reg_ で始まる引数名は, マクロ定義/呼び出し用です", $expr);
            } else {
                $this->errorLine_("未知の式です", $expr);
            }
            return null;
        }
        return $param;
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

    // MARK: stripBackReg_()
    /** 裏レジスタ表記を通常表記に変換して返します. それ以外は何もしません */
    private function stripBackReg_(string $value): string
    {
        if (preg_match('/^(?:' . Parser::BACK_REG_ . ')$/x', $value)) {
            //echo("stripBackReg_: [$value] --> " . trim($value, '_') . "\n");
            return trim($value, '_'); // 最後の '_' を削る
        }
        return $value;
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
    private function Z80ANA_GLOBAL_(string $expr): string { return $this->checkParamNN_S_('global', $expr); }

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
            $p = $this->createParamObject_($param);
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
        if ($expr !== 'A' && $expr !== 'A_') {
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
    /** 命令は引数無しですが, 関数は 2 つの引数 ('A', 値) を持ちます. 例: rrca(a, n) -> rrca を n 個 */
    private function checkParam02_AV_(string $opcode, string $expr): string
    {
        $p = $this->checkParamSub_($opcode, $expr, 2);
        if ($p === null) { return ''; }

        if ($p[0]->value !== 'A' && $p[0]->value !== 'A_') {
            $this->errorLine_("$opcode の第1引数で使用できるのは, A のみです", $expr);
            return '';
        }
        if ($p[1]->type !== Param::TYPE_VAL) {
            $this->errorLine_("$opcode の第2引数で使用できるのは, 値 のみです", $expr);
            return '';
        }

        // 値が数値ならば直接展開(8以上はエラー), そうでなければ rept
        $out = '';
        //echo($p[1]->value . "\n");
        $n = $this->str2int_($p[1]->value);
        if ($n !== false) { // 数値
            if (8 < $n) {
                $this->errorLine_("$opcode の第2引数値が大きすぎます", $expr);
            } else {
                for ($i = 0; $i < $n; $i++) {
                    if ($i != 0) { $out .= $this->getIndentStr_(); }
                    $out .= $opcode;
                    if ($i != $n - 1) { $out .= "\n"; }
                }
            }
        } else { // 数値以外
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

        return "$opcode " . $this->stripBackReg_($params[0]);
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
            return "$opcode " . $this->stripBackReg_($p[0]->value);
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
                $out = "$opcode " . $this->stripBackReg_($p[0]->value);
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
                ($p[0]->value !== 'HL' && $p[0]->value !== 'HL_' &&
                $p[0]->value !== 'IX' && $p[0]->value !== 'IY'))) {
            $this->errorLine_("$opcode の引数は, 値 HL|IX|IY のみです", $expr);
            return '';
        }

        $p[0]->adjustVal();

        if ($p[0]->type === Param::TYPE_REG) {
            $p[0]->value = '(' . $p[0]->value . ')';
        }

        return "$opcode " . $this->stripBackReg_($p[0]->value);
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

        // 値が数値ならば直接展開(8以上はエラー), そうでなければ rept
        $out = '';
        //echo($p[1]->value . "\n");
        $n = $this->str2int_($p[1]->value);
        if ($n !== false) { // 数値
            if (8 < $n) {
                $this->errorLine_("$opcode の第2引数値が大きすぎます", $expr);
            } else {
                for ($i = 0; $i < $n; $i++) {
                    if ($i != 0) { $out .= $this->getIndentStr_(); }
                    $out .= "$opcode " . $p[0]->value;
                    if ($i != $n - 1) { $out .= "\n"; }
                }
            }
        } else { // 数値以外
            $out .= "rept " . $p[1]->value . "\n";
            $this->incIndent_();
            $out .= $this->getIndentStr_() . "$opcode " . $this->stripBackReg_($p[0]->value) . "\n";
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

        return "$opcode " . $this->stripBackReg_($expr);
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

        return "$opcode " . $this->stripBackReg_($p[0]->value) . ', ' .  $this->stripBackReg_($p[1]->value);
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

        return "$opcode " . $this->stripBackReg_($p[0]->value) . ', ' .  $p[1]->value;
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

        return "$opcode " . $this->stripBackReg_($p[0]->value) . ', ' .  $p[1]->value;
    }

    // MARK: checkParam22_A_RM_()
    /** 命令, 関数共に 2つの引数 ('A', 二番目はレジスタ|メモリ|値) を持ちます */
    private function checkParam22_A_RM_(string $opcode, string $expr): string
    {
        $p = $this->checkParamSub_($opcode, $expr, 2);
        if ($p === null) { return ''; }

        if ($p[0]->value !== 'A' && $p[0]->value !== 'A_') {
            $this->errorLine_("$opcode の第1引数で使用できるのは, 'A' のみです", $p[0]->value);
        }
        if ($p[1]->type !== Param::TYPE_REG && $p[1]->type !== Param::TYPE_MEM && $p[1]->type !== Param::TYPE_VAL) {
            $this->errorLine_("$opcode の第2引数で使用できるのは, レジスタ|メモリ|値 のみです", $p[1]->value);
        }
        $p[1]->adjustVal();

        return "$opcode " . $this->stripBackReg_($p[0]->value) . ', ' .  $this->stripBackReg_($p[1]->value);
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

        return "$opcode " . $this->stripBackReg_($p[0]->value) . ', ' .  $this->stripBackReg_($p[1]->value);
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

        return "$opcode " . $p[0]->value . ', ' .   $this->stripBackReg_($p[1]->value);
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

        return "$opcode " . $this->stripBackReg_($p[0]->value) . ', ' .  $this->stripBackReg_($p[1]->value);
    }


    // MARK: checkParam20_A_M_()
    /** 命令, 関数共に 2つの引数 ('A', メモリ) を持ちます */
    private function checkParam20_A_M_(string $opcode, string $expr): string
    {
        $p = $this->checkParamSub_($opcode, $expr, 2);
        if ($p === null) { return ''; }

        if ($p[0]->value !== 'A' && $p[0]->value !== 'A_') {
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

        if ($p[0]->value !== 'B' && $p[0]->value !== 'B_') {
            $this->errorLine_("$opcode の第1引数で使用できるのは, 'B' のみです", $p[0]->value);
        }
        if ($p[1]->type !== Param::TYPE_VAL) {
            $this->errorLine_("$opcode の第2引数で使用できるのは, 値 のみです", $p[1]->value);
        }

        $p[1]->adjustVal();

        return "$opcode " . $this->stripBackReg_($p[0]->value) . ', ' .  $p[1]->value;
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

        return "$opcode " . $p[0]->value . ', ' .  $this->stripBackReg_($p[1]->value);
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

        return "$opcode " . $p[0]->value . ', ' .  $p[1]->value . ', ' .  $this->stripBackReg_($p[2]->value);
    }


    // MARK: checkParam1N_R_()
    /** 命令は引数1. 関数は1～n個の引数 (全てレジスタ) を持ちます  */
    private function checkParam1N_R_(string $opcode, string $expr): string
    {
        $params = $this->explodeByComma_($expr);
        $b_err = false;
        $out = '';

        foreach($params as $i => $param) {
            $p = $this->createParamObject_($param);
            if ($p === null || $p->type !== Param::TYPE_REG) {
                $this->errorLine_("$opcode で使用できる引数は, レジスタ のみです", $p->value);
                $b_err = true;
            } else {
                if ($i !== 0) { $out .= $this->getIndentStr_(); }
                $out .= "$opcode " . $this->stripBackReg_($p->value);
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
        $out = $opcode . ' ';
        foreach($params as $i => $param) {
            $out .= $this->stripBackReg_($param);
            if ($i !== count($params) - 1) { $out .= ", "; }
        }
        return $out;
    }


    // MARK: checkParamNN_S_()
    /** 命令は引数n. 関数もn個の引数 (全てシンボル値) を持ちます  */
    private function checkParamNN_S_(string $opcode, string $expr): string
    {
        $params = $this->explodeByComma_($expr);
        $b_err = false;
        $out = $opcode . ' ';

        foreach($params as $i => $param) {
            $p = $this->createParamObject_($param);
            if ($p === null || !preg_match('/^' . Parser::SYMBOL . '$/x', $p->value)) {
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
            $p = $this->createParamObject_($param);
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
