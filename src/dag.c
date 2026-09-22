#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "parser.h"
#include "dag.h"

// funcion que retorna el indice de una tarea utilizando su id
static int find_task(const TASK *tasks, int count, const char *id) {
    for (int i = 0; i < count; i++) {
        if (strcmp(tasks[i].id, id) == 0) {
            return i;
        }
    }

    return -1;
}

// construir el DAG
int dag_build(Dag *dag, const TASK *tasks, int task_count) {
    if (dag == NULL || tasks == NULL || task_count <= 0) { // verificacion de mal input
        return -1;
    }

    // Asignar memoria para guardar nodos
    dag->nodes = calloc((size_t)task_count, sizeof(DagNode));
    if (dag->nodes == NULL) {
        return -1;
    }

    dag->node_count = task_count;

    // inicializar nodos y calcular indegree (cantidad de dependencias)
    for (int i = 0; i < task_count; i++) {
        dag->nodes[i].task_index = i;
        dag->nodes[i].indegree = tasks[i].dep_count;

        for (int j = 0; j < tasks[i].dep_count; j++) {
            int dependency_index = find_task(
                tasks,
                task_count,
                tasks[i].dependencies[j]
            );

            dag->nodes[dependency_index].dependant_count++;

        }

    }

    for (int i = 0; i < task_count; i++) {

        // cantidad de tareas que dependen de nodo actual
        int count = dag->nodes[i].dependant_count;

        // Si es que existen tareas que dependan del nodo actual, reservar memoria para ellas
        if (count > 0) {
            dag->nodes[i].dependents = malloc(
                (size_t) count * sizeof(int)
            );
        }

    }

    // WIP WIP WIP WIP WIP

}