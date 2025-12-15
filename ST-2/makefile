.SUFFIXES:
.PHONY:    all clean depend release clean_loader clean_game clean_cg clean_music clean_text


# ---------------------------------------------------------------- マクロ
# -------- 最終出力ファイルや名前
OUTFILE     := ST-2
OUTFILE_BIN := ST-2.bin
ifeq ($(DEBUG), -DDEBUG=0)
	OUTFILE_MZT := bin/$(OUTFILE).mzt
	OUTFILE_WAV := bin/$(OUTFILE).wav
	OPT			:=
	PROG_NAME   := 'ST-2'
else
	OUTFILE_MZT := bin/$(OUTFILE)-debug.mzt
	OUTFILE_WAV := bin/$(OUTFILE)-debug.wav
	OPT			:=
#	OPT			:= --max-allocs-per-node200000 あまりきつい最適化はバグる
	PROG_NAME   := 'ST-2 DEBUG'
endif

# -------- Loader
LOADER_DIR			:= loader
LOADER_OBJS			:= $(LOADER_DIR)/loader-main.o $(LOADER_DIR)/deexo3.o
LOADER_CRT0			:= $(LOADER_DIR)/crt0.asm
LOADER_OUTFILE		:= $(LOADER_DIR)/loader
LOADER_OUTFILE_BIN	:= $(LOADER_OUTFILE).bin
LOADER_ORG 			:= 0x1c00
LOADER_STACK		:= 0x1e00
LOADER_DEEXO_WORK	:= 0x1b00
LOADER_GAME_TMP     := 0xd000

# -------- Game
GAME_DIR			:= game
# GAME_OBJS is defined in depends.mk
GAME_CRT0			:= $(GAME_DIR)/src/crt0.asm
GAME_OUTFILE		:= $(GAME_DIR)/game
GAME_OUTFILE_BIN	:= $(GAME_OUTFILE).bin
GAME_OUTFILE_MZT	:= $(GAME_OUTFILE).mzt
GAME_OUTFILE_EXO	:= $(GAME_OUTFILE).exo
GAME_OUTFILE_PAK	:= $(GAME_OUTFILE).pak
GAME_ORG			:= 0x1e00
GAME_STACK			:= 0xfc00
GAME_DEPENDS_GAME	:= $(GAME_DIR)/depends-game.mk
GAME_DEPENDS_OBJWORKS:= $(GAME_DIR)/depends-objwoork.mk
GAME_DEPENDS_SCENES	:= $(GAME_DIR)/depends-scenes.mk
GAME_DEPENDS_SYSTEM	:= $(GAME_DIR)/depends-system.mk
GAME_DEPENDS_CG		:= $(GAME_DIR)/depends-cg.mk
GAME_DEPENDS_MUSIC	:= $(GAME_DIR)/depends-music.mk
GAME_DEPENDS_TEXT	:= $(GAME_DIR)/depends-text.mk


# -------- ツールやフラグ
DEBUG       := -DDEBUG=1	# overriden by release
TOOLS     	:= ../tools
CC        	:= zcc
PHP       	:= php
RMF			:= $(PHP) $(TOOLS)/rmf.php
CFLAGS    	:=\
	+mz -lndos -compiler=sdcc -OS3 $(DEBUG) \
	-DADDR_GAME_ORG=$(GAME_ORG) -DADDR_DEEXO_WORK=$(LOADER_DEEXO_WORK) -DADDR_GAME_TMP=$(LOADER_GAME_TMP) \
	$(OPT) \
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
# ざっくりのビルド フロー(上から下へ)
#
# ---- ソール, リソースのパターン ルール
#
#  Cソース                         音楽
#  *.aal.c   キャラグラ        music/*.dms
#    |        cg/*.cgedit.txt       :          テキスト
#   *.c       cg/*.cgedit.json  music/*.mid     text/*.txt
#    |(00)       |(01)              |(02)         |(03)
#   *.o       cg/*.h            music/*.h       text/*.h
#
# ---- Game / Loader
#
#   *.c                      *.c
#    |                        |
#   *.o    crt0.asm          *.o   crt0.asm
#    |       |                |       |
# game.bin --+             loader.bin-+
# game.exo                    |
# game.pak  (10)              |(11)
#    |                        |
#    +------------------------+
#    |
#    |(12)
# ST-2.bin
# ST-2.mzt
# ST-2.wav

