.SUFFIXES:
.PHONY:    all clean depend release clean_loader clean_game clean_cg clean_music clean_text


# ---------------------------------------------------------------- マクロ
# -------- 最終出力ファイルや名前
OUTFILE     := ST-2
OUTFILE_BIN := ST-2.bin
OUTFILE_MZ  := ST-2
ifeq ($(DEBUG), -DDEBUG=0)
	OUTFILE_MZT := $(OUTFILE).mzt
	PROG_NAME   := '$(OUTFILE_MZ)'
else
	OUTFILE_MZT := $(OUTFILE)-debug.mzt
	PROG_NAME   := '$(OUTFILE_MZ) DEBUG'
endif

# -------- Loader
LOADER_DIR			:= loader
LOADER_OBJS			:= $(LOADER_DIR)/obj/loader-main.o $(LOADER_DIR)/obj/deexo3.o
LOADER_CRT0			:= $(LOADER_DIR)/crt0.asm
LOADER_OUTFILE		:= $(LOADER_DIR)/Loader
LOADER_OUTFILE_BIN	:= $(LOADER_OUTFILE).bin
LOADER_ORG 			:= 0x1c00
LOADER_STACK		:= 0x1e00
LOADER_DEEXO_WORK	:= 0x1b00
LOADER_GAME_TMP     := 0xd000 # 0x4800 分は確保しておきたい

# -------- Game
GAME_DIR			:= game
# GAME_OBJS is defined in depends.mk
GAME_CRT0			:= $(GAME_DIR)/src/crt0.asm
GAME_OUTFILE		:= $(GAME_DIR)/Game
GAME_OUTFILE_BIN	:= $(GAME_OUTFILE).bin
GAME_OUTFILE_EXO	:= $(GAME_OUTFILE).exo
GAME_OUTFILE_PAK	:= $(GAME_OUTFILE).pak
GAME_ORG			:= 0x1e00
GAME_STACK			:= 0xfc00
GAME_DEPENDS_PROG	:= $(GAME_DIR)/depends_prog.mk
GAME_DEPENDS_CG		:= $(GAME_DIR)/depends_cg.mk
GAME_DEPENDS_MUSIC	:= $(GAME_DIR)/depends_music.mk
GAME_DEPENDS_TEXT	:= $(GAME_DIR)/depends_text.mk


# -------- ツールやフラグ
DEBUG       := -DDEBUG=1	# overriden by release
OPT         :=				# overriden by release
TOOLS     	:= ../tools
CC        	:= zcc
PHP       	:= php
RMF			:= $(PHP) $(TOOLS)/rmf.php
CFLAGS    	:=\
	+mz -lndos -compiler=sdcc -OS3 $(DEBUG) \
	--opt-code-speed=sub16,inlineints,intcompare
LFLAGS    :=\
	-m -s\
	-lndos\
	-pragma-define:CRT_ENABLE_STDIO=0\
	-pragma-define:CRT_INITIALIZE_BSS=0\
	-pragma-define:CLIB_EXIT_STACK_SIZE=0

# -------- サーチパス
vpath %.c   $(GAME_DIR)/src $(GAME_DIR)/src/system $(GAME_DIR)/src/scenes $(GAME_DIR)/src/game $(GAME_DIR)/src/objworks $(LOADER_DIR)
vpath %.mid $(GAME_DIR)/music
vpath %.txt $(GAME_DIR)/text

# ---------------------------------------------------------------- 依存関係
#
# ST-2.mzt (1)
# ST-2.bin
#    |
#    +------------------------------------------------+
#    |                                                |
# game.pak (3)                                        |
# game.exo                                            |(2)
# game.bin --+                                   loader.bin --+
#    |       |                                        |       |
#   *.o   crt0.asm                                   *.o    crt0.asm
#    |(4)                                             |(5)
#   *.c ---+-------------------+----------+          *.c
#    |     |                   |          |           |
#   *.h  cg/*.h            music/*.h    text/*.h     *.h
#          |(6)                |(7)       |(8)
#        cg/*.cgedit.json  music/*.mid  text/*.txt
#        cg/*.cgedit.txt

# -------- (1) ST-2.mzt 生成
all: $(OUTFILE_MZT)
$(OUTFILE_MZT): $(GAME_OUTFILE_PAK) $(LOADER_OUTFILE_BIN)
	@echo Generating ... [$@]
	@$(PHP) $(TOOLS)/merger.php $(LOADER_ORG) $(LOADER_OUTFILE_BIN) $(GAME_ORG) $(GAME_OUTFILE_PAK) $(OUTFILE_BIN)
	@$(PHP) $(TOOLS)/make_mzt.php $(OUTFILE_BIN) $(PROG_NAME) $(LOADER_ORG) $(LOADER_ORG) $(OUTFILE_MZT)

# --------  (2) Loader 生成
$(LOADER_DIR)/obj/loader-main.o: $(LOADER_DIR)/loader-main.c $(LOADER_DIR)/deexo3.h
$(LOADER_DIR)/obj/deexo3.o:      $(LOADER_DIR)/deexo3.c      $(LOADER_DIR)/deexo3.h
$(LOADER_OUTFILE_BIN): $(LOADER_OBJS) $(LOADER_CRT0)
	@echo Linking loader ... [$@]
	@$(CC) +mz -mz80 -o $@ $(LOADER_OBJS) -crt0$(LOADER_CRT0) -zorg=$(LOADER_ORG) $(LFLAGS) -Ca-DSTACK=$(LOADER_STACK)

