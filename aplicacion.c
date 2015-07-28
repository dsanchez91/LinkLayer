/*
 ** Fichero: aplicacion.c
 ** Autores:
 ** Alvaro Valiente Herrero 
 ** Diego Sanchez Moreno 
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <windows.h>
#include "fisico.h"
#include "enlace.h"


int main(int argc, char *argv[])
{
    unsigned char *data;
  
  //Comprobar Parametros   
    if (argc != 3)
	{
		fprintf(stderr,"\nHow to use: %s I|D funcion.ini\n",argv[0]);
		return 1;
	}
	else
	{  
         if (toupper(argv[1][0])!='I' && toupper(argv[1][0])!='D')
	      {
            fprintf(stderr,"\nThe first parameter has to be I or D\n");
            return 1;
         }
    
      	strcpy(route,".\\");
      	strcat(route,argv[2]);
      	
      	//obtenemos los parametros del .ini
      	errors=GetPrivateProfileInt("DLL","Errores",0,route);
      	velocity=GetPrivateProfileInt("Serie","Velocidad",80,route);
      	tam=GetPrivateProfileInt("Enlace","tamTrama",1500,route);
      	mode=GetPrivateProfileInt("Preferencias","verTramasYprimitivas",0,route);        
   }
   
  //cargamos la dll
  if(loadDll() == 1)
  {
      fprintf(stderr,"\nError loading: loadDll\n");
      return 1;       
  }
  
  //si es el lado I(emisor)  
  if(toupper(argv[1][0]) == 'I')
  {           
      system("cls");
	  printf("=====================================\n");
	  printf("|           Transmitter             |\n");
	  printf("=====================================\n");
	  printf("\n");      
      initiateSideI(errors,velocity,mode); 

	//conexion
	switch(L_CONNECT_request(0,0,0))
	{
      case 0://ha llegado ACK
         L_CONNECT_confirm(0,0,0);  
         break;
         
       case 1://trys agotados
            fprintf(stderr,"\nTry to connection time out\n");
            L_DISCONNECT_request(0,0);
            return 0;
            break;       
   }
      
    //datos 
    while(1)
    {

      data = (char*) malloc(sizeof(char)*80);
      printf("\nEnter a phrase: ");fgets(data,MTU,stdin);
      
      //si introducimos FIN provocamos el disconnect
      if(strcmp(data,"FIN\n")==0){break;}
      
      switch(L_DATA_request(0,0,data))
      {
                case 0:
                  //nos ha llegado un dato en morse               
                  L_DATA_indication(0,0);
                  printf("\nIn morse: %s",fI->data);                   
                break; 
                
                case 1:
                   L_DISCONNECT_request(0,0);
                   return 0;  
                  
               break;                            
      }
   }
   
   L_DISCONNECT_request(0,0);
   
   return 0;
  }
  
  //si es el lado D(receptor)
  if(toupper(argv[1][0]) == 'D')
  {                    
      system("cls");
	  printf("=====================================\n");
	  printf("|           Receptor                |\n");
	  printf("=====================================\n");
	  printf("\n");    
      initiateSideD(errors,velocity,mode);
   
      //conexion
     do
     {          
         switch(L_CONNECT_indication(0,0,0))
         {
            case 0:               
               L_CONNECT_response(0,0,0);
            break;
            
            case 1:
               L_DISCONNECT_indication(0,0,0);
               return 0;            
            break;  
         }        
      }while(conexionOK != 1);
         
      //datos    
      
      while(1)
      {      
         switch(L_DATA_request(0,0,NULL))
         {                          
            case 0:                
                  L_DATA_indication(0,0);
                  //este sleep es para que no se envien al mismo tiempo ACK y morse
                  Sleep(2000);                
                  L_DATA_request(0,0,fD->data);                                 
            break;
                   
            case 1:        
                  L_DISCONNECT_indication(0,0,0);                   
                  return 0;                   
            break;                                    
         }     
      }
   }
}
