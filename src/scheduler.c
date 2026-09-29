#define _DEFAULT_SOURCE
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
#include "scheduler.h"
#include "dag.h"
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


static TaskControl *g_ctrl = NULL;
static int g_total_tasks = 0;

static int puede_ejecutar (const Dag *dag, const TaskControl *ctrl, int indice_tarea) {

    // solo se ejecutan tareas que estan esperando
    if (ctrl[indice_tarea].state != estado_esperando) {
        return 0;
    }

    // el dag mantiene la cantidad de dependencias que faltan, de ser 0, se desbloquea la tarea
    return dag->nodos[indice_tarea].grado_entrada == 0;

}

// funcion que decrementa el grado de entrada de una tarea representando sus dependencias
static void desbloquear_dependientes (Dag *dag, int indice_tarea) {
    DagNode *nodo = &dag->nodos[indice_tarea];

    for (int i = 0; i < nodo->cantidad_dependientes; i++) {
        int indice_dependiente = nodo->dependientes[i];

        if (dag->nodos[indice_dependiente].grado_entrada > 0) {
            dag->nodos[indice_dependiente].grado_entrada--;
        }
    }
}

// funcion recursiva que una vez falla un nodo propaga dicho fallo a todas las tareas que dependen de él, para que no queden esperando eternamente
static void propagar_fallo (const Dag *dag, TaskControl *ctrl, int indice_tarea) {

    DagNode *nodo = &dag->nodos[indice_tarea];

    for (int i = 0; i < nodo->cantidad_dependientes; i++) {
    
        int dependiente = nodo->dependientes[i];
    
        if (ctrl[dependiente].state == estado_esperando) {
            ctrl[dependiente].state = estado_fallida;
            propagar_fallo (dag, ctrl, dependiente);
        }
    }
}

// se detienen todos los procesos activos simulando lo de la seremi
void manejar_seremi(int sig) {
    (void)sig;
    printf("\n(seremi) abortacion de actividades en curso\n");

    for (int i = 0; i < g_total_tasks; i++) {
        if (g_ctrl && g_ctrl[i].state == estado_ejecutandose && g_ctrl[i].pid > 0) {
            kill(g_ctrl[i].pid, SIGKILL); // Mata el proceso hijo activo inmediatamente
        }
    }
    exit(130);
}

// se simula el tiempo del trabajo del hijo y envia el insumo al pipe
void ejecutar_hijo(TASK *t, int write_fd) {
    // Duerme los milisegundos indicados (1 ms = 1000 microsegundos)
    usleep(t->duration * 1000);

    // Escribe el mensaje con el insumo generado en la tuberia
    char msj[64];
    snprintf(msj, sizeof(msj), "LISTO:%s", t->id);
    write(write_fd, msj, strlen(msj) + 1);
    close(write_fd);

    exit(0); // El hijo siempre finaliza con exit()
}


//con esto se aplican todas las funciionalidades de las funciones funcionadas anteriormente para runear el scheduler

