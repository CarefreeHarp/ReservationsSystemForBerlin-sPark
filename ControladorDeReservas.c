/*************************************************************************************
 * Pontificia Universidad Javeriana                                                  *
 * Fecha:
 * Autores: Daniel Felipe Ramirez Vargas - Guillermo Andrés Aponte Cárdenas          *
 * Materia: Sistemas Operativos                                                      *
 * Descripción: 
*************************************************************************************/

#include "ModulosDeDefinicion/ModuloControlador.h" //Se incluye el modulo que contiene las declaraciones de las funciones y estructuras


int main(int argc, char *argv[]){
    pthread_t hilos[10];
    RetornoArgumentos argumentos;
    Reloj* reloj = malloc(sizeof(Reloj));
    Parque parques[12];

    argumentos = tomarArgumentosControlador(argc, argv);
    if(argumentos.retorno == -1){
        return -1;
    }
    reloj->segHoras = argumentos.segHoras;
    reloj->horaIni = argumentos.horaIni;
    reloj->horaFin = argumentos.horaFin;
    pthread_create(hilos, NULL, manipularReloj, reloj);

    pthread_create(hilos+2, NULL, recibirSolicitudes, argumentos.pipeRecibe);
    

    void* retornoPipe;
    pthread_join(hilos[2], &retornoPipe);
    if((long)retornoPipe == -1){ //int no porque long es del mismo tamaño que void*
        return -1;
    }
    pthread_join(hilos[0], NULL);


    return 0;
}