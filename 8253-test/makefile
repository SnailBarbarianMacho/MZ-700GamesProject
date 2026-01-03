.PHONY:    all clean depend

# ざっくりのビルド フロー(上から下へ)
#
#               main.aal.c
#                 |
#               main.c
#                 |
#  crt0.asm     main.o
#      |          |
#      +----------+
#                 |
#      +----------+----------+
#      |                     |
# 8253-test.bin       8253-test.map
# 8253-test.mzt       8253-test.emuz.sym
# 8253-test.wav
#

# -------- macros
TOOLS   	:= ../tools
CRT0 		:= crt0.asm
NAME 		:= 8253-test
PROG_NAME 	:= '8253 TEST'
PHP         := php
CC      	:= zcc
DEBUG   	:= -DDEBUG=1
ORG     	:= 0x1200
CFLAGS  	:= +mz -lndos -compiler=sdcc -OS3 $(DEBUG)
LFLAGS      := +mz -mz80 -m\
	-lndos\
	-crt0$(CRT0)\
	-pragma-define:CRT_ENABLE_STDIO=0\
	-pragma-define:CRT_ORG_CODE=$(ORG)\
	-pragma-define:CLIB_EXIT_STACK_SIZE=0
RMF			:= $(PHP) $(TOOLS)/rmf.php
GAME_DEPENDS_PROG	:= depends-prog.mk

# -------- ビルド
all: bin/$(NAME).mzt
-include $(GAME_DEPENDS_PROG)

bin/$(NAME).mzt: $(OBJS) $(CRT0)
	@echo Linking ... [$@]
	@$(CC) $(LFLAGS) -o $(NAME).bin $(OBJS)
	@$(PHP) $(TOOLS)/map2emuzsym.php $(NAME).map $(NAME).emuz.sym
	@$(PHP) $(TOOLS)/bss-remover.php $(NAME).bin $(NAME).map $(NAME).code_data.bin
	@$(PHP) $(TOOLS)/bin2mzt.php $(NAME).code_data.bin $(PROG_NAME) $(ORG) $(ORG) $@
	@$(PHP) $(TOOLS)/mzt2wav.php $@ bin/$(NAME).wav

%.c: %.aal.c
	@echo Converting ... [$<]
	@$(PHP) $(TOOLS)/aal80.php $< $@

%.o: %.c
	@echo Compiling ... [$<]
	@$(CC) $(CFLAGS) $(DEBUG) $(OPT) -o $@ -c $<

# -------- 依存関係生成
depend $(GAME_DEPENDS_PROG):
	@echo Generating dependencies...
	@$(PHP) $(TOOLS)/make-depend.php $(GAME_DEPENDS_PROG)  --prog  . OBJS PROG_CLEAN_FILES

# -------- clean
clean: clean_prog
	@echo Cleaning binaries ...
	@$(RMF) *.bin
	@$(RMF) *.map
	@$(RMF) *.sym
	@$(RMF) bin/$(NAME).mzt
	@$(RMF) bin/$(NAME).wav

clean_prog:
	@echo Cleaning programs ...
	@$(RMF) $(PROG_CLEAN_FILES)
