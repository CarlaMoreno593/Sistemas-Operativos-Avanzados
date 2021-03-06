/*
 * This is sample code generated by rpcgen.
 * These are only templates and you can use them
 * as a guideline for developing your own functions.
 */

#include "comando.h"


void
comando_prg_1(char *host, char *argumento)
{
        CLIENT *clnt;
        char * *result_1;
        struct comando  ejecucioncomando_1_arg;

#ifndef DEBUG
        clnt = clnt_create (host, COMANDO_PRG, COMANDO_1, "udp");
        if (clnt == NULL) {
                clnt_pcreateerror (host);
                exit (1);
        }
#endif  /* DEBUG */

        strcpy(ejecucioncomando_1_arg.elementos,argumento);
        printf("Comando: %s\n",ejecucioncomando_1_arg.elementos);
        
        /*result_1 = malloc(PATH_MAX+1);*/ 
        result_1 = ejecucioncomando_1(&ejecucioncomando_1_arg, clnt);
        if (result_1 == (char **) NULL) {
                clnt_perror (clnt, "call failed");
        }
        printf("Resultados: \n %s \n", *result_1);
        
#ifndef DEBUG
        clnt_destroy (clnt);
#endif   /* DEBUG */
}


int
main (int argc, char *argv[])
{
        
        char *host;
        char comando[1024] = {0};

        if (argc < 3) {
                printf ("usage: %s server_host command\n", argv[0]);
                exit (1);
        }

        //printf("Entro al for\n");

        for(int i=2; i<argc; i++){

                //printf("Concateno comando con arg\n");
                strcat(comando, argv[i]);
                strcat(comando," ");
        }

        host = argv[1];
        printf("Llamada al procedimiento\n");
        comando_prg_1 (host, comando);

        exit (0);

}
