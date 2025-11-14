#include "../ModulosDeDefinicion/ModuloAgente.h" //Se incluye el modulo que contiene las declaraciones de las funciones y estructuras


int tomarArgumentosAgente(int argc, char *argv[]) {
    if(argc < 10) {
        printf("Error: Número insuficiente de argumentos.\n");
        return 1;
    }

    bool argumentos[5] = {false, false, false, false, false};
    int posiciones[5] = {-1, -1, -1, -1, -1};

    for(int i = 0; i < 10; i++){
        if(argv[i] == "-i"){
            argumentos[0] = true;
            posiciones[0] = i;
        }else if(argv[i] == "-f"){
            argumentos[1] = true;
            posiciones[1] = i;
        }else if(argv[i] == "-s"){
            argumentos[2] = true; 
            posiciones[2] = i;
        }else if(argv[i] == "-t"){
            argumentos[3] = true;
            posiciones[3] = i;
        }else if(argv[i] == "-p"){
            argumentos[4] = true;
            posiciones[4] = i;
        }
    }
    for(int i = 0; i < 5; i++){
        if(!argumentos[i]){
            printf("Error: Falta el argumento %d.\n", i+1);
            return -1;
        }
    }

    int horaIni = atoi(argv[posiciones[0] + 1]);
    int horaFin = atoi(argv[posiciones[1] + 1]);
    int segHoras = atoi(argv[posiciones[2] + 1]);
    int total = atoi(argv[posiciones[3] + 1]);
    char *pipeRecibe = argv[posiciones[4] + 1];
    
    if(horaIni < 7 || horaIni > 19){
        printf("Error: La hora de inicio debe estar entre 7 y 19.\n");
        return -1;
    }

    if(horaFin < 7 || horaFin > 19){
        printf("Error: La hora de fin debe estar entre 7 y 19.\n");
        return -1;
    }

    
    return 0;
}