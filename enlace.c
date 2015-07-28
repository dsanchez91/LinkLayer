/*
 ** Fichero: enlace.c
 ** Autores:
 ** Alvaro Valiente Herrero 
 ** Diego Sanchez Moreno 
 ** Usuario: i8112600
 */

#include "enlace.h"
#include "fisico.h"
#include "windows.h"

 TIPO_F_INICIO_PUERTO	f_inicio_puerto;
 TIPO_F_ESCRIBIR		f_escribir;
 TIPO_F_PUEDO_ESCRIBIR	f_puedo_escribir;
 TIPO_F_LEER			f_leer;
 TIPO_F_ESPERAR_LEER	f_esperar_leer;
 TIPO_F_PUEDO_LEER		f_puedo_leer;       
 TIPO_F_ESPERAR_ESCRIBIR	f_esperar_escribir;
 
 
DWORD loadDll(void)
{
    mydll=LoadLibrary(dllname);
    
	if (mydll==NULL)
	{
		PERROR("LoadLibrary");
		return 1;
	}

    f_inicio_puerto=(TIPO_F_INICIO_PUERTO)GetProcAddress(mydll,"f_inicio_puerto");
    if (f_inicio_puerto==NULL)
	{
		PERROR("GetProcAddress:f_inicio_puerto");
		return 1;
	}

    f_escribir=(TIPO_F_ESCRIBIR)GetProcAddress(mydll,"f_escribir");
    if (f_escribir==NULL)
	{
		PERROR("GetProcAddress:f_escribir");
		return 1;
	}

    f_leer=(TIPO_F_LEER)GetProcAddress(mydll,"f_leer");
    if (f_leer==NULL)
	{
		PERROR("GetProcAddress:f_leer");
		return 1;
	}

    f_puedo_escribir=(TIPO_F_PUEDO_ESCRIBIR)GetProcAddress(mydll,"f_puedo_escribir");
    if (f_puedo_escribir==NULL)
	{
		PERROR("GetProcAddress:f_puedo_escribir");
		return 1;
	}

    f_puedo_leer=(TIPO_F_PUEDO_LEER)GetProcAddress(mydll,"f_puedo_leer");
    if (f_puedo_leer==NULL)
	{
		PERROR("GetProcAddress:f_puedo_leer");
		return 1;
	}
    f_esperar_leer=(TIPO_F_ESPERAR_LEER)GetProcAddress(mydll,"f_esperar_leer");
    if (f_esperar_leer==NULL)
	{
		PERROR("GetProcAddress:f_esperar_leer");
		return 1;
	}

    f_esperar_escribir=(TIPO_F_ESPERAR_ESCRIBIR)GetProcAddress(mydll,"f_esperar_escribir");
    if (f_esperar_escribir==NULL)
	{
		PERROR("GetProcAddress:f_esperar_escribir");
		return 1;
	}
    
    return 0;
}
 
 
DWORD liberateDll(void)
{    
      FreeLibrary(mydll);
      return 0;     
}

DWORD initiateSideI(int errors, int velocity, int mode)
{
      side = 0;//al lado izquierdo le corresponde el 0
      maxTrys = 5;
      timeout = 4000;
      nSecuenceNext = 0;
      
      fI = (struct Frame*) malloc(sizeof(Frame));
         
      //iniciamos el puerto
      f_inicio_puerto('I',NULL,"1234",velocity,NULL,errors);
      
      //esto nos devuelve el manejador del output de pantalla
      hstdoutI=GetStdHandle(STD_OUTPUT_HANDLE);
      
      //hilo que esperara la lectura de tramas y de alguna manera avisara    
      if((waitingI = CreateThread(NULL,0,waiting_for,fI,0,NULL)) == NULL)
      {
       fprintf(stderr,"\nError making wating thread\n");
       return 1;
      }
   	//semaforo que indicara si hay datos para leer
   	side_I_data_ready = CreateSemaphore(NULL,0,1,"semaphoreI");
   	//semaforo para indicar que se ha producido la conexion
   	conexion = CreateSemaphore(NULL,0,1,"semaphoreConexion");
      
      return 0; 
}

