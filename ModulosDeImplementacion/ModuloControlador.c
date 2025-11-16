#include "../ModulosDeDefinicion/ModuloControlador.h" //Se incluye el modulo que contiene las declaraciones de las funciones y estructuras

pthread_mutex_t reportePorHoraM = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t condiReportePorHora = PTHREAD_COND_INITIALIZER;
bool terminado = false;
bool notificar = false;
int solicitudesNegadas = 0;
int solicitudesAceptadas = 0;
int solicitudesReProgramadas = 0;

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

int reporteFinal(Parque *parques, RetornoArgumentos argumentos) {
  int mayorCantidad = parques->cantPersonas;
  int horasPico[12];
  horasPico[0] = argumentos.horaIni;
  int j = 1;

  int menorCantidad = parques->cantPersonas;
  int horasMenosPico[12];
  horasMenosPico[0] = argumentos.horaIni;
  int k = 1;

  for (int i = 1; i < argumentos.horaFin - argumentos.horaIni; i++) {
    if ((parques + i)->cantPersonas > mayorCantidad) {
      j = 1;
      horasPico[0] = argumentos.horaIni + i;
      mayorCantidad = (parques + i)->cantPersonas;
    } else if ((parques + i)->cantPersonas < menorCantidad) {
      k = 1;
      horasMenosPico[0] = argumentos.horaIni + i;
      menorCantidad = (parques + i)->cantPersonas;
    } else if ((parques + i)->cantPersonas == mayorCantidad) {
      j++;
      horasPico[j - 1] = argumentos.horaIni + i;
    } else if ((parques + i)->cantPersonas == menorCantidad) {
      k++;
      horasMenosPico[k - 1] = argumentos.horaIni + i;
    }
  }
  printf("====================================================\n");
  printf("        REPORTE FINAL DEL DIA DE HOY          \n");
  printf(" CANTIDAD DE SOLICITUDES NEGADAS:         %d  \n", solicitudesNegadas);
  printf(" CANTIDAD DE SOLICITUDES ACEPTADAS:       %d  \n", solicitudesAceptadas);
  printf(" CANTIDAD DE SOLICITUDES REPROGRAMADAS:   %d  \n", solicitudesReProgramadas);
  printf(" HORAS PICO:");
  for (int i = 0; i < j; i++) {
    if (i != j - 1) {
      printf(" %d -", horasPico[i]);
    } else {
      printf(" %d\n", horasPico[i]);
    }
  }
  printf(" HORA MENOS CONCURRIDA:");
  for (int i = 0; i < k; i++) {
    if (i != k - 1) {
      printf(" %d -", horasMenosPico[i]);
    } else {
      printf(" %d\n", horasMenosPico[i]);
    }
  }
  printf("====================================================\n");
  char nombreNamedPipe[256];
  strcpy(nombreNamedPipe, "/tmp/");
  strcat(nombreNamedPipe, argumentos.pipeRecibe);
  int fd_write = open(nombreNamedPipe, O_RDWR);

  if (fd_write < 0) {
    perror("PipeRecibe (FINAL): ");
    return -1;
  }
  Peticion *terminarEjecucion = malloc(sizeof(Peticion));
  write(fd_write, terminarEjecucion, sizeof(Peticion));

  close(fd_write);
}

