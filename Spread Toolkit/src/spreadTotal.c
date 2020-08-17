/*Argumentos:
	
	-ID del nodo
	-N=Máxima cantidad de nodos del grupo
	-C=Cantidad de mensajes a enviar luego de que se conecten TODOS LOS NODOS
	-L=Longitud del mensaje a enviar. Bytes.

*/	

//Defino variables globales
#include "sp.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h> 

//static char usuario[80]; //Sería el nombre privado, pero paso un null sin problema
static char daemon[80]; //puerto@IP -> dirección donde el demonio está conectado
static char usuario[80]; //Nombre del usuario que se conecta al grupo. Debe ser único por grupo.
static int nodosMax; //Cantidad de nodos máximos que espera a que se conecten
static int cantMensajes; //Cantidad de mensajes a enviar
static int longitudMax; //Longitud máxima de mensaje

static char private_group[MAX_GROUP_NAME]; //Se retorna en la llamada SP_Connect

static int bytesMembresiaEnv; //Acumula los bytes totales, incluidos los de membresía
static int bytesMembresiaRec;
static int bytesMensajesEnv; //Acumula solo los bytes de los mensajes enviados
static int bytesMensajesRec;
static  mailbox Mbox; //Connection handler


//Defino método auxiliares
static int inicializar(int argc, char *argv[]);
static int armarMensaje(char *mensaje, int tam);
static int recibirMembresia();
static int recibirRegular();
static int recibirMensaje();

int main(int argc, char const *argv[]){
	/*
		Esquema general Nodo:

			-Se conecta al demonio (sp_conect)
			-Se une al grupo (sp_join) -> hace multicast de membresía

			-Comienza a enviar de a un mensaje de longitud L -> multicast
			-Espera que los demás le envien un mensaje (N mensajes)
			-Repite lo anterior hasta llegar a C.
			
			-Debe medir tiempo desde que arranca intercambio de mensajes hasta que termina
			-Además debe medir el Throughput del sistema (cantidad de bytes enviados en total)
			
			-Probar con entrega Causal y entrega Total Order
	*/

	
	//Inicializo varibles
		if(inicializar(argc, argv)==-1) return -1;

	//Conexión a Demonio
		
		//daemon -> puerto@IP donde el demonio está ejecutando
		//usuario -> Es el nombre de conexión que se le asigna a la máquina que ejecuta
		//0 -> indica que no se aplicará prioridad para esta conexión.
		//1 -> indica que permite recibir mensajes de membresía
		//Mbox -> 
		//private_group -> Contendrá el nombre del grupo donde estará esta conexión. 
		int con = SP_connect(daemon, usuario, 0, 1, &Mbox, private_group);

		//Verifica que no haya error con conexión
		if( con < 0 ) { SP_error( con ); exit(0); }

	//Unión al grupo. Este join causa el envío de un multicast.  
		int join = SP_join( Mbox, "TP-VSOA");
		if( join < 0 ) { SP_error( join ); exit(0); }

	//Espera a que los nodosMax se conecten antes de comenzar con el intercambio de mensajes
	//Esta espera termina luego de recibir nodosMax mensajes de Membresía causados por los join. Compara la cantidad de integrantes actuales con el nro. maximo 
	//pasado como argumento
		int memb=1;

		printf("Espera Conexión de Todos los Nodos\n");
		do{

			//Lee un mensaje de membresia
			memb=recibirMensaje();
			printf("Bytes recibidos: %d\n", bytesMembresiaRec);
			printf("----------------------------------------------------------------------------\n");
		
			//Compara si la cantidad de nodos actuales conectados es igual o no a la cantidad de nodos esperados.
			//Si no se conectaron todos los nodos, entonces repite el receive (bloqueante)
		}while(memb!=nodosMax && memb>0);

		if(memb<0){printf("Hubo un error con los mensajes de membresía. Reinicie la aplicación\n"); exit(0);}

		printf("Todos los nodos se han conectado\n");
		printf("================================================================================\n");
		
	//Comienza el intercambio de mensajes

		//Arranca el cronómetro
		clock_t tiempo; 
    	tiempo = clock(); 
		
		int n=0;
		char mensaje[longitudMax];
		int ret;

		while(n<cantMensajes){

			armarMensaje(mensaje, longitudMax);

			//Hace un multicast
			ret = SP_multicast( Mbox, AGREED_MESS, "TP-VSOA", 1, longitudMax, mensaje);
			if( ret < 0 ) { SP_error( ret ); exit(0); }

			bytesMensajesEnv = ret + bytesMensajesEnv;
			int i=0;
			int reg=0;

			while(i<nodosMax){
				//Lee nodosMax-1 mensajes del resto de los nodos y el que se envía a si mismo por broadcast
				reg=recibirMensaje();
				if (reg<0){printf("Hubo un error al intentar recibir el mensaje. Reinicie la aplicación.\n"); exit(0);}
				printf("Bytes recibidos: %d\n", bytesMensajesRec);
				printf("----------------------------------------------------------------------------\n");
		
				i++;
			}

			n++;
		}

		int leave = SP_leave(Mbox, "TP-VSOA");
		if (leave<0){ SP_error( leave ); exit(0);}

		int disc =  SP_disconnect(Mbox);
		if (disc<0){ SP_error( disc ); exit(0);}

	//Detiene el cronómetro
		tiempo = clock() - tiempo; 
    	double tiempoTotal = ((double)tiempo)/CLOCKS_PER_SEC; // in seconds 
		

		printf("================================================================================\n");
		printf("Tiempo Total de Ejecución: %f\n",tiempoTotal);
		printf("Datos respecto al Intercambio de Mensajes: \n");
		printf("Bytes Totales Intercambiados: %d\n", (bytesMembresiaEnv+bytesMensajesEnv+bytesMensajesRec+bytesMembresiaRec));
		printf("Bytes Totales Membresía: %d\n", (bytesMembresiaRec+bytesMembresiaEnv));
		printf("Bytes Recibidos de Membresía: %d\n", bytesMembresiaRec);
		printf("Bytes Totales de Mensajes Regulares: %d\n", (bytesMensajesEnv+bytesMensajesRec));
		printf("Bytes Enviados de Mensajes Regulares: %d\n", bytesMensajesEnv );
		printf("Bytes Recibidos de Mensajes Regulares: %d\n", bytesMensajesRec);


	return 0;

}

