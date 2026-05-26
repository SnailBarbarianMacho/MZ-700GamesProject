# **** This file is made by ../tools/make-depend.php. DO NOT MODIFY ! ****

# prog
./main.c: ./main.aal.c ../src-common/aal80.h ../src-common/common.h ../src-common/hard.h sound.h addr.h music/chronos.h
./main.o: ./main.c
./sound-sd6.c: ./sound-sd6.aal.c ../src-common/common.h ../src-common/hard.h ../src-common/asm-macros.h ../src-common/aal80.h addr.h sound.h music/sd6-data.h
./sound-sd6.o: ./sound-sd6.c
OBJS := ./main.o ./sound-sd6.o
PROG_CLEAN_FILES := ./main.c ./main.o ./sound-sd6.c ./sound-sd6.o
