# **** This file is made by ../tools/make_depend.php. DO NOT MODIFY ! ****

# prog
./main.o: ./main.c ../src-common/common.h ../src-common/hard.h sound.h addr.h music/chronos.h
./sound-sd6.c: ./sound-sd6.z80ana.c ../src-common/common.h ../src-common/hard.h ../src-common/asm_macros.h ../src-common/z80ana.h addr.h sound.h music/sd6_data.h
./sound-sd6.o: ./sound-sd6.c
OBJS := ./main.o ./sound-sd6.o
PROG_CLEAN_FILES := ./main.o ./sound-sd6.c ./sound-sd6.o
