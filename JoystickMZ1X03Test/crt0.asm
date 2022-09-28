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

; -------- スタックを設定してジャンプ(no return)
        ld      sp, $2000
        jp      _main

; -------- 関数ポインタの飛び先
_funcPtr:
l_dcal:
        jp	(hl)

; -------- これが無いと DATA 領域が空っぽ
        INCLUDE "crt/classic/crt_section.asm"
