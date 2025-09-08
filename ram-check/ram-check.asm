; RAM Check
;
; @author Snail Barbarian Macho (NWK)
; @date 2025.09.05

    extern  __head
    extern  __tail

    ; -------- コード開始
start:
    di

    ; -------- チェックサム計算
    ld      HL, __head
    ld      BC, __tail-__head
    ld      DE, 0x0000                                  ; DE = 計算したチェックサム値

checkSum:
        ld      A, E
        add     A, (HL)
        ld      E, A
        jr      nc, checkSum_100
            inc     D
checkSum_100:
        inc     HL
        dec     BC 
        ld      A, B
        or      C       
    jr      nz, checkSum

    ld      HL, (__tail)                                ; チェックサム値が入ってる
    or      A
    sbc     HL, DE
    jr      z, checkSum_ok

    call    0x0006                                      ; 改行
    ld      DE,  checkSumErrorStr
    call    0x0015                                      ; ASCII文字列表示
    halt
checkSumErrorStr:
    db      "CHECK SUM ERROR", 0x0d

checkSum_ok:

    ; -------- バンク切替マクロ
macro   bankAllRam
        ld      C, 0xe0
        out     (C), A
        inc     C
        out     (C), A
    endm

macro   bankRomVram
        ld      C, 0xe4
        out     (C), A
    endm

    defc    vram_head  = 0xd000
    defc    vram_tail  = 0xe000
    defc    vram_size  = vram_tail-vram_head
    defc    ram_head   = __tail
    defc    ram_tail   = __head
    defc    ram_size   = ram_tail-ram_head

    ; -------- RAM テスト(1)(2)用マクロ
    ; 0x01→0xff 順でメモリ書き込み
    ; 破壊: HL, BC, E, A
    ; @param addr           開始アドレス
    ; @param size           バイト サイズ
    ; @param inc_op         アドレスの増減(「inc」又は「dec」)
    macro   memWrite1  addr, size, inc_op
        local   memWrite1_loop, memWrite1_loop2
        ld      HL, addr
        ld      BC, size
        ld      E, 1
memWrite1_loop:
            ld      (HL), E
            inc     E
            jr      nz, memWrite1_loop2
                inc     E
memWrite1_loop2:
            inc_op  HL
            dec     BC
            ld      A, B
            or      C
        jr      nz, memWrite1_loop
    endm

    ; 0x01→0xff 順でメモリ チェック
    ; 破壊: HL, BC, E, A
    ; @param addr           開始アドレス
    ; @param size           バイト サイズ
    ; @param inc_op         アドレスの増減(「inc」又は「dec」)
macro   memCheck1  addr, size, inc_op
        local   memCheck1_loop, memCheck1_loop2
        ld      HL, addr
        ld      BC, size
        ld      E, 1
memCheck1_loop:
            ld      A, (HL)
            cp      E
            jp      nz, ramError                        ; RAM エラー
            inc     E
            jr      nz, memCheck1_loop2
                inc     E
memCheck1_loop2:
            inc_op  HL
            dec     BC
            ld      A, B
            or      C
        jr      nz, memCheck1_loop
    endm

    ; -------- RAM テスト(1)
    ;      +-----------------+
    ; head | 0x01→0xff→... | tail
    ;      +-----------------+
    memWrite1   vram_head, vram_size, inc
    bankAllRam
        memWrite1   ram_head, ram_size, inc
        memCheck1   ram_head, ram_size, inc
    bankRomVram
    memCheck1   vram_head, vram_size, inc

    ; -------- RAM テスト(2)
    ;      +-----------------+
    ; head | ...←0xff←0x01 | tail
    ;      +-----------------+
    bankAllRam
        memWrite1   ram_tail-1, ram_size, dec
    bankRomVram
    memWrite1   vram_tail-1, vram_size, dec
    memCheck1   vram_tail-1, vram_size, dec
    bankAllRam
        memCheck1   ram_tail-1, ram_size, dec
    bankRomVram

    ; -------- RAM テスト(3)(4)用マクロ
    ; 0x00←0xfe 順でメモリ書き込み
    ; 破壊: HL, BC, E, A
    ; @param addr           開始アドレス
    ; @param size           バイト サイズ
    ; @param inc_op         アドレスの増減(「inc」又は「dec」)
    ; @param start          開始値