DWORD initiateSideD(int errors,int velocity ,int mode)
{ 
      side = 1;//al lado derecho le corresponde el 1
     maxTrys = 5;
     timeout = 4000;
     nSecuenceWait = 0;
    
     fD = (struct Frame*) malloc(sizeof(Frame));
         
      //iniciamos el puerto
      f_inicio_puerto('D',NULL,"1234",velocity,NULL,errors);
      
      //esto nos devuelve el manejador del output de pantalla
     hstdoutD=GetStdHandle(STD_OUTPUT_HANDLE);
     
      //hilo que esperara la lectura de tramas y de alguna manera avisara
      if((waitingD = CreateThread(NULL,0,waiting_for,fD,0,NULL)) == NULL)
		{
			fprintf(stderr,"\nError making wating thread\n");
         return 1;
		}
	
    	//semaforo que indicara si hay datos para leer
      side_D_data_ready = CreateSemaphore(NULL,0,1,"semaphoreD");
     
      return 0;    
}

DWORD makeFrame(int type, unsigned char *data)
{
      int ret,i,j,k;
      struct Frame *f;
      unsigned char *in_morse;
      unsigned short crc;
      
      //la cabecera sera igual para todos
      char head[5] = {DLE,SYN,DLE,SYN,DLE};
      
      in_morse = (unsigned char*) malloc(sizeof(unsigned char)*8);
      
      f = (struct Frame*) malloc(sizeof(Frame));
      
      f->length = 0;
    
      //ponemos la cabecera que sera igual para todos indistintamente
      for(i=0;i<5;i++)
      {
         f->body[i]= head[i];
         f->length++;                   
      }
      
      switch(type)//dependiendo del tipo de trama...
      {
        case enqFrame://ENQ       
             f->body[i] = ENQ;
             i++;
             f->length++;
             f->type = enqFrame;                        
        break;
             
        case ackFrame://ACK          
             f->body[i] = ACK;
             i++;
             f->length++;           
             f->type = ackFrame;                 
        break;
        
        case eotFrame://EOT
               f->body[i] = EOT;
               i++;
               f->length++;              
               f->type = eotFrame;    
        break;
        
        case dataFrame://DATA
                        
            f->body[i] = SOH;//inicio cabecera de datos
            i++;
            f->length++;
             
            if(nSecuenceNext == 0)//numero de sencuencia
            {
               f->body[i] = '0';
            }
            else
            {
               f->body[i] = '1';
            }
            
            f->nSecuence = nSecuenceNext;
            i++;
            f->length++;
             
            f->body[i] = DLE;//DLE
            i++;
            f->length++;
             
            f->body[i] = STX;//inicio de datos
            i++;
            f->length++;
             
            for(j=0;j< strlen(data);j++)//datos de la trama
            {       
               f->body[i] = data[j]; 
               if(data[j] == DLE)//si aparece DLE en el texto, este mismo se duplica
               {
                  f->body[i+1] = DLE;
                  i++;
                  f->length++;
               }
                       
               i++;
               f->length++;                                                       
             }
             
             f->body[i] = DLE;//DLE anterior a ETX
             i++;
             f->length++;
             
             f->body[i] = ETX;//fin de datos
             i++;
             f->length++;
             
            //codigo de control de errores CRC
            strcat(data,"\0");
            crc =icrc(0,(unsigned char *)data,strlen(data), 255, -1); 
                       
            f->body[i]=LOBYTE(crc);        
            f->body[i+1]=HIBYTE(crc);
    
            f->length = f->length +2; 
            
            f->type = dataFrame;                                
        break; 
        
        case morseFrame:        
             
            f->body[i] = SOH;//inicio cabecera de datos
            i++;
            f->length++;
             
            if(nSecuenceWait == 0)//numero de sencuencia
            {
               f->body[i] = '0';
            }
            else
            {
               f->body[i] = '1';
            }
             
            f->nSecuence = nSecuenceWait;
            i++;
            f->length++;
             
            f->body[i] = DLE;//DLE
            i++;
            f->length++;
             
            f->body[i] = STX;//inicio de datos
            i++;
            f->length++;
            
            //datos en morse
            for(j=0;j<(strlen(data));j++)
            {
               morse(data[j],in_morse);//llama la funcion que traduce a morse
               
               for(k=0;k<strlen(in_morse);k++)
               {
                  f->body[i] = in_morse[k];
                  if(data[j] == DLE)//si aparece DLE en el texto, este mismo se duplica
                  {
                     f->body[i+1] = DLE;
                     i++;
                     f->length++;
                  }
                  i++;
                  f->length++;
               }
               
               //caracter de separacion entre cada palabra de morse 
               f->body[i] = '#';
               i++;
               f->length++;                                                                                                       
            }
            
            f->body[i] = DLE;//DLE anterior a ETX
            i++;
            f->length++;
             
            f->body[i] = ETX;//fin de datos
            i++;
            f->length++;
             
             //codigo de control de errores CRC
            catchData(f);
            
            crc =icrc(0,(unsigned char *)f->data,strlen(f->data), 255, -1); 
            f->body[i]=LOBYTE(crc);                           
            f->body[i+1]=HIBYTE(crc);
    
            f->length = f->length +2;
            
            f->type = dataFrame;       
         break;                                                 
      }
      //trama creada, procedemos a enviarla   
      ret = sendFrame(*f);
        
      return ret;//devolvemos lo que nos devuelve la funcion de enviar la trama          
}

