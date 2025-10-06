; ローダー用 CRT0
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
;        PUBLIC  l_dcal          ; 関数ポインタの飛び先 ローダーでは不要

        org     CRT_ORG_CODE    ; -zorg で定義

; -------- コード開始
start:
        di
;        call	crt0_init_bss ローダーでは不要

        call    0x0006          ; LETNL
        ld      DE, msg_starting
        call    0x0015          ; MSG

; -------- モニタ ROM, VRAM を切り離して オール RAM 構成にします
        ld      c, 0xe0
        out     (c), a          ; モニタ ROM を RAM に. 値は何でもいい
        inc     c
        out     (c), a          ; VRAM を RAM に.       値は何でもいい

; -------- スタックを設定してジャンプ(no return)
        ld      sp, STACK       ; リンカで定義
        jp      _main

; -------- 関数ポインタの飛び先
;l_dcal:
;        jp	(hl)


; -------- 起動中メッセージ表示
msg_starting:
        db      "STARTING UP...", 0x0d

; -------- これが無いと DATA 領域が空っぽ
;        INCLUDE "crt/classic/crt_section.asm" ローダーでは不要
