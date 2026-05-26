# **** This file is made by ../tools/make-depend.php. DO NOT MODIFY ! ****

# prog
./main.c: ./main.aal.c ../src-common/aal80.h ../src-common/common.h ../src-common/hard.h sound.h addr.h music/chronos.h
./main.o: ./main.c
./sound-sd4.c: ./sound-sd4.aal.c ../src-common/common.h ../src-common/hard.h ../src-common/asm-macros.h ../src-common/aal80.h addr.h sound.h music/drum1.h music/drum2.h music/drum3.h
./sound-sd4.o: ./sound-sd4.c
OBJS := ./main.o ./sound-sd4.o
PROG_CLEAN_FILES := ./main.c ./main.o ./sound-sd4.c ./sound-sd4.o