void run_scheduler(TASK *tasks, int total, int K) {
    
    // validacion de parametros
    if (tasks == NULL || total <= 0 || K <= 0) {
        perror("Parametros invalidos.\n");
        return;
    }

    // Creacion del dag
    Dag dag = {0};
    if (dag_build(&dag, tasks, total) != 0) {
        perror("error al construir el dag\n");
        return;
    }
    
    // calloc limpia toda la memoria en 0 (todas arrancan en estado_esperando)
    TaskControl *ctrl = calloc((size_t) total, sizeof(TaskControl));

    if (!ctrl) {
        perror("error al asignar memoria para TaskControl\n");
        dag_free(&dag);
        return;
    }

    // variables globales para que la seremi sepa a quienes detener con ctrl+c
    g_ctrl = ctrl;
    g_total_tasks = total;
    signal(SIGINT, manejar_seremi);

    int activos = 0;

    // bucle principal corre hasta que no queden procesos ni tareas por lanzar
    while (1) {
        // primero, lanzar tareas si estan listas y hay espacio en el limite K
        for (int i = 0; i < total && activos < K; i++) {
            if (puede_ejecutar(&dag, ctrl, i)) {
                
                // crear tuberia para pasar el insumo
                if (pipe(ctrl[i].out_pipe) < 0) {
                    perror("error al crear pipe");
                    continue;
                }

                pid_t pid = fork();

                if (pid == 0) {
                    // codigo del hijo: solo escribe en la tuberia
                    signal(SIGINT, SIG_DFL); // restaurar comportamiento normal de SIGINT para que el hijo no ejecute manejar_seremi()                
                    close(ctrl[i].out_pipe[0]);
                    ejecutar_hijo(&tasks[i], ctrl[i].out_pipe[1]);
                } else if (pid > 0) {
                    // codigo del padre: solo lee de la tuberia
                    close(ctrl[i].out_pipe[1]);
                    ctrl[i].pid = pid;
                    ctrl[i].state = estado_ejecutandose;
                    activos++;
                    printf("[INICIO] Tarea %s (%s) lanzada [PID %d] (Activos: %d/%d)\n", 
                           tasks[i].id, tasks[i].name, pid, activos, K);
                } else {
                    perror("error en fork");
                    close(ctrl[i].out_pipe[0]);
                    close(ctrl[i].out_pipe[1]);
                }
            }
        }

        // si no hay ningun hijo corriendo y no se pudo lanzar nada mas termina
        if (activos == 0) {
            break;
        }

        // segundo, esperar a que termine un hijo (evita busy waiting)
        int status;
        pid_t pid_terminado = waitpid(-1, &status, 0);

        if (pid_terminado > 0) {
            activos--;

            // buscar cual tarea termino usando el pid
            for (int i = 0; i < total; i++) {
                if (ctrl[i].pid == pid_terminado) {
                    // revisar si termino bien o con error
                    if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
                        ctrl[i].state = estado_completada;
                        // leer el insumo que mando el hijo por el pipe
                        read(ctrl[i].out_pipe[0], ctrl[i].output_msj, sizeof(ctrl[i].output_msj) - 1);
                        printf("[FIN] Tarea %s (%s) completada -> Insumo: \"%s\"\n", 
                               tasks[i].id, tasks[i].name, ctrl[i].output_msj);
                        // actualizar dag para que las tareas que dependian de esta se puedan ejecutar
                        desbloquear_dependientes(&dag, i);
                    } else {
                        ctrl[i].state = estado_fallida;
                        printf("[FALLO] Tarea %s termino con error\n", tasks[i].id);
                        propagar_fallo(&dag, ctrl, i);
                    }

                    // cerrar la lectura del pipe para liberar recursos
                    close(ctrl[i].out_pipe[0]);
                    break;
                }
            }
        }
    }

    // liberar memoria antes de salir
    free(ctrl);
    dag_free(&dag);
    g_ctrl = NULL;
    g_total_tasks = 0;
}

#ifdef TEST_MAIN

int main(int argc, char *argv[]) {
    const char *archivo = "plan.txt";
    int K = 2; // limite de concurrencia por defecto

    // Permite pasar parametros por consola ./test_scheduler plan.txt 
    if (argc >= 2) {
        archivo = argv[1];
    }
    if (argc >= 3) {
        K = atoi(argv[2]);
    }

    printf("PRUEBA DEL PARSER OCUPANDO PARSER\n");
    printf("Leyendo: %s | Concurrencia K = %d\n\n", archivo, K);

    TASK *tasks = NULL;
    int total = 0;

    // 1. Llamar al parser para cargar el archivo en memoria heap
    if (readFile(archivo, &tasks, &total) != EXIT_SUCCESS || tasks == NULL) {
        fprintf(stderr, "Error: no se pudo procesar el archivo %s\n", archivo);
        return EXIT_FAILURE;
    }

    // 2. Ejecutar el planificador con las tareas parseadas
    run_scheduler(tasks, total, K);

    // 3. Liberar la memoria dinamica que reservo readFile con malloc
    free(tasks);

    printf("\n EJECUCION FINALIZADA CON EXITO\n");
    return EXIT_SUCCESS;
}

#endif