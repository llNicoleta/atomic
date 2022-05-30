compile:
	gcc ./src/main.c ./src/ad.c ./src/at.c ./src/mv.c ./src/gc.c -o ./src/alex
run:
	./src/alex ./tests/test.c