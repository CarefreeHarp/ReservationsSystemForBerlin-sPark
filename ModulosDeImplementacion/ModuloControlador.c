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
  terminado = true;
  printf("La hora acual es %d:00\n", reloj->horaActual);
  pthread_mutex_lock(&reportePorHoraM);
  notificar = true;
  pthread_cond_signal(&condiReportePorHora);
  pthread_mutex_unlock(&reportePorHoraM);
  // Reporte final
  return NULL;
}

void *recibirMensajes(void *paquete) {
  Paquete *Parametro = (Paquete *)paquete;

  char nombreNamedPipe[256];
  strcpy(nombreNamedPipe, "/tmp/");
  strcat(nombreNamedPipe, Parametro->argumentos.pipeRecibe);
  Peticion *peticion = malloc(sizeof(Peticion));
  Peticion *respuesta = malloc(sizeof(Peticion));
  bool admitido = true;
  int error;

  mkfifo(nombreNamedPipe, 0640);
  int fdread = open(nombreNamedPipe, O_RDONLY);

  if (fdread < 0) {
    perror("PipeRecibe: ");
    return (void *)-1;
  }

  while (!terminado) {
    error = read(fdread, peticion, sizeof(Peticion));
    if (error == -1) {
      perror("Error leyendo en el pipe principal: ");
      exit(EXIT_FAILURE);
    }
    if (peticion->reserva) {
      respuesta->cantPersonas = peticion->cantPersonas;
      respuesta->horaSolicitada = peticion->horaSolicitada;
      strcpy(respuesta->nombreAgente, peticion->nombreAgente);

      if (peticion->cantPersonas < Parametro->argumentos.total) {
        if (peticion->horaSolicitada < Parametro->argumentos.horaFin - 1) {
          int a = Parametro->argumentos.horaIni;
          int b = peticion->horaSolicitada;
          int c = b - a;
          Parque *aux = Parametro->parques;
          int personasEnElParque = (aux + c)->cantPersonas + peticion->cantPersonas;
          if (personasEnElParque < Parametro->argumentos.total && peticion->horaSolicitada >= Parametro->reloj->horaActual) {
            strcpy(respuesta->respuesta, "RESERVA OK: TODO BIEN\n");
            // La hora solicitada sirve
          } else {
            for (int i = c; i < Parametro->argumentos.horaFin - Parametro->argumentos.horaIni - 1; i++) {
              personasEnElParque = (aux + i)->cantPersonas + peticion->cantPersonas;
              if (personasEnElParque < Parametro->argumentos.total && peticion->horaSolicitada + i > Parametro->reloj->horaActual) {
                strcpy(respuesta->respuesta, "RESERVA OK: REPROGRAMADA PARA EL MISMO DIA\n");
                // hay una hora mas adelante que sirve
                admitido = true;
              }
            }
            if (admitido == false) {
              strcpy(respuesta->respuesta, "RESERVA NEGADA: NO HAY DISPONIBILIDAD PARA ESA HORA NI PARA EL RESTO DEL DIA");
              // No se encontraron parques posteriores que sirvan
            }
          }
        } else {
          strcpy(respuesta->respuesta, "RESERVA NEGADA: NO HAY BLOQUE DE DOS HORAS DISPONIBLE (PARQUE CERRADO)");
          // La hora solicitada es mayor a la maxima
        }
      } else {
        strcpy(respuesta->respuesta, "RESERVA NEGADA: SU FAMILIA EXCEDE EL AFORO MÁXIMO DEL PARQUE");
        // Negada en cualquier caso
      }

      admitido = false;
      if (!terminado) {
        // responde;
        strcpy(nombreNamedPipe, "/tmp/");
        strcat(nombreNamedPipe, respuesta->nombreAgente);
        int fdwrite = open(nombreNamedPipe, O_WRONLY);
        error = write(fdwrite, respuesta, sizeof(Peticion));
        if (error == -1) {
          perror("Error escribiendo en el pipe principal: ");
          exit(EXIT_FAILURE);
        }
        close(fdwrite);
      }
    }
    printf("ENCONTRE UN AGENTE, SE LLAMA: %s\n",peticion->nombreAgente);
    /*
    


    */
  }
  close(fdread);
  strcpy(nombreNamedPipe, "/tmp/");
  strcat(nombreNamedPipe, Parametro->argumentos.pipeRecibe);
  unlink(nombreNamedPipe);
  return NULL;
}

void *reportePorHora(void *parques) {
  Parque *parquesActuales = (Parque *)parques;
  Familia *aux;
  int i = 0;
  bool resultados = false;
  while (true) {
    pthread_mutex_lock(&reportePorHoraM);
    while (!notificar) {
      pthread_cond_wait(&condiReportePorHora, &reportePorHoraM);
      if (terminado == true) {
        break;
      }
    }
    if (terminado == true) {
      pthread_mutex_unlock(&reportePorHoraM);
      break;
    }
    printf("La cantidad de personas que entran es %d\n", (parquesActuales + i)->cuantasEntran);
    if ((parquesActuales + i)->hora != -1) {
      if ((parquesActuales + i)->familias != NULL) {
        aux = (parquesActuales + i)->familias;
        for (int j = 0; j < (parquesActuales + i)->cantFamilias; j++, aux++) {
          if (aux->horaLlegada == (parquesActuales + i)->hora) {
            if (!resultados) {
              printf("Las familias que entran son:\n");
            }
            printf("- %s\n", aux->nombre);
            resultados = true;
          }
        }
      }
    }
    if (resultados == false) {
      printf("No hay familias entrando\n");
    }
    resultados = false;

    printf("La cantidad de personas que salen es %d\n", (parquesActuales + i)->cuantasSalen);
    if ((parquesActuales + i)->hora != -1) {
      if ((parquesActuales + i)->familias != NULL) {
        aux = (parquesActuales + i)->familias;
        for (int j = 0; j < (parquesActuales + i)->cantFamilias; j++, aux++) {
          if (aux->horaLlegada == (parquesActuales + i)->hora - 2) {
            if (!resultados) {
              printf("Las familias que salen son:\n");
            }
            printf("- %s\n", aux->nombre);
          }
        }
      }
    }
    if (resultados == false) {
      printf("No hay familias saliendo\n\n");
    }
    notificar = false;
    i++;
    pthread_mutex_unlock(&reportePorHoraM);
  }
  return NULL;
}

void inicializarParques(RetornoArgumentos argumentos, Parque parques[]) {
  int cantidadDeParques = argumentos.horaFin - argumentos.horaIni;
  for (int i = 0; i < 13; i++) {
    parques[i].hora = -1;
  }
  for (int i = 0; i < cantidadDeParques; i++) {
    parques[i].hora = i + argumentos.horaIni;
    parques[i].aforoMaximo = i + argumentos.total;
    parques[i].cantFamilias = 0;
    parques[i].cuantasEntran = 0;
    parques[i].cuantasSalen = 0;
    parques[i].cantPersonas = 0;
    parques[i].familias = NULL;
  }
}
