// Estructura utilizada en dag.c

// Estructura de cada nodo del DAG
// NOTA: Hay un diagrama del funcionamiento del DAG en /docs/DAG.pdf.

#ifndef DAG_H
#define DAG_H

#include "parser.h"

typedef struct {
    int indice_tarea; // índice de la tarea
    int *dependientes; // tareas por desbloquear una vez esta termine (contrario de indegree)
    int cantidad_dependientes; // cantidad de tareas que dependen de esta tarea
    int grado_entrada; // cantidad de dependencias
} DagNode;

// Estructura del DAG, compuesto por los nodos ya descritos
typedef struct {
    DagNode *nodos;
    int cantidad_nodos;
} Dag;

int dag_build(Dag *dag, const TASK *tareas, int cantidad_tareas);
void dag_free(Dag *dag);

#endif