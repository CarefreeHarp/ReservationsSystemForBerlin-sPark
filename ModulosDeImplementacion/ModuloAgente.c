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
  int fd_write, fd_read, error;
  int *horaRecibida;
  char nombreNamedPipe[256];
  char pipeRecibe[256];
  char bufferArchivo[256];
  char bufferAux[256];
  Peticion *saludoInicial = malloc(sizeof(Peticion));
  Peticion *solicitudReserva;
  Peticion *respuesta;
  strcpy(nombreNamedPipe, "/tmp/");
  strcat(nombreNamedPipe, argumentos.pipeRecibe);

  strcpy(pipeRecibe, "/tmp/");
  strcat(pipeRecibe, argumentos.nombre);

  fd_write = open(nombreNamedPipe, O_WRONLY);

  mkfifo(pipeRecibe, 0640);
  fd_read = open(pipeRecibe, O_RDWR);
  FILE *file;
  file = fopen(argumentos.fileSolicitud, "r");
  if (file == NULL) {
    perror("Error al abrir el archivo de solicitudes: ");
    return -1;
  }
  int horaActual = 0;
  strcpy(saludoInicial->nombreAgente, argumentos.nombre);
  saludoInicial->reserva = false;
  error = write(fd_write, saludoInicial, sizeof(Peticion));
  if (error == -1) {
    perror("Error escribiendo en el pipe del agente");
    exit(EXIT_FAILURE);
  }
  error = read(fd_read, horaRecibida, sizeof(int));
  printf("DESPUES\n\n\n");
  if (error == -1) {
    perror("Error leyendo en el pipe del agente:");
    exit(EXIT_FAILURE);
  }
  printf("HORA ACTUAL RECIBIDA DESDE EL CONTROLADOR %d\n", *horaRecibida);

  char buffer[256];
  while (fgets(bufferArchivo, sizeof(bufferArchivo), file)) {
    bufferArchivo[strcspn(bufferArchivo, "\n")] = 0;
    strcpy(bufferAux, bufferArchivo);
    char *tokens = strtok(bufferAux, ",");
    tokens = strtok(NULL, ",");
    strcpy(solicitudReserva->nombreFamilia, tokens);
    tokens = strtok(NULL, ",");
    solicitudReserva->horaSolicitada = atoi(tokens);
    tokens = strtok(NULL, ",");
    solicitudReserva->cantPersonas = atoi(tokens);
    solicitudReserva->reserva = true;
    strcpy(solicitudReserva->nombreAgente, argumentos.nombre);

    write(fd_write, solicitudReserva, sizeof(Peticion));
    read(fd_read, respuesta, sizeof(Peticion));
    printf("PETICION: {\nNombreFamilia:%s\nHoraSolicitada:%d\nPersonasSolicitadas:%d\n} RESPUESTA: %s", respuesta->nombreFamilia, respuesta->horaSolicitada, respuesta->cantPersonas, respuesta->respuesta);
    sleep(2);
  }
  close(fd_read);
  close(fd_write);

  strcpy(pipeRecibe, "/tmp/");
  strcat(pipeRecibe, argumentos.nombre);
  unlink(pipeRecibe);
}