void *manipularReloj(void *recibe) {
  Reloj *reloj = (Reloj *)recibe;
  reloj->horaActual = reloj->horaIni;
  while (reloj->horaActual < reloj->horaFin) {
    printf("==============================================\n");
    printf("La hora acual es %d:00\n", reloj->horaActual);
    pthread_mutex_lock(&reportePorHoraM);
    notificar = true;
    pthread_cond_signal(&condiReportePorHora);
    pthread_mutex_unlock(&reportePorHoraM);
    sleep(reloj->segHoras);
    reloj->horaActual++;
  }
  terminado = true;
  printf("\n\nLA HORA ACUTAL ES %d:00 Y SE CIERRA EL PARQUE ~~~\n", reloj->horaActual);
  pthread_mutex_lock(&reportePorHoraM);
  notificar = true;
  pthread_cond_signal(&condiReportePorHora);
  pthread_mutex_unlock(&reportePorHoraM);

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
  int fdread = open(nombreNamedPipe, O_RDWR);

  if (fdread < 0) {
    perror("PipeRecibe: ");
    return (void *)-1;
  }

  while (!terminado) {
    error = read(fdread, peticion, sizeof(Peticion));
    if (terminado) {
      break;
    }
    pthread_mutex_lock(&reportePorHoraM);
    if (error == -1) {
      perror("Error leyendo en el pipe principal: ");
      exit(EXIT_FAILURE);
    }
    if (peticion->reserva) {
      respuesta->cantPersonas = peticion->cantPersonas;
      respuesta->horaSolicitada = peticion->horaSolicitada;
      strcpy(respuesta->nombreFamilia, peticion->nombreFamilia);
      strcpy(respuesta->nombreAgente, peticion->nombreAgente);

      if (peticion->cantPersonas <= Parametro->argumentos.total) {
        if (peticion->horaSolicitada < Parametro->argumentos.horaFin - 1) {
          int c;
          if (peticion->horaSolicitada > Parametro->argumentos.horaIni) {
            int a = Parametro->argumentos.horaIni;
            int b = peticion->horaSolicitada;
            c = b - a;
          } else {
            c = 0;
          }
          Parque *aux = Parametro->parques;
          int personasEnElParque = (aux + c)->cantPersonas + peticion->cantPersonas;
          if (personasEnElParque <= Parametro->argumentos.total && peticion->horaSolicitada >= Parametro->reloj->horaActual) {
            int numeroFamilia = (aux + c)->cantFamilias;
            Familia *aux2 = (aux + c)->familias;
            aux2 += numeroFamilia;

            aux2->cantPersonas = peticion->cantPersonas;
            aux2->horaLlegada = Parametro->argumentos.horaIni + c;
            strcpy(aux2->nombre, peticion->nombreFamilia);

            numeroFamilia = (aux + c + 1)->cantFamilias;
            aux2 = (aux + c + 1)->familias;
            aux2 += numeroFamilia;

            aux2->cantPersonas = peticion->cantPersonas;
            aux2->horaLlegada = Parametro->argumentos.horaIni + c;
            strcpy(aux2->nombre, peticion->nombreFamilia);

            (aux + c)->cantFamilias++;
            (aux + c + 1)->cantFamilias++;
            (aux + c)->cantPersonas = personasEnElParque;
            (aux + c + 1)->cantPersonas += peticion->cantPersonas;
            (aux + c)->cuantasEntran += peticion->cantPersonas;
            (aux + c + 2)->cuantasSalen += peticion->cantPersonas;
            strcpy(respuesta->respuesta, "RESERVA OK: TODO BIEN\n");
            solicitudesAceptadas++;
            // La hora solicitada sirve
          } else {
            for (int i = c; i < Parametro->argumentos.horaFin - Parametro->argumentos.horaIni - 1; i++) {
              personasEnElParque = (aux + i)->cantPersonas + peticion->cantPersonas;
              if (personasEnElParque <= Parametro->argumentos.total && (aux + i)->hora >= Parametro->reloj->horaActual) {
                int numeroFamilia = (aux + i)->cantFamilias;
                Familia *aux2 = (aux + i)->familias;
                aux2 += numeroFamilia;

                aux2->cantPersonas = peticion->cantPersonas;
                aux2->horaLlegada = Parametro->argumentos.horaIni + i;
                strcpy(aux2->nombre, peticion->nombreFamilia);

                numeroFamilia = (aux + i + 1)->cantFamilias;
                aux2 = (aux + i + 1)->familias;
                aux2 += numeroFamilia;

                aux2->cantPersonas = peticion->cantPersonas;
                aux2->horaLlegada = Parametro->argumentos.horaIni + i;
                strcpy(aux2->nombre, peticion->nombreFamilia);

                (aux + i)->cantFamilias++;
                (aux + i + 1)->cantFamilias++;
                (aux + i)->cantPersonas = personasEnElParque;
                (aux + i + 1)->cantPersonas += peticion->cantPersonas;
                (aux + i)->cuantasEntran += peticion->cantPersonas;
                (aux + i + 2)->cuantasSalen += peticion->cantPersonas;

                strcpy(respuesta->respuesta, "RESERVA REPROGRAMADA: PARA EL MISMO DIA A LAS ");
                solicitudesReProgramadas++;
                int horaNueva = (aux + i)->hora;
                char horaNuevaChar[256];
                sprintf(horaNuevaChar, "%d", horaNueva);
                strcat(respuesta->respuesta, horaNuevaChar);
                strcat(respuesta->respuesta, ":00\n");
                // hay una hora mas adelante que sirve
                admitido = true;
                break;
              }
            }
            if (admitido == false) {
              strcpy(respuesta->respuesta, "RESERVA NEGADA: NO HAY DISPONIBILIDAD PARA ESA HORA NI PARA EL RESTO DEL DIA");
              solicitudesNegadas++;
              // No se encontraron parques posteriores que sirvan
            }
          }
        } else {
          strcpy(respuesta->respuesta, "RESERVA NEGADA: NO HAY BLOQUE DE DOS HORAS DISPONIBLE (PARQUE CERRADO)");
          solicitudesNegadas++;
          // La hora solicitada es mayor a la maxima
        }
      } else {
        strcpy(respuesta->respuesta, "RESERVA NEGADA: SU FAMILIA EXCEDE EL AFORO MÁXIMO DEL PARQUE");
        solicitudesNegadas++;
        // Negada en cualquier caso
      }

      admitido = false;
      if (!terminado) {
        // responde;
        strcpy(nombreNamedPipe, "/tmp/");
        strcat(nombreNamedPipe, respuesta->nombreAgente);
        int fdwrite = open(nombreNamedPipe, O_RDWR);
        error = write(fdwrite, respuesta, sizeof(Peticion));
        if (error == -1) {
          perror("Error escribiendo en el pipe principal: ");
          exit(EXIT_FAILURE);
        }
        close(fdwrite);
      }
    } else {
      printf("UN AGENTE SE REGISTRÓ, SE LLAMA: %s\n", peticion->nombreAgente);
      agentesTotalesRegistrados++;
      strcpy(nombreNamedPipe, "/tmp/");
      strcat(nombreNamedPipe, peticion->nombreAgente);
      int fdwrite = open(nombreNamedPipe, O_RDWR);
      int *horaEnviar = &Parametro->reloj->horaActual;
      error = write(fdwrite, horaEnviar, sizeof(int));
      if (error == -1) {
        perror("Error escribiendo en el pipe principal: ");
        exit(EXIT_FAILURE);
      }
      close(fdwrite);
    }
    pthread_mutex_unlock(&reportePorHoraM);
  }
  close(fdread);
  strcpy(nombreNamedPipe, "/tmp/");
  strcat(nombreNamedPipe, Parametro->argumentos.pipeRecibe);
  unlink(nombreNamedPipe);
  return NULL;
}

void *reportePorHora(void *parques) {
  Parque *parquesActuales = (Parque *)parques;
  Familia *aux = malloc(sizeof(Familia));
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
    if (resultados == false) {
      printf("No hay familias entrando\n");
    }
    resultados = false;

    printf("La cantidad de personas que salen es %d\n", (parquesActuales + i)->cuantasSalen);
    if ((parquesActuales + i)->hora != -1) {
      if (i > 1) {
        aux = (parquesActuales + i - 1)->familias;
        for (int j = 0; j < (parquesActuales + i - 1)->cantFamilias; j++, aux++) {
          if (aux->horaLlegada == (parquesActuales + i)->hora - 2) {
            if (!resultados) {
              printf("Las familias que salen son:\n");
            }
            printf("- %s\n", aux->nombre);
            resultados = true;
          }
        }
      }
    }
    if (resultados == false) {
      printf("No hay familias saliendo\n");
      printf("==============================================\n\n\n");
    } else {
      printf("==============================================\n\n\n");
    }
    resultados = false;
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
    parques[i].familias = malloc(argumentos.total * sizeof(Familia));
  }
}
