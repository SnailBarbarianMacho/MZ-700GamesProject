.SUFFIXES:
.PHONY:    all clean depend release clean_loader clean_game clean_cg clean_music clean_text

# dependencies:
#
# ST-2.mzt - ST-2.bin -+---------------------- loader.bin -+- *.o --- *.c --- *.h
#                      |                                   +- crt0.asm
#                      |
#                      +- game.pak - game.exo - game.bin -+- *.o --- *.c --- *.h -+- *.txt
#                                                         +- crt0.asm             +- *.mid

# Target macros
OUTFILE     := ST-2
OUTFILE_BIN := ST-2.bin
PROG_NAME 	:= ST-2

# Loader macros
LOADER_DIR			:= loader
LOADER_OBJS			:= $(LOADER_DIR)/obj/loader-main.o $(LOADER_DIR)/obj/deexo3.o
LOADER_CRT0			:= $(LOADER_DIR)/crt0.asm
LOADER_OUTFILE		:= $(LOADER_DIR)/Loader
LOADER_OUTFILE_BIN	:= $(LOADER_OUTFILE).bin
LOADER_ORG 			:= 0x1c00
LOADER_STACK		:= 0x1e00
LOADER_DEEXO_WORK	:= 0x1b00
LOADER_GAME_TMP     := 0xd000 # 0x4800 分は確保しておきたい

# Game macros
GAME_DIR			:= game
#GAME_OBJS is defined in depends.mk
GAME_CRT0			:= $(GAME_DIR)/src/crt0.asm
GAME_OUTFILE		:= $(GAME_DIR)/Game
GAME_OUTFILE_BIN	:= $(GAME_OUTFILE).bin
GAME_OUTFILE_EXO	:= $(GAME_OUTFILE).exo
GAME_OUTFILE_PAK	:= $(GAME_OUTFILE).pak
GAME_ORG			:= 0x1e00
GAME_STACK			:= 0xfc00
GAME_DEPENDS   		:= $(GAME_DIR)/depends.mk

# Macros
DEBUG       := -DDEBUG=1	# overriden by release
OPT         :=				# overriden by release
TOOLS     	:= ../tools
CC        	:= zcc
PHP       	:= php
RMF			:= del
#RMF      	:= rm -f
OPT       	:=
CFLAGS    	:=\
	+mz -lndos -compiler=sdcc -OS3 $(DEBUG) $(OPT) \
	--opt-code-speed=sub16,inlineints,intcompare
LFLAGS    :=\
	-m -s\
	-lndos\
	-pragma-define:CRT_ENABLE_STDIO=0\
	-pragma-define:CRT_INITIALIZE_BSS=0\
	-pragma-define:CLIB_EXIT_STACK_SIZE=0

# Search pathes
vpath %.c   $(GAME_DIR)/src $(GAME_DIR)/src/system $(GAME_DIR)/src/steps $(GAME_DIR)/src/game $(GAME_DIR)/src/objworks $(LOADER_DIR)
vpath %.mid $(GAME_DIR)/music
vpath %.txt $(GAME_DIR)/text

# All
all: $(OUTFILE_BIN)

# Make combined binary
ifeq ($(DEBUG), -DDEBUG=0)
  OUTFILE_MZT := $(OUTFILE).mzt
else
  OUTFILE_MZT := $(OUTFILE)-debug.mzt
endif
$(OUTFILE_BIN): $(GAME_OUTFILE_PAK) $(LOADER_OUTFILE_BIN)
	@echo Marging ... [$@]
	@$(PHP) $(TOOLS)/merger.php $(LOADER_ORG) $(LOADER_OUTFILE_BIN) $(GAME_ORG) $(GAME_OUTFILE_PAK) $@
	@$(PHP) $(TOOLS)/makeMzt.php $(OUTFILE_BIN) $(PROG_NAME) $(LOADER_ORG) $(LOADER_ORG) $(OUTFILE_MZT)

# Make loader binary
$(LOADER_DIR)/obj/loader-main.o: $(LOADER_DIR)/loader-main.c $(LOADER_DIR)/deexo3.h
$(LOADER_DIR)/obj/deexo3.o:      $(LOADER_DIR)/deexo3.c      $(LOADER_DIR)/deexo3.h
$(LOADER_OUTFILE_BIN): $(LOADER_OBJS) $(LOADER_CRT0)
	@echo Linking loader ... [$@]
	@$(CC) +mz -mz80 -o $@ $(LOADER_OBJS) -crt0$(LOADER_CRT0) -zorg=$(LOADER_ORG) $(LFLAGS) -Ca-DSTACK=$(LOADER_STACK)
