compile-alex:
	gcc ./src/main.c -o ./src/alex
run-alex:
	./src/alex ./tests/test.c