compile-alex:
	gcc ./src/lexical.c -o ./src/alex
run-alex:
	./src/alex ./tests/test.c