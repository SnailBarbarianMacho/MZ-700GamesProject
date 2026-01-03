.PHONY: all release clean

all:
	@cd test && make
	@cd memory-dirtinizer && make
	@cd ram-check && make
	@cd joystick-mz1x03-test && make
	@cd 8253-test && make
	@cd sound-test1 && make
	@cd SoundTest2 && make
	@cd SoundTest3 && make
	@cd SoundTest4 && make
	@cd ST-2 && make

release:
	@cd test && make
	@cd memory-dirtinizer && make
	@cd ram-check && make
	@cd joystick-mz1x03-test && make
	@cd 8253-test && make
	@cd sound-test1 && make
	@cd Sound-test2 && make
	@cd Sound-test3 && make
	@cd Sound-test4 && make
	@cd ST-2 && make release

clean:
	@cd test && make clean
	@cd memory-dirtinizer && make clean
	@cd ram-check && make clean
	@cd joystick-mz1x03-test && make clean
	@cd 8253-test && make clean
	@cd sound-test1 && make clean
	@cd SoundTest2 && make clean
	@cd SoundTest3 && make clean
	@cd SoundTest4 && make clean
	@cd ST-2 && make clean
