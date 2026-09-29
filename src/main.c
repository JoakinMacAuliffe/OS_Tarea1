#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "parser.h"
#include "scheduler.h"

int main (int argc, char *argv[]) {

    if (argc != 3) {
        fprintf(stderr, "Uso: %s plan.txt [limite_concurrencia]\n", argv[0]);
        return EXIT_FAILURE;
    }

    // primer argumento es el archivo
    const char *archivo = argv[1];

    // limite concurrencia
    int K = atoi(argv[2]);

    if (K <= 0) {
        fprintf(stderr, "Error: el límite de concurrencia debe ser mayor a 0\n");
    return EXIT_FAILURE;
    }

    // inicializar semilla aleatoria para parser (duracion aleatoria)
    srand((unsigned int) time(NULL));

    TASK *tareas = NULL;
    int cantidad_tareas = 0;

    if (readFile(archivo, &tareas, &cantidad_tareas) != EXIT_SUCCESS) {
        fprintf(stderr, "Error: no se pudo leer el archivo %s\n", archivo);
        return EXIT_FAILURE;
    }

    if (tareas == NULL || cantidad_tareas <= 0) {
        fprintf(stderr, "Error: no se cargaron tareas válidas\n");
        free(tareas);
        return EXIT_FAILURE;
    }

    // ejecutar scheduler
    run_scheduler (tareas, cantidad_tareas, K);

    // liberar memoria reservada por readFile()
    free(tareas);

    return EXIT_SUCCESS;

}
