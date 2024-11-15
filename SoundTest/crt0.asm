; 殆どの機能を削った CRT0 (テスト プログラム用)
;
; - main(void) に飛びます. カセットからロードした場合, リターンするとモニター ROM に戻ります
; - bss セクションの初期化は行いません
; - ℤ88dk の RAM モデルの為, data セクション, bss セクションの変数を変更すると, 再起動しても元に戻りません
;
; @author Snail Barbarian Macho (NWK)
; @date 2024.10.25

    defc    crt0 = 1
    INCLUDE "zcc_opt.def"

    EXTERN  _main           ; main() のシンボル
    PUBLIC  l_dcal          ; 関数ポインタの飛び先
    PUBLIC  _funcPtr        ; ↑と同じ(Cから使う)

    org     CRT_ORG_CODE

; -------- コード開始
start:
    di

; -------- サウンドのため, 8255 の初期化
; (e008) = 0x00 ※音を鳴らすなら 0x01
; (e007) = チャンネル0 下位バイト→上位バイトR/W モード3
    xor     A
    ld      HL, 0xe008
    ld      (HL), A
    dec     L
    ld      (HL), 0x36

; -------- モニタ ROM, VRAM を切り離して オール RAM 構成にします
    ld      C, 0xe0
    out     (C), A  ; モニタ ROM を RAM に. 値は何でもいい
    inc     C
    out     (C), A  ; VRAM を RAM に.      値は何でもいい

; -------- スタックを設定してジャンプ(no return)
;    ld      SP, 0x0000 テスト用なので, モニターROMのをそのまま使う
    jr      _main

; -------- 関数ポインタの飛び先
_funcPtr:
l_dcal:
    jp      (HL)

; -------- これが無いと DATA 領域が空っぽ
;   INCLUDE "crt/classic/crt_section.inc"

; -------- 各セクションの定義
;    INCLUDE "crt/classic/crt_section.inc"
    SECTION CODE
    SECTION code_compiler
    SECTION code_crt_init
    SECTION code_l
    SECTION code_l_sccz80
    SECTION code_l_sdcc
    SECTION code_clib
    SECTION code_math
    SECTION code_error
    SECTION CODE_END

    SECTION RODATA
    SECTION rodata_compiler
    SECTION RODATA_END

    SECTION DATA
    SECTION data_compiler
    SECTION DATA_END

    SECTION BSS
    SECTION bss_compiler
    SECTION bss_clib
    SECTION bss_error
    SECTION BSS_END
