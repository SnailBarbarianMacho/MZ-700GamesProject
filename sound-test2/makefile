.PHONY:    all clean
TOOLS   	:= ../tools
CRT0 		:= crt0.asm
NAME 		:= sound-test2
PROG_NAME 	:= 'SOUND TEST 2'
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

all: bin/$(NAME).mzt

OBJS		:= main.o sound.o
main.o: main.c sound.h
sound.o: sound.c sound.h

bin/$(NAME).mzt: $(OBJS) $(CRT0)
	@echo Linking ... [$@]
	@$(CC) $(LFLAGS) -o $(NAME).bin $(OBJS)
	@$(PHP) $(TOOLS)/map2emuzsym.php $(NAME).map $(NAME).emuz.sym
	@$(PHP) $(TOOLS)/bss-remover.php $(NAME).bin $(NAME).map $(NAME)-code-data.bin
	@$(PHP) $(TOOLS)/bin2mzt.php $(NAME)-code-data.bin $(PROG_NAME) $(ORG) $(ORG) $@
	@$(PHP) $(TOOLS)/mzt2wav.php $@ bin/$(NAME).wav

%.o: %.c
	@echo Compiling ... [$<]
	@$(CC) $(CFLAGS) $(DEBUG) $(OPT) -o $@ -c $<

clean:
	@echo Cleaning ...
	@$(RMF) *.o
	@$(RMF) *.bin
	@$(RMF) *.map
	@$(RMF) *.sym
	@$(RMF) bin/$(NAME).mzt
	@$(RMF) bin/$(NAME).wav