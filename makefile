.PHONY: all release clean

all:
	cd ram-check && make
	cd SoundTest && make
	cd SoundTest2 && make
	cd SoundTest3 && make
	cd SoundTest4 && make
	cd JoystickMZ1X03Test && make
	cd ST-2 && make
	cd Baryon && make

release:
	@cd ram-check && make release
	@cd SoundTest && make
	@cd SoundTest2 && make
	@cd SoundTest3 && make
	@cd SoundTest4 && make
	@cd JoystickMZ1X03Test && make
	@cd ST-2 && make release
	@cd Baryon && make release

clean:
	@cd ram-check && make clean
	@cd SoundTest && make clean
	@cd SoundTest2 && make clean
	@cd SoundTest3 && make clean
	@cd SoundTest4 && make clean
	@cd JoystickMZ1X03Test && make clean
	@cd ST-2 && make clean
