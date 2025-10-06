.SUFFIXES:
.PHONY: all clean release

TOOLS     	:= ../tools
ASM      	:= z80asm
PHP       	:= php
RMF			:= $(PHP) $(TOOLS)/rmf.php

NAME 		:= ram-check
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
	@$(PHP) ../tools/checksum.php $(BIN) $(SUM) -b
	@$(PHP) ../tools/meow.php $(BIN) $(SUM) $(BIN)
	@$(PHP) ../tools/bin2mzt.php $(BIN) "RAM CHECK" 0x1200 0x1200 $(MZT)
	@$(PHP) ../tools/mzt2wav.php $(MZT) $(WAV)

clean:
	@echo cleaning...
	$(RMF) $(OBJ) $(BIN) $(MAP) $(SUM) $(MZT) $(WAV)