DWORD sendFrame(struct Frame f) {
    int ret,i,u;
    
    for(i=0;i<f.length;i++)//hasta la longitud de la trama 
    {                          
       if(f_esperar_escribir(INFINITE))//esperamos hasta que podamos escribir
       {
          ret=f_escribir(f.body[i]);//escribimos un caracter
                   
          if(ret == -1){break;}//si retorna -1 paramos
          
          switch(ret)
          {
              case 0://sin problemas al enviar
              break;
                   
              case -1://error de sobreescritura  
                   return 1;
              break;                 
          }   
       }
    }
   //si esta activo el modo debug, mostramos la trama enviada
   if(mode == 1)
   {
      printf("\n-------------------------------SEND------------------------------------\n");
      
      for(u=0;u<f.length;u++)
      {
            printData(f.body[u]);
            printf(" ");
      }
         printf("\n"); 
   }
   else
   {
      if(side == 1)//solo si es el lado del receptor y no esta en modo debug
      {
         if(f.type == dataFrame)
         {
            catchData(&f);
            printf("\nSend: %s",f.data);
         }
          
      }
      
   }
    
   return ret;
}


DWORD WINAPI waiting_for(LPVOID p)
{            
      int ret,first = 1;
      int i=0;
      unsigned char c;
      int flag;
      unsigned short crc;
      int u;
      
      struct Frame *f;
      
      f = (struct Frame*) p;
   
      f->length =0; 
      f->type = 0; 
      
      if(side == 0)//LADO IZQUIERDO (EMISOR)
      {
          while(1)
          {                    
              flag = 0;
              
              while(flag ==0)
              {
                 if(first == 1)//es el primer caracter, por tanto esperamos hasta que exista uno en el buffer que poder leer
                  {
                    f_esperar_leer(INFINITE);
                    clearFrame(f);//aprovechamos para limpiar la trama
                    i=0;    
                  }
                  else
                  {
                    f_esperar_leer(800);//este ya no es el primer caracter
                  }
                  
                  ret=f_leer(&c);                
                  first= 0;
                  		
                  switch (ret)
                  {               
                     case 0://sin problemas al leer                   
                        fI->body[i] = c;
                        fI->length++;
            				i++;          				 
                     break;
           				
            			case -1://sobreescritura
            				if(mode == 1){fprintf(stderr,"Error de sobrescritura al leer\n");}		
           				break;
           				
            			case -2://no queda nada que leer
                                          
            			   //evaluamos la trama
                        fI->type = evaluateFrame(fI);  
                                 
            			   if(mode == 1)
                        { 
               			   printf("\n-------------------------------RECEIVED------------------------------------\n");                                                                        			   
               			   if(fI->type != -1)
               			   {
                           	SetConsoleTextAttribute(hstdoutI, FOREGROUND_GREEN); //Letra verde para trama correcta
                           }
                           else
                           {
                              SetConsoleTextAttribute(hstdoutI, FOREGROUND_RED); //Letra roja para trama incorrecta
                           }
               			   for(u=0;u<fI->length;u++)
                           {
                              printData(fI->body[u]);
                              printf(" ");
                           }
                           
                           printf("\n");
                           SetConsoleTextAttribute(hstdoutI, FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_BLUE); //Letra blanca
                        }
                        
            			   //indicamos que hay una trama lista
                        ReleaseSemaphore(side_I_data_ready,1,NULL);
                        
                        //reiniciamos los valores de control
                        first=1;
                        flag = 1;
           				break;
           				
            			case -3:
                        fprintf(stderr,"\nComunication off\n");
                        conexionOK = 0;
                        ExitThread(1);
           				break;
           			} 
               } 
          } 
       }
       else//LADO DERECHO (RECEPTOR)
       { 
          while(1)
          {       
            flag = 0;
             
              while(flag ==0)
              {
                  if(first == 1)//es el primer caracter, por tanto esperamos hasta que exista uno en el buffer que poder leer
                  {
                     f_esperar_leer(INFINITE);
                     clearFrame(f);//aprovechamos para limpiar la trama
                     i=0;   
                  }
                  else
                  {
                      f_esperar_leer(800);//no es el primera caracter
                  }
                  
                  ret=f_leer(&c);                
                  first = 0;
                
                  switch (ret)
                  {
                	    case 0://sin problemas al leer
                           fD->body[i] = c;
                           fD->length++;
                				i++;
                		break;
               				
                		case -1://sobreescritura
                			if(mode == 1){fprintf(stderr,"Read overrrun error\n");}		
               		break;
               				
                		case -2://no queda nada que leer
                        
                        //evaluamos la trama
                        fD->type = evaluateFrame(fD);
                        
                        if(mode == 1)
                        {
                           printf("\n-------------------------------RECEIVED------------------------------------\n");
                                                 			                   			                 			    
                   			if(fD->type != -1)
               			   {                         
                           	SetConsoleTextAttribute(hstdoutD, FOREGROUND_GREEN); //Letra verde para trama correcta         
                           }
                           else
                           {
                              SetConsoleTextAttribute(hstdoutD, FOREGROUND_RED); //Letra roja para trama incorrecta
       
                           }
                   			for(u=0;u<fD->length;u++)
                           {
                              printData(fD->body[u]);
                              printf(" ");
                           }                                            
                           printf("\n");
                           SetConsoleTextAttribute(hstdoutD, FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_BLUE); //Letra blanca
                        }
                        else
                        {
                           //para que cuando no este en modo debug muestre los datos que recibe
                           if(fD->type == dataFrame)
                           {
                              catchData(fD);
                              printf("\nReceived: %s",fD->data);
                           }
                           
                        }
                        
                        //indicamos que hay una trama lista
                			ReleaseSemaphore(side_D_data_ready,1,NULL);
                			
                			//reiniciamos los valores de control
                			first=1;
                		   flag = 1;                                     
               		 break;
               				
                		case -3://la comunicacion se ha cortado
                			fprintf(stderr,"\nComunication offn\n");
                			conexionOK = 0;
                			fD->type = -5;
                			ExitThread(1);
               		break;
               	} 
               }  
            } 
      }       
}
              

