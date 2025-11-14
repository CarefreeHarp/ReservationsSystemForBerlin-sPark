  /*Se verifica si el identificador __MODULO_CONTROLADOR_H__ ha sido definido antes.
 Sirve para evitar que el contenido de este archivo se incluya varias veces en un mismo programa.
 Si ya estuviera definido, el compilador se saltaría todo el contenido hasta el #endif.*/
#ifndef __MODULO_CONTROLADOR_H__

 /*Se define el identificador __MODULO_CONTROLADOR_H__
De tal forma se marca que el archivo ya fue incluido una vez, evitando duplicaciones posteriores.
En este espacio se colocan las declaraciones, estructuras, constantes o prototipos de funciones
que se necesitan compartir entre varios archivos .c del proyecto.*/
#define __MODULO_CONTROLADOR_H__

#include <stdlib.h> // Funciones generales de utilidad (exit, malloc)
#include <stdio.h> // Entrada y salida estándar (printf, fgets) 
#include <unistd.h> // Funciones de manejo de procesos y pipes (read, write, close) 
#include <sys/types.h> // Definición de tipos de datos (pid_t, etc.) 
#include <string.h> // Manejo de cadenas (strcpy, strcmp, strlen) 
#include <sys/stat.h> // Constantes y estructuras para manejo de archivos 
#include <fcntl.h> // Control de apertura de archivos 
#include <stdbool.h> // Tipo de dato booleano (true, false)
#include <pthread.h> 

extern pthread_mutex_t reportePorHoraM;
extern pthread_cond_t condiReportePorHora;
extern bool terminado; 
extern bool notificar;

typedef struct {
    int horaIni;
    int horaFin;
    int segHoras;
    int total;
    char* pipeRecibe;
    int retorno;
} RetornoArgumentos;

typedef struct {
    int horaIni;
    int segHoras;
    int horaActual;
    int horaFin;
} Reloj;

typedef struct{
    int cantPersonas;
    int horaLlegada;
    char* nombre;
} Familia;

typedef struct {
    int cantFamilias;
    Familia* familias;
    int cuantasSalen;
    int cuantasEntran;
    int aforoMaximo;
    int hora;
} Parque;


RetornoArgumentos tomarArgumentosControlador(int argc, char *argv[]);
void* manipularReloj(void* recibe);
void* recibirSolicitudes(void* parametro);
void* reportePorHora(void* parques);

#endif
