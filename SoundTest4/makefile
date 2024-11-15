.PHONY:    all clean depend

# ざっくりのビルド フロー
#
#        music/chronos.dms music/drum[1-3].bmp
#                 :              |
#        music/chronos.mid       +--------------------+
#                 |              |                    |
#        music/chronos.h   music/drum[1-3].h   music/drum[1-3].wav
#                 |              |
#                 |        sound-sd4.z80ana.c
#                 |              |
#               main.c     sound-sd4.c
#                 |              |
#  crt0.asm     main.o     sounc-sd4.o
#      |          |              |
#      +----------+--------------+
#                 |
#      +----------+----------+
#      |                     |
# soundTest4.bin       soundTest4.map
# soundTest4.mzt       soundTest4.emuz.sym
#

# -------- macros
# 特別な命令に使われる音長. この音長の音符は使えません(makefile で定義)
SD6_EXCEPT_LEN		:= 180
SD6_EXCEPT_LEN_SZ	:= 24	# 0,4,8,12,16,29,24 の 7つ

TOOLS   	:= ../tools
CRT0 		:= crt0.asm
OUTFILE 	:= SoundTest4
PROG_NAME 	:= 'SOUND TEST 4'
PHP         := php
CC      	:= zcc
DEBUG   	:= -DDEBUG=1
ORG     	:= 0x1200
CFLAGS  	:= +mz -lndos -compiler=sdcc -OS3 $(DEBUG) -DSD6_EXCEPT_LEN=$(SD6_EXCEPT_LEN) -DSD6_EXCEPT_LEN_SZ=$(SD6_EXCEPT_LEN_SZ)
LFLAGS      := +mz -mz80 -m\
	-lndos\
	-crt0$(CRT0)\
	-pragma-define:CRT_ENABLE_STDIO=0\
	-pragma-define:CRT_ORG_CODE=$(ORG)\
	-pragma-define:CLIB_EXIT_STACK_SIZE=0
RMF			:= $(PHP) $(TOOLS)/rmf.php
MUSIC_DIR   := music
GAME_DEPENDS_PROG   := depends-prog.mk
GAME_DEPENDS_MUSIC	:= depends-music.mk

# -------- ビルド
all: $(OUTFILE).mzt
-include $(GAME_DEPENDS_PROG)
-include $(GAME_DEPENDS_MUSIC)

$(OUTFILE).mzt: $(OBJS) $(CRT0)
	@echo Linking ... [$@]
	@$(CC) $(LFLAGS) -o $(OUTFILE).bin $(OBJS)
	@$(PHP) $(TOOLS)/map2emuzsym.php $(OUTFILE).map $(OUTFILE).emuz.sym
	@$(PHP) $(TOOLS)/bss_remover.php $(OUTFILE).bin $(OUTFILE).map $(OUTFILE).code_data.bin
	@$(PHP) $(TOOLS)/make_mzt.php $(OUTFILE).code_data.bin $(PROG_NAME) $(ORG) $(ORG) $@

%.c: %.z80ana.c
	@echo Converting ... [$<]
	@$(PHP) $(TOOLS)/z80ana.php $< $@

%.o: %.c
	@echo Compiling ... [$<]
	@$(CC) $(CFLAGS) $(DEBUG) $(OPT) -o $@ -c $<

%.h: %.mid
	@echo Converting midi file ... [$<]
	@$(PHP) $(TOOLS)/sd4_midi_conv.php $< $@ --exceptLengths=$(SD6_EXCEPT_LEN)#$(SD6_EXCEPT_LEN_SZ) --verbose

$(MUSIC_DIR)/%.h: $(MUSIC_DIR)/%.bmp
	@echo Converting pcm drum file ... [$<]
	@$(PHP) $(TOOLS)/sd4_wav_conv.php $< $@ $(basename $<).wav

# -------- 依存関係生成
depend $(GAME_DEPENDS_PROG) $(GAME_DEPENDS_MUSIC):
	@echo Generating dependencies...
	@$(PHP) $(TOOLS)/make_depend.php $(GAME_DEPENDS_PROG)  --prog  . . OBJS PROG_CLEAN_FILES
	@$(PHP) $(TOOLS)/make_depend.php $(GAME_DEPENDS_MUSIC) --music music

# -------- clean
clean: clean_prog clean_music
	@echo Cleaning binaries ...
	@$(RMF) *.bin
	@$(RMF) *.map
	@$(RMF) *.sym
#	@$(RMF) $(OUTFILE).mzt

clean_prog:
	@echo Cleaning programs ...
	@$(RMF) $(PROG_CLEAN_FILES)

clean_music:
	@echo Cleaning music ...
	@$(RMF) $(MUSIC_DIR)/*.h