DWORD evaluateFrame(struct Frame *f)
{
     
      int i;
      unsigned short crc;
      
      char head[5] = {DLE,SYN,DLE,SYN,DLE};
      
      for(i=0;i<5;i++)//la cabecera sera siempre la misma para todos
      {                        
          if(f->body[i] != head[i]){return -1;}
      }
     
      if(f->body[i] == ENQ){f->type = enqFrame; return enqFrame;}//ENQ
      if(f->body[i] == ACK){f->type = ackFrame; return ackFrame;}//ACK
      if(f->body[i] == EOT){f->type = eotFrame; return eotFrame;}//EOT
      
      if(f->body[i] != SOH){return -1;};//SOH
      i++;
      
      if( (f->body[i] != '0' && f->body[i] != '1'))//numero de secuencia(es un caracter, hay que tratarlo como tal)
      {
         return -1;
      }
      else
      {   
         if(f->body[i] == '0')
         {
            f->nSecuence = 0;
         }
         else
         {
            f->nSecuence = 1;
         }
      }
      i++;
      
      if(f->body[i] != DLE){return -1;};//DLE
      i++;
      
      if(f->body[i] == STX)//DATOS
      {
         i++;
            while(f->body[i] != DLE && f->body[i+1] != ETX)
            {
                  // DLE dentro de data
                  if(f->body[i] == DLE)
                  {
                     if(f->body[i+1] != DLE){return -1;}
                  }
                  i++;
                  if(i == MTU){return -1;}//no llega nunca el ETX, la trama esta mal          
            }               
          
      if(f->body[i] != DLE){return -1;}//DLE
      i++;
      
      if(f->body[i] ==  ETX)//ETX
      {
         //CRC
         catchData(f);
      
         crc =icrc(0,(unsigned char *)f->data,strlen(f->data), 255, -1);

         if(LOBYTE(crc)!=(unsigned char)f->body[i+1]){return -1;}         
                                
         if(HIBYTE(crc) !=(unsigned char)f->body[i+2]){return -1;}
         
         f->type = dataFrame;
         
         return dataFrame;
      }
      
   }   
    
   return -1;      
}


