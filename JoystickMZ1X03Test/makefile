.PHONY:    all clean
TOOLS   	:= ../tools
CRT0 		:= crt0.asm
OUTFILE 	:= JoystickMZ1X03Test
PROG_NAME 	:= 'MZ-1X03 TEST'
CC      	:= zcc
DEBUG   	:= -DDEBUG=1
CFLAGS  	:= +mz -lndos -compiler=sdcc -OS3 $(DEBUG)
LFLAGS      :=\
	-m -s\
	-lndos\
	-pragma-define:CRT_ENABLE_STDIO=0\
	-pragma-define:CRT_INITIALIZE_BSS=0\
	-pragma-define:CLIB_EXIT_STACK_SIZE=0
# 0x1200 = 4608
ORG     	:= 4608
RMF     	:= del
#RMF    	:= rm -f

all: $(OUTFILE).bin

OBJS		:= main.o
main.o: main.c

$(OUTFILE).bin: $(OBJS) $(CRT0)
	@echo Linking ... [$@]
	@$(CC) +mz -mz80 $(LFLAGS) -o $@ $(OBJS) -crt0$(CRT0) -zorg=$(ORG)
	@php $(TOOLS)/make_mzt.php $@ $(PROG_NAME) $(ORG) $(ORG) $(OUTFILE).mzt

%.o: %.c
	@echo Compiling ... [$<]
	@$(CC) $(CFLAGS) $(DEBUG) $(OPT) -o $@ -c $<

clean:
	@echo Cleaning ...
	@$(RMF) *.o
	@$(RMF) *.bin
#	@$(RMF) $(OUTFILE).mzt
