zcc +mz -mz80 -O3 --opt-code-speed=all -o bench_sccz80 -create-app main.c
zcc +mz -mz80 -OS3 -compiler=sdcc --max-allocs-per-node200000 -lndos -crt0my_crt0 -o bench_sdcc -create-app main.c
