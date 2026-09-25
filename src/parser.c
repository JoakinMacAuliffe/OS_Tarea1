// parser.c se encarga de parsear (valga la redundancia) el archivo de planificacion de tal manera de que se pueda utilizar
// para la posterior construccion del DAG y la ejecucion concurrente en el scheduler.
// 
// Utiliza la estructura TASK importada desde parser.h para almacenar los atributos de cada actividad separados por ":".
// Asigna duracion aleatoria entre 100 y 5000 ms si no viene especificada en el archivo, procesa las dependencias separadas por coma
// y carga dinamicamente las tareas en un arreglo en memoria heap para ser consumido por los demas modulos.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include "parser.h"
#define MAX_LINE_LENGTH 256 // Largo máximo de una línea

// Función para quitar los espacios de cada entrada, (ej. "   prender_carbon " = "prender_carbon")
static char *trim(char *text) {
    while (isspace((unsigned char)*text)) {
        text++; // Mover puntero text al primer espacio
    }

    char *end = text + strlen(text);

    while (end > text && isspace((unsigned char)end[-1])) {
        end--; // Mover puntero end al último espacio
        *end = '\0';
    }
    return text;
}

int readFile (const char *path, TASK **out_tasks, int *out_task_count) {
    FILE *file = fopen(path, "rb"); // Abrir archivo plan.txt
    
    // NOTA: El archivo se abre desde el working directory

    if (!file) {
        perror("Error al abrir plan.txt");
        return EXIT_FAILURE;   
    }

    char buffer[MAX_LINE_LENGTH];

    // Arreglo de TASKs
    TASK *tasks = malloc(MAX_TASKS * sizeof *tasks);
    int task_count = 0; // Utilizado para insertar tareas en el arreglo
    if (tasks == NULL) {
        perror("error de malloc");
        fclose(file);
        return EXIT_FAILURE;
    }

    // Leer línea por línea hasta EOF o límite
    // Esta condicion guarda cada línea del archivo en el buffer
    while (fgets(buffer, sizeof(buffer), file) && task_count < MAX_TASKS) {
        buffer[strcspn(buffer, "\r\n")] = '\0'; //recorre el buffer hasta el \n y lo reemplaza por un \0

        // Se saltan las líneas en blanco
        if (buffer[0] == '\0') {
            continue;
        }

        // Guardar línea como objeto del struct TASK
        TASK task = {0};

        // strtok permite separar cada campo con :

        // separar id
        char *field = strtok(buffer, ":"); //reemplaza el primer ":" por \0 en el buffer y apunta field al inicio del campo (ID)
        if (!field) {
            continue;
        }
        
        strcpy(task.id, trim(field));

        // separar nombre
        field = strtok(NULL, ":"); //sigue cortando la misma cadena desde el byte donde quedó esperando el buffer
        if (!field) {
            continue;
        }
        strcpy(task.name, trim(field));

        // separar duración
        field = strtok(NULL, ":");
        if (!field) {
            continue;
        }
        
        char *dur_str = trim(field);
        int parsed_duration = atoi(dur_str);

        // Si no tiene duracion o es 0, asignar aleatorio entre 100 y 5000 ms segun enunciado
        if (parsed_duration <= 0) {
            task.duration = (rand() % 4901) + 100;
        } else {
            task.duration = parsed_duration;
        }
        
        // separar dependencias
        field = strtok(NULL, ":");
        if (field != NULL) {  // se comprueba que tenga dependencias
            // separar cada dependencia con ,
            char *dependency = strtok(field, ",");
            while (dependency != NULL && task.dep_count < MAX_DEPS) {
                // Para cada dependencia, guardar en objeto task
                strcpy(task.dependencies[task.dep_count], trim(dependency));
                task.dep_count++;
                dependency = strtok(NULL, ",");
            }
        } 
        
        // Guardar objeto TASK en el arreglo
        tasks[task_count++] = task;

        // Imprimir linea parseada
        printf("ID_Actividad: %s\n", task.id);
        printf("Nombre_Actividad: %s\n", task.name);
        printf("Duración: %d\n", task.duration);
        printf("Dependencias: "); 
            for (int i = 0; i < task.dep_count; i++) {
                printf("%s", task.dependencies[i]);
                if (i < task.dep_count - 1) {
                    printf(", ");
                }
            }
        printf("\n");
        printf("Cantidad de dependencias: %d\n\n", task.dep_count);
    }

    *out_tasks = tasks; 
    *out_task_count = task_count;
    fclose(file);
    printf("%d tareas cargadas correctamente en la memoria.\n", task_count);

    return EXIT_SUCCESS;
}

#ifdef TEST_PARSER
int main(void) {

    // USO:

    // srand((unsigned int)time(NULL));
    
    // TASK *tasks;
    // int task_count;
    // readFile("plan.txt", &tasks, &task_count); // test para testear que el testeo esté testeadamente testeado

    return 0;
}

#endif 