.PHONY:    all clean depend

# ざっくりのビルド フロー(上から下へ)
#
#        music/chronos.dms music/drum[1-3].bmp
#                 :              |
#        music/chronos.mid       +--------------------+
#                 |              |                    |
#        music/chronos.h   music/drum[1-3].h   music/drum[1-3].wav
#                 |              |
#                 |        sound-sd4.llm.c
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
# soundTest4.wav
#

# -------- macros
# Lead にて特別な命令に使われる音長. この音長の音符は使えません(4の倍数, makefile で定義). 曲にあわせて設定してください
SD4_EXCEPT_LEN		:= 180
# 180,184,188,192,196,209,224 の7個の音長が使用不可. このマクロは MIDI コンバーターで使用します
SD4_EXCEPT_LEN_SZ	:= 24

TOOLS   	:= ../tools
CRT0 		:= crt0.asm
NAME 		:= sound-test4
PROG_NAME 	:= 'SOUND TEST 4'
PHP         := php
CC      	:= zcc
DEBUG   	:= -DDEBUG=1
ORG     	:= 0x1200
CFLAGS  	:= +mz -lndos -compiler=sdcc -OS3 $(DEBUG) \
	-DSD4_EXCEPT_LEN=$(SD4_EXCEPT_LEN)
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

# -------- ビルド
all: bin/$(NAME).mzt
-include $(GAME_DEPENDS_PROG)
-include $(GAME_DEPENDS_MUSIC)

bin/$(NAME).mzt: $(OBJS) $(CRT0)
	@echo Linking ... [$@]
	@$(CC) $(LFLAGS) -o $(NAME).bin $(OBJS)
	@$(PHP) $(TOOLS)/map2emuzsym.php $(NAME).map $(NAME).emuz.sym
	@$(PHP) $(TOOLS)/bss-remover.php $(NAME).bin $(NAME).map $(NAME).code_data.bin
	@$(PHP) $(TOOLS)/bin2mzt.php $(NAME).code_data.bin $(PROG_NAME) $(ORG) $(ORG) $@
	@$(PHP) $(TOOLS)/mzt2wav.php $@ bin/$(NAME).wav

%.c: %.llm.c
	@echo Converting ... [$<]
	@$(PHP) $(TOOLS)/llm80.php $< $@

%.o: %.c
	@echo Compiling ... [$<]
	@$(CC) $(CFLAGS) $(DEBUG) $(OPT) -o $@ -c $<

%.h: %.mid
	@echo Converting midi file ... [$<]
	@$(PHP) $(TOOLS)/sd4-midi-conv.php $< $@ --exceptLengths=$(SD4_EXCEPT_LEN)#$(SD4_EXCEPT_LEN_SZ) --verbose

$(MUSIC_DIR)/%.h: $(MUSIC_DIR)/%.bmp
	@echo Converting pcm drum file ... [$<]
	@$(PHP) $(TOOLS)/sd4-wav-conv.php $< $@ $(basename $<).wav

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
