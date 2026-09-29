#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "parser.h"
#include "dag.h"

// funcion que elimina el DAG liberando la memoria que este utiliza.
void dag_free(Dag *dag) {
    if (dag == NULL) {
        return;
    }

    // aplicar funcion free para cada nodo del dag
    for (int i = 0; i < dag->cantidad_nodos; i++) {
        free(dag->nodos[i].dependientes);
    }

    free(dag->nodos);
    dag->nodos = NULL;
    dag->cantidad_nodos = 0;
}

// funcion que retorna el indice de una tarea utilizando su id
static int find_task(const TASK *tareas, int cantidad, const char *id) {
    for (int i = 0; i < cantidad; i++) {
        if (strcmp(tareas[i].id, id) == 0) {
            return i;
        }
    }

    return -1;
}

// construir el DAG
int dag_build(Dag *dag, const TASK *tareas, int cantidad_tareas) {
    if (dag == NULL || tareas == NULL || cantidad_tareas <= 0) { // verificacion de mal input
        return -1;
    }

    // Asignar memoria para guardar nodos
    dag->nodos = calloc((size_t)cantidad_tareas, sizeof(DagNode));
    if (dag->nodos == NULL) {
        return -1;
    }

    // hay tantos nodos como tareas en el programa
    dag->cantidad_nodos = cantidad_tareas;

    // inicializar nodos y calcular indegree (cantidad de dependencias)
    for (int i = 0; i < cantidad_tareas; i++) {
        dag->nodos[i].indice_tarea = i;
        dag->nodos[i].grado_entrada = tareas[i].dep_count;

        for (int j = 0; j < tareas[i].dep_count; j++) {
            int indice_dependencia = find_task(
                tareas,
                cantidad_tareas,
                tareas[i].dependencies[j]
            );

            dag->nodos[indice_dependencia].cantidad_dependientes++;

        }

    }

    // ciclo que reserva memoria para cada arreglo de dependencias
    for (int i = 0; i < cantidad_tareas; i++) {

        // cantidad de tareas que dependen de nodo actual
        int cantidad = dag->nodos[i].cantidad_dependientes;

        // Si es que existen tareas que dependan del nodo actual, reservar memoria para ellas
        if (cantidad > 0) {
            dag->nodos[i].dependientes = malloc(
                (size_t) cantidad * sizeof(int)
            );
            if (dag->nodos[i].dependientes == NULL) {
                dag_free(dag);
                return -1;
            }
        }
    }

    // arreglo utilizado para determinar próximo espacio libre donde guardar el próximo dependiente de cada tarea
    int *posiciones = calloc((size_t) cantidad_tareas, sizeof(int));

    if (posiciones == NULL) {
        dag_free(dag);
        return -1;
    }

    // ciclo anidado que recorre todas las dependencias de cada tarea
    for (int tarea = 0; tarea < cantidad_tareas; tarea++) {
        // para cada tarea, recorrer todas sus dependencias
        for (int dependencia = 0; dependencia < tareas[tarea].dep_count; dependencia++) {
            // buscar el índice de la dependencia de la tarea actual
            int indice_dependencia = find_task(
                tareas,
                cantidad_tareas,
                tareas[tarea].dependencies[dependencia]
            );

            if (indice_dependencia < 0) {
                free(posiciones);
                dag_free(dag);
                return -1;
            }

            /*
            Estas tres líneas que vienen registran que al tarea actual depende de la dependencia
            cuyo índice es indice_dependencia, para que cuando esa dependencia termine, el scheduler
            sepa qué tarea desbloquear.
            */

            int posicion = posiciones[indice_dependencia];
            posiciones[indice_dependencia]++;
            dag->nodos[indice_dependencia].dependientes[posicion] = tarea;
        
        }

    }

free(posiciones);

return 0;

}