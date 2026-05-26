<?php
/** Z80 代数アセンブリ言語 (Algebraic Assembly Language) フィルタ - パーサー(構文解析)
 * @author Snail Barbarian Macho (NWK) 2024.06.06
 */

declare(strict_types = 1);
namespace nwk\aal;
require_once(__DIR__ . '/../utils/error.class.php');
require_once('aal80-operand.class.php');


// MARK: Parser
/** パーサー(構文解析) */
Class Parser
{
    private string  $mode_;         // モード(関数 or マクロ定義)
    private bool    $is_naked_;     // __naked が付いてるか
    private int     $line_nr_;      // ソースの行
    private string  $funcname_;     // 関数またはマクロ名
    private bool    $is_ended_;     // 末端に AAL_ENDM, AAL_NO_RETURN, AAL_FALL_THROUGH を指定したかのフラグ

    private int     $label_ct_;
    private array   $labels_;       //

    private int     $indent_lv_;

    private \nwk\utils\Error $error_;

    // モードの値
    public  const MODE_FUNC  = 'func';  // 関数モード
    public  const MODE_MACRO = 'macro'; // マクロ定義モード

    private const EXPR_CHAR_ = '\w\!\?\=\+\-\*\/\%\&\|\^\~\(\)\<\>\,\.\ \t';  // 式を構成する文字 A-Za-z0-9!?=_+-*%/()<>,. くらい?
    private const EXPR_LIST_ = '[' . Parser::EXPR_CHAR_ . '\n\[\]]*';   // 式リスト     (EXPR_CHAR_ に \n[] を追加. 0文字以上)
    private const EXPR_MEM_  = '[' . Parser::EXPR_CHAR_ . '\n\[\]]+';   // メモリ式     (EXPR_CHAR_ に \n[] を追加. 1文字以上)
    public  const EXPR_NMEM  = '[' . Parser::EXPR_CHAR_ . '\n]+';       // メモリ無し式 (EXPR_CHAR_ に \n   を追加. 1文字以上)
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
    private const FLAG_NEG_CONV_TAB_ = array( // FLAG_CONV_TAB のフラグを反転したもの
        ''   => 'false',
        'z'  => 'nz',
        'nz' => 'z',
        'c'  => 'nc',
        'nc' => 'c',
        'p'  => 'm',
        'm'  => 'p',
        'v'  => 'nv',
        'nv' => 'v',
        'pe' => 'po',
        'po' => 'pe',
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
            'false' => '',
            'B--'   => 'djnz B,',
            '--B'   => 'djnz B,',
            'B_--'  => 'djnz B,',
            '--B_'  => 'djnz B,',
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
                $this->errorLine_("マクロ定義の末端には, AAL_ENDM; を追加してくだい");
            } else if ($this->is_naked_) {
                $this->errorLine_("__naked 関数の末端には, AAL_NO_RETURN; または AAL_FALL_THROUGH; を追加してくだい");
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

        // 'int xxxx, yyyy;' は無視
        if (preg_match('/(int\s+[\w,\s]+)/Ax', $r_str, $matches, 0, $r_offset)) {
            return $this->parseInt_($r_str, $r_offset, $matches[1]);
        }

        // 'AAL_' で始まる指示語
        if (preg_match('/(AAL_[A-Z_]+)/Ax', $r_str, $matches, 0, $r_offset)) {
            return $this->parseZ80aalDirective_($r_str, $r_offset, $matches[1]);
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
        //if (preg_match('/(goto) \s* (.+)\;/Ax', $r_str, $matches, 0, $r_offset)) {
        //    return $this->parseGoto_($r_str, $r_offset, $matches[0], $matches[1], $matches[2]);
        //}

        // extern <式>[, ...];
        if (preg_match('/(extern) \s* ([\w\s\,]+) \s* \;/Ax', $r_str, $matches, 0, $r_offset)) {
            return $this->parseExtern_($r_str, $r_offset, $matches[0], $matches[1], $matches[2]);
        }

        // if (<式>) <文> else <文>
        if (preg_match('/if \s* (\( (?:[^()]+ | (?1))* \))/Ax', $r_str, $matches, 0, $r_offset)) {
            return $this->parseIf_($r_str, $r_offset, $matches[0], $matches[1], $continue, $break);
        }

        // do <文> while (<式>);
        if (preg_match('/do [\w\s,]+ /Ax', $r_str, $matches, 0, $r_offset)) {
            return $this->parseDoWhile_($r_str, $r_offset);
        }

        // while (<式>) <文>
        if (preg_match('/while \s* (\( (?:[^()]+ | (?1))* \))/Ax', $r_str, $matches, 0, $r_offset)) {
            // ↑正規表現の説明
            // a. (?: ... )* は, カッコの中身を0回以上繰り返す
            // b. [^()]*     は, カッコ以外の文字0文字以上にマッチ
            // c. |          または
            // d. (?1)       再帰的に a. のカッコを再帰します
            // e. ... /Ax    Aは最初にマッチ, x は正規表現の切れ目をスペースで分かりやすく見せる
            // 'while (abc(de))' は, '(abc(de))' にマッチします. 両側のカッコを取り除いて使ってください
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
        if (preg_match('/(' . Operand::SYMBOL . '\:)/Ax', $r_str, $matches, 0, $r_offset)) {
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

    // MARK: parseInt_()
    /** 'int xxxxx' のパース */
    private function parseInt_(string &$r_str, int &$r_offset, string $match): string
    {
        $r_offset  += strlen($match);
        return '';
    }

    // MARK: parseZ80aalDirective_()
    /** 'AAL_' で始まる指示語. インデントなし */
    private function parseZ80aalDirective_(string &$r_str, int &$r_offset, string $directive): string
    {
        // echo("directive1[$r_offset $this->line_nr_][$directive]\n");
        $offset_old = $r_offset;
        $r_offset += strlen($directive);
        $ret  = '';

        // カッコの無いディレクティブ
        switch ($directive) {
            case 'AAL_ELSE':
                $this->decIndent_();
                $ret = $this->getIndentStr_() . "else\n";
                $this->incIndent_();
                return $ret;
            case 'AAL_ENDIF':
                $this->decIndent_();
                return $this->getIndentStr_() . "endif\n";
            case 'AAL_ENDR':
                $this->decIndent_();
                return $this->getIndentStr_() . "endr\n";
            case 'AAL_DEF_VARS':
                $this->defVarsDirective_($r_str, $offset_old);
                return '';
            case 'AAL_ENDM':
                $this->endmDirective_($r_str, $r_offset);
                return '';
            case 'AAL_NO_RETURN':
                $this->noReturnDirective_($r_str, $r_offset);
                return '';
            case 'AAL_FALL_THROUGH':
                $this->fallThroughDirective_($r_str, $r_offset);
                return '';
        }

        // 引数のあるディレク底部
        $args = $this->extractParentheses_($r_str, $r_offset);
        switch ($directive) {
            case 'AAL_DEF_DUMMY_VARS':
                return "\n";
            case 'AAL_LOCAL':
                return $this->getIndentStr_() . $this->parseLocalGlobal_('local', $args) . "\n";
            case 'AAL_GLOBAL':
                return $this->getIndentStr_() . $this->parseLocalGlobal_('global', $args) . "\n";
            case 'AAL_DB':
                return $this->getIndentStr_() . $this->parseDb_($args) . "\n";
            case 'AAL_DW':
                return $this->getIndentStr_() . $this->parseDw_($args) . "\n";
            case 'AAL_DS':
                return $this->getIndentStr_() . $this->parseDs_($args) . "\n";
            case 'AAL_IF':
                $ret = $this->getIndentStr_() . $this->parseIfRept_('if', $args) . "\n";
                $this->incIndent_();
                return $ret;
            case 'AAL_ELIF':
                $this->decIndent_();
                $ret = $this->getIndentStr_() . $this->parseIfRept_('elif', $args) . "\n";
                $this->incIndent_();
                return $ret;
            case 'AAL_REPT':
                $ret = $this->getIndentStr_() . $this->parseIfRept_('rept', $args) . "\n";
                $this->incIndent_();
                return $ret;
            case 'AAL_REPTC':
                $ret = $this->getIndentStr_() . $this->parseReptc_($args) . "\n";
                $this->incIndent_();
                return $ret;
            case 'AAL_REPTI':
                $ret = $this->getIndentStr_() . $this->parseRepti_($args) . "\n";
                $this->incIndent_();
                return $ret;
        }

        $this->errorLine_("この AAL ディレクティブは存在しません", $directive);
        return $ret;
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
    private function parseGoto_(string &$r_str, int &$r_offset, string $match, string $directive, string $to): string
    {
        $r_offset  += strlen($match);
        $this->line_nr_ += substr_count($match, "\n");

        $opcode = 'goto';
        $operands = $this->createOperands_($opcode, $to, 1);
        if ($operands === null) { return ''; }

        if ($this->checkTypeOneOf_($opcode, $operands[0], 1, [Operand::TYPE_NUM, Operand::TYPE_SYM, Operand::TYPE_EXPR])) {
            return $this->getIndentStr_() . "jp " . $operands[0]->out . "\n";
        }

        return '';
    }


    // MARK: parseExtern_()
    /** extern <式>[, ...]; */
    private function parseExtern_(string &$r_str, int &$r_offset, string $match, string $directive, string $labels): string
    {
        $r_offset  += strlen($match);
        $this->line_nr_ += substr_count($match, "\n");

        $opcode = 'extern';
        $ret = $this->parseVariadicCommon_($opcode, $labels, 1, $opcode . ' ',
            function(string $opcode, Operand $operand, int $index) : string {
                if ($this->checkTypeOneOf_($opcode, $operand, $index, [Operand::TYPE_SYM])) {
                    return $operand->value;
                }
                return '';
            },
        '', ', ');
        return $this->getIndentStr_() . $ret . "\n";
    }


    // MARK: parseIf_()
    /** if (<式>) <文1> else <文2> */
    private function parseIf_(string &$r_str, int &$r_offset, string $match, string $expr, ?string $continue, ?string $break): string
    {
        $r_offset = $this->skipStringSpaces_($r_str, $r_offset, $match);

        // $expr をオペランドに変換し, 型のチェック
        $expr = substr($expr, 1, -1); // 両側のカッコを取り除く
        $operands = $this->createOperands_('if', $expr, 1);
        if ($operands === null) { return ''; }

        if (!$this->checkTypeOneOf_('if', $operands[0], 1, [Operand::TYPE_FLAG_COMMON, Operand::TYPE_FLAG_IF])) { return ''; }
        $flag = $operands[0]->out;

        // ラベル生成・登録
        [$label_else, $label_endif] = $this->genRegistLabels_(["else", "endif"]);

        // <式> の解析
        $opcode_if   = 'jp';    // if() で使う条件ジャンプ命令
        $opcode_else = 'jp';    // <文1> 後の無条件ジャンプ命令 (else が無ければ無視)
        if (str_ends_with($flag, '_jr_else_jr') ||
            (str_ends_with($flag, '_jr') && !str_ends_with($flag, '_else_jr'))) {
            $opcode_if = 'jr';
        }
        if (str_ends_with($flag, '_else_jr')) {
            $opcode_else = 'jr';
        }
        $jump_if      = $label_endif;
        $flag         = Parser::FLAG_CONV_TAB_[$flag];
        $opcode_if   .= ' ' . Parser::FLAG_NEG_CONV_TAB_[$flag] . ', ';
        $opcode_else .= ' ' . $label_endif;

        // <文1> の解析
        $this->incIndent_();
        $sentence1 = $this->parseSentence_($r_str, $r_offset, $continue, $break);
        $sentence2 = '';
        $this->decIndent_();
        $r_offset = $this->skipStringSpaces_($r_str, $r_offset);

        // else の検出
        if (!preg_match('/else\W/Ax', $r_str, $matches, 0, $r_offset)) {
            $label_else = '';

            // else 無し
            // 最適化
            //   <文1> が「単文,フラグ判定無しの jp/jr/call/ret 命令」になってる場合, 1命令に最適化します
            //   1命令でできるなら, opcode_if を書き換えて, sentence1 は破棄
            //   原始的ですが, アセンブラソース状態で判別します. カンマが入ってたらダメ
            if (preg_match('/^ \s* (jp|jr|call) \s+ ([^,\s]+) \s* (\/\/.*)?$/Ax', $sentence1, $matches)) {
                $opcode_if = $matches[1] . ' ' . $flag . ', ' . $matches[2] . (count($matches) === 4 ? $matches[3] : '');
                $jump_if = '';
                $sentence1 = '';
                $label_endif = '';
            } else if (preg_match('/^ \s* (ret) \s* $/Ax', $sentence1, $matches)) {
                $opcode_if = $matches[1] . ' ' . $flag;
                $jump_if = '';
                $sentence1 = '';
                $label_endif = '';
            }

        } else {
            // else 有
            $jump_if = $label_else;
            $r_offset = $this->skipStringSpaces_($r_str, $r_offset, $matches[0]);

            // <文2> の解析
            $this->incIndent_();
            $sentence2 = $this->parseSentence_($r_str, $r_offset, $continue, $break);
            $this->decIndent_();
            $r_offset = $this->skipStringSpaces_($r_str, $r_offset);
        }

    //echo("[$opcode_if $jump_if][$opcode_else][$r_offset][$sentence1][$sentence2]\n");
    // 出力文字列作成
    $indent = $this->getIndentStr_();
    $out =
        $indent . $opcode_if . $jump_if . "\n" .
        $sentence1 .
        (($sentence2 !== '') ? (
            $indent . $opcode_else . "\n" .
            $indent . $label_else . ":\n" .
            $sentence2) : '') .
        (($label_endif !== '') ? (
            $indent . $label_endif . ":\n") : '') .
        "\n";

    // YET
    // 後処理の最適化
    // xxxxx: jp label_endif
    // となってるコードを探し, xxxxx にジャンプしてるコードを全て label_endif に書き換えます
    if ($label_endif !== '') {
        if (preg_match_all('/\n \s* (' . Operand::SYMBOL . '): \s* (jp|jr) \s+' . $label_endif . '/x', $out, $matches)) {
            if ($matches) {
                for ($i = 0; $i < count($matches[0]); $i++) {
                    $label_org = $matches[1][$i]; // 元のジャンプ先
                    //echo("最適化[". $matches[0][$i] . "] $label_org -> $label_endif\n");
                    $out = preg_replace('/('. $label_org . ')([^:])/', $label_endif . ' // optimized from ' . $label_org . '$2' , $out, -1, $count);
                    if ($count !== 0) {
                        //echo("note: ネストする if - else 節の多重ジャンプが最適化されました: $label_org -> $label_endif, $count 箇所\n");
                    }/**/
                }
            }
        }
    }

    /*
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
            $r_offset = $this->skipStringSpaces_($r_str, $r_offset);
            $this->incIndent_();
            $out .= $this->parseSentence_($r_str, $r_offset, $continue, $break);
            $this->decIndent_();
            $r_offset = $this->skipStringSpaces_($r_str, $r_offset);

            // else <文2> 節の後のラベル. if が最適化された場合はラベル無し
            if ($opcode_else !== '') {
                $out .= "$label_endif: // } endif\n";
            }
        }
    */
        return $out;
    }

    // MARK: parseDoWhile_()
    /** do <文> while (<式>); */
    private function parseDoWhile_(string &$r_str, int &$r_offset): string
    {
        $r_offset = $this->skipStringSpaces_($r_str, $r_offset, 'do');

        // ラベル生成・登録
        [$label_loop_top, $label_loop_end, $label_loop_exit] = $this->genRegistLabels_(["loop_top", "loop_end", "loop_exit"]); // ループ先頭と末尾,外のラベル

        // <文> の解析
        $this->incIndent_();
        $sentence = $this->parseSentence_($r_str, $r_offset, $label_loop_end, $label_loop_exit);
        $this->decIndent_();
        $r_offset = $this->skipStringSpaces_($r_str, $r_offset);

        // while (<式>) の抽出
        //         // 正規表現については, parseWhile_() 呼び出し部分を参考に
        if (!preg_match('/while \s* (\( (?:[^()]+ | (?1))* \))/Ax', $r_str, $matches, 0, $r_offset)) {
            $this->errorLine_("do に対する while 句が無いか, あってもカッコの中が間違ってます");
            return '';
        }

        // <式> の解析
        $expr = substr($matches[1], 1, -1); // 両側のカッコを取り除く
        $operands = $this->createOperands_('while', $expr, 1);
        $r_offset += strlen($matches[0]);
        if ($operands === null) { return ''; }
        if (!$this->checkTypeOneOf_('while', $operands[0], 1, [Operand::TYPE_FLAG_COMMON, Operand::TYPE_FALSE, Operand::TYPE_FLAG_DO_WHILE])) { return ''; }

        // ループ末端の jump 命令
        $loop_end_inst = '';    // while (false) の場合, ループ末端のジャンプ命令は省く
        $loop_end_op = Parser::DO_WHILE_OP_TAB_[$operands[0]->out];
        if ($loop_end_op !== '') {
            $loop_end_inst = $loop_end_op . ' ' . $label_loop_top;
        }

        // 文字列作成
        $indent = $this->getIndentStr_();
        $out =
            $indent . $label_loop_top . ": // do {\n" .
            $sentence .
            $indent . $label_loop_end . ":\n" .
            $indent . $loop_end_inst . " // } while (" . $operands[0]->out. ")\n" .
            $indent . $label_loop_exit . ": // loop exit\n\n";

        return $out;
    }


    /** while (<式>) <文> */
    private function parseWhile_(string &$r_str, int &$r_offset, string $match, string $expr): string
    {
        $r_offset = $this->skipStringSpaces_($r_str, $r_offset, $match);

        // $expr をオペランドに変換し, 型のチェック
        $expr = substr($expr, 1, -1); // 両側のカッコを取り除く
        $operands = $this->createOperands_('while', $expr, 1);
        if ($operands === null) { return ''; }
        if (!$this->checkTypeOneOf_('while', $operands[0], 1, [Operand::TYPE_FLAG_COMMON, Operand::TYPE_TRUE, Operand::TYPE_FLAG_WHILE])) { return ''; }

        // ラベル生成・登録
        [$label_loop_top, $label_loop_exit] = $this->genRegistLabels_(["loop_top", "loop_exit"]); // ループ先頭とループ外のラベル

        // <式> の解析
        $loop_top_op = '';
        if ($operands[0]->out !== 'true') {
            $loop_top_op = Parser::WHILE_OP_TAB_[$operands[0]->out] . ' ' . $label_loop_exit . "\n";
        }

        // <文> の解析
        $out_sentence = $this->parseSentence_($r_str, $r_offset, $label_loop_top, $label_loop_exit);

        // 文字列作成
        $indent1 = $this->getIndentStr_();
        $this->incIndent_();
        $indent2 = $this->getIndentStr_();
        $this->decIndent_();
        $out =
            $indent1 . $label_loop_top . ": // while (" . $operands[0]->out . ") {\n" .
            ($loop_top_op === '' ? "" : ($indent2 . $loop_top_op)) .
            $indent1 . $out_sentence .
            $indent2 . 'jp ' . $label_loop_top . " // }\n" .
            $indent1 . $label_loop_exit . ": // loop exit\n\n";

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

        $r_offset = $this->skipStringSpaces_($r_str, $r_offset, $match);

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

        $r_offset = $this->skipStringSpaces_($r_str, $r_offset, $match);

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
            $l_value = trim(substr($l_value, 0, -1));
        } else if (2 <= strlen($l_value)) {
            $op = $l_value[-2] . $op;
            if ($op === '<<=' || $op === '>>=') {
                $l_value = trim(substr($l_value, 0, -2));
            } else {
                $op = '=';
            }
        } else {
            $op = '=';
        }
        //echo("assignOp1[$r_offset $this->line_nr_][$l_value][$op][$r_value]\n");

        // 型によって命令が変わるので, 右辺値, 左辺値の型を調べる
        $ret = '';
        $expr = $l_value . "," . $r_value;

        // $op によって命令を決める
        switch ($op) {
            case '=': $ret =  $this->parseLdInOut_($expr); break;
            case '+=': $ret = $this->parseAddAdc_($expr); break;
            case '-=': $ret = $this->parseSubSbc_($expr); break;
            case '&=': $ret = $this->parseAnd_('and', $expr); break;
            case '|=': $ret = $this->parseAnd_('or',  $expr); break;
            case '^=': $ret = $this->parseAnd_('xor', $expr); break;
            case '>>=': $ret = $this->parseRShift_($expr, 2); break;
            case '<<=': $ret = $this->sla_n_($expr); break;
        }

        $r_offset  += strlen($match);
        $this->line_nr_ += substr_count($match, "\n");
        //echo("assignOp2[$r_offset $this->line_nr_][$l_value][$op][$r_value]\n");

        return $this->getIndentStr_() . "$ret\n";
    }


    // MARK: parseUnaryOp_()
    /** 単項演算子 */
    private function parseUnaryOp_(string &$r_str, int &$r_offset, string $match, string $op, string $expr): string
    {
        //echo("unaryOp1[$r_offset $this->line_nr_][$op][$expr]\n");

        // $op によって命令を決める
        $ret = '';
        switch ($op) {
            case '++': $ret = $this->parseIncDec_('inc', $expr); break;
            case '--': $ret = $this->parseIncDec_('dec', $expr); break;
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
            // ヘタに加工するとマクロ展開がおかしくなるので, パラメータはそのまま渡す(裏レジスタを除く)
            $ret = $funcname . ' ';
            foreach($params as $i => $operand) {
                $ret .= Operand::stripBackReg($operand);
                //echo("マクロ呼び出し {$ret} [{$operand}]\n");
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


    // -------------------------------- 関数やマクロの最後のディレクティブ
    // MARK: DEF_VARS
    private function defVarsDirective_(string &$r_str, int $offset): void
    {
        // $offset より前の文字列が全て空白文字でなければエラー
        if ($offset !== 0 && ctype_space(substr($r_str, 0, $offset)) === false) {
            $this->errorLine_("AAL_DEF_VARS は関数かマクロの先頭に書いてください");
        }
    }

    // MARK: ENDM
    private function endmDirective_(string &$r_str, int $offset): void
    {
        //echo('noReturnDirective(): [' . substr($r_str, $offset) . "]\n");
        if ($this->mode_ !== Parser::MODE_MACRO) {
            $this->errorLine_("AAL_ENDM は, マクロ用定義用です");
        } else {
            // $offset から後の文字列が全て空白文字かセミコロンでなければエラー
            $str = substr($r_str, $offset);
            if (preg_match('/^\s*;\s*$/', $str) === false) {
                $this->errorLine_("AAL_ENDM は, マクロの一番最後に記述してください");
            }
            $this->is_ended_ = true;
        }
    }

    // MARK: NO_RETURN
    private function noReturnDirective_(string &$r_str, int $offset): void
    {
        //echo('noReturnDirective(): [' . substr($r_str, $offset) . "]\n");
        if ($this->mode_ !== Parser::MODE_FUNC) {
            $this->errorLine_("AAL_NO_RETURN は, 関数用です");
        } else if (!$this->is_naked_) {
            $this->errorLine_("AAL_NO_RETURN は, __naked でない関数にはつけてはいけなせん");
        } else {
            // $offset から後の文字列が全て空白文字かセミコロンでなければエラー
            $str = substr($r_str, $offset);
            if (preg_match('/^\s*;\s*$/', $str) === false) {
                $this->errorLine_("AAL_NO_RETURN は, 関数の一番最後に記述してください");
            }
            $this->is_ended_ = true;
        }
    }

    // MARK: FALL_THROUGH
    private function fallThroughDirective_(string &$r_str, int $offset): void
    {
        if ($this->mode_ !== Parser::MODE_FUNC) {
            $this->errorLine_("AAL_FALL_THROUGH は, 関数用です");
        } else if (!$this->is_naked_) {
            $this->errorLine_("AAL_FALL_THROUGH は, __naked でない関数にはつけてはいけなせん");
        } else {
            // $offset から後の文字列が全て空白文字かセミコロンでなければエラー
            $str = substr($r_str, $offset);
            if (preg_match('/^\s*;\s*$/', $str) === false) {
                $this->errorLine_("AAL_FALL_THROUGH は, 関数の一番最後に記述してください");
            }
            $this->is_ended_ = true;
        }
    }


    // ---------------------------------------------------------------- 命令ディスパッチャ
    // MARK: command dispatches
    //private function ld_(string $expr): string { return $this->parseLdInOut_($expr); }
    private function ldi_(string $expr): string { return $this->parseNop_('ldi', $expr); }
    private function ldd_(string $expr): string { return $this->parseNop_('ldd', $expr); }
    private function ldir_(string $expr): string { return $this->parseNop_('ldir', $expr); }
    private function lddr_(string $expr): string { return $this->parseNop_('lddr', $expr); }
    private function ex_(string $expr): string { return $this->parseEx_('ex', $expr); }
    private function exx_(string $expr): string { return $this->parseNop_('exx', $expr); }
    private function push_(string $expr): string { return $this->parsePushPop_('push', $expr); }
    private function pop_(string $expr): string { return $this->parsePushPop_('pop', $expr); }
    //private function and_(string $expr): string { return $this->parseAnd_('and', $expr); }
    //private function or_( string $expr): string { return $this->parseAnd_('or', $expr); }
    //private function xor_(string $expr): string { return $this->parseAnd_('xor', $expr); }
    //private function cpl_(string $expr): string { return $this->parseA_('cpl', $expr); }
    private function not_(string $expr): string { return $this->parseA_('cpl', $expr); }
    private function cp_(string $expr): string { return $this->parseAnd_('cp', $expr); }
    private function cpi_(string $expr): string { return $this->parseNop_('cpi', $expr); }
    private function cpd_(string $expr): string { return $this->parseNop_('cpd', $expr); }
    private function cpir_(string $expr): string { return $this->parseNop_('cpir', $expr); }
    private function cpdr_(string $expr): string { return $this->parseNop_('cpdr', $expr); }
    //private function add_(string $expr): string { return $this->parseAdd_('add', $expr); }
    //private function adc_(string $expr): string { return $this->parseAdc_('adc', $expr); }
    //private function sub_(string $expr): string { return $this->parseSub_('sub', $expr); }
    //private function sbc_(string $expr): string { return $this->parseAdc_('sbc', $expr); }
    //private function inc_(string $expr): string { return $this->parseIncDec_('inc', $expr); }
    //private function dec_(string $expr): string { return $this->parseIncDec_('dec', $expr); }
    private function neg_(string $expr): string { return $this->parseA_('neg', $expr); }
    private function daa_(string $expr): string { return $this->parseA_('daa', $expr); }
    private function bit_(string $expr): string { return $this->parseBit_('bit', $expr); }
    private function set_(string $expr): string { return $this->parseBit_('set', $expr); }
    private function res_(string $expr): string { return $this->parseBit_('res', $expr); }
    private function bit3_(string $expr): string { return $this->parseBit3_('bit', $expr); }// 未定義命令 bit b, mem[ix + d], r
    private function set3_(string $expr): string { return $this->parseBit3_('set', $expr); }// 未定義命令 set b, mem[ix + d], r
    private function res3_(string $expr): string { return $this->parseBit3_('res', $expr); }// 未定義命令 res b, mem[ix + d], r
    private function sla_(string $expr): string { return $this->parseShift_('sla', 'sla', 'rl', true,  $expr, 1); }
    private function sll_(string $expr): string { return $this->parseShift_('sll', 'sll', 'rl', true,  $expr, 1); }
    private function sra_(string $expr): string { return $this->parseShift_('sra', 'sra', 'rr', false, $expr, 1); }
    private function srl_(string $expr): string { return $this->parseShift_('srl', 'srl', 'rr', false, $expr, 1); }
    private function rl_( string $expr): string { return $this->parseRotate_('rl',  $expr, 1); }
    private function rr_( string $expr): string { return $this->parseRotate_('rr',  $expr, 1); }
    private function rlc_(string $expr): string { return $this->parseRotate_('rlc', $expr, 1); }
    private function rrc_(string $expr): string { return $this->parseRotate_('rrc', $expr, 1); }
    private function sla_n_(string $expr): string { return $this->parseShift_('sla', 'sla', 'rl', true,  $expr, 2); }
    private function sll_n_(string $expr): string { return $this->parseShift_('sll', 'sll', 'rl', true,  $expr, 2); }
    private function sra_n_(string $expr): string { return $this->parseShift_('sra', 'sra', 'rr', false, $expr, 2); }
    private function srl_n_(string $expr): string { return $this->parseShift_('sra', 'sra', 'rr', false, $expr, 2); }
    private function rln_( string $expr): string { return $this->parseRotate_('rl',  $expr, 2); }
    private function rrn_( string $expr): string { return $this->parseRotate_('rr',  $expr, 2); }
    private function rlc_n_(string $expr): string { return $this->parseRotate_('rlc', $expr, 2); }
    private function rrc_n_(string $expr): string { return $this->parseRotate_('rrc', $expr, 2); }
    private function sla2_(string $expr): string { return $this->parseShift2_('sla', $expr); }// 未定義命令 sla mem[ix + d], r
    private function sll2_(string $expr): string { return $this->parseShift2_('sll', $expr); }// 未定義命令 sll mem[ix + d], r
    private function sra2_(string $expr): string { return $this->parseShift2_('sra', $expr); }// 未定義命令 sra mem[ix + d], r
    private function srl2_(string $expr): string { return $this->parseShift2_('srl', $expr); }// 未定義命令 srl mem[ix + d], r
    private function rl2_( string $expr): string { return $this->parseShift2_('rl', $expr); } // 未定義命令 rl mem[ix + d], r
    private function rr2_( string $expr): string { return $this->parseShift2_('rr', $expr); } // 未定義命令 rr mem[ix + d], r
    private function rlc2_(string $expr): string { return $this->parseShift2_('rlc', $expr); }// 未定義命令 rlc mem[ix + d], r
    private function rrc2_(string $expr): string { return $this->parseShift2_('rrc', $expr); }// 未定義命令 rrc mem[ix + d], r
    private function rla_ (string $expr): string { return $this->parseA_('rla', $expr); }
    private function rra_ (string $expr): string { return $this->parseA_('rra', $expr); }
    private function rlca_(string $expr): string { return $this->parseA_('rlca', $expr); }
    private function rrca_(string $expr): string { return $this->parseA_('rrca', $expr); }
    private function rla_n_ (string $expr): string { return $this->parseRotateA_('rla', $expr); }
    private function rra_n_ (string $expr): string { return $this->parseRotateA_('rra', $expr); }
    private function rlca_n_(string $expr): string { return $this->parseRotateA_('rlca', $expr); }
    private function rrca_n_(string $expr): string { return $this->parseRotateA_('rrca', $expr); }
    private function rld_(string $expr): string { return $this->parseRld_('rld', $expr); }
    private function rrd_(string $expr): string { return $this->parseRld_('rrd', $expr); }
    private function jp_(string $expr): string { return $this->parseJp_($expr); }
    private function jp_z_(string $expr): string { return $this->parseJpJrCall_('jp z,', $expr); }
    private function jp_eq_(string $expr): string { return $this->jp_z_($expr); }
    private function jp_nz_(string $expr): string { return $this->parseJpJrCall_('jp nz,', $expr); }
    private function jp_ne_(string $expr): string { return $this->jp_nz_($expr); }
    private function jp_c_(string $expr): string { return $this->parseJpJrCall_('jp c,', $expr); }
    private function jp_lt_(string $expr): string { return $this->jp_c_($expr); }
    private function jp_nc_(string $expr): string { return $this->parseJpJrCall_('jp nc,', $expr); }
    private function jp_ge_(string $expr): string { return $this->jp_nc_($expr); }
    private function jp_p_(string $expr): string { return $this->parseJpJrCall_('jp p,', $expr); }
    private function jp_m_(string $expr): string { return $this->parseJpJrCall_('jp m,', $expr); }
    private function jp_v_(string $expr): string { return $this->parseJpJrCall_('jp v,', $expr); }
    private function jp_nv_(string $expr): string { return $this->parseJpJrCall_('jp nv,', $expr); }
    private function jp_pe_(string $expr): string { return $this->parseJpJrCall_('jp pe,', $expr); }
    private function jp_po_(string $expr): string { return $this->parseJpJrCall_('jp po,', $expr); }
    private function jr_(string $expr): string { return $this->parseJpJrCall_('jr', $expr); }
    private function jr_z_(string $expr): string { return $this->parseJpJrCall_('jr z,', $expr); }
    private function jr_eq_(string $expr): string { return $this->jr_z_($expr); }
    private function jr_nz_(string $expr): string { return $this->parseJpJrCall_('jr nz,', $expr); }
    private function jr_ne_(string $expr): string { return $this->jr_nz_($expr); }
    private function jr_c_(string $expr): string { return $this->parseJpJrCall_('jr c,', $expr); }
    private function jr_lt_(string $expr): string { return $this->jr_c_($expr); }
    private function jr_nc_(string $expr): string { return $this->parseJpJrCall_('jr nc,', $expr); }
    private function jr_ge_(string $expr): string { return $this->jr_nc_($expr); }
    private function djnz_(string $expr): string { return $this->parseDjnz_($expr); }
    private function call_(string $expr): string { return $this->parseJpJrCall_('call', $expr); }
    private function call_z_(string $expr): string { return $this->parseJpJrCall_('call z,', $expr); }
    private function call_eq_(string $expr): string { return $this->call_z_($expr); }
    private function call_nz_(string $expr): string { return $this->parseJpJrCall_('call nz,', $expr); }
    private function call_ne_(string $expr): string { return $this->call_nz_($expr); }
    private function call_c_(string $expr): string { return $this->parseJpJrCall_('call c,', $expr); }
    private function call_lt_(string $expr): string { return $this->call_c_($expr); }
    private function call_nc_(string $expr): string { return $this->parseJpJrCall_('call nc,', $expr); }
    private function call_ge_(string $expr): string { return $this->call_nc_($expr); }
    private function call_p_(string $expr): string { return $this->parseJpJrCall_('call p,', $expr); }
    private function call_m_(string $expr): string { return $this->parseJpJrCall_('call m,', $expr); }
    private function call_v_(string $expr): string { return $this->parseJpJrCall_('call v,', $expr); }
    private function call_nv_(string $expr): string { return $this->parseJpJrCall_('call nv,', $expr); }
    private function call_pe_(string $expr): string { return $this->parseJpJrCall_('call pe,', $expr); }
    private function call_po_(string $expr): string { return $this->parseJpJrCall_('call po,', $expr); }
    private function rst_(string $expr): string { return $this->parseRst_($expr); }
    //private function ret_(string $expr): string { return $this->parseNop_('ret', $expr); }
    private function ret_z_(string $expr): string { return $this->parseNop_('ret z', $expr); }
    private function ret_eq_(string $expr): string { return $this->ret_z_($expr); }
    private function ret_nz_(string $expr): string { return $this->parseNop_('ret nz', $expr); }
    private function ret_ne_(string $expr): string { return $this->ret_nz_($expr); }
    private function ret_c_(string $expr): string { return $this->parseNop_('ret c', $expr); }
    private function ret_lt_(string $expr): string { return $this->ret_c_($expr); }
    private function ret_nc_(string $expr): string { return $this->parseNop_('ret nc', $expr); }
    private function ret_ge_(string $expr): string { return $this->ret_ge_($expr); }
    private function ret_p_(string $expr): string { return $this->parseNop_('ret p', $expr); }
    private function ret_m_(string $expr): string { return $this->parseNop_('ret m', $expr); }
    private function ret_v_(string $expr): string { return $this->parseNop_('ret v', $expr); }
    private function ret_nv_(string $expr): string { return $this->parseNop_('ret nv', $expr); }
    private function ret_pe_(string $expr): string { return $this->parseNop_('ret pe', $expr); }
    private function ret_po_(string $expr): string { return $this->parseNop_('ret po', $expr); }
    private function reti_(string $expr): string { return $this->parseNop_('reti', $expr); }
    private function retn_(string $expr): string { return $this->parseNop_('retn', $expr); }
    private function nop_(string $expr): string { return $this->parseNop_('nop', $expr); }
    private function halt_(string $expr): string { return $this->parseNop_('halt', $expr); }
    private function di_(string $expr): string { return $this->parseNop_('di', $expr); }
    private function ei_(string $expr): string { return $this->parseNop_('ei', $expr); }
    private function im0_(string $expr): string { return $this->parseNop_('im 0', $expr); }
    private function im1_(string $expr): string { return $this->parseNop_('im 1', $expr); }
    private function im2_(string $expr): string { return $this->parseNop_('im 2', $expr); }
    private function scf_(string $expr): string { return $this->parseNop_('scf', $expr); }
    private function ccf_(string $expr): string { return $this->parseNop_('ccf', $expr); }
    //private function in_(string $expr): string { return $this->checkParam22_R_P_('in', $expr); }
    //private function out_(string $expr): string { return $this->checkParam22_P_RI_('out', $expr); }
    private function ini_(string $expr): string { return $this->parseNop_('ini', $expr); }
    private function ind_(string $expr): string { return $this->parseNop_('ind', $expr); }
    private function inir_(string $expr): string { return $this->parseNop_('inir', $expr); }
    private function indr_(string $expr): string { return $this->parseNop_('indr', $expr); }
    private function outi_(string $expr): string { return $this->parseNop_('outi', $expr); }
    private function outd_(string $expr): string { return $this->parseNop_('outd', $expr); }
    private function otir_(string $expr): string { return $this->parseNop_('otir', $expr); }
    private function otdr_(string $expr): string { return $this->parseNop_('otdr', $expr); }

    // MARK: local,global
    private function parseLocalGlobal_(string $opcode, string $operandsStr): string
    {
        return $this->parseVariadicCommon_($opcode, $operandsStr, 1, '',
            function(string $opcode, Operand $operand, int $index) : string {
                if ($this->checkTypeOneOf_($opcode, $operand, $index, [Operand::TYPE_SYM])) {
                    return "$opcode " . $operand->value;
                }
                return '';
            },
        $this->getIndentStr_(), "\n");
    }


    // MARK: db
    private function parseDb_(string $operandsStr): string
    {
        // db 123, 45h, “ABCDEFG”, LABEL
        $opcode = 'db';
        return $this->parseVariadicCommon_($opcode, $operandsStr, 1, $opcode . ' ',
            function(string $opcode, Operand $operand, int $index) : string {
                if ($this->checkTypeOneOf_($opcode, $operand, $index, [Operand::TYPE_NUM, Operand::TYPE_SYM, Operand::TYPE_EXPR, Operand::TYPE_STR])) {
                    return $operand->out;
                }
                return '';
            },
        '', ', ');
    }


    // MARK: dw
    private function parseDw_(string $operandsStr): string
    {
        // dw 123, VAL, LABEL
        $opcode = 'dw';
        return $this->parseVariadicCommon_($opcode, $operandsStr, 1, $opcode . ' ',
            function(string $opcode, Operand $operand, int $index) : string {
                if ($this->checkTypeOneOf_($opcode, $operand, $index, [Operand::TYPE_NUM, Operand::TYPE_SYM, Operand::TYPE_EXPR])) {
                    return $operand->out;
                }
                return '';
            },
        '', ', ');
    }


    // MARK: ds
    private function parseDs_(string $operandsStr): string
    {
        // ds 123
        // ds 123, VAL, 3, 4
        // ds 123, "ABCDEFG"
        $opcode = 'ds';
        return $this->parseVariadicCommon_($opcode, $operandsStr, 1, $opcode . ' ',
            function(string $opcode, Operand $operand, int $index) : string {
                if ($index == 1) {  // 第1引数はサイズ
                    if (!$this->checkTypeOneOf_($opcode, $operand, $index, [Operand::TYPE_NUM, Operand::TYPE_SYM, Operand::TYPE_EXPR])) {
                        return "";
                    }
                } else if ($operand->type === Operand::TYPE_STR) {
                    // 第2引数は文字列の場合は1つだけ
                    if ($index !== 2) {
                        $this->errorLine_("$opcode 文字列は1つだけです");
                        return "";
                    }
                } else {
                    if (!$this->checkTypeOneOf_($opcode, $operand, $index, [Operand::TYPE_NUM, Operand::TYPE_SYM, Operand::TYPE_EXPR])) {
                        return "";
                    }
                }
                return $operand->out;
            },
        '', ', ');
    }

    // MARK: if,elif,rept
    private function parseIfRept_(string $opcode, string $operandsStr): string
    {
        $operands = $this->createOperands_($opcode, $operandsStr, 1);
        if ($operands === null) { return ''; }

        if ($this->checkTypeOneOf_($opcode, $operands[0], 1, [Operand::TYPE_NUM, Operand::TYPE_SYM, Operand::TYPE_EXPR])) {
            return "$opcode " . $operands[0]->out;
        }
        return '';
    }

    // MARK: reptc
    private function parseReptc_(string $operandsStr): string
    {
        $opcode = 'reptc';
        $operands = $this->createOperands_($opcode, $operandsStr, 2);
        if ($operands === null) { return ''; }

        if ($this->checkTypeOneOf_($opcode, $operands[0], 1, [Operand::TYPE_SYM])) {
            if ($this->checkTypeOneOf_($opcode, $operands[1], 2, [Operand::TYPE_STR])) {
                return "$opcode " . $operands[0]->value . ", " . $operands[1]->out;
            }
        }
        return '';
    }


    // MARK: repti
    private function parseRepti_(string $operandsStr): string
    {
        $opcode = 'repti';
        return $this->parseVariadicCommon_($opcode, $operandsStr, 2, $opcode . ' ',
            function(string $opcode, Operand $operand, int $index) : string {
                if ($index === 1) {
                    if ($this->checkTypeOneOf_($opcode, $operand, $index, [Operand::TYPE_SYM])) {
                        return $operand->value;
                    }
                } else {
                    if ($this->checkTypeOneOf_($opcode, $operand, $index, [Operand::TYPE_NUM, Operand::TYPE_SYM, Operand::TYPE_EXPR, Operand::TYPE_STR])) {
                        return $operand->out;
                    }
                }
                return '';
            },
        '', ', ');
    }


    // -------------------------------- パラメータの型チェックとコード生成 (引数なし)
    // MARK: nop,ret,ldir,im0,scf,...
    private function parseNop_(string $opcode, string $operandsStr): string
    {
        if ($operandsStr !== '') {
            $this->errorLine_("$opcode は引数不要です", $operandsStr);
        }
        return $opcode;
    }


    // -------------------------------- パラメータの型チェックとコード生成 (引数 1)
    // MARK: neg,cpl,rca,...
    /** 引数が A のみ */
    private function parseA_(string $opcode, string $operandsStr): string
    {
        $params = $this->createOperands_($opcode, $operandsStr, 1);
        if ($params === null) { return ''; }

        if ($this->checkTypeOneOf_($opcode, $params[0], 1, [Operand::TYPE_A])) {
            return $opcode;
        }
        return '';
    }


    // MARK: inc,dec
    private function parseIncDec_(string $opcode, string $operandsStr): string
    {
        $params = $this->createOperands_($opcode, $operandsStr, 1);
        if ($params === null) { return ''; }

        if ($this->checkTypeOneOf_($opcode, $params[0], 1, [
            Operand::TYPE_A, Operand::TYPE_B, Operand::TYPE_CDE8, Operand::TYPE_HL8, Operand::TYPE_IX8, Operand::TYPE_MEM_HL, Operand::TYPE_MEM_IX,
            Operand::TYPE_HL, Operand::TYPE_DE, Operand::TYPE_BC, Operand::TYPE_SP, Operand::TYPE_IX])) {
            return "$opcode " . $params[0]->out;
        }
        return '';
    }


    // -------------------------------- パラメータの型チェックとコード生成 (引数 2)
    // MARK: ld,in,out
    private function parseLdInOut_(string $operandsStr): string
    {
        $opcode = "ld";
        $params = $this->createOperands_($opcode, $operandsStr, 2);
        if ($params === null) { return ''; }

        if ($params[0]->type === Operand::TYPE_PORT) {
            // 第1引数が port なら out (n), A
            $opcode = 'out';
            $this->checkTypeOneOf_($opcode, $params[1], 2, [Operand::TYPE_A]);
        } else if ($params[0]->type === Operand::TYPE_PORT_C) {
            // 第1引数が port なら out (C), r  out (C), n
            $opcode = 'out';
            if ($this->checkTypeOneOf_($opcode, $params[1], 2, [Operand::TYPE_A, Operand::TYPE_B, Operand::TYPE_CDE8, Operand::TYPE_HL8, Operand::TYPE_NUM])) {
                if ($params[1]->type === Operand::TYPE_NUM && ($params[1]->value != 0 && $params[1]->value != 255)) {
                    $this->errorLine_("$opcode の第2引数が即値の場合は, 0 または 255 だけです", $params[1]->org);
                }
            }
        } else if ($params[1]->type === Operand::TYPE_PORT) {
            // 第2引数が port なら in A, (n)  in F, (n)
            $opcode = 'in';
            $this->checkTypeOneOf_($opcode, $params[0], 1, [Operand::TYPE_A, Operand::TYPE_F]);
        } else if  ($params[1]->type === Operand::TYPE_PORT_C) {
            // 第2引数が port なら in r, (C)
            $opcode = 'in';
            $this->checkTypeOneOf_($opcode, $params[0], 1, [Operand::TYPE_A, Operand::TYPE_B, Operand::TYPE_CDE8, Operand::TYPE_HL8, Operand::TYPE_NUM]);
        } else {
            // それ以外は ld. 条件ややこい
            switch ($params[0]->type) {
                case Operand::TYPE_IR:              // fall through
                case Operand::TYPE_MEM_DEBC:
                    $this->checkTypeOneOf_($opcode, $params[1], 2, [Operand::TYPE_A]);
                    break;
                case Operand::TYPE_MEM:
                    $this->checkTypeOneOf_($opcode, $params[1], 2, [
                        Operand::TYPE_A, Operand::TYPE_HL, Operand::TYPE_DE, Operand::TYPE_BC, Operand::TYPE_IX, Operand::TYPE_SP]);
                    break;
                case Operand::TYPE_A:
                    $this->checkTypeOneOf_($opcode, $params[1], 2, [
                        Operand::TYPE_IR, Operand::TYPE_A, Operand::TYPE_B, Operand::TYPE_CDE8, Operand::TYPE_HL8, Operand::TYPE_IX8,
                        Operand::TYPE_MEM, Operand::TYPE_MEM_HL, Operand::TYPE_MEM_DEBC, Operand::TYPE_MEM_IX,
                        Operand::TYPE_NUM, Operand::TYPE_TRUE, Operand::TYPE_FALSE, Operand::TYPE_SYM, Operand::TYPE_EXPR]);
                    break;
                case Operand::TYPE_B:               // fall through
                case Operand::TYPE_CDE8:
                    $this->checkTypeOneOf_($opcode, $params[1], 2, [
                        Operand::TYPE_A, Operand::TYPE_B, Operand::TYPE_CDE8, Operand::TYPE_HL8, Operand::TYPE_IX8,
                        Operand::TYPE_MEM_HL,
                        Operand::TYPE_NUM, Operand::TYPE_TRUE, Operand::TYPE_FALSE, Operand::TYPE_SYM, Operand::TYPE_EXPR]);
                    break;
                case Operand::TYPE_HL8:
                    $this->checkTypeOneOf_($opcode, $params[1], 2, [
                        Operand::TYPE_A, Operand::TYPE_B, Operand::TYPE_CDE8, Operand::TYPE_HL8,
                        Operand::TYPE_MEM_HL,
                        Operand::TYPE_NUM, Operand::TYPE_TRUE, Operand::TYPE_FALSE, Operand::TYPE_SYM, Operand::TYPE_EXPR]);
                    break;
                case Operand::TYPE_IX8:
                    $this->checkTypeOneOf_($opcode, $params[1], 2, [
                        Operand::TYPE_A, Operand::TYPE_B, Operand::TYPE_CDE8, Operand::TYPE_IX8,
                        Operand::TYPE_NUM, Operand::TYPE_TRUE, Operand::TYPE_FALSE, Operand::TYPE_SYM, Operand::TYPE_EXPR]);
                    break;
                case Operand::TYPE_MEM_HL:
                    $this->checkTypeOneOf_($opcode, $params[1], 2, [
                        Operand::TYPE_A, Operand::TYPE_B, Operand::TYPE_CDE8, Operand::TYPE_HL8,
                        Operand::TYPE_NUM, Operand::TYPE_TRUE, Operand::TYPE_FALSE, Operand::TYPE_SYM, Operand::TYPE_EXPR]);
                    break;
                case Operand::TYPE_MEM_IX:
                    $this->checkTypeOneOf_($opcode, $params[1], 2, [
                        Operand::TYPE_A, Operand::TYPE_B, Operand::TYPE_CDE8, Operand::TYPE_HL8,
                        Operand::TYPE_NUM, Operand::TYPE_TRUE, Operand::TYPE_FALSE, Operand::TYPE_SYM, Operand::TYPE_EXPR]);
                    break;
                case Operand::TYPE_HL:
                    $this->checkTypeOneOf_($opcode, $params[1], 2, [
                        Operand::TYPE_HL, Operand::TYPE_DE, Operand::TYPE_BC,
                        Operand::TYPE_MEM,
                        Operand::TYPE_NUM, Operand::TYPE_TRUE, Operand::TYPE_FALSE, Operand::TYPE_SYM, Operand::TYPE_EXPR]);
                    break;
                case Operand::TYPE_IX:
                    $this->checkTypeOneOf_($opcode, $params[1], 2, [
                        Operand::TYPE_IX, Operand::TYPE_DE, Operand::TYPE_BC,
                        Operand::TYPE_MEM,
                        Operand::TYPE_NUM, Operand::TYPE_TRUE, Operand::TYPE_FALSE, Operand::TYPE_SYM, Operand::TYPE_EXPR]);
                    break;
                case Operand::TYPE_DE:              // fall through
                case Operand::TYPE_BC:
                    $this->checkTypeOneOf_($opcode, $params[1], 2, [
                        Operand::TYPE_HL, Operand::TYPE_DE, Operand::TYPE_BC, Operand::TYPE_IX,
                        Operand::TYPE_MEM,
                        Operand::TYPE_NUM, Operand::TYPE_TRUE, Operand::TYPE_FALSE, Operand::TYPE_SYM, Operand::TYPE_EXPR]);
                    break;
                case Operand::TYPE_SP:
                    $this->checkTypeOneOf_($opcode, $params[1], 2, [
                        Operand::TYPE_HL, Operand::TYPE_IX,
                        Operand::TYPE_MEM,
                        Operand::TYPE_NUM, Operand::TYPE_TRUE, Operand::TYPE_FALSE, Operand::TYPE_SYM, Operand::TYPE_EXPR]);
                    break;
                default:
                    $this->errorLine_("$opcode の第1引数は, I|R|A|B|C|D|E|H|L|IXH等|mem[addr]|mem[HL]|mem[IX+d]等|HL|DE|BC|IX等|SP|port[addr]|port[C]等 が使用できます", $params[0]->org);
            }
        }
        return "$opcode " . $params[0]->out . ', ' .  $params[1]->out;
    }


    // MARK: add,adc
    private function parseAddAdc_(string $operandsStr): string
    {
        $opcode = '+=';
        $params = $this->createOperands_($opcode, $operandsStr, 2);
        if ($params === null) { return ''; }

        switch ($params[0]->type) {
            case Operand::TYPE_A:   // 8bit
                $this->checkTypeOneOf_($opcode, $params[1], 2, [
                    Operand::TYPE_A, Operand::TYPE_B, Operand::TYPE_CDE8, Operand::TYPE_HL8,
                    Operand::TYPE_A_C, Operand::TYPE_B_C, Operand::TYPE_CDE8_C, Operand::TYPE_HL8_C,
                    Operand::TYPE_IX8, Operand::TYPE_MEM_HL, Operand::TYPE_MEM_IX, Operand::TYPE_NUM, Operand::TYPE_SYM, Operand::TYPE_EXPR,
                    Operand::TYPE_IX8_C, Operand::TYPE_MEM_HL_C, Operand::TYPE_MEM_IX_C, Operand::TYPE_NUM_C, Operand::TYPE_SYM_C, Operand::TYPE_EXPR_C]);
                break;
            case Operand::TYPE_HL:  // 16bit
                $this->checkTypeOneOf_($opcode, $params[1], 2, [
                    Operand::TYPE_HL, Operand::TYPE_DE, Operand::TYPE_BC, Operand::TYPE_SP,
                    Operand::TYPE_HL_C, Operand::TYPE_DE_C, Operand::TYPE_BC_C, Operand::TYPE_SP_C]);
                break;
            case Operand::TYPE_IX:  // 16bit
                $this->checkTypeOneOf_($opcode, $params[1], 2, [
                    Operand::TYPE_IX, Operand::TYPE_DE, Operand::TYPE_BC, Operand::TYPE_SP]);  // IX += 系命令には adc は無い
                break;
            default:
                if ($params[1]->isWithCarry()) {
                    $this->errorLine_("adc の第1引数は, A|HL が使用できます", $params[0]->org);
                } else {
                    $this->errorLine_("add の第1引数は, A|HL|IX が使用できます", $params[0]->org);
                }
                return '';
        }

        return ($params[1]->isWithCarry() ? 'adc' : 'add') . " " . $params[0]->out . ', ' .  $params[1]->out;
    }

    // MARK: sub,sbc
    private function parseSubSbc_(string $operandsStr): string
    {
        $opcode = '-=';
        $params = $this->createOperands_($opcode, $operandsStr, 2);
        if ($params === null) { return ''; }

        switch ($params[0]->type) {
            case Operand::TYPE_A:   // 8bit
                $this->checkTypeOneOf_($opcode, $params[1], 2, [
                    Operand::TYPE_A, Operand::TYPE_B, Operand::TYPE_CDE8, Operand::TYPE_HL8,
                    Operand::TYPE_A_C, Operand::TYPE_B_C, Operand::TYPE_CDE8_C, Operand::TYPE_HL8_C,
                    Operand::TYPE_IX8, Operand::TYPE_MEM_HL, Operand::TYPE_MEM_IX, Operand::TYPE_NUM, Operand::TYPE_SYM, Operand::TYPE_EXPR,
                    Operand::TYPE_IX8_C, Operand::TYPE_MEM_HL_C, Operand::TYPE_MEM_IX_C, Operand::TYPE_NUM_C, Operand::TYPE_SYM_C, Operand::TYPE_EXPR_C]);
                break;
            case Operand::TYPE_HL:  // 16bit
                $this->checkTypeOneOf_($opcode, $params[1], 2, [
                    Operand::TYPE_HL_C, Operand::TYPE_DE_C, Operand::TYPE_BC_C, Operand::TYPE_SP_C]);   // 16bit sub 命令はありません
                break;
            default:
                if ($params[1]->isWithCarry()) {
                    $this->errorLine_("sbc の第1引数は, A|HL が使用できます", $params[0]->org);
                } else {
                    $this->errorLine_("sub の第1引数は, A が使用できます", $params[0]->org);
                }
                return '';
        }

        return ($params[1]->isWithCarry() ? 'sbc' : 'sub') . " " . $params[0]->out . ', ' .  $params[1]->out;
    }


    // MARK: and,or,xor,cp
    private function parseAnd_(string $opcode, string $operandsStr): string
    {
        $params = $this->createOperands_($opcode, $operandsStr, 2);
        if ($params === null) { return ''; }

        $this->checkTypeOneOf_($opcode, $params[0], 1, [Operand::TYPE_A]);
        $this->checkTypeOneOf_($opcode, $params[1], 2, [
            Operand::TYPE_A, Operand::TYPE_B, Operand::TYPE_CDE8, Operand::TYPE_HL8, Operand::TYPE_IX8,
            Operand::TYPE_MEM_HL, Operand::TYPE_MEM_IX, Operand::TYPE_NUM, Operand::TYPE_SYM, Operand::TYPE_EXPR]);

        return "$opcode " . $params[0]->out . ', ' .  $params[1]->out;
    }


    // MARK: ex
    private function parseEx_(string $opcode, string $operandsStr): string
    {
        $params = $this->createOperands_($opcode, $operandsStr, 2);
        if ($params === null) { return ''; }

        switch ($params[0]->type) {
            case Operand::TYPE_AF:                  // ex(AF, AF)
                $this->checkTypeOneOf_($opcode, $params[1], 2, [Operand::TYPE_AF]);
                break;
            case Operand::TYPE_DE:                  // ex(DE, HL)
                $this->checkTypeOneOf_($opcode, $params[1], 2, [Operand::TYPE_HL]);
                break;
            case Operand::TYPE_MEM_SP:              // ex(mem[SP], HL)
                $this->checkTypeOneOf_($opcode, $params[1], 2, [Operand::TYPE_HL, Operand::TYPE_IX]);
                break;
            default:
                $this->errorLine_("$opcode の第1引数は, AF|DE|mem[SP] が使用できます", $params[0]->org);
                return '';
        }

        return "$opcode " . $params[0]->out . ', ' .  $params[1]->out;
    }


    // MARK: bit,set,res
    private function parseBit_(string $opcode, string $operandsStr): string
    {
        $params = $this->createOperands_($opcode, $operandsStr, 2);
        if ($params === null) { return ''; }

        if ($this->checkTypeOneOf_($opcode, $params[0], 1, [Operand::TYPE_NUM, Operand::TYPE_SYM, Operand::TYPE_EXPR]) &&
            $this->checkTypeOneOf_($opcode, $params[1], 2, [
                Operand::TYPE_A, Operand::TYPE_B, Operand::TYPE_CDE8, Operand::TYPE_HL8,
                Operand::TYPE_IX8, Operand::TYPE_MEM_HL, Operand::TYPE_MEM_IX])) {
            return "$opcode " . $params[0]->out . ', ' . $params[1]->out;
        }
        return '';
    }

    // -------------------------------- パラメータの型チェックとコード生成 (シフト/ローテート)
    // MARK:parseShiftRotate8Core_()
    /** シフト・ローテートの出力中核部分(8bit版)
     * @param $opcode       8bit  時の opcode
     * @param $params       Operand の配列(操作対象と回数)
     * @param $use_param0   rrca のように オペランドが無ければ false, rrc A のように必要ならば true
     */
    private function parseShiftRotate8Core_(string $opcode, array $params, bool $use_param0): string
    {
        $out = '';

        // 第2引数が TYPE_NUM の場合は値を調査して展開, TYPE_SYM の場合は rept 疑似命令
        if ($params[1]->type === Operand::TYPE_NUM) {
            $n = $params[1]->value;
            if ($n < 1 || 8 <= $n) {
                $this->errorLine_("$opcode 第二引数の範囲は, [1, 7]です", $params[1]->org);
                return $out;
            }
            for ($i = 0; $i < $n; $i++) {
                if ($i != 0) { $out .= $this->getIndentStr_(); }
                $out .= $opcode . ($use_param0 ? (' ' . $params[0]->out) : '');
                if ($i != $n - 1) { $out .= "\n"; }
            }
        } else {
            $out .= "rept " . $params[1]->out . "\n";
            $this->incIndent_();
            $out .= $this->getIndentStr_() . $opcode . ($use_param0 ? (' ' . $params[0]->out) : '') . "\n";
            $this->decIndent_();
            $out .= $this->getIndentStr_() . "endr";
        }

        return $out;
    }

    // MARK:parseShiftRotate16Core_()
    /** シフト・ローテートの出力中核部分(16bit版)
     * @param $opcode1   16bit 時の最初に実行される opcode
     * @param $opcode2   16bit 時の2番目に実行される opcode
     * @param $is_left   左シフトなら true (16bit時は下8bit->上8bitの順で実行)
     * @param $params    Operand の配列(操作対象と回数)
     */
    private function parseShiftRotate16Core_(string $opcode1, string $opcode2, bool $is_left, array $params): string
    {
        $out = '';

        // 第2引数が TYPE_NUM の場合は展開, TYPE_SYM, Operand::TYPE_EXPR の場合はリピートシンボル
        if ($is_left) {
            $reg1 = substr($params[0]->value, 1);    // L
            $reg2 = substr($params[0]->value, 0, 1); // H
        } else {
            $reg1 = substr($params[0]->value, 0, 1); // H
            $reg2 = substr($params[0]->value, 1);    // L
        }

        if ($params[1]->type === Operand::TYPE_NUM) {
            $n = $params[1]->value;
            if ($n < 1 || 8 <= $n) {
                $this->errorLine_("$opcode のリピート範囲[1, 7]です", $params[1]->org);
                return $out;
            }
            for ($i = 0; $i < $n; $i++) {
                if ($i != 0) { $out .= $this->getIndentStr_(); }
                $out .= $opcode1 . ' ' . $reg1 . "\n";
                $out .= $this->getIndentStr_();
                $out .= $opcode2 . ' ' . $reg2;
                if ($i != $n - 1) { $out .= "\n"; }
            }
        } else {
            $out .= "rept " . $params[1]->out . "\n";
            $this->incIndent_();
            $out .= $this->getIndentStr_() . $opcode1 . ' ' . $reg1 . "\n";
            $out .= $this->getIndentStr_() . $opcode2 . ' ' . $reg2 . "\n";
            $this->decIndent_();
            $out .= $this->getIndentStr_() . "endr";
        }

        return $out;
    }


    // MARK: sla,sll,sra,srl,<<=
    /**
     * @param $opcode    8bit  時の opcode
     * @param $opcode1   16bit 時の最初に実行される opcode
     * @param $opcode2   16bit 時の2番目に実行される opcode
     * @param $is_left   左シフトなら true (16bit時は下8bit->上8bitの順で実行)
     * @param $nr_params 引数の数 1(1回のみ) or 2(n回)
     */
    private function parseShift_(string $opcode, string $opcode1, string $opcode2, bool $is_left, string $operandsStr, int $nr_params): string
    {
        $params = $this->createOperands_($opcode, $operandsStr, $nr_params);
        if ($params === null) { return ''; }

        // 第1引数のチェック
        if (!$this->checkTypeOneOf_($opcode, $params[0], 1, [
            Operand::TYPE_A, Operand::TYPE_B, Operand::TYPE_CDE8, Operand::TYPE_HL8, Operand::TYPE_MEM_HL, Operand::TYPE_MEM_IX,
            Operand::TYPE_HL, Operand::TYPE_DE, Operand::TYPE_BC])) { return ''; }

        // 第2引数のチェック
        if ($nr_params === 1) {
            $params[] = new Operand('1', Parser::MODE_FUNC); // なければ数値の1を追加
        } else if (!$this->checkTypeOneOf_($opcode, $params[1], 2, [Operand::TYPE_NUM, Operand::TYPE_SYM], Operand::TYPE_EXPR)) {
            return '';
        }

        // コード生成
        if ($params[0]->isTypeOneOf([Operand::TYPE_HL, Operand::TYPE_DE, Operand::TYPE_BC])) {
            return $this->parseShiftRotate16Core_($opcode1, $opcode2, $is_left, $params);
        }
        return $this->parseShiftRotate8Core_($opcode, $params, true);
    }


    // MARK: >>=
    private function parseRShift_(string $operandsStr, int $nr_params): string
    {
        $opcode = '>>=演算子';

        $params = $this->createOperands_($opcode, $operandsStr, 2);
        if ($params === null) { return ''; }

        if (!$this->checkTypeOneOf_($opcode, $params[0], 1, [
            Operand::TYPE_A, Operand::TYPE_B, Operand::TYPE_CDE8, Operand::TYPE_HL8, Operand::TYPE_MEM_HL, Operand::TYPE_MEM_IX,
            Operand::TYPE_HL, Operand::TYPE_DE, Operand::TYPE_BC])) { return ''; }

        // 第1引数のチェック
        if (!$this->checkTypeOneOf_($opcode, $params[0], 1, [
            Operand::TYPE_A, Operand::TYPE_B, Operand::TYPE_CDE8, Operand::TYPE_HL8, Operand::TYPE_MEM_HL, Operand::TYPE_MEM_IX,
            Operand::TYPE_HL, Operand::TYPE_DE, Operand::TYPE_BC])) { return ''; }

        // 第2引数のチェック
        if ($nr_params === 1) {
            $params[] = new Operand('1', Parser::MODE_FUNC); // 無ければ数値の1を追加. 文法エラーは起きないので cretaeOperand_() は使わない
        } else if (!$this->checkTypeOneOf_($opcode, $params[1], 2, [Operand::TYPE_NUM, Operand::TYPE_SYM, Operand::TYPE_EXPR])) {
            return '';
        }

        // コード生成
        if ($params[0]->isTypeOneOf([Operand::TYPE_HL, Operand::TYPE_DE, Operand::TYPE_BC])) {
            return $this->parseShiftRotate16Core_('sra', 'rr', false, $params);// 16bit ならば, signed
        }
        return $this->parseShiftRotate8Core_('srl', $params, true); // 8bit ならば, unsigned
    }


    // MARK: rl,rr,rlc,rrc
    /**
     * @param $nr_params 引数の数 1(1回のみ) or 2(n回)
     */
    private function parseRotate_(string $opcode, string $operandsStr, int $nr_params): string
    {
        $params = $this->createOperands_($opcode, $operandsStr, $nr_params);
        if ($params === null) { return ''; }

        // 第1引数のチェック
        if (!$this->checkTypeOneOf_($opcode, $params[0], 1, [Operand::TYPE_A, Operand::TYPE_B, Operand::TYPE_CDE8, Operand::TYPE_HL8, Operand::TYPE_MEM_HL, Operand::TYPE_MEM_IX])) {
            return '';
        }

        // 第2引数のチェック
        if ($nr_params === 1) {
            $params[] = new Operand('1', Parser::MODE_FUNC); // 無ければ数値の1を追加. 文法エラーは起きないので cretaeOperand_() は使わない
        } else {
            if (!$this->checkTypeOneOf_($opcode, $params[1], 2, [Operand::TYPE_NUM, Operand::TYPE_SYM, Operand::TYPE_EXPR])) {
                return '';
            }
        }

        return $this->parseShiftRotate8Core_($opcode, $params, true);
    }


    // MARK: sla2,sll2,sra2,srl2,rl2,rr2,rlc2,rrc2
    /** sll mem[ix + d], r 等 */
    private function parseShift2_($opcode, $operandsStr)
    {
        $params = $this->createOperands_($opcode, $operandsStr, 2);
        if ($params === null) { return ''; }

        if ($this->checkTypeOneOf_($opcode, $params[0], 1, [Operand::TYPE_MEM_IX])) {
            if ($this->checkTypeOneOf_($opcode, $params[1], 2, [Operand::TYPE_A, Operand::TYPE_B, Operand::TYPE_CDE8, Operand::TYPE_HL8])) {
                return "$opcode " . $params[0]->out . ', ' .  $params[1]->out;
            }
        }

        return '';
    }


    // MARK: rla_n,rra_n,rlca_n,rrca_n
    private function parseRotateA_(string $opcode, string $operandsStr): string
    {
        $params = $this->createOperands_($opcode, $operandsStr, 2);
        if ($params === null) { return ''; }

        if ($this->checkTypeOneOf_($opcode, $params[0], 1, [Operand::TYPE_A])) {
            if ($this->checkTypeOneOf_($opcode, $params[1], 2, [Operand::TYPE_NUM, Operand::TYPE_SYM, Operand::TYPE_EXPR])) {
                return $this->parseShiftRotate8Core_($opcode, $params, false);
            }
        }

        return '';
    }


    // MARK: rld,rrd
    private function parseRld_(string $opcode, string $operandsStr): string
    {
        $params = $this->createOperands_($opcode, $operandsStr, 2);
        if ($params === null) { return ''; }

        if ($this->checkTypeOneOf_($opcode, $params[0], 1, [Operand::TYPE_A])) {
            if ($this->checkTypeOneOf_($opcode, $params[1], 2, [Operand::TYPE_MEM_HL])) {
                return "$opcode " . $params[0]->out . ', ' .  $params[1]->out;
            }
        }

        return '';
    }


    // MARK: bit3,set3,res3
    private function parseBit3_(string $opcode, string $operandsStr): string
    {
        $params = $this->createOperands_($opcode, $operandsStr, 3);
        if ($params === null) { return ''; }

        $this->checkTypeOneOf_($opcode, $params[0], 1, [Operand::TYPE_NUM, Operand::TYPE_SYM, Operand::TYPE_EXPR]);
        $this->checkTypeOneOf_($opcode, $params[1], 2, [Operand::TYPE_MEM_IX]);
        $this->checkTypeOneOf_($opcode, $params[2], 3, [Operand::TYPE_A, Operand::TYPE_B, Operand::TYPE_CDE8, Operand::TYPE_HL8]);

        return "$opcode " . $params[0]->out . ', ' .  $params[1]->out . ', ' .  $params[2]->out;
    }


    // -------------------------------- パラメータの型チェックとコード生成 (ジャンプ/コール)
    // MARK: jp
    private function parseJp_(string $operandsStr): string
    {
        $opcode = 'jp';
        $params = $this->createOperands_($opcode, $operandsStr, 1);
        if ($params === null) { return ''; }

        if ($this->checkTypeOneOf_($opcode, $params[0], 1, [Operand::TYPE_NUM, Operand::TYPE_SYM, Operand::TYPE_EXPR, Operand::TYPE_HL, Operand::TYPE_IX])) {
            // HL, IX の場合のみ (HL) のようにカッコを付ける必要があります
            if ($params[0]->isTypeOneOf([Operand::TYPE_HL, Operand::TYPE_IX])) {
                return "$opcode (" . $params[0]->out . ')';
            } else {
                return "$opcode " . $params[0]->out;
            }
        }

        return '';
    }


    // MARK: jp,jr,call
    private function parseJpJrCall_(string $opcode, string $operandsStr): string
    {
        $params = $this->createOperands_($opcode, $operandsStr, 1);
        if ($params === null) { return ''; }

        if ($this->checkTypeOneOf_($opcode, $params[0], 1, [Operand::TYPE_NUM, Operand::TYPE_SYM, Operand::TYPE_EXPR])) {
            return "$opcode " . $params[0]->out;
        }

        return '';
    }


    // MARK: djnz
    private function parseDjnz_(string $operandsStr): string
    {
        $opcode = 'djnz';

        $params = $this->createOperands_($opcode, $operandsStr, 2);
        if ($params === null) { return ''; }

        if ($this->checkTypeOneOf_($opcode, $params[0], 1, [Operand::TYPE_B])) {
            if ($this->checkTypeOneOf_($opcode, $params[1], 2, [Operand::TYPE_NUM, Operand::TYPE_SYM, Operand::TYPE_EXPR])) {
                return "$opcode B," . $params[1]->out;
            }
        }

        return '';
    }


    // MARK: rst
    private function parseRst_(string $operandsStr): string
    {
        $opcode = 'rst';

        $params = $this->createOperands_($opcode, $operandsStr, 1);
        if ($params === null) { return ''; }

        if ($this->checkTypeOneOf_($opcode, $params[0], 1, [Operand::TYPE_NUM, Operand::TYPE_SYM, Operand::TYPE_EXPR])) {
            if ($params[0]->type === Operand::TYPE_NUM) {
                if ($params[0]->value < 0 || 0x38 < $params[0]->value || ($params[0]->value & 0x07) !== 0) {
                    $this->errorLine_("$opcode の引数は, 0x00,0x08,0x10,0x18,0x20,0x28,0x30,0x38 が使用できます", $params[0]->org);
                    return '';
                }
            }
            return "$opcode " . $params[0]->out;
        }

        return '';
    }


    // -------------------------------- パラメータの型チェックとコード生成 (引数 n)
    // MARK: push,pop
    private function parsePushPop_(string $opcode, string $operandsStr): string
    {
        return $this->parseVariadicCommon_($opcode, $operandsStr, 1, '',
            function(string $opcode, Operand $operand, int $index) : string {
                if ($this->checkTypeOneOf_($opcode, $operand, $index, [Operand::TYPE_HL, Operand::TYPE_DE, Operand::TYPE_BC, Operand::TYPE_AF, Operand::TYPE_IX])) {
                    return "$opcode " . $operand->out;
                }
                return '';
            },
        $this->getIndentStr_(), "\n");
    }


    // ---------------------------------------------------------------- ユーティリティ
    // -------------------------------- システム
    // MARK: utils: system
    private function errorLine_(string $msg, string $src = ''): void
    {
        $this->error_->errorLine($this->line_nr_, $msg, $src);
    }


    // -------------------------------- ラベル
    // MARK: utils: label
    /** ラベルを初期化します */
    private function initLabels_(): void
    {
        $this->label_ct_ = 0;
        $this->labels_ = [];
    }


    /** 複数のラベル名を生成し, 登録もしますます
     * - 関数名が foo ならば, genRegistLabels_(["while", "wend"]) と書くと,
     *   foo_while10, foo_wehd10 といったラベルを生成し, 登録します
     * @return ラベル名の配列
    */
    private function genRegistLabels_(array $strings): array
    {
        $ret = [];
        foreach ($strings as $str) {
            $label = $this->funcname_ . '__' . $str . $this->label_ct_;
            $this->registLabel_($label);
            $ret[] = $label;
        }
        $this->label_ct_++;
        return $ret;
    }
    private function genRegistLabel_(): string
    {
        $ret = $this->funcname_ . '__' . $this->label_ct_;
        $this->label_ct_++;
        $this->registLabel_($ret);
        return $ret;
    }


    /** ラベル名を登録します */
    private function registLabel_(string $label): void
    {
        $this->labels_[] = $label;
    }


    /** 登録したラベル一覧を出力します. マクロ時の local 疑似命令に使います */
    public function getLabels(): array
    {
        return $this->labels_;
    }


    // -------------------------------- インデント
    // MARK: utils: indent
    /** インデントを初期化します */
    private function initIndent_(): void { $this->indent_lv_ = 0; }


    /** インデント レベルを進めます */
    private function incIndent_(): void { $this->indent_lv_++; }


    /** インデント レベルを戻します */
    private function decIndent_(): void { $this->indent_lv_--; }


    /** インデント文字列を生成します */
    private function getIndentStr_(): string
    {
        $ret = '';
        for ($i = 0; $i < $this->indent_lv_; $i++) {
            $ret .= '  ';
        }
        return $ret;
    }


    // -------------------------------- 空白文字(半角スペースと改行)
    // MARK: utils: spaces
    /**
     * $r_str の $r_offset 文字目から, $string の文字数分と, それに続く空白文字をスキップします.
     * $string や空白文字内に改行があれば行数もカウントします
     * @param $r_str 対象文字列
     * @param $offset $r_str の文字位置(0～)
     * @param $string スキップする文字列
     * @return スキップした空白文字の次の位置
     */
    private function skipStringSpaces_(string &$r_str, int $offset, string $string = ''): int
    {
        $offset += strlen($string);
        $this->line_nr_ += substr_count($string, "\n");

        if (preg_match('/[\ \n]+/Ax', $r_str, $matches, 0, $offset)) {
            $offset += strlen($matches[0]);
            $this->line_nr_ += substr_count($matches[0], "\n");
        }

        return $offset;
    }


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


    // -------------------------------- 括弧内の検出
    // MARK: utils: parentheses
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


    /** 空白文字に続く中カッコ {...} で囲まれた中の文字列を抽出します
     * - 引数などは, 上の extractParentheses_() を参照してください
     */
    private function extractCurlyBracket_(string &$r_str, int &$r_offset): string|false
    {
        return $this->extractParenthesesSub_($r_str, $r_offset, '{', '}');
    }


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


    // -------------------------------- オペランドの生成, 型チェック
    // MARK: utils: operands
    /** expr を Operand オブジェクトの配列にします
     * n 個の Operand が得られなければエラー処理して null を返します
     * @param $opcode エラー(n 個のオペランドが得られなかっや)時に表示するオペコード
     * @param $expr 判別したい文字列
     * @param $n    期待されるオペランドの数
     */
    private function createOperands_(string $opcode, string $expr, int $n): array|null
    {
        $params = $this->explodeByComma_($expr);
        if (count($params) != $n) {
            //echo("------"); print_r($expr); echo("------>"); print_r($params);
            $this->errorLine_("$opcode の引数は $n 個です", $expr);
            return null;
        }

        $ret = [];
        foreach($params as $i => $operand) {
            $p = $this->createOperand_($operand);
            if ($p !== null) {
                $ret[] = $p;
            }
        }
        if (count($ret) !== $n) { return null; }
        return $ret;
    }


    /** 式から, オペランド オブジェクトを生成します. new Operand のエラー付バージョン
     * モードによって挙動が異なります
     * @return エラーなら, エラー表示して null
     */
    private function createOperand_(string $expr): Operand|null
    {
        $operand = new Operand($expr, $this->mode_);
        if ($operand->type === Operand::TYPE_OTHER) {
            $this->errorLine_("未知の式です", $expr);
            return null;
        }
        return $operand;
    }



    /**
     * パラメータ $operand の型が, $types のいずれでも無いなら errorLine_() します
     * $opcode はエラー時のメッセージに含まれる命令, $index は同じくエラー時に表示される引数番号(1～)です
     * @return エラーなら false
     */
    private function checkTypeOneOf_(string $opcode, Operand $operand, int $index, array $types): bool
    {
        if ($operand->isTypeOneOf($types)) {
            return true;
        }

        // エラー文字列の凡例の作成
        $legends = [];
        foreach ($types as $type) {
            $legends[] = Operand::LEGEND_TAB[$type];
        }

        $this->errorLine_("$opcode の第 $index 引数は, " . implode('|', $legends) . " が使用できます", $operand->org);
        return false;
    }


    // -------------------------------- パース関係
    // MARK: utils: parse
    /** 可変パラメータ系 parse() の共通関数
     * @param $opcode       出力する際のオペコード
     * @param $operandsStr  カッコの中の文字列(オペランド文字列)
     * @param $out          出力初期値
     * @param $min_nr_args  最低引数の数
     * @param $callback     引数毎に呼び出される関数. 引数は,
     *    string  $opcode
     *    Operand   $operand
     *    int     $index(引数番号. 1～)
     *    returns その引数を処理した結果の文字列
     * @param $top  $callback 戻値の前につける文字列. 一番最初の引数には付きません
     * @param $last $callback 戻値の後につける文字列. 一番最後の引数には付きません
     */
    private function parseVariadicCommon_(string $opcode, string $operandsStr, int $min_nr_args, string $out,
        callable $callback, string $top, string $last): string
    {
        $paramStrs = $this->explodeByComma_($operandsStr);
        if (count($paramStrs) < $min_nr_args) {
            $this->errorLine_("$opcode: 引数は最低でも $min_nr_args 個必要です");
            return '';
        }

        foreach($paramStrs as $i => $paramStr) {
            $operand = $this->createOperand_($paramStr);
            if ($operand === null) {
                $this->errorLine_("$opcode 引数 ". ($i + 1) . " のエラー[", $paramStr, "]");
                return '';
            }

            if ($i !== 0) { $out .= $top; }
            $out .= $callback($opcode, $operand, $i + 1);
            if ($i !== count($paramStrs) - 1) { $out .= $last; }
        }

        return $out;
    }


    // -------------------------------- その他文字処理
    // MARK: utils: etc
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


    private function swap_(&$a, &$b): void
    {
        $c = $a;
        $a = $b;
        $b = $c;
    }
}
