compile:
	gcc ./src/main.c ./src/ad.c ./src/at.c -o ./src/alex
run:
	./src/alex ./tests/test.c