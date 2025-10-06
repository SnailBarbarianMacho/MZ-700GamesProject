# **** This file is made by ../tools/make-depend.php. DO NOT MODIFY ! ****

# prog
game/src/game/bgm.o: game/src/game/bgm.c ../src-common/common.h game/src/system/obj.h game/src/system/sound.h game/src/system/math.h game/src/objworks/obj-player.h game/src/game/stage.h game/src/game/bgm.h
game/src/game/game-mode.o: game/src/game/game-mode.c ../src-common/common.h game/src/system/vram.h game/src/system/addr.h game/src/system/sound.h game/src/game/se.h game/src/game/game-mode.h
game/src/game/score.o: game/src/game/score.c ../src-common/common.h ../src-common/hard.h game/src/system/addr.h game/src/system/sys.h game/src/system/vram.h game/src/system/sound.h game/src/system/math.h game/src/system/print.h ../src-common/display-code.h game/src/system/input.h game/src/game/game-mode.h game/src/objworks/obj-item.h game/src/system/obj.h game/src/objworks/obj-enemy.h game/src/game/se.h game/src/game/score.h game/text/game-over.h game/text/input-am7j.h game/text/input-mz1x03.h
game/src/game/se.o: game/src/game/se.c ../src-common/common.h game/src/system/sound.h game/src/system/math.h game/src/game/se.h
game/src/game/stage.o: game/src/game/stage.c game/src/system/sys.h ../src-common/common.h game/src/system/sound.h game/src/system/math.h game/src/objworks/obj-enemy.h game/src/system/obj.h game/src/objworks/obj-player.h game/src/game/bgm.h game/src/game/se.h game/src/game/game-mode.h game/src/game/stage.h
game/src/game/stars.o: game/src/game/stars.c ../src-common/common.h game/src/system/sys.h game/src/system/addr.h game/src/system/obj.h game/src/system/vram.h game/src/system/print.h ../src-common/display-code.h game/src/system/math.h game/src/game/stars.h
GAME_GAME_OBJS := game/src/game/bgm.o game/src/game/game-mode.o game/src/game/score.o game/src/game/se.o game/src/game/stage.o game/src/game/stars.o
GAME_GAME_CLEAN_FILES := game/src/game/bgm.o game/src/game/game-mode.o game/src/game/score.o game/src/game/se.o game/src/game/stage.o game/src/game/stars.o