DWORD L_CONNECT_request(long source, long destination, int priority)//envia ENQ
{      
      struct Frame *f;
      int trys = 0;
      trysStadistics = 0;
      received=0;
      
      f = (struct Frame*) malloc(sizeof(Frame));
      
      if(mode == 1){printf("\nL_CONNECT_request\n");}

      while(trys != maxTrys)
      {
         //actualizamos la variable  maximo de trys intentados
         if(trysStadistics<trys) {trysStadistics=trys;}
         
         //creamos y enviamos la trama ENQ
        makeFrame(enqFrame, NULL);
         
         //esperamos hasta que haya algo para leer(quizas sea ACK o quizas no)       
         WaitForSingleObject(side_I_data_ready,timeout);
         
         //actualizamos las recibidas
         received++;
         
         switch(fI->type)
         {
            case ackFrame:
                  //actualizamos las correctas
                  correct++;       
                  return 0;
               break;
               
            default:               
                  trys++;              
               break;          
         }
      }
      //si finalmente se han agotado los intentos es que no hemos conseguido contactar cn el receptor
      return 1;     
}

DWORD L_CONNECT_indication(long source, long destination, int priority)//comprueba que le ha llegado ENQ
{ 
      do
      {  
         //esperamos a que nos indique que hay datos para leer      
         WaitForSingleObject(side_D_data_ready,timeout);
         
         //actualizamos las recibidas
         received++;
         
         switch(fD->type)
         {
            case enqFrame:
                  //actualizamos las correctas
                  correct++;
                   if(mode == 1){printf("\nL_CONNECT_indication\n");}
                  return 0;               
               break;
               
            case eotFrame:
                  //actualizamos las correctas
                  correct++;
                  return 1;              
               break;
               
            case -5://se ha cortado la comunicacion
                  return 1;
               break;
                             
            default://lo que ha llegado es incorrecto de lo que se esperaba        
                  return 2;                 
            break;    
         }
      }while(conexionOK != 1);      
}

DWORD L_CONNECT_response(long source, long destination, int priority)//envia ACK
{
      int ret;
      HANDLE c;
      
      if(mode == 1){printf("\nL_CONNECT_response\n");}
      
      //abrimos el semaforo que indicara que la conexion se ha realizado
      c = OpenSemaphore(SEMAPHORE_ALL_ACCESS,TRUE,"semaphoreConexion");
      
      //creamos y enviamos la trama ACK
      makeFrame(ackFrame,NULL);
      
      //esperamos hasta que nos confirmen la conexion
      ret = WaitForSingleObject(c,timeout);
      
      //si no se ha complido el timeout significa que hemos conseguido conectarnos
      if(ret != WAIT_TIMEOUT){conexionOK = 1;}
             
      return 0;
}
      
DWORD L_CONNECT_confirm(long source, long destination, int priority)//comprueba que le ha llegado ACK
{ 
      if(mode == 1){printf("\nL_CONNECT_confirm\n");}
      
      //indicamos que se ha realizado la conexion
      conexionOK = 1;
      
      //confirmamos al emisor que hemso conectado
      ReleaseSemaphore(conexion,1,NULL);
           
      if(mode ==1){printf("\nConnected\n");}
 
     return 0;
}

