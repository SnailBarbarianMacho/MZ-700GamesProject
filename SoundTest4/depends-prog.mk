# **** This file is made by ../tools/make_depend.php. DO NOT MODIFY ! ****

# prog
./main.o: ./main.c ../src-common/common.h ../src-common/hard.h sound.h addr.h music/chronos.h
./sound-sd4.c: ./sound-sd4.z80ana.c ../src-common/common.h ../src-common/hard.h ../src-common/asm_macros.h ../src-common/z80ana-def.h addr.h sound.h music/drum1.h music/drum2.h music/drum3.h
./sound-sd4.o: ./sound-sd4.c
OBJS := ./main.o ./sound-sd4.o
PROG_CLEAN_FILES := ./main.o ./sound-sd4.c ./sound-sd4.o
