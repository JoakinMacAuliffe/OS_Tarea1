#ifndef PARSER_H // comprobar si no está definido, caso contrario salta a #endif
#define PARSER_H // definirlo

// evita que el archivo parser.h se procese mas de una vez durante la compilacion

#define MAX_NAME_LEN    64   // limite maximo para el nombre de la actividad
#define MAX_ID_LENGTH   32   // limite maximo id
#define MAX_DEPS        32   // dependenciad maximas por tarea 32 para soportar posibles complejidades de ramas paralelas
#define MAX_TASKS    20000   // cantidad maxima de tareas, margen de seguridad del 100% para pruebas de estres

// estructura de datos para guardar cada línea del documento

typedef struct {  
    char id[MAX_ID_LENGTH]; // ID_Actividad
    char name[MAX_NAME_LEN]; // Nombre_Actividad
    int duration; // tiempo_ms
    char dependencies[MAX_DEPS][MAX_ID_LENGTH];
    int dep_count;
} TASK;

// lee el archivo de planificacion y guarda las tasks en memoria dinamica
// retorna 0 en exito o -1 en caso de error
int readFile(const char *path, TASK **out_tasks, int *out_task_count);

#endif 