#Make file para RPC
CC = gcc
all: clienteSCK servidorSCK
servidorSCK:server_sck.c 
	${CC} -o servidorSCK server_sck.c

clienteSCK:client_sck.c 
	${CC} -o clienteSCK client_sck.c

clean:
	rm servidorSCK clienteSCK