DWORD L_DATA_request(long source, long destination, char* data)//envia los datos
{
      int ret;
      int trys = 0;
      trysStadistics = 0;
      
      if(mode == 1){printf("\nL_DATA_request\n");}
      
      if(side == 0)//si es el lado izquierdo seran datos normales (EMISOR)
      {        
         makeFrame(dataFrame,data);
           
         while(trys != maxTrys && conexionOK == 1)
         {
            //actualizamos los trys
            if(trysStadistics<trys){trysStadistics=trys;}
            
            //esperamos a que haya datos listos
            ret = WaitForSingleObject(side_I_data_ready,timeout);
                      
            if(ret == WAIT_TIMEOUT)//se ha cumplido el timeout, tenemos que reenviar la trama
            {
               trys++;
               makeFrame(dataFrame,data);
            }
            else//ha llegado un dato al buffer, vemos que es
            {
               //actualizamos recibidos
              received++;
              
              switch(fI->type)
              {
                  case ackFrame:
                        //actualizamos correctas
                        correct++;
                        //nos ha llegado confirmacion de que ha llegado el dato, cmbiamos la trama esperada
                        if(nSecuenceNext == 0)
                        {
                           nSecuenceNext = 1;
                        }
                        else
                        {
                           nSecuenceNext = 0;
                        }
                  break;
                     
                  case dataFrame:
                        //actualizamos correctas
                        correct++;
                        
                        //cmbiamos el T-siguiente y salimos para llamar a indication                       
                        if(nSecuenceNext == 0)
                        {
                           nSecuenceNext = 1;
                        }
                        else
                        {
                           nSecuenceNext = 0;
                        }
                       
                        return 0;
                  break;         
               }    
            }
         }
         return 1;
      }
      else//si es el lado derecho tiene que traducirlos a morse(RECEPTOR)
      {
          
         if(data == NULL)//en esta llamada no tiene datos que enviar, solo espera por ellos
         {   
            //esperamos por los datos   
            ret = WaitForSingleObject(side_D_data_ready,timeout);
            
            //antes de hacer cualquier cosa comprobamos si seguimos conectados
            if(conexionOK == 0){return 1;}
            
            //se ha cumplido el timeout, no ha llegado nada
            if(ret == WAIT_TIMEOUT){return 3;}
            
            //actualizamos los recibidos
            received++;
            
            switch(fD->type)
            {
               case dataFrame:                
                     //actualizamos correctas
                     correct++;
                     
                     //el numero de secuencia de la trama es el mismo que el esperado, por tanto lo cambiamos
                     if(fD->nSecuence == nSecuenceWait)
                     {
                        if(nSecuenceWait == 0)
                        {
                           nSecuenceWait = 1;
                        }
                        else
                        {
                           nSecuenceWait = 0;
                        }
                        
                        return 0;                     
                     }
                     else
                     {
                        //no es el mismo, por tanto tenemos que reenviar el dato anterior
                        return 0;                 
                     }
                     
                  break;
                
                  case eotFrame:
                     //actualizamos correctas
                     correct++;
                     return 1;                     
                  break;
                     
                  default://no ha llegado nada correcto
                        return 3;
                  break;              
               }
         }
         else
         {     
                     
            if(conexionOK == 0){return 1;}//si por cualquier casual se corta la conexion
            
            //tenemos que enviar el dato en morse
            makeFrame(morseFrame,data);
            
            return 2;
         }
      }    
}


DWORD L_DATA_indication(long source, long destination)
{   
      
    if(mode == 1){printf("\nL_DATA_indication\n");}
    
   if(side == 0)//nos llega un dato correcto en el emisor, por tanto, lo obtenemos
   {
      catchData(fI);
   }
   else//nos llega un dato correcto en el receptor y por tanto tenemos que mandar ACK y obtenerlo
   {
      makeFrame(ackFrame,NULL);      
      catchData(fD);
   }
      
   return 0;
}

DWORD L_DISCONNECT_request(long source, long destination)
{    
      int ret;
      
      if(mode == 1){printf("\nL_DISCONNECT_request\n");}
      
      ret = makeFrame(eotFrame, NULL);
      
      //estadisticas
      printStadistics();
      
      //para que no se cierre la tuberia antes de enviar
      Sleep(2000);
       
      //cerramos todos los ipcs
      liberateDll();
      CloseHandle(side_I_data_ready);
      CloseHandle(conexion);
      CloseHandle(waitingI);
       
      printf("\nDisconnect\n");
           
      return 0;          
}

