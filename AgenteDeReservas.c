/*************************************************************************************
 * Pontificia Universidad Javeriana                                                                            *
 * Fecha:
 * Autores: Daniel Felipe Ramirez Vargas - Guillermo Andrés Aponte Cárdenas - David Tobar Artunduaga           *
 * Materia: Sistemas Operativos                                                                                *
 * Descripción:
 *************************************************************************************/

#include "ModulosDeDefinicion/ModuloAgente.h" //Se incluye el modulo que contiene las declaraciones de las funciones y estructuras

int main(int argc, char *argv[]) {

  sem_t *fd_sem = sem_open("/terminarAgentes", 0);
  if (fd_sem == SEM_FAILED) {
    perror("Abriendo el semaforo");
    return -1;
  }

  RetornoAgentes argumentos = tomarArgumentosAgente(argc, argv);
  if (argumentos.retorno == -1) {
    return -1;
  }
  if (leerArchivo(argumentos) == -1) {
    return -1;
  }

  sem_wait(fd_sem);
  printf("AGENTE %s TERMINA.\n", argumentos.nombre);
  free(argumentos.nombre);
  free(argumentos.fileSolicitud);
  free(argumentos.pipeRecibe);
  return 0;
}