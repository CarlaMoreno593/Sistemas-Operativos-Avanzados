#Make file para spreadTotal y spreadCausal
CC = gcc
all: crearDir spreadTotal spreadCausal

crearDir:
	mkdir obj

spreadTotal:spreadTotal.c obj
	${CC} -o total spreadTotal.c -lspread
	mv total obj

spreadCausal: spreadCausal.c
	${CC} -o causal spreadCausal.c -lspread 
	mv causal obj

clean: obj
	rm obj/total obj/causal
	rmdir obj
