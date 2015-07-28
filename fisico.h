/*
 ** Fichero: fisico.h
 ** Autores:
 ** Alvaro Valiente Herrero
 ** Diego Sanchez Moreno
 ** Usuario: i8112600
 */

// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the TUBODLL_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// TUBODLL_API functions as being imported from a DLL, wheras this DLL sees symbols
// defined with this macro as being exported.
#ifdef TUBODLL_EXPORTS
#define TUBODLL_API __declspec(dllexport)
#else
#define TUBODLL_API __declspec(dllimport)
#endif

typedef DWORD  (*TIPO_F_PUEDO_ESCRIBIR)(void);
typedef DWORD  (*TIPO_F_ESPERAR_ESCRIBIR)(DWORD);
typedef DWORD (*TIPO_F_ESCRIBIR)(char);
typedef DWORD (*TIPO_F_PUEDO_LEER)(void);
typedef DWORD (*TIPO_F_ESPERAR_LEER)(DWORD);
typedef DWORD (*TIPO_F_LEER)(char *);
typedef BOOL  (*TIPO_F_INICIO_PUERTO)(char, char*, char*,DWORD, void (*)(void),DWORD);
