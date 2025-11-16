/*************************************************************************************
 * Pontificia Universidad Javeriana                                                                          *
 * Fecha:
 * Autores: Daniel Felipe Ramirez Vargas - Guillermo Andrés Aponte Cárdenas - David Tobar Artunduaga         *
 * Materia: Sistemas Operativos                                                                              *
 * Descripción:
 *************************************************************************************/

#include "ModulosDeDefinicion/ModuloControlador.h" //Se incluye el modulo que contiene las declaraciones de las funciones y estructuras

int agentesTotalesRegistrados = 0;

int main(int argc, char *argv[]) {
  pthread_t hilos[10];
  RetornoArgumentos argumentos;
  Reloj *reloj = malloc(sizeof(Reloj));
  Parque parques[13];
  void *retorno;
  sem_t *fd_sem = sem_open("/terminarAgentes", O_CREAT, 0660, 0);
  if (fd_sem == SEM_FAILED) {
    perror("Creando Semáforo");
    return -1;
  }

  Paquete *paquete;
  int err;

  argumentos = tomarArgumentosControlador(argc, argv);
  if (argumentos.retorno == -1) {
    return -1;
  }
  reloj->segHoras = argumentos.segHoras;
  reloj->horaIni = argumentos.horaIni;
  reloj->horaFin = argumentos.horaFin;
  inicializarParques(argumentos, parques);
  err = pthread_create(hilos, NULL, manipularReloj, reloj);
  if (err != 0) {
    printf("Error creando hilo: %s\n", strerror(err));
    return -1;
  }
  err = pthread_create(hilos + 1, NULL, reportePorHora, parques);
  if (err != 0) {
    printf("Error creando hilo: %s\n", strerror(err));
    return -1;
  }

  /*Empaquetado*/
  paquete = malloc(sizeof(Paquete));
  paquete->reloj = reloj;
  paquete->argumentos = argumentos;
  paquete->parques = parques;

  err = pthread_create(hilos + 2, NULL, recibirMensajes, paquete);
  if (err != 0) {
    printf("Error creando hilo: %s\n", strerror(err));
    return -1;
  }

  pthread_join(hilos[0], NULL);
  pthread_join(hilos[1], NULL);

  if (reporteFinal(parques, argumentos) == -1) {
    return -1;
  }

  pthread_join(hilos[2], &retorno);
  if ((long)retorno == -1) { // int no porque long es del mismo tamaño que void*
    return -1;
  }

  for (int i = 0; i < agentesTotalesRegistrados; i++) {
    if (sem_post(fd_sem) == -1) {
      perror("sem_post");
      return -1;
    }
  }
  free(reloj);
  free(paquete);
  free(retorno);

  return 0;
}