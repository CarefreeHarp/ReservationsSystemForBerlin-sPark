#include "../ModulosDeDefinicion/ModuloAgente.h" //Se incluye el modulo que contiene las declaraciones de las funciones y estructuras

RetornoAgentes tomarArgumentosAgente(int argc, char *argv[]) {

  RetornoAgentes retorno;
  retorno.nombre = malloc(256);
  retorno.fileSolicitud = malloc(256);
  retorno.pipeRecibe = malloc(256);

  if (argc < 7) {
    printf("Error: Número insuficiente de argumentos.\n");
    retorno.retorno = -1;
    return retorno;
  }

  bool argumentos[3] = {false, false, false};
  int posiciones[3] = {-1, -1, -1};
  for (int i = 0; i < 6; i++) {
    if (strcmp(argv[i], "-s") == 0) {
      argumentos[0] = true;
      posiciones[0] = i;
    } else if (strcmp(argv[i], "-a") == 0) {
      argumentos[1] = true;
      posiciones[1] = i;
    } else if (strcmp(argv[i], "-p") == 0) {
      argumentos[2] = true;
      posiciones[2] = i;
    }
  }

  for (int i = 0; i < 3; i++) {
    if (!argumentos[i]) {
      printf("Error: Falta el argumento\n");
      retorno.retorno = -1;
      return retorno;
    }
  }

  strcpy(retorno.nombre, argv[posiciones[0] + 1]);
  strcpy(retorno.fileSolicitud, argv[posiciones[1] + 1]);
  strcpy(retorno.pipeRecibe, argv[posiciones[2] + 1]);
  retorno.retorno = 0;

  return retorno;
}

int leerArchivo(RetornoAgentes argumentos) {
  int fd_write, fd_read, error, horaActual;
  int *horaRecibida = malloc(sizeof(int));
  char nombreNamedPipe[256];
  char pipeRecibe[256];
  char bufferArchivo[256];
  char bufferAux[256];
  Peticion *saludoInicial = malloc(sizeof(Peticion));
  Peticion *solicitudReserva = malloc(sizeof(Peticion));
  Peticion *respuesta = malloc(sizeof(Peticion));
  strcpy(nombreNamedPipe, "/tmp/");
  strcat(nombreNamedPipe, argumentos.pipeRecibe);

  strcpy(pipeRecibe, "/tmp/");
  strcat(pipeRecibe, argumentos.nombre);

  fd_write = open(nombreNamedPipe, O_RDWR);
  if (fd_write < 0) {
    perror("Abriendo PipeRecibe");
    return -1;
  }

  unlink(pipeRecibe);
  if(mkfifo(pipeRecibe, 0640) == -1){
    perror("Creando el Pipe Principal");
    return -1;
  }
  fd_read = open(pipeRecibe, O_RDWR);
  if (fd_read < 0) {
    perror("Abriendo Pipe del Agente");
    return -1;
  }
  FILE *file;
  file = fopen(argumentos.fileSolicitud, "r");
  if (file == NULL) {
    perror("Error al abrir el archivo de solicitudes");
    return -1;
  }
  horaActual = 0;
  strcpy(saludoInicial->nombreAgente, argumentos.nombre);
  saludoInicial->reserva = false;
  error = write(fd_write, saludoInicial, sizeof(Peticion));
  if (error == -1) {
    perror("Error escribiendo en el pipe del agente");
    return -1;
  }
  error = read(fd_read, horaRecibida, sizeof(int));
  if (error == -1) {
    perror("Error leyendo en el pipe del agente");
    return -1;
  }
  printf("HORA ACTUAL RECIBIDA DESDE EL CONTROLADOR %d\n", *horaRecibida);

  while (true) {
    if (fgets(bufferArchivo, sizeof(bufferArchivo), file) == NULL) {
      if (!feof(file)) {
        perror("Leyendo del archivo");
        return -1;
      } else {
        break;
      }
    }

    bufferArchivo[strcspn(bufferArchivo, "\n")] = 0;
    strcpy(bufferAux, bufferArchivo);
    char *tokens = strtok(bufferAux, ",");
    if (!tokens)
      continue;

    strcpy(solicitudReserva->nombreFamilia, tokens);
    tokens = strtok(NULL, ",");
    if (!tokens)
      continue;

    solicitudReserva->horaSolicitada = atoi(tokens);
    tokens = strtok(NULL, ",");
    if (!tokens)
      continue;

    solicitudReserva->cantPersonas = atoi(tokens);
    solicitudReserva->reserva = true;
    strcpy(solicitudReserva->nombreAgente, argumentos.nombre);

    if (write(fd_write, solicitudReserva, sizeof(Peticion)) == -1) {
      perror("Error escribiendo en el Pipe Principal");
      return -1;
    }
    if (read(fd_read, respuesta, sizeof(Peticion)) == -1) {
      perror("Error leyendo del Pipe Principal");
      return -1;
    }

    printf("PETICION: {\nNombreFamilia: %s\nHoraSolicitada: %d:00\nPersonasSolicitadas: %d\n} RESPUESTA: %s\n\n", respuesta->nombreFamilia, respuesta->horaSolicitada, respuesta->cantPersonas, respuesta->respuesta);
    sleep(2);
  }
  close(fd_read);
  close(fd_write);

  strcpy(pipeRecibe, "/tmp/");
  strcat(pipeRecibe, argumentos.nombre);
  unlink(pipeRecibe);

  free(saludoInicial);
  free(solicitudReserva);
  free(respuesta);
}