#	@$(PHP) $(TOOLS)/makeMzt.php $@ $(PROG_NAME) $(LOADER_ORG) $(LOADER_ORG) $(LOADER_OUTFILE).mzt

# Make game binary
-include $(GAME_DEPENDS)   # dependency list, make cg, define $(GAME_OBJS)
$(GAME_OUTFILE_PAK): $(GAME_OBJS) $(GAME_CRT0)
	@echo Linking game ... [$@]
	@$(PHP) $(TOOLS)/incBuildNr.php $(GAME_DIR)/src/build_nr.asm
	@$(CC) +mz -mz80 -o $(GAME_OUTFILE_BIN) $(GAME_OBJS) -crt0$(GAME_CRT0) -zorg=$(GAME_ORG) $(LFLAGS) -Ca-DSTACK=$(GAME_STACK)
#	@$(PHP) $(TOOLS)/makeMzt.php $(GAME_OUTFILE_BIN) $(PROG_NAME) $(GAME_ORG) $(GAME_ORG) $(GAME_OUTFILE).mzt
	@$(TOOLS)/exomizer.exe raw -B $(GAME_OUTFILE_BIN) -o $(GAME_OUTFILE_EXO)
#	$(TOOLS)/exomizer.exe raw -B -d $(GAME_OUTFILE).exo -o a.out
#	diff $(GAME_OUTFILE_BIN) a.out
#	$(RMF) a.out
	$(PHP) $(TOOLS)/packer.php $(GAME_OUTFILE_EXO) 0x4000 $@

# Pattern rules
$(GAME_DIR)/obj/%.o: %.c
	@echo Compiling ... [$<]
	@$(CC) $(CFLAGS) -o $@ -c $<

$(LOADER_DIR)/obj/%.o: %.c
	@echo Compiling ... [$<]
	@$(CC) $(CFLAGS)\
		-DADDR_GAME_ORG=$(GAME_ORG) -DADDR_DEEXO_WORK=$(LOADER_DEEXO_WORK) -DADDR_GAME_TMP=$(LOADER_GAME_TMP)\
		--max-allocs-per-node200000 -o $@ -c $<

$(GAME_DIR)/music/%.h: %.mid
	@echo Generating music data...[$@]
	@$(PHP) ../tools/midi2sd3mml.php $< $@

$(GAME_DIR)/text/%.h: %.txt
	@echo Generating text data...[$@]
	@$(PHP) ../tools/text2c.php $< $@


# Crean files for MS... :-(
clean:
	@echo Cleaning...
	@$(RMF) *.bin
	@make clean_loader
	@make clean_game
	@make clean_cg
	@make clean_music
	@make clean_text

clean_loader:
	@$(RMF) $(LOADER_DIR)\obj\*.o
	@$(RMF) $(LOADER_DIR)\*.bin
	@$(RMF) $(LOADER_DIR)\*.sym
	@$(RMF) $(LOADER_DIR)\*.map

clean_game:
	@$(RMF) $(GAME_DIR)\obj\*.o
	@$(RMF) $(GAME_DIR)\*.bin
	@$(RMF) $(GAME_DIR)\*.sym
	@$(RMF) $(GAME_DIR)\*.map
	@$(RMF) $(GAME_DIR)\*.exo
	@$(RMF) $(GAME_DIR)\*.pak

clean_cg:
	@echo Cleaning CG data...
	@$(RMF) $(GAME_DIR)\cg\*.h

clean_music:
	@echo Cleaning music data...
	@$(RMF) $(GAME_DIR)\music\*.h

clean_text:
	@echo Cleaning text data...
	@$(RMF) $(GAME_DIR)\text\*.h

# Create game dependencies
depend $(GAME_DEPENDS):
	@echo Generating dependencies...
	@$(PHP) $(TOOLS)/makeDepend.php\
		$(GAME_DIR)/src $(GAME_DIR)/cg $(GAME_DIR)/music $(GAME_DIR)/text $(GAME_DIR)/obj\
		$(GAME_DIR)/cg/st2.cgedit.txt $(GAME_DIR)/cg/st2.cgedit.json\
		GAME_OBJS $(GAME_DEPENDS)

# Release build(need clean)
release:
	make DEBUG=-DDEBUG=0 OPT=--max-allocs-per-node200000

# Generate NWK-logo image data
#logo:
#	php $(TOOLS)/genGraphData.php $(GAME_DIR)/img/nwk.png $(TOOLS)/mz700fon.jp > $(GAME_DIR)/img/nwk.c
