.PHONY:    all clean depend

# ざっくりのビルド フロー(上から下へ)
#
#        music/chronos.dms music/drum0.txt
#                 :        music/drum1.txt
#                 :        music/drum2.txt
#        music/chronos.mid music/pulse.txt
#                 |              |
#        music/chronos.h   music/sd6-data.h
#                 |              |
#                 |        sound-sd6.z80ana.c
#                 |              |
#               main.c     sound-sd6.c
#                 |              |
#  crt0.asm     main.o     sounc-sd6.o
#      |          |              |
#      +----------+--------------+
#                 |
#      +----------+----------+
#      |                     |
# soundTest3.bin       soundTest3.map
# soundTest3.mzt       soundTest3.emuz.sym
# soundTest3.wav
#

# -------- macros
# Lead にて特別な命令に使われる音長. この音長の音符は使えません(4の倍数, makefile で定義). 曲にあわせて設定してください
SD6_EXCEPT_LEN		:= 180
# 180,184,188,192,196,209,224 の7個の音長が使用不可. このマクロは MIDI コンバーターで使用します
SD6_EXCEPT_LEN_SZ	:= 24

TOOLS   	:= ../tools
CRT0 		:= crt0.asm
NAME 		:= SoundTest3
PROG_NAME 	:= 'SOUND TEST 3'
PHP         := php
CC      	:= zcc
DEBUG   	:= -DDEBUG=1
ORG     	:= 0x1200
CFLAGS  	:= +mz -lndos -compiler=sdcc -OS3 $(DEBUG) \
	-DSD6_EXCEPT_LEN=$(SD6_EXCEPT_LEN)
LFLAGS      := +mz -mz80 -m\
	-lndos\
	-crt0$(CRT0)\
	-pragma-define:CRT_ENABLE_STDIO=0\
	-pragma-define:CRT_ORG_CODE=$(ORG)\
	-pragma-define:CLIB_EXIT_STACK_SIZE=0
RMF			:= $(PHP) $(TOOLS)/rmf.php
MUSIC_DIR   := music
GAME_DEPENDS_PROG	:= depends-prog.mk
GAME_DEPENDS_MUSIC	:= depends-music.mk
SD6_DRUMS   := $(MUSIC_DIR)/drum0.txt $(MUSIC_DIR)/drum1.txt $(MUSIC_DIR)/drum2.txt
SD6_PULSE   := $(MUSIC_DIR)/pulse.txt
SD6_DATA    := $(MUSIC_DIR)/sd6-data.h

# -------- ビルド
all: bin/$(NAME).mzt
-include $(GAME_DEPENDS_PROG)
-include $(GAME_DEPENDS_MUSIC)

$(SD6_DATA): $(SD6_DRUMS) $(SD6_PULSE)
	@echo Genating sd6 table ... [$@]
	@$(PHP) $(TOOLS)/sd6-gen-data.php $(SD6_PULSE) $(SD6_DRUMS) $@

bin/$(NAME).mzt: $(OBJS) $(CRT0)
	@echo Linking ... [$@]
	@$(CC) $(LFLAGS) -o $(NAME).bin $(OBJS)
	@$(PHP) $(TOOLS)/map2emuzsym.php $(NAME).map $(NAME).emuz.sym
	@$(PHP) $(TOOLS)/bss-remover.php $(NAME).bin $(NAME).map $(NAME).code_data.bin
	@$(PHP) $(TOOLS)/bin2mzt.php $(NAME).code_data.bin $(PROG_NAME) $(ORG) $(ORG) $@
	@$(PHP) $(TOOLS)/mzt2wav.php $@ bin/$(NAME).wav

%.c: %.z80ana.c
	@echo Converting ... [$<]
	@$(PHP) $(TOOLS)/z80ana.php $< $@

%.o: %.c
	@echo Compiling ... [$<]
	@$(CC) $(CFLAGS) $(DEBUG) $(OPT) -o $@ -c $<

%.h: %.mid
	@echo Converting midi file ... [$<]
	$(PHP) $(TOOLS)/sd6-midi-conv.php $< $@ --exceptLengths=$(SD6_EXCEPT_LEN)#$(SD6_EXCEPT_LEN_SZ) --verbose

# -------- 依存関係生成
depend $(GAME_DEPENDS_PROG) $(GAME_DEPENDS_MUSIC):
	@echo Generating dependencies...
	@$(PHP) $(TOOLS)/make-depend.php $(GAME_DEPENDS_PROG)  --prog  . OBJS PROG_CLEAN_FILES
	@$(PHP) $(TOOLS)/make-depend.php $(GAME_DEPENDS_MUSIC) --music music

# -------- clean
clean: clean_prog clean_music
	@echo Cleaning binaries ...
	@$(RMF) *.bin
	@$(RMF) *.map
	@$(RMF) *.sym
	@$(RMF) bin/$(NAME).mzt
	@$(RMF) bin/$(NAME).wav

clean_prog:
	@echo Cleaning programs ...
	@$(RMF) $(PROG_CLEAN_FILES)

clean_music:
	@echo Cleaning music ...
	@$(RMF) $(MUSIC_DIR)/*.h
