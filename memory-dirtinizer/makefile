.SUFFIXES:
.PHONY: all clean release

TOOLS     	:= ../tools
ASM      	:= z80asm
PHP       	:= php
RMF			:= $(PHP) $(TOOLS)/rmf.php

NAME 		:= memory-dirtinizer
SRC			:= $(NAME).asm
SUM			:= $(NAME)-sum.bin
OBJ			:= $(NAME).o
BIN			:= $(NAME).bin
MAP			:= $(NAME).map
MZT			:= bin/$(NAME).mzt
WAV			:= bin/$(NAME).wav

all: bin/$(NAME).wav

$(WAV): $(SRC)
	@echo building...[$(WAV)]
	@$(ASM) -mz80 -b -r0x1200 $(SRC) -m
	@$(PHP) ../tools/bin2mzt.php $(BIN) "MEM DIRTINIZER" 0x1200 0x1200 $(MZT)
	@$(PHP) ../tools/mzt2wav.php $(MZT) $(WAV)

clean:
	@echo cleaning...
	@$(RMF) $(OBJ) $(BIN) $(MAP) $(SUM) $(MZT) $(WAV)
