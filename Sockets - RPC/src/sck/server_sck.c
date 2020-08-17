
// Server side C/C++ program to demonstrate Socket programming 
#include <unistd.h> 
#include <stdio.h> 
#include <sys/socket.h> 
#include <stdlib.h> 
#include <netinet/in.h> 
#include <string.h> 
#include <errno.h>

struct argumentos {
    char *a;
    char *b;
};
#define PORT 8080 

#define PATH_MAX 30

int main(int argc, char const *argv[]) 
{ 
   
    // 
    // Creating socket file descriptor 
    //int sockfd = socket(domain, type, protocol)->Retorna un entero que es el socket descriptor
    //AF_INET -> IPv4, SOCK_STREAM: Orientado a la conexión, confiable
    //0: IP
    int server_fd;
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) 
    { 
        perror("socket failed"); 
        exit(EXIT_FAILURE); 
    } 
     

    //Manipula opciones para el socket referido por el file descriptor 
    //server_fd. 
    //int setsockopt(int sockfd, int level, int optname, const void *optval, socklen_t optlen);
    //level: SOL_SOCKET

    // Forcefully attaching socket to the port 8080 
    //Se setea SO_REUSEADDR y el otro en 1 para que pueda ser reutilizada
    //la dirección ip y puerto en otras conexiones
    int opt = 1; 
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, 
                                                  &opt, sizeof(opt))) 
    { 
        perror("setsockopt"); 
        exit(EXIT_FAILURE); 
    } 


    //Define estructura del socket: dirección IP, puerto (PORT definido arriba), etc
    struct sockaddr_in address; 
    address.sin_family = AF_INET; 
    //Esto es para configurar la dirección IP del socket.
    //Con INADDR_ANY, el bind va a tomar todas las interfaces, no solo 
    //Localhost.
    address.sin_addr.s_addr = INADDR_ANY; 
    address.sin_port = htons( PORT ); 
       
    // Forcefully attaching socket to the port 8080 
    if (bind(server_fd, (struct sockaddr *)&address,  
                                 sizeof(address))<0) 
    { 
        perror("bind failed"); 
        exit(EXIT_FAILURE); 
    } 

    //Si pudo ejecutarse el bind, el servidor intenta ejecutar el listen
    //que pone al servidor a escuchar peticiones
    //El 3 indica la longitud maxima a la que la cola pendiente de conexiones puede
    //crecer. Si llega un request cuando esta llena (son más de 3 peticiones), el cliente recibe un error

    if (listen(server_fd, 3) < 0) 
    { 
        perror("listen"); 
        exit(EXIT_FAILURE); 
    } 


    //Revisa la cola de conexiones pendientes, y si tiene elementos, toma
    //el primero, crea un nuevo socket para realizar la conexión, y new_socket
    //contiene el nuevo fd del socket que referencia. Se establece conexión 
    //entre cliente y servidor

    //El servidor se queda escuchando peticiones

    while(1){

        int new_socket;
        int addrlen = sizeof(address); //Longitud de la dirección
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address,  
                           (socklen_t*)&addrlen))<0) 
        { 
           	 	perror("accept"); 
            	exit(EXIT_FAILURE); 
        } 


        //Declaro la estructura donde voy a almacenar lo que envió el cliente
        char buffer[1024] = {0}; 
        //Almacena el file descriptor del buffer 
        //Lee lo que vino desde cliente y lo coloca en el buffer definido
        int valread; 
        valread = read( new_socket , buffer, 1024); 

        printf("Comando a Ejecutar: %s\n", buffer);


        FILE *fp;
        int status;
        char path[PATH_MAX];

        fp = popen(buffer, "r");
        if (fp == NULL) {

            send(new_socket , "Hubo un error" , strlen("Hubo un error") , 0);

        }

        //Se inicializa en 0 los elementos, porque si no almacena basura
        //Aparacen simbolos raros en la salida del cliente. 
        char mensaje[1024]={0};

        while (fgets(path, PATH_MAX, fp) != NULL ) {

            if((strlen(mensaje)+strlen(path))>1024){

                send(new_socket , mensaje , strlen(mensaje) , 0);
                valread = read( new_socket , buffer, 1024); 
                //printf("%s\n", buffer);

                //Inicializo en cero el array:
                for (int i = 0; i < strlen(mensaje) ; i++){ 
                    mensaje[i] = 0; 
                }

            }

            strcat(mensaje, path);

        }

        send(new_socket , mensaje, strlen(mensaje), 0);
        send(new_socket , "RECIBIDO" , sizeof("RECIBIDO") , 0);
        //printf("Salgo del while\n");
        status = pclose(fp);
        if (status == -1) {
            /* Error reported by pclose() */
        } else {
            /* Use macros described under wait() to inspect `status' in order
            to determine success/failure of command executed by popen() */
        }

    }

    return 0;
} 

