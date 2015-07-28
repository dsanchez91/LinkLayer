/*
 ** Fichero: enlace.h
 ** Autores:
 ** Alvaro Valiente Herrero 
 ** Diego Sanchez Moreno 
 ** Usuario: i8112600
 */

#include <stdio.h>
#include <stdlib.h>
#include <windows.h>

#define PERROR(a) \
	{             \
	     LPVOID lpMsg;                     \
		 FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | \
		               FORMAT_MESSAGE_FROM_SYSTEM |     \
					   FORMAT_MESSAGE_IGNORE_INSERTS, NULL, \
					   GetLastError(), \
					   MAKELANGID(LANG_NEUTRAL,SUBLANG_DEFAULT), \
					   (LPTSTR) &lpMsg, 0, NULL);  \
		 fprintf(stderr,"%s:(%d)%s\n",a,GetLastError(),lpMsg); \
         LocalFree(lpMsg); \
     }

//enlace.h

//DEFINES
#define MTU 1500 //max length of frame
#define dllname "fisico.dll" //name of dll
     
//FRAME PARTS
#define SOH 0x01
#define STX 0x02
#define ETX 0x03
#define EOT 0x04
#define ENQ 0x05
#define ACK 0x06
#define DLE 0x10
#define NAK 0x15
#define SYN 0x16
#define ETB 0x17

//FRAME TYPES

#define enqFrame 1
#define ackFrame 2
#define dataFrame 3
#define morseFrame 4
#define eotFrame 5


//STRUCTS

struct Frame
{
       char body[MTU];
       unsigned char data[MTU];
       int length; 
       int type;
       int nSecuence;
   
}Frame;

//GLOBAL VARIABLES

int side;//determina si es lado izquierdo(emisor) o lado derecho(receptor)
struct Frame *fI;//frame que manejara el hilo del emisor
struct Frame *fD;//frame que manejara el hilo del receptor
int conexionOK;//indica si se esta conectado
int nSecuenceWait;//la maneja el receptor
int nSecuenceNext;//la maneja el emisor

int tam;//tamaño de la trama
char route[50];//ruta del .ini
int errors;//tasa de errores
int velocity;//velocidad de transferencia
int tam;//tamaño maximo de trama
int mode;//modo debug
int timeout;//tiempo de espera

int maxTrys;//maximo numero de trys antes de desconexion

int received;//total tramas recibidad
int correct;//tramas recibidas correctas
int trysStadistics;//maximo numero de reintentos en una sesion
  

//HANDLES

HANDLE mydll;//manejador de la dll

HANDLE hstdoutI;//manejador de la stdout del terminal para emisor
HANDLE hstdoutD;//manejador de la stdout del terminal para receptor
HANDLE waitingI;//manejador del hilo que recogera las tramas y las evaluara del emisor
HANDLE waitingD;//manejador del hilo que recogera las tramas y las evaluara del receptor

HANDLE side_I_data_ready;//semaforo que indicara cuando hay una trama lista en el emisor
HANDLE side_D_data_ready;//semaforo que indicara cuando hay una trama lista en el receptor

HANDLE conexion;//semaforo que indicara cuando se realiza la conexion


//DLL FUNCTIONS

DWORD loadDll(void);
DWORD liberateDll(void);

//FUNCTIONS

DWORD initiateSideI(int,int,int/*errors, velocity, viewMode*/);
DWORD initiateSideD(int,int,int/*errors, velocity, viewMode*/);

DWORD makeFrame(int, unsigned char*);
DWORD sendFrame(struct Frame);
DWORD evaluateFrame(struct Frame*);
DWORD loadFrame(struct Frame*);
DWORD clearFrame(struct Frame*);

DWORD printData(char );
void morse(char,unsigned char*);
void catchData(struct Frame*);
void printStadistics(void);

//THREAD FUNCTION

DWORD WINAPI waiting_for(LPVOID);


//CONNECTION SERVICE PRIMITIVES

//connection
DWORD L_CONNECT_request(long, long, int/*source address, destination address, priority*/);
DWORD L_CONNECT_indication(long, long, int/*source address, destination address, priority*/);
DWORD L_CONNECT_response(long, long, int/*source address, destination address, priority*/);
DWORD L_CONNECT_confirm(long, long, int/*source address, destination address, priority*/);

//disconnection
DWORD L_DISCONNECT_request(long, long/*source address, destination address*/);
DWORD L_DISCONNECT_indication(long, long, int/*source address, destination address, reason*/);

//data send
DWORD L_DATA_request(long, long, char*/*source address, destination address, data*/);
DWORD L_DATA_indication(long, long/*source address, destination address*/);
