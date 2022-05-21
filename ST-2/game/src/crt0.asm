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
        PUBLIC  _func_ptr       ; ↑と同じ(Cから使う)
        PUBLIC  _build_nr       ; ビルド番号(Cから使う)

        org     CRT_ORG_CODE    ; -zorg で定義

; -------- コード開始
start:
        di
        call	crt0_init_bss

; -------- バンク切り替え
        ld      c, 0xe3
        out     (c), a          ; 上位アドレスをメモリマップドI/Oに

; -------- サウンドのため, 8255 の初期化
; (e008) = 0x00 ※音を鳴らすなら 0x01
; (e007) = チャンネル0 下位バイト→上位バイトR/W モード3
        ld      hl,  0xe008
        xor     a
        ld      (hl), a
        dec     l
        ld      (hl), 0x36

; -------- タイマーのため, 8255 の初期化
; (e007) = チャンネル1 下位バイト→上位バイトR/W モード4
; (e005) = タイマ値 0x0001
        ld      (hl), 0x78
        ld      l, 0x05
        ld      (hl), 0x01
        ld      (hl), a

; -------- モニタ ROM, VRAM を切り離して オール RAM 構成にします
        ld      c, 0xe0
        out     (c), a          ; モニタ ROM を RAM に. 値は何でもいい
        inc     c
        out     (c), a          ; VRAM を RAM に.      値は何でもいい

; -------- スタックを設定してジャンプ(no return)
        ld      sp, STACK       ; リンカで定義
        jp      _main

; -------- 関数ポインタの飛び先
_func_ptr:
l_dcal:
        jp	(hl)

; -------- これが無いと DATA 領域が空っぽ
        INCLUDE "crt/classic/crt_section.asm"

; -------- ビルド番号が収められる
_build_nr:
       INCLUDE "build_nr.asm"
