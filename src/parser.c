// parser.c se encarga de parsear (valga la redundancia) el archivo plan.txt de tal manera de que se pueda utilizar
// para la posterior ejecución del programa

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_NAME_LEN 64 // Longitud máxima del nombre de la actividad 
#define MAX_DEPS 16 // Cantidad máxima de dependencias
#define MAX_TASKS 20000 // Cantidad máxima de tareas (el ejercicio pide 10000 pero pondré 20000 porque sí)
#define MAX_LINE_LENGTH 256 // Largo máximo de una línea

typedef struct {
    int id; // ID_Actividad
    char name[MAX_NAME_LEN]; // Nombre_Actividad
    int duration; // tiempo_ms
    int dependencies[MAX_DEPS];
    int dep_count;
} TASK;

int readFile (const char *path) {
    FILE *file = fopen(path, "rb"); // Abrir archivo plan.txt
    
    // NOTA: El archivo se abre desde el working directory

    if (!file) {
        perror("Error al abrir plan.txt");
        return EXIT_FAILURE;   
    }

    // Reservar memoria dinámica para un arreglo de MAX_TASKS punteros
    char **lines = malloc(MAX_TASKS * sizeof(char*));
    if (!lines) {
        perror("Memory allocation failed");
        fclose(file);
        return EXIT_FAILURE;
    }

    char buffer[MAX_LINE_LENGTH];    
    int line_count = 0;

    // Leer línea por línea hasta EOF o límite
    // Esta condicion guarda cada línea del archivo en el buffer
    while (fgets(buffer, sizeof(buffer), file) && line_count < MAX_TASKS) {
        buffer[strcspn(buffer, "\r\n")] = '\0';

        // Se saltan las líneas en blanco
        if (buffer[0] == '\0') {
            continue;
        }

        // Guardar linea en el buffer
        lines[line_count] = strdup(buffer);
        line_count++;
    }

    fclose(file);
    printf("%d líneas cargadas correctamente en la memoria.\n", line_count);

    // Liberar memoria
    for (int i = 0; i < line_count; i++) {
        free(lines[i]);
    }
    free(lines);

    return EXIT_SUCCESS;

}

int main(void) {

readFile("plan.txt"); // test para testear que el testeo esté testeadamente testeado

return 0;

}

