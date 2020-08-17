// Client side C/C++ program to demonstrate Socket programming 
#include <stdio.h> 
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <unistd.h> 
#include <string.h> 
#define PORT 8080 
   
int main(int argc, char const *argv[]) 
{ 

//Lógica de Sockets

    //Crea socket del cliente con el que se comunicará con servidor
    //Tiene mismo protocolo, domain, y type
    int sock = 0;
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
    { 
        printf("\n Socket creation error \n"); 
        return -1; 
    } 
    

    //Se define una estructura con la dirección del servidor.
    //Donde especifico IP?
    struct sockaddr_in serv_addr; 
    serv_addr.sin_family = AF_INET; 
    serv_addr.sin_port = htons(PORT); 
       
    // Convert IPv4 and IPv6 addresses from text to binary form 
    //Esto convierte el "127.0.0.1" en AF_INET para que pueda ser usado
    //en la estructura de serv_addr
    //AQUI ES DONDE VA ARGV[1]
    if(inet_pton(AF_INET, argv[1], &serv_addr.sin_addr)<=0)  
    { 
        printf("\nInvalid address/ Address not supported \n"); 
        return -1; 
    } 

    //Conecta el socket del cliente (de fd = sock) con el socket del servidor

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) 
    { 
        printf("\nConnection Failed \n"); 
        return -1; 
    } 

//Lógica de la conexión. Envío de mensajes

    //Defino variable que almacenará el fd de la conexion?
    int valread; 
    char buffer[1024] = {0}; 
    char comando[1024] = {0};

    //Armo el comando que quiere ejecutar
    int i=2;
    while(i<argc){

        strcat(comando, argv[i]);
        strcat(comando, " ");

        i++;
    }

    send(sock , comando , strlen(comando) , 0 ); 

    //Imprime la respuesta del servidor
    valread = read( sock, buffer, 1024); 

    while(valread>0){


        if(strstr(buffer, "RECIBIDO")){

            if(strlen(buffer) == strlen("RECIBIDO")){
 
                send(sock, "Recibido", strlen("Recibido"), 0);
                return 0;  
                
            }

            char aux[1024] = {0};
            strncpy(aux, buffer, strlen(buffer)-sizeof("RECIBIDO"));
            printf("%s\n",aux); 
            send(sock, "Recibido", strlen("Recibido"), 0);
            return 0;

        }

        printf("%s\n",buffer ); 
        send(sock, "Recibido", strlen("Recibido"), 0);
        valread = read( sock, buffer, 1024); 


    }
    
    
    
    
    return 0; 
} 

