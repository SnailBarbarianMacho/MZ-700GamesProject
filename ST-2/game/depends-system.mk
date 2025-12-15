# **** This file is made by ../tools/make-depend.php. DO NOT MODIFY ! ****

# prog
game/src/system/assert.o: game/src/system/assert.c game/src/system/assert.h ../src-common/common.h ../src-common/hard.h ../src-common/display-code.h
game/src/system/input.c: game/src/system/input.aal.c ../src-common/common.h ../src-common/hard.h ../src-common/aal80.h ../src-common/asm-macros.h game/src/system/assert.h game/src/system/input.h
game/src/system/input.o: game/src/system/input.c
game/src/system/main.o: game/src/system/main.c ../src-common/common.h game/src/system/sys.h game/src/system/input.h game/src/system/vram.h game/src/system/addr.h game/src/system/print.h ../src-common/display-code.h game/src/system/sound.h game/src/system/obj.h game/src/system/math.h game/src/scenes/scene-logo.h game/src/game/stars.h game/src/game/score.h game/src/game/game-mode.h game/src/objworks/obj-item.h
game/src/system/math.o: game/src/system/math.c ../src-common/common.h game/src/system/addr.h game/src/system/math.h
game/src/system/obj.o: game/src/system/obj.c ../src-common/common.h game/src/system/addr.h game/src/system/obj.h game/src/system/vram.h game/src/system/sound.h
game/src/system/print.o: game/src/system/print.c ../src-common/common.h game/src/system/addr.h game/src/system/sys.h game/src/system/vram.h game/src/system/print.h ../src-common/display-code.h
game/src/system/sound.o: game/src/system/sound.c ../src-common/common.h ../src-common/hard.h game/src/system/addr.h game/src/system/vram.h game/src/system/math.h game/src/game/bgm.h game/src/game/se.h game/src/system/sound.h
game/src/system/sys.o: game/src/system/sys.c ../src-common/common.h game/src/system/addr.h game/src/system/sound.h game/src/system/input.h game/src/system/vram.h game/src/system/sys.h game/src/game/bgm.h game/src/scenes/scene-title.h game/src/scenes/scene-game-mode.h
game/src/system/utils.o: game/src/system/utils.c ../src-common/common.h game/src/system/utils.h
game/src/system/vram.o: game/src/system/vram.c ../src-common/common.h ../src-common/hard.h game/src/system/addr.h game/src/system/input.h game/src/system/vram.h game/src/system/math.h
GAME_SYSTEM_OBJS := game/src/system/assert.o game/src/system/input.o game/src/system/main.o game/src/system/math.o game/src/system/obj.o game/src/system/print.o game/src/system/sound.o game/src/system/sys.o game/src/system/utils.o game/src/system/vram.o
GAME_SYSTEM_CLEAN_FILES := game/src/system/assert.o game/src/system/input.c game/src/system/input.o game/src/system/main.o game/src/system/math.o game/src/system/obj.o game/src/system/print.o game/src/system/sound.o game/src/system/sys.o game/src/system/utils.o game/src/system/vram.o