all: $(OUTFILE_MZT)
# -------------------------------- ソール, リソースのパターン ルール
# -------- (00) ソースからオブジェクト ファイルへ
%.c: %.aal.c
	@echo ---- Converting ... [$<]
	@$(PHP) $(TOOLS)/aal80.php $< $@

%.o: %.c
	@echo ---- Compiling ... [$<]
	@$(CC) $(CFLAGS) -o $@ -c $<

# -------- (01) CG の生成
define BUILD_CG
	@echo ---- Generating resource:CG from ... [$^]
	@$(PHP) $(TOOLS)/cgedit2c.php $^
endef

# ↓$(BUILD_CG) を呼び出してます
-include $(GAME_DEPENDS_CG)

# -------- (02) Music の生成 パターンルール
-include $(GAME_DEPENDS_MUSIC)

$(GAME_DIR)/music/%.h: $(GAME_DIR)/music/%.mid
	@echo ---- Generating resource:music ... [$<]
	@$(PHP) $(TOOLS)/sd3-midi-conv.php $< $@

# -------- (03) Text の生成 パターンルール
-include $(GAME_DEPENDS_TEXT)

$(GAME_DIR)/text/%.h: $(GAME_DIR)/text/%.txt
	@echo Generating text data from ... [$<]
	@$(PHP) $(TOOLS)/text2c.php $< $@

# -------------------------------- Game/Loader
# -------- (10) Game のバイナリを生成し, 圧縮し, pak 化
-include $(GAME_DEPENDS_GAME)
-include $(GAME_DEPENDS_OBJWORKS)
-include $(GAME_DEPENDS_SCENES)
-include $(GAME_DEPENDS_SYSTEM)
GAME_OBJS := $(GAME_GAME_OBJS) $(GAME_OBJWORKS_OBJS) $(GAME_SCENES_OBJS) $(GAME_SYSTEM_OBJS)
$(GAME_OUTFILE_PAK): $(GAME_OBJS) $(GAME_CRT0)
	@echo Linking game ... [$@]
	@$(CC) +mz -mz80 -o $(GAME_OUTFILE_BIN) $(GAME_OBJS) -crt0$(GAME_CRT0) -zorg=$(GAME_ORG) $(LFLAGS) -Ca-DSTACK=$(GAME_STACK)
	@$(PHP) $(TOOLS)/inc-buildnr.php $(GAME_DIR)/src/build-nr.asm
	@$(PHP) $(TOOLS)/map2emuzsym.php $(GAME_OUTFILE).map $(GAME_OUTFILE).emuz.sym
	@$(TOOLS)/exomizer.exe raw -B $(GAME_OUTFILE_BIN) -o $(GAME_OUTFILE_EXO)
	@$(PHP) $(TOOLS)/size-checker.php $(GAME_OUTFILE_EXO) 0x4000
	@$(PHP) $(TOOLS)/packer.php $(GAME_OUTFILE_EXO) $@

# -------- (11) Loader 生成
$(LOADER_DIR)/loader-main.o: $(LOADER_DIR)/loader-main.c $(LOADER_DIR)/deexo3.h
$(LOADER_DIR)/deexo3.o:      $(LOADER_DIR)/deexo3.c      $(LOADER_DIR)/deexo3.h

