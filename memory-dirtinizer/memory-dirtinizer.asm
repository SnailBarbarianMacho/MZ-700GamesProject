; Memory Dirtinizer
;
; @author Snail Barbarian Macho (NWK)
; @date 2021.10.19

; -------- コード開始
start:
        di

; -------- モニタ ROM, VRAM を切り離して オール RAM 構成にします
        ld      c, $e0
        out     (c), a
        inc     c
        out     (c), a

; -------- メモリを汚します

        ld      hl, end
        ld      de, end + 1
        ld      bc, $fff0
        ldir

; -------- リセットしてモニターに戻ります
        ld      c, $e4
        out     (c), a
        jp      $0000

; -------- このデータで汚染します
end:
        halt