# -------- (3) Game のバイナリを生成し, 圧縮し, pak 化
-include $(GAME_DEPENDS_PROG)   # $(GAME_OBJS)
$(GAME_OUTFILE_PAK): $(GAME_OBJS) $(GAME_CRT0)
	@echo Linking game ... [$@]
	@$(CC) +mz -mz80 -o $(GAME_OUTFILE_BIN) $(GAME_OBJS) -crt0$(GAME_CRT0) -zorg=$(GAME_ORG) $(LFLAGS) -Ca-DSTACK=$(GAME_STACK)
	@$(PHP) $(TOOLS)/inc_build_nr.php $(GAME_DIR)/src/build_nr.asm
	@$(PHP) $(TOOLS)/map2emuzsym.php $(GAME_OUTFILE).map $(GAME_OUTFILE).emuz.sym
	@$(PHP) $(TOOLS)/make_mzt.php $(GAME_OUTFILE_BIN) $(PROG_NAME) $(GAME_ORG) $(GAME_ORG) $(GAME_OUTFILE).mzt
	@$(TOOLS)/exomizer.exe raw -B $(GAME_OUTFILE_BIN) -o $(GAME_OUTFILE_EXO)
	@$(PHP) $(TOOLS)/size_checker.php $(GAME_OUTFILE_EXO) 0x4000
	@$(PHP) $(TOOLS)/packer.php $(GAME_OUTFILE_EXO) $@

# -------- (4) Game プログラム パターン ルール
$(GAME_DIR)/obj/%.o: %.c
	@echo Compiling ... [$<]
	@$(CC) $(CFLAGS) -o $@ -c $<

# -------- (5) Loader プログラム パターン ルール
$(LOADER_DIR)/obj/%.o: %.c
	@echo Compiling ... [$<]
	@$(CC) $(CFLAGS)\
		-DADDR_GAME_ORG=$(GAME_ORG) -DADDR_DEEXO_WORK=$(LOADER_DEEXO_WORK) -DADDR_GAME_TMP=$(LOADER_GAME_TMP)\
		--max-allocs-per-node200000 -o $@ -c $<

# -------- (6) CG の生成
-include $(GAME_DEPENDS_CG)  # $(BUILD_CG) の定義が必要
define BUILD_CG
	@echo Generating CG data from... [$^]
	@$(PHP) ../tools/cg_edit2c.php $^
endef

# -------- (7) 音楽パターン ルール
-include $(GAME_DEPENDS_MUSIC)
$(GAME_DIR)/music/%.h: %.mid
	@echo Generating music data from ... [$<]
	@$(PHP) ../tools/sd3_midi_conv.php $< $@

# -------- (8) テキスト パターン ルール
-include $(GAME_DEPENDS_TEXT)
$(GAME_DIR)/text/%.h: %.txt
	@echo Generating text data from ... [$<]
	@$(PHP) ../tools/text2c.php $< $@


# ---------------------------------------------------------------- 各種引数
# -------- クリーン
clean:
	@echo Cleaning...
	@$(RMF) *.bin
	@make clean_loader
	@make clean_game
	@make clean_cg
	@make clean_music
	@make clean_text

clean_loader:
	@echo Cleaning loader...
	@$(RMF) $(LOADER_DIR)/obj/*.o
	@$(RMF) $(LOADER_DIR)/*.bin
	@$(RMF) $(LOADER_DIR)/*.sym
	@$(RMF) $(LOADER_DIR)/*.map

clean_game:
	@echo Cleaning game binaries, symbols...
	@$(RMF) $(GAME_DIR)/obj/*.o
	@$(RMF) $(GAME_DIR)/*.bin
	@$(RMF) $(GAME_DIR)/*.sym
	@$(RMF) $(GAME_DIR)/*.map
	@$(RMF) $(GAME_DIR)/*.exo
	@$(RMF) $(GAME_DIR)/*.pak

clean_cg:
	@echo Cleaning CG data...
	@$(RMF) $(GAME_DIR)/cg/*.h

clean_music:
	@echo Cleaning music data...
	@$(RMF) $(GAME_DIR)/music/*.h

clean_text:
	@echo Cleaning text data...
	@$(RMF) $(GAME_DIR)/text/*.h

# -------- 依存関係生成
depend $(GAME_DEPENDS_PROG) $(GAME_DEPENDS_CG) $(GAME_DEPENDS_MUSIC) $(GAME_DEPENDS_TEXT):
	@echo Generating dependencies...
	@$(PHP) $(TOOLS)/make_depend.php $(GAME_DEPENDS_PROG)     --prog   $(GAME_DIR)/src $(GAME_DIR)/obj GAME_OBJS GAME_CLEAN_FILES
	@$(PHP) $(TOOLS)/make_depend.php $(GAME_DEPENDS_CG)       --cg     $(GAME_DIR)/cg BUILD_CG
	@$(PHP) $(TOOLS)/make_depend.php $(GAME_DEPENDS_MUSIC)    --music  $(GAME_DIR)/music
	@$(PHP) $(TOOLS)/make_depend.php $(GAME_DEPENDS_TEXT)     --text   $(GAME_DIR)/text

# -------- リリース ビルド (クリーンしてからやってください)
release:
	make DEBUG=-DDEBUG=0 OPT=--max-allocs-per-node200000

# -------- NWK ロゴ イメージ作成
# 誤操作防止の為に通常はコメント化してます
#logo:
#	php $(TOOLS)/gen_graph_data.php $(GAME_DIR)/img/nwk.png $(TOOLS)/mz700fon.dat > $(GAME_DIR)/img/nwk.c
