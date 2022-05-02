compile-alex:
	gcc ./src/main.c ./src/ad.c -o ./src/alex
run-alex:
	./src/alex ./tests/test.c