static int inicializar(int argc, char *argv[]){

	//./nodo [demonio] [cantNodos] [cantMensajes] [longMensaje]
	if(argc<6){
		
		printf("================================================================================\n");
		printf("Forma de Ejecución: ./obj/total [Demonio] [Usuario] [cantNodos] [cantMensajes] [longMensaje]\n");
		printf("Formatos válidos de demonio:\n");
		printf("Nro. de puerto (si el demonio ejecuta en la misma máquina) -> pppp\n");
		printf("Nro. de puerto más IP (si el demonio ejecuta en otra máquina) -> pppp@xxx.xxx.xxx\n");
		printf("================================================================================\n");
				
		return -1;
	}

	//Puerto al que el nodo se va a conectar.
	strcpy(daemon, argv[1]); 

	//Nombre del usuario
	strcpy(usuario, argv[2]);

	//Cantidad de nodos máximos que espera a que se conecten
	nodosMax = atoi(argv[3]); 
	
	//Cantidad de mensajes a enviar
	cantMensajes= atoi(argv[4]);

	//Longitud de mensajes que va a enviar
	longitudMax= atoi(argv[5]); 

	bytesMensajesEnv=0;
	bytesMensajesRec=0;
	bytesMembresiaEnv=0;
	bytesMembresiaRec=0;

	return 0;
	
}

static int recibirMensaje(){
	
	int n=0;
	int tipoServicio; //Tipo de servicio. Usado en Is_..._mess()
	char grupo[MAX_GROUP_NAME]; //Almacenará el nombre del proceso emisor
	char destinos[100][MAX_GROUP_NAME]; //Como máximo, se podrá enviar el mensaje a 100 grupos
	int cantidadInt=0; //Cantidad de grupos a los que el mensaje fue enviado
		
	int16 tipoMensaje; //Indica el tipo de mensaje que se va a recibir
	membership_info memb_info; //Para almacenar la información de membresía
	int endian_mismatch;

	char mensaje[102400]; 

	int rec = SP_receive(Mbox, &tipoServicio, grupo, 100, &cantidadInt, destinos, 
		&tipoMensaje, &endian_mismatch, sizeof(mensaje), mensaje);

	if(rec<0){ SP_error(rec); exit(0); }
	
	//Si es de MEMBRESÍA
	if(Is_membership_mess(tipoServicio)){

		bytesMembresiaRec = bytesMembresiaRec+rec;

		rec = SP_get_memb_info(mensaje, tipoServicio, &memb_info);
		if(rec<0){SP_error(rec); exit(0);}
		

		printf("%d\n",rec );
		if (Is_reg_memb_mess(tipoServicio)){

			printf("Se recibió un mensaje de MEMBRESÍA REGULAR\n");
			if( Is_caused_join_mess( tipoServicio ) ) printf("causado por JOIN ");
			if( Is_caused_leave_mess( tipoServicio ) ) printf("causado por LEAVE ");
			if( Is_caused_disconnect_mess( tipoServicio ) ) printf("causado por DISCONNECT");
					
			printf("\nGrupo: %s - Cantidad: %d\n", grupo, cantidadInt);
					
			for( int i=0; i < cantidadInt; i++ ) 
				printf("\t%s\n", &destinos[i][0] );

			printf("Membership ID:  (%d , %d )\n",memb_info.gid.id[0], memb_info.gid.id[1]);

		}else if( Is_transition_mess(tipoServicio)) {
			printf("Se recibió un mensaje de MEMBRESÍA de tipo TRANSITIONAL\n");
		}else if(Is_caused_leave_mess(tipoServicio)){
			printf("Se recibió un mensaje de MEMBRESÍA de tipo LEAVE MESS\n");
		}else printf("Tipo de mensaje indeterminado %d\n", tipoServicio);
		
		return cantidadInt;
	}

	//Si es REGULAR
	if( Is_regular_mess( tipoServicio ) ){

		bytesMensajesRec= rec+bytesMensajesRec;

		char regular[longitudMax];
		strncpy(regular, mensaje, rec);
		regular[rec] = 0;

		if     ( Is_unreliable_mess( tipoServicio ) ) printf("received UNRELIABLE ");
		else if( Is_reliable_mess(   tipoServicio ) ) printf("received RELIABLE ");
		else if( Is_fifo_mess(       tipoServicio ) ) printf("received FIFO ");
		else if( Is_causal_mess(     tipoServicio ) ) printf("received CAUSAL ");
		else if( Is_agreed_mess(     tipoServicio ) ) printf("received AGREED ");
		else if( Is_safe_mess(       tipoServicio ) ) printf("received SAFE ");
		printf("message from %s of type %d (endian %d), to %d groups \n(%d bytes): %s\n",
			grupo, tipoMensaje, endian_mismatch, cantidadInt, rec, regular );

		return 1;
	}

	return -1;

}


static int armarMensaje(char *mensaje, int tam){

	for (int i = 0; i < tam; ++i){
		
		strcat(mensaje, "M");

	}

}