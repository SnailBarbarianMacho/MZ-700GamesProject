; 殆どの機能を削った CRT0
;
; - スタックを設定して main() にジャンプするだけ
; - main() は引数無し, リターンしない(すると暴走する)
; - bss セクションのクリアをしないので, 静的変数は各自初期化してください
;
; @author Snail Barbarian Macho (NWK)
; @date 2012.06.23

        defc    crt0 = 1
        INCLUDE "zcc_opt.def"

        EXTERN  _main           ; main() のシンボル
        PUBLIC  l_dcal          ; 関数ポインタの飛び先
        PUBLIC  _funcPtr        ; ↑と同じ(Cから使う)

        IF      !DEFINED_CRT_ORG_CODE
                defc    CRT_ORG_CODE = $2000
        ENDIF
        org     CRT_ORG_CODE

; -------- コード開始
start:
        di
        call	crt0_init_bss

; -------- サウンドのため, 8255 の初期化
; (e008) = 0x00 ※音を鳴らすなら 0x01
; (e007) = チャンネル0 下位バイト→上位バイトR/W モード3
        ld      hl,  $e008
        xor     a
        ld      (hl), a
        dec     l
        ld      (hl), $36

; -------- モニタ ROM, VRAM を切り離して オール RAM 構成にします
        ld      c, $e0
        out     (c), a  ; モニタ ROM を RAM に. 値は何でもいい
        inc     c
        out     (c), a  ; VRAM を RAM に.      値は何でもいい

; -------- スタックを設定してジャンプ(no return)
        ld      sp, $0000
        jp      _main

; -------- 関数ポインタの飛び先
_funcPtr:
l_dcal:
        jp	(hl)

; -------- これが無いと DATA 領域が空っぽ
        INCLUDE "crt/classic/crt_section.asm"