$(LOADER_OUTFILE_BIN): $(LOADER_OBJS) $(LOADER_CRT0)
	@echo Linking loader ... [$@]
	@$(CC) +mz -mz80 -o $@ $(LOADER_OBJS) -crt0$(LOADER_CRT0) -zorg=$(LOADER_ORG) $(LFLAGS) -Ca-DSTACK=$(LOADER_STACK)
	@$(PHP) $(TOOLS)/map2emuzsym.php $(LOADER_OUTFILE).map $(LOADER_OUTFILE).emuz.sym

# -------- (12) ST-2.mzt 生成
$(OUTFILE_MZT): $(GAME_OUTFILE_PAK) $(LOADER_OUTFILE_BIN)
	@echo Generating ... [$@]
	@$(PHP) $(TOOLS)/merger.php $(LOADER_ORG) $(LOADER_OUTFILE_BIN) $(GAME_ORG) $(GAME_OUTFILE_PAK) $(OUTFILE_BIN)
	@$(PHP) $(TOOLS)/bin2mzt.php $(OUTFILE_BIN) $(PROG_NAME) $(LOADER_ORG) $(LOADER_ORG) $(OUTFILE_MZT)
	@$(PHP) $(TOOLS)/mzt2wav.php $(OUTFILE_MZT) $(OUTFILE_WAV)


# ---------------------------------------------------------------- 各種引数
# -------- クリーン
clean: clean_loader clean_game clean_cg clean_music clean_text
	@echo Cleaning...
	@$(RMF) *.bin
	@$(RMF) $(OUTFILE_WAV) $(OUTFILE_MZT)

clean_loader:
	@echo Cleaning loader...
	@$(RMF) $(LOADER_DIR)/*.o
	@$(RMF) $(LOADER_DIR)/*.bin
	@$(RMF) $(LOADER_DIR)/*.sym
	@$(RMF) $(LOADER_DIR)/*.map

clean_game:
	@echo Cleaning game binaries, symbols...
	@$(RMF) $(GAME_GAME_CLEAN_FILES)
	@$(RMF) $(GAME_OBJWORKS_CLEAN_FILES)
	@$(RMF) $(GAME_SCENES_CLEAN_FILES)
	@$(RMF) $(GAME_SYSTEM_CLEAN_FILES)
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
depend:
	@echo Generating dependencies...
	@$(PHP) $(TOOLS)/make-depend.php $(GAME_DEPENDS_GAME)     --prog   $(GAME_DIR)/src/game     GAME_GAME_OBJS     GAME_GAME_CLEAN_FILES
	@$(PHP) $(TOOLS)/make-depend.php $(GAME_DEPENDS_OBJWORKS) --prog   $(GAME_DIR)/src/objworks GAME_OBJWORKS_OBJS GAME_OBJWORKS_CLEAN_FILES
	@$(PHP) $(TOOLS)/make-depend.php $(GAME_DEPENDS_SCENES)   --prog   $(GAME_DIR)/src/scenes   GAME_SCENES_OBJS   GAME_SCENES_CLEAN_FILES
	@$(PHP) $(TOOLS)/make-depend.php $(GAME_DEPENDS_SYSTEM)   --prog   $(GAME_DIR)/src/system   GAME_SYSTEM_OBJS   GAME_SYSTEM_CLEAN_FILES
	@$(PHP) $(TOOLS)/make-depend.php $(GAME_DEPENDS_CG)       --cg     $(GAME_DIR)/cg BUILD_CG
	@$(PHP) $(TOOLS)/make-depend.php $(GAME_DEPENDS_MUSIC)    --music  $(GAME_DIR)/music
	@$(PHP) $(TOOLS)/make-depend.php $(GAME_DEPENDS_TEXT)     --text   $(GAME_DIR)/text

# -------- リリース ビルド (クリーンしてからやってください)
release:
	make DEBUG=-DDEBUG=0

# -------- NWK ロゴ イメージ作成
# 誤操作防止の為に通常はコメント化してます
#logo:
#	php $(TOOLS)/gen_graph_data.php $(GAME_DIR)/img/nwk.png $(TOOLS)/mz700fon.dat > $(GAME_DIR)/img/nwk.c
