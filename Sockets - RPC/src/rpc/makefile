
#Make file para RPC
CC = gcc
all: clienteRPC servidorRPC
servidorRPC:comandoServidor.c comando_svc.c comando_xdr.c 
	${CC} -o servidorRPC comandoServidor.c comando_svc.c comando_xdr.c -lrpcsvc -lnsl

clienteRPC: comandoCliente.c comando_clnt.c comando_xdr.c 
	${CC} -o clienteRPC comandoCliente.c comando_clnt.c comando_xdr.c -lnsl 

clean: 
	rm servidorRPC clienteRPC
