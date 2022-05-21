.PHONY: all release clean

all:
	cd SoundTest && make
	cd SoundTest2 && make
	cd ST-2 && make

release:
	@cd SoundTest && make
	@cd SoundTest2 && make
	@cd ST-2 && make release

clean:
	@cd SoundTest && make clean
	@cd SoundTest2 && make clean
	@cd ST-2 && make clean