macro   memWrite2  addr, size, inc_op
        local   memWrite2_loop, memWrite2_loop2
        ld      HL, addr
        ld      BC, size
        ld      E, 0xff
memWrite2_loop:
            dec     E
            ld      A, E
            cp      0xff
            jr      nz, memWrite2_loop2
                dec     E
memWrite2_loop2:
            ld      (HL), E

            inc_op  HL
            dec     BC
            ld      A, B
            or      C
        jr      nz, memWrite2_loop
    endm

    ; 0x00←0xfe 順でメモリ チェック
    ; 破壊: HL, BC, E, A
    ; @param addr           開始アドレス
    ; @param size           バイト サイズ
    ; @param inc_op         アドレスの増減(「inc」又は「dec」)
macro   memCheck2  addr, size, inc_op
        local   memCheck2_loop, memCheck2_loop2
        ld      HL, addr
        ld      BC, size
        ld      E, 0xff
memCheck2_loop:
            dec     E
            ld      A, E
            cp      0xff
            jr      nz, memCheck2_loop2
                dec     E
memCheck2_loop2:
            ld      A, (HL)
            cp      E
            jp      nz, ramError                        ; RAM エラー

            inc_op  HL
            dec     BC
            ld      A, B
            or      C
        jr      nz, memCheck2_loop
    endm

    ; -------- RAM テスト(3)
    ;      +-----------------+
    ; head | 0xfe→0x00→... | tail
    ;      +-----------------+
    memWrite2   vram_head, vram_size, inc
    bankAllRam
        memWrite2   ram_head, ram_size, inc
        memCheck2   ram_head, ram_size, inc
    bankRomVram
    memCheck2   vram_head, vram_size, inc

    ; -------- RAM テスト(4)
    ;      +-----------------+
    ; head | ...←0x00←0xfe | tail
    ;      +-----------------+
    bankAllRam
        memWrite2   ram_tail-1, ram_size, dec
    bankRomVram
    memWrite2   vram_tail-1, vram_size, dec
    memCheck2   vram_tail-1, vram_size, dec
    bankAllRam
        memCheck2   ram_tail-1, ram_size, dec
    bankRomVram

    ; -------- RAM Check OK
    ld      HL, text_ram_ok
    ld      DE, 0xd000+40*11+20-6/2
    ld      BC, 6
    ldir
    ld      HL, 0xd800+40*11+20-6/2
    ld      DE, 0xd800+40*11+20-6/2+1
    ld      BC, 6-1
    ld      (HL), 0x70
    ldir
    halt

    ; -------- RAM Check Bad
ramError:    
    bankRomVram
    ; HL にエラーが起きたアドレスが入ってる
    exx
        ld      HL, text_ram_error
        ld      DE, 0xd000+40*11+20-12/2
        ld      BC, 12
        ldir
        ld      HL, 0xd800+40*11+20-12/2
        ld      DE, 0xd800+40*11+20-12/2+1
        ld      BC, 12-1
        ld      (HL), 0x70
        ldir
    exx
    ld  DE, 0xd000+40*11+20-12/2+8

    ; Aの下4bit を (DE) に出力
macro   dispNibble 
    local   dispNibble_10
    and     A, 0x0f
    cp      10
    jr      nc, dispNibble_10
        add     A, 0x20-0x01+10 ; 0-9
dispNibble_10:
    add     A, 0x01-10          ; A-F    
    ld      (DE), A
    endm

    ld      A, H
    rrca
    rrca
    rrca
    rrca
    dispNibble
    inc     DE
    ld      A, H
    dispNibble
    inc     DE
    ld      A, L
    rrca
    rrca
    rrca
    rrca
    dispNibble
    inc     DE
    ld      A, L
    dispNibble
    halt

text_ram_ok: ; "RAM OK"
    db      0x12, 0x01, 0x0d, 0x00
    db      0x0f, 0x0b
text_ram_error: ; "RAM BAD xxxx"
    db      0x12, 0x01, 0x0d, 0x00
    db      0x02, 0x01, 0x04
    db      0x00, 0x00, 0x00, 0x00, 0x00
