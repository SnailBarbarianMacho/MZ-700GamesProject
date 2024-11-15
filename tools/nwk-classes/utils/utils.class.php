<?php
/** ユーティリティ
 *
 * @author Snail Barbarian Macho (NWK) 2024.06.06
 */

declare(strict_types = 1);
namespace nwk\utils;


// MARK: Utils
Class Utils
{

    // ---------------------------------------------------------------- 引数解析
    // MARK: getOpt()
    /**
     * getopt() の高機能バージョン
     * @param $argv        引数一覧を下さい
     * @param $short_opts  '-' で指定する1文字形式オプション(文字列の配列)
     *                     getopt() では文字列ですが, getOpt() では $long_opts と同じ配列形式です
     *                     getopt() と同じ. ':'(=値は必須) や '::'(=値は任意) も使えます
     * @param $long_opts   '--' で指定するロング形式オプション(文字列の配列)
     *                     getopt() と同じ. ':'(=値は必須) や '::'(=値は任意) も使えます
     * @param $r_left_args オプション以外の引数を返します (文字列の配列)
     * @param $r_err       問題の起きた引数一覧を返します (スペースで区切った文字列. エラー無ければ空文字)
     *                     重複, 未知のオプション, 値の指定の間違いなど.
     * @return getopt() と同じ
     * @example
     *   $args = [];
     *   $errs = '';
     *
     *   // -h, --help, -c123, --channel=123, --bars が使えます
     *   $options = \nwk\utils\Utils::getOpt($argv,
     *       array('h', 'c:'),
     *       array('help', 'channels:', 'bars::'),
     *       $args, $errs);
     *
     *   if ($errs) {
     *       echo("Invalid option(s):$errs\n"); exit(1);
     *   }
     *   if (isset($options['help'] || isset($options['h']) {
     *       usage();
     *   }
     *   if (isset($options['channels'] || isset($options['c'])) {
     *       // $t に引数が入ります. 両方とも指定した場合どちらを採用するかは実装依存(手抜き)
     *       $t = isset($options['channels']) ? $options['channels'] : $options['c'];
     *       // ...
     *   }
     */
    public static function getOpt(array $argv,
        array $short_opts, array $long_opts,
        array &$r_left_args, string &$r_err): ?array
    {
        $argv = array_slice($argv, 1);
        $r_left_args = [];
        $r_err = '';
        $ret = [];

        $short_opts_arr = [];
        $long_opts_arr = [];

        // オプションリストを検索しやすい形に置き換える
        foreach ($short_opts as $opt) {
            if (preg_match('/^([0-9A-Za-z])(\:{0,2})$/', $opt, $matches) !== false) {
                switch ($matches[2]) {
                    default:   $short_opts_arr[$matches[1]] = ''; break;
                    case ':':  $short_opts_arr[$matches[1]] = ':'; break;
                    case '::': $short_opts_arr[$matches[1]] = '::'; break;
                }
            }
        }

        foreach ($long_opts as $opt) {
            if (preg_match('/^([0-9A-Za-z_]+)(\:{0,2})$/', $opt, $matches) !== false) {
                switch ($matches[2]) {
                    default:   $long_opts_arr[$matches[1]] = ''; break;
                    case ':':  $long_opts_arr[$matches[1]] = ':'; break;
                    case '::': $long_opts_arr[$matches[1]] = '::'; break;
                }
            }
        }

        // オプションの調査
        foreach ($argv as $arg) {
            if (preg_match('/^-([0-9A-Za-z_])(([\s\S]*)?)$/', $arg, $matches)) {
                $opt = $matches[1];
                $val = $matches[2];
                //echo("$opt $val\n");
                if (array_key_exists($opt, $short_opts_arr) === false) { // 未知のオプション
                    $r_err .= ' ' . $matches[0];
                } else if (isset($ret[$opt])) { // 多重オプション指定
                    unset($ret[$opt]);
                    $r_err .= ' ' . $matches[0];
                } else {
                    $mode = $short_opts_arr[$opt];
                    if ($mode === '' && $val !== '') {
                        $r_err .= ' ' . $matches[0]; // 値禁止なのに値が設定されている
                    } else if ($mode === ':' && $val === '') {
                        $r_err .= ' ' . $matches[0]; // 値必須なのに値が設定されてない
                    } else {
                        $ret[$opt] = $val;
                    }
                }

            } else if (preg_match('/^--([0-9A-Za-z_]+)((=[\s\S]*)?)$/', $arg, $matches)) {
                $opt = $matches[1];
                $val = substr($matches[2], 1); // '--aaa=' のように = で終ってる場合は,「値無し」とみなす
                if (array_key_exists($opt, $long_opts_arr) === false) {// 未知のオプション
                    $r_err .= ' ' . $matches[0];
                } else if (isset($ret[$opt])) { // 多重オプション指定
                    unset($ret[$opt]);
                    $r_err .= ' ' . $matches[0];
                } else {
                    $mode = $long_opts_arr[$opt];
                    if ($mode === '' && $val !== '') {
                        $r_err .= ' ' . $matches[0]; // 値禁止なのに値が設定されている
                    } else if ($mode === ':' && $val === '') {
                        $r_err .= ' ' . $matches[0]; // 値必須なのに値が設定されてない
                    } else {
                        $ret[$opt] = $val;
                    }
                }
            } else {
                $r_left_args[] = $arg;
            }
        }

        $r_err = trim($r_err);
        return $ret;
    }


    // MARK: getRangeList()
    /** 範囲文字列を配列に変更します
     * @param $range_str 範囲文字列
     * - [0-9,-#]の他, 文字列「all」,「none」 が使えます
     * - 重複, はみ出しは無視します
     * - 不正文字列ならエラー
     * @param $max 範囲の最大値
     * @returns 範囲を変換した配列. エラー発生時は false
     * @example
     * getRangeListt('-1,4-6,8#2,11-', 0, 12);
     * → array(0,1,4,5,6,8,9,11,12) を返します
     * getRangeListt('-1,30,99-', 5, 10);
     * getRangeList('none', 0, 10);
     * → これらは空配列を返します
     * getRangeList('all', 0, 10);
     * → array(0,1,2,3,4,5,6,7,8,9,10) を返します
     * getRangeList('4-2', 0, 10);
     * getRangeList('123a!', 0, 10);
     * → これらは false を返します
     */
    public static function getRangeList(string $range_str, int $min, int $max): array|false
    {
        if (!is_string($range_str) || !is_integer($max)) {
            return false;
        }

        $ret = [];
        switch ($range_str) {
            case 'none': break;

            case 'all':
                for ($i = $min; $i <= $max; $i++) {
                    $ret[] = $i;
                }
                break;

            default:
                $list = explode(',', $range_str);
                foreach($list as $val) {
                    // '-' が先頭にある数字
                    if (str_starts_with($val, '-')) {
                        $val = substr($val, 1);
                        if (!is_numeric($val)) {
                            return false;
                        }
                        $val = min((int)$val, $max);
                        for ($i = $min; $i <= $val; $i++) {
                            $ret[] = $i;
                        }
                        continue;
                    }
                    // '-' が末尾にある数字
                    if (str_ends_with($val, '-')) {
                        $val = substr($val, 0, -1);
                        if (!is_numeric($val)) {
                            return false;
                        }
                        $val = max((int)$val, $min);
                        for ($i = $val; $i < $max; $i++) {
                            $ret[] = $i;
                        }
                        continue;
                    }
                    // '数字#数字' 形式
                    $vals = explode('#', $val);
                    if (2 <= count($vals)) {
                        if (count($vals) !== 2) {
                            return false;
                        }
                        $val0 = $vals[0];
                        $val1 = $vals[1];
                        if (!is_numeric($val0) || !is_numeric($val1)) {
                            return false;
                        }
                        if ($val0 < 0 || $val1 < 0) {
                            return false;
                        }
                        $val0 = max((int)$val0, $min);
                        $val1 = min((int)$val0 + $val1, $max);
                        //echo("$val0 $val1\n");
                        for ($i = $val0; $i <= $val1; $i++) {
                            $ret[] = $i;
                        }
                        continue;
                    }
                    // '数字-数字' 形式
                    $vals = explode('-', $val);
                    if (2 <= count($vals)) {
                        if (count($vals) !== 2) {
                            return false;
                        }
                        $val0 = $vals[0];
                        $val1 = $vals[1];
                        if (!is_numeric($val0) || !is_numeric($val1)) {
                            return false;
                        }
                        if ($val1 < $val0) {
                            return false;
                        }
                        $val0 = max((int)$val0, $min);
                        $val1 = min((int)$val1, $max);
                        //echo("$val0 $val1\n");
                        for ($i = $val0; $i <= $val1; $i++) {
                            $ret[] = $i;
                        }
                        continue;
                    }
                    // 数字単体
                    {
                        if (!is_numeric($val)) {
                            return false;
                        }
                        if ($min <= $val && $val <= $max) {
                            $ret[] = (int)$val;
                        }
                        continue;
                    }
                } // foreach ($list)

                $ret = array_unique($ret);
                $ret = array_values($ret);
                sort($ret);
        }

        return $ret;
    }


    // MARK: isRangeListContinuous()
    /** getRangeList() で得た配列が, 連続してるか調査します */
    public static function isRangeListContinuous(array $range_list): bool
    {
        $v = $range_list[0] + 1;
        for($i = 1; $i < count($range_list); $i++) {
            if ($v !== $range_list[$i]) {
                return false;
            }
            $v++;
        }
        return true;
    }

    // MARK: getRangeListString()
    /** getRangeList() で得た配列から, verbose 用の文字列を作ります
     * @param $is_all true ならば ' all' を返します
     */
    public static function getRangeListString(array $range_list, bool $is_all): string
    {
        if ($is_all) {
            return ' all';
        }

        if (count($range_list) === 0) {
            return ' none';
        }

        $str = '';
        foreach($range_list as $v) {
            $str .= " $v";
        }
        return $str;
    }


    // ---------------------------------------------------------------- ダンプ
    // MARK: byteDumpDec()
    /** C ソース風バイト ダンプ(10進版) */
    static function byteDumpDec(array $data): string
    {
        $str = '';
        foreach ($data as $i => $val) {
            $str .= "$val, ";
            if (($i & 0x03) === 0x03) { $str .= " "; }
            if (($i & 0x0f) === 0x0f) { $str .= "\n"; }
            if (($i & 0x3f) === 0x3f) { $str .= "\n"; }
        }
        return $str;
    }


    // MARK: byteDumpHex()
    /** C ソース風バイト ダンプ(16進版) */
    static function byteDumpHex(array $data): string
    {
        $str = '';
        foreach ($data as $i => $val) {
            $str .= sprintf("0x%02x, ", $val);
            if (($i & 0x03) === 0x03) { $str .= " "; }
            if (($i & 0x0f) === 0x0f) { $str .= "\n"; }
            if (($i & 0x3f) === 0x3f) { $str .= "\n"; }
        }
        return $str;
    }


}