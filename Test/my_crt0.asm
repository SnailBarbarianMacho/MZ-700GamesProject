; 殆どの機能を削った CRT0
;
; - スタックを設定して $1200 にジャンプするだけ
; - main() は引数無し, リターンしない(すると暴走する)
; - bss セクションのクリアをしないので, 静的変数は各自初期化してください
;
; @author Snail Barbarian Macho (NWK)
; @date 2012.06.23

        defc    crt0 = 1
        INCLUDE "zcc_opt.def"

        EXTERN  _main           ; main() のシンボル
        PUBLIC  l_dcal          ; 関数ポインタの飛び先

        IF      !DEFINED_CRT_ORG_CODE
                defc    CRT_ORG_CODE = $1200
        ENDIF
        org     CRT_ORG_CODE

; -------- コード開始
start:
        call	crt0_init_bss

; -------- サウンドのため, 8255 の初期化
; (e008) = 0x01
; (e007) = 0x36
        ld      hl,  $e008
        ld      (hl), $01
        dec     l
        ld      (hl), $36
        
; -------- スタックを設定してジャンプ
        ld      sp, $d000
        jp      _main

; -------- 関数ポインタの飛び先
l_dcal:	
        jp	(hl)

; -------- これが無いと DATA 領域が空っぽ
        INCLUDE "crt/classic/crt_section.asm"
