OBJ = main.o lexer.o
lexical.exe : $(OBJ)
	gcc -o $@ $^
clean:
	rm *.exe *.o