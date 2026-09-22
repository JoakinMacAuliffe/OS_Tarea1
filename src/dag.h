// Estructura utilizada en dag.c

// Estructura de cada nodo del DAG
// NOTA: Hay un diagrama del funcionamiento del DAG en /docs/DAG.pdf.
typedef struct {
    int task_index; // índice de la tarea
    int *dependents; // tareas por desbloquear una vez esta termine (contrario de indegree)
    int dependant_count; // cantidad de tareas que dependen de esta tarea
    int indegree; // cantidad de dependencias
} DagNode;

// Estructura del DAG, compuesto por los nodos ya descritos
typedef struct {
    DagNode *nodes;
    int node_count;
} Dag;