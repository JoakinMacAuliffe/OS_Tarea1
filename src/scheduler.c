// scheduler.c se encarga de planificar la ejecucion de las actividades del DAG de manera concurrente
// controlando que nunca se sobrepase el limite maximo de K procesos simultaneos exigido por el sistema.
// Utiliza fork() para crear procesos hijos que simulan el tiempo de cada tarea en milisegundos con usleep()
// dirige los insumos a las tareas dependientes por tuberias es decir pipes, y sincroniza la recoleccion
// con waitpid() para evitar el consumo de CPU y no causar busy waiting, tomando tambien en cosideracion manejo de errores y la senal SIGINT.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <stdbool.h>
#include "parser.h"

// se definen los posibles estados de cada tarea

typedef enum{
    estado_esperando,     // espera a que sus dependencias terminen
    estado_ejecutandose,  // ejecutandose en un proceso hijo
    estado_completada,    // terminada con exito
    estado_fallida,       // abortada por fallo
} TaskState;

// estructura interna de control por tarea

typedef struct{
    TaskState state;
    pid_t pid;             // identificador del proceso hijo
    int out_pipe[2];       // pipe para que el hijo envie su insumo terminado 
    char output_msj[64];   // insumo guardado en el padre

}TaskControl;


static TaskControl *g_crtl = NULL;
static int g_total_tasks = 0;


// buscador de posicion de una tarea por su ID

int buscar_tarea(TASK *tasks, int total, const char *id) {
    for (int i = 0; i < total; i++) {
        if (strcmp(tasks[i].id, id) == 0) {
            return i;
        }
    }
    return -1;
}

// Evaluador de si una tarea puede ejecutarse
/*
si la tarea no esta esperando no se puede ejecutar, sus dependencias no pueden haber terminado con algun fallo,
tambien todas sus dependencias deben estar completadas, si aun no terminan, la tarea debe esperar

*/

int puede_ejecutar(TASK *tasks, TaskControl *ctrl, int total, int i){
    if(ctrl[i].state != estado_esperando){   // la tarea se encuentra en otro estado asique no se ejecuta
        return 0;
    }





}