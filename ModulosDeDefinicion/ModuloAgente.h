/*Se verifica si el identificador __MODULO_AGENTES_H__ ha sido definido antes.
Sirve para evitar que el contenido de este archivo se incluya varias veces en un mismo programa.
Si ya estuviera definido, el compilador se saltaría todo el contenido hasta el #endif.*/
#ifndef __MODULO_AGENTES_H__

/*Se define el identificador __MODULO_AGENTES_H__
De tal forma se marca que el archivo ya fue incluido una vez, evitando duplicaciones posteriores.
En este espacio se colocan las declaraciones, estructuras, constantes o prototipos de funciones
que se necesitan compartir entre varios archivos .c del proyecto.*/
#define __MODULO_AGENTES_H__

#include <fcntl.h> // Control de apertura de archivos
#include <pthread.h>
#include <stdbool.h>   // Tipo de dato booleano (true, false)
#include <stdio.h>     // Entrada y salida estándar (printf, fgets)
#include <stdlib.h>    // Funciones generales de utilidad (exit, malloc)
#include <string.h>    // Manejo de cadenas (strcpy, strcmp, strlen)
#include <sys/stat.h>  // Constantes y estructuras para manejo de archivos
#include <sys/types.h> // Definición de tipos de datos (pid_t, etc.)
#include <unistd.h>    // Funciones de manejo de procesos y pipes (read, write, close)
#include <semaphore.h>

typedef struct {
  char *nombre;
  char *fileSolicitud;
  char *pipeRecibe;
  int retorno;
} RetornoAgentes;

typedef struct {
  bool reserva;
  char nombreAgente[256];
  int horaSolicitada;
  int cantPersonas;
  char respuesta[256];
  char nombreFamilia[256];
} Peticion;

RetornoAgentes tomarArgumentosAgente(int argc, char *argv[]);
int registroControlador(RetornoAgentes argumentos);
int leerArchivo(RetornoAgentes argumentos);
#endif