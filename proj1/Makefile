album: main.c
	gcc -g main.c -o main

complete:
	make album
	./main $(ARGS)
	pandoc index.html -o index.pdf
 
test:
	gcc main.c -o main
	cat ./tests/test$(TEST).txt | ./main ./photos/*
	mkdir -p ./tests/test$(TEST)
	mv ./index.html ./tests/test$(TEST)
	ls ./tests/test$(TEST)/truth.html && cmp ./tests/test$(TEST)/index.html ./tests/test$(TEST)/truth.html
