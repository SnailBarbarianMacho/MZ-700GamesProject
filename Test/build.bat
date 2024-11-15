zcc +mz -mz80 -O3 --opt-code-speed=all -o bench-sccz -create-app main.c -m
zcc +mz -mz80 -OS3 -compiler=sdcc --max-allocs-per-node200000 -o bench-sdcc -create-app main.c -m
