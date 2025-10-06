.SUFFIXES:
.PHONY: all clean release

NAME_SCCZ 	:= test-sdcc
NAME_SDCC 	:= test-sccz
SRC 		:= main.c
MZT_SCCZ	:= $(NAME_SCCZ).mzt
MZT_SDCC	:= $(NAME_SDCC).mzt
TOOLS     	:= ../tools
CC        	:= zcc
PHP       	:= php
RMF			:= $(PHP) $(TOOLS)/rmf.php

all: $(MZT_SCCZ) $(MZT_SDCC)

$(MZT_SCCZ): $(SRC)
	@echo building ...[$@]
	@$(CC) +mz -mz80 -O3 --opt-code-speed=all -o $(NAME_SCCZ) -create-app $(SRC) -m

$(MZT_SDCC): $(SRC)
	@echo building ...[$@]
	@$(CC) +mz -mz80 -OS3 -compiler=sdcc --max-allocs-per-node200000 -o $(NAME_SDCC) -create-app $(SRC) -m

clean:
	@echo cleaning...
	@$(RMF) $(MZT_SCCZ) $(MZT_SDCC)

release:
	@make