DWORD L_DISCONNECT_indication(long source, long destination, int reason)
{
      if(mode == 1){printf("\nL_DISCONNECT_indication\n");}
      
      //estadisticas
      printStadistics();
      
      //cerramos todos los ipcs
      liberateDll();
      CloseHandle(side_D_data_ready);
      CloseHandle(waitingD);
 
      printf("\nDisconnect\n");
               
      return 0;    
}

DWORD printData(char c)
{
        switch(c)
        {
           case 0x01:
                printf("SOH ");
                break;
           case 0x02:
                printf("STX ");
                break;
           case 0x03:
                printf("ETX ");
                break;
           case 0x04:
                printf("EOT ");
                break;
           case 0x05:
                printf("ENQ ");
                break;
           case 0x06:
                printf("ACK ");
                break;
           case 0x10:
                printf("DLE ");
                break;
           case 0x15:
                printf("NAK ");
                break;
           case 0x16:
                printf("SYN ");
                break;
           case 0x17:
                printf("ETB ");
                break;
            case '0':
               printf("0");
               break;
            case '1':
               printf("1");
               break;
           default:
                   printf("%c ",c);
                   break;
        }
}

void morse(char c, unsigned char *f)
{
   char code[8];
   
  switch(toupper(c))
  {
  // Letter Morse
  case 'A': strcpy(code, ".-");  break;
  case 'B': strcpy(code, "-..."); break;
  case 'C': strcpy(code, "-.-."); break;
  case 'D': strcpy(code, "-..");  break;
  case 'E': strcpy(code, ".");  break;
  case 'F': strcpy(code, "..-."); break;
  case 'G': strcpy(code, "--.");  break;
  case 'H': strcpy(code, "...."); break;
  case 'I': strcpy(code, "..");  break;
  case 'J': strcpy(code, "-.-."); break;
  case 'K': strcpy(code, "-.-");  break;
  case 'L': strcpy(code, ".-.."); break;
  case 'M': strcpy(code, "--");  break;
  case 'N': strcpy(code, "-.");  break;
  case 'O': strcpy(code, "---");  break;
  case 'P': strcpy(code, ".--."); break;
  case 'Q': strcpy(code, "--.-"); break;
  case 'R': strcpy(code, ".-.");  break;
  case 'S': strcpy(code, "...");  break;
  case 'T': strcpy(code, "-");  break;
  case 'U': strcpy(code, "..-");  break;
  case 'V': strcpy(code, "...-"); break;
  case 'W': strcpy(code, ".--");  break;
  case 'X': strcpy(code, "-..-"); break;
  case 'Y': strcpy(code, "-.--"); break;
  case 'Z': strcpy(code, "--.."); break;

  // Digit Morse
  case '0': strcpy(code, "-----"); break;
  case '1': strcpy(code, ".----"); break;
  case '2': strcpy(code, "..---"); break;
  case '3': strcpy(code, "...--"); break;
  case '4': strcpy(code, "....-"); break;
  case '5': strcpy(code, "....."); break;
  case '6': strcpy(code, "-...."); break;
  case '7': strcpy(code, "--..."); break;
  case '8': strcpy(code, "---.."); break;
  case '9': strcpy(code, "----."); break;

  // Others
  case '.': strcpy(code, ".-.-.-"); break;
  case ',': strcpy(code, "--..--"); break;
  case '?': strcpy(code, "..--.."); break;
  
}
strcpy(f,code);
}

DWORD clearFrame(struct Frame* f)
{
 int i;

   for(i=0;i<MTU;i++)
   {
        f->body[i]= ' ';
        f->data[i] = ' ';                                
   }
   
    f->length =0; 
    f->type = 0;        
}

void catchData(struct Frame* f)
{
     int i= 0,j=0;
         while(f->body[i] != STX)
         {
           i++;                                    
         }
         i++;               
         
         while(f->body[i] != DLE && f->body[i+1] != ETX)
         {
             f->data[j] = f->body[i];
    
             i++;
             j++;                                
         }
         f->data[j] = '\0';        
}

void printStadistics(void) 
{  
   printf("\n");
	printf("Frames received:   %d\n", received);
	printf("Correct Frames received:    %d\n", correct);
	printf("Incorrect Frames received         %d\n", (received-correct));
	printf("Maximun number of trys: %d\n", trysStadistics);
	printf("\n");
}
