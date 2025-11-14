#include "../ModulosDeDefinicion/ModuloControlador.h" //Se incluye el modulo que contiene las declaraciones de las funciones y estructuras


pthread_mutex_t reportePorHoraM = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t condiReportePorHora = PTHREAD_COND_INITIALIZER;
bool terminado = false; 
bool notificar = false;

RetornoArgumentos tomarArgumentosControlador(int argc, char *argv[]) {
  RetornoArgumentos retorno;
  retorno.retorno = 0;

  if (argc < 11) {
    printf("Error: Número insuficiente de argumentos.\n");
    retorno.retorno = -1;
    return retorno;
  }

  bool argumentos[5] = {false, false, false, false, false};
  int posiciones[5] = {-1, -1, -1, -1, -1};

  for (int i = 0; i < 10; i++) {
    if (strcmp(argv[i], "-i") == 0) {
      argumentos[0] = true;
      posiciones[0] = i;
    } else if (strcmp(argv[i], "-f") == 0) {
      argumentos[1] = true;
      posiciones[1] = i;
    } else if (strcmp(argv[i], "-s") == 0) {
      argumentos[2] = true;
      posiciones[2] = i;
    } else if (strcmp(argv[i], "-t") == 0) {
      argumentos[3] = true;
      posiciones[3] = i;
    } else if (strcmp(argv[i], "-p") == 0) {
      argumentos[4] = true;
      posiciones[4] = i;
    }
  }
  for (int i = 0; i < 5; i++) {
    if (!argumentos[i]) {
      printf("Error: Falta el argumento.\n");
      retorno.retorno = -1;
      return retorno;
    }
  }

  retorno.horaIni = atoi(argv[posiciones[0] + 1]);
  retorno.horaFin = atoi(argv[posiciones[1] + 1]);
  retorno.segHoras = atoi(argv[posiciones[2] + 1]);
  retorno.total = atoi(argv[posiciones[3] + 1]);

  char *pipeRecibe = argv[posiciones[4] + 1];

  if (retorno.horaIni < 7 || retorno.horaIni > 19) {
    printf("Error: La hora de inicio debe estar entre 7 y 19.\n");
    retorno.retorno = -1;
    return retorno;
  }

  if (retorno.horaFin < 7 || retorno.horaFin > 19 || retorno.horaFin < retorno.horaIni) {
    printf("Error: La hora de fin debe estar entre 7 y 19.\n");
    retorno.retorno = -1;
    return retorno;
  }

  retorno.pipeRecibe = pipeRecibe;

  return retorno;
}

void *manipularReloj(void *recibe) {
  Reloj *reloj = (Reloj *)recibe;
  reloj->horaActual = reloj->horaIni;
  while (reloj->horaActual < reloj->horaFin) {
    printf("La hora acual es %d:00\n", reloj->horaActual);
    pthread_mutex_lock(&reportePorHoraM);
    notificar = true;
    pthread_cond_signal(&condiReportePorHora);
    pthread_mutex_unlock(&reportePorHoraM);
    sleep(reloj->segHoras);
    reloj->horaActual++;
  }
  printf("La hora acual es %d:00\n", reloj->horaActual);
  return NULL;
}



void *recibirSolicitudes(void *parametro) {
  char nombreNamedPipe[256];
  strcpy(nombreNamedPipe, "/tmp/");
  strcat(nombreNamedPipe, (char *)parametro);

  mkfifo(nombreNamedPipe, 0640);
  int fdread = open(nombreNamedPipe, O_RDONLY);

  if (fdread < 0) {
    perror("PipeRecibe: ");
    return (void *)-1;
  }

  return NULL;

  close(fdread);
}



void *reportePorHora(void *parques) {
  Parque **parquesActuales = (Parque **)parques;
  int i = 0;
  bool resultados = false;
  while (!terminado) {
    pthread_mutex_lock(&reportePorHoraM);
    while (!notificar) {
      pthread_cond_wait(&condiReportePorHora, &reportePorHoraM);
    }
    printf("La cantidad de personas que entran es %d\n", ((*parquesActuales) + i)->cuantasEntran);
    while ((*(parquesActuales + i))->hora != -1) {
      if ((*(parquesActuales + i))->familias->horaLlegada = (*(parquesActuales + i))->hora) {
        if(!resultados){
            printf("Las familias que entran son:\n");
        }
        printf("- %s\n", (*(parquesActuales + i))->familias->nombre);
        resultados = true;
      }
    }
    if(resultados == false){
        printf("No hay familias entrando");
    }
    resultados = false;


    printf("La cantidad de personas que salen es %d", ((*parquesActuales) + i)->cuantasSalen);
    while ((*(parquesActuales + i))->hora != -1) {
      if ((*(parquesActuales + i))->familias->horaLlegada = (*(parquesActuales + i))->hora - 2) {
        if(!resultados){
            printf("Las familias que salen son:\n");
        }
        printf("- %s\n", (*(parquesActuales + i))->familias->nombre);
      }
    }
    if(resultados == false){
        printf("No hay familias saliendo");
    }
    pthread_mutex_unlock(&reportePorHoraM);
    notificar = false;
    i++;
  }
  return NULL;
}



void inicializarParques(RetornoArgumentos argumentos, Parque parques[]) {
  int cantidadDeParques = argumentos.horaFin - argumentos.horaIni;
  for (int i = 0; i < 12; i++) {
    parques[i].hora = -1;
  }
  for (int i = 0; i < cantidadDeParques; i++) {
    parques[i].hora = i + argumentos.horaIni;
  }
}
