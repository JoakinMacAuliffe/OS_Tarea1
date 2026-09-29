# Planificador Dieciochero

## Descripción

Este proyecto consiste en un simulador y planificador concurrente de actividades dieciocheras desarrollado en lenguaje C (estándar C17) para organizar las celebraciones del señor Loyola. Las actividades, sus tiempos estimados y sus relaciones de dependencia se modelan mediante un Grafo Acíclico Dirigido (DAG).

El planificador coordina la ejecución concurrente de las actividades respetando en todo momento el límite de procesos simultáneos (K). Los insumos generados por cada tarea se transmiten mediante tuberías unidireccionales (pipes), las ramas dependientes de una actividad que falla se descartan sin detener al resto del plan, y la llegada imprevista de la autoridad sanitaria (la Seremi) se maneja mediante señales POSIX (`SIGINT`). Todo el sistema se basa en procesos independientes creados con `fork()` y sincronizados con llamadas al sistema, sin utilizar threads ni librerías de hilos.

## Modo de uso

El programa utiliza como entrada un archivo de texto con las distintas tareas (`plan.txt`) y un número entero positivo que define el límite de concurrencia K. Para ejecutar el programa, utilizar el ejecutable ya compilado:

```
./planificador plan.txt [Límite de concurrencia]
```

Ejecución básica con concurrencia K = 2:

```
./planificador plan.txt 2
```

Ejecución con un plan de 10.000 actividades y concurrencia K = 8:

```
./planificador plan_10000.txt 8
```

### Formato de `plan.txt`

El archivo `plan.txt` utiliza `:` para separar los campos principales y `,`
para separar las dependencias:

```text
ID_Actividad : Nombre_Actividad : Duración : Dependencias
```

Por ejemplo:

```text
1 : prender_carbon : 500 :
2 : comprar_carne : 1200 :
3 : comprar_pan : 300 :
4 : asar_longaniza : 800 : 1, 2
5 : armar_choripan : 250 : 3, 4
6 : servir_mesa : 100 : 5
```

Los campos de cada tarea son:

- `ID_Actividad`: identificador único de la tarea, almacenado como texto para
	permitir identificadores alfanuméricos.
- `Nombre_Actividad`: nombre descriptivo de la tarea.
- `Duración`: duración en milisegundos, almacenada como un entero. Si el campo viene vacío o con un valor menor o igual a 0, el programa le asigna una duración aleatoria entre 100 y 5000 ms.
- `Dependencias`: identificadores de las tareas que deben terminar antes de que esta pueda iniciar.
	Puede estar vacío si la tarea no depende de ninguna otra.

## Archivos

El proyecto está organizado de forma modular en cuatro componentes dentro de la carpeta `src/`.

### parser.c

Es el archivo encargado de parsear el contenido de `plan.txt` al programa. Dentro del archivo se implementan las siguientes funciones:

- `trim(char *text)`: función auxiliar que quita los espacios en blanco y saltos de línea al inicio y al final de un texto, usando `isspace()`.
- `readFile(const char *path, TASK **out_tasks, int *out_task_count)`: abre el archivo y lo lee línea por línea con `fgets()`. Separa los campos con `strtok()` usando `:`. Si la tarea no tiene duración, le asigna un valor aleatorio entre 100 y 5000 ms con `rand()`. Separa las dependencias por comas y, tras aplicar `trim()`, verifica con `strlen()` que no estén vacías, para no registrar dependencias "fantasma". Almacena las tareas parseadas en un arreglo de structs `TASK` en el heap.

### dag.c

Módulo que construye y libera el Grafo Acíclico Dirigido en memoria dinámica. Contiene las siguientes funciones:

- `find_task(const TASK *tareas, int cantidad, const char *id)`: función estática que busca una tarea por su ID y devuelve su índice dentro del arreglo, o -1 si no existe.
- `dag_build(Dag *dag, const TASK *tareas, int cantidad_tareas)`: construye el DAG. Inicializa los nodos, calcula el `grado_entrada` de cada tarea (cuántas dependencias le faltan) y cuenta cuántas tareas dependen de cada una. Luego reserva el arreglo dinámico `dependientes` de cada nodo y guarda en él los índices de las tareas sucesoras, para que el scheduler sepa a quién desbloquear cuando una tarea termina. Valida que cada dependencia exista antes de acceder a memoria.
- `dag_free(Dag *dag)`: libera la memoria del DAG, recorriendo cada arreglo de dependientes y luego el arreglo principal de nodos.

### scheduler.c

Módulo que planifica la ejecución concurrente, controlando que nunca se supere el límite K de procesos simultáneos. Además, maneja las tuberías, evita la espera activa y gestiona las señales. Contiene las siguientes funciones:

- `puede_ejecutar(const Dag *dag, const TaskControl *ctrl, int indice_tarea)`: comprueba si la tarea está en `estado_esperando` y si su `grado_entrada` en el DAG es 0, es decir, si ya no tiene dependencias pendientes.
- `desbloquear_dependientes(Dag *dag, int indice_tarea)`: cuando una tarea termina, recorre su lista de dependientes y resta 1 a su `grado_entrada`.
- `propagar_fallo(const Dag *dag, TaskControl *ctrl, int indice_tarea)`: función recursiva que, cuando falla una tarea, marca como `estado_fallida` a todas las tareas que dependen de ella, directa o indirectamente, para que no queden esperando indefinidamente.
- `ejecutar_hijo(TASK *t, int write_fd)`: rutina que ejecuta el proceso hijo tras el `fork()`. Simula la duración de la actividad con `usleep()`, escribe en la tubería el mensaje de insumo `"LISTO:"`, cierra el descriptor de escritura y termina con `exit(0)`.
- `manejar_seremi(int sig)`: manejador de la señal `SIGINT` (Ctrl+C). Aborta las actividades en curso enviando `SIGKILL` a los procesos hijos activos y termina con código 130.
- `run_scheduler(TASK *tasks, int total, int K)`: función principal del planificador, que utiliza las anteriores. Construye el DAG, inicializa la tabla de control `TaskControl`, lanza procesos hijos respetando el límite K, espera su término con `waitpid()` bloqueante, lee los insumos de las tuberías, desbloquea a los dependientes y libera la memoria al final.

### main.c

Es el punto de entrada del programa. Contiene la función:

- `main(int argc, char *argv[])`: valida los argumentos de la línea de comandos y que K sea mayor a 0. Inicializa la semilla aleatoria con `srand(time(NULL))`, llama a `readFile()` para cargar las tareas, ejecuta el planificador con `run_scheduler()` y libera la memoria reservada por el parser.

## Compilación y ejecución

Desde la raíz del repositorio, compilar con GCC utilizando las banderas exigidas por la rúbrica:

```
gcc -Wall -Wextra -std=c17 src/main.c src/parser.c src/scheduler.c src/dag.c -o planificador
```

Para ejecutar el programa con un plan de tareas y un límite K:

```
./planificador plan.txt 2
```

## Decisiones de diseño

### 1. DAG con índices numéricos en lugar de búsqueda por texto

Comprobar las dependencias comparando nombres con `strcmp` obligaría a repetir esas comparaciones cada vez que el scheduler evalúa una tarea, y el costo crecería con la cantidad de tareas. Por eso las dependencias se preprocesan una sola vez en `dag_build()`: cada ID se traduce a un índice numérico (de 0 a N-1) y cada tarea guarda un contador `grado_entrada`. Así, saber si una tarea está lista es consultar si `grado_entrada == 0`, una operación de tiempo constante O(1), y cuando una tarea termina solo se recorren sus dependientes directos para restarles 1.

### 2. Sincronización sin espera activa (busy waiting)

Para no gastar CPU innecesariamente, el programa no usa bucles de consulta con `WNOHANG` ni pausas con `sleep()`.

El bucle principal lanza todas las tareas que estén listas hasta alcanzar el límite K y, cuando ya no puede lanzar más, se bloquea esperando con:

```c
pid_t pid_terminado = waitpid(-1, &status, 0);
```

El proceso padre queda suspendido hasta que el sistema operativo le avisa que un hijo terminó. Entonces despierta, lee el pipe, desbloquea las tareas siguientes y vuelve a lanzar las que estén listas.

### 3. Cierre estricto de descriptores de pipes

En Linux cada proceso tiene un límite de descriptores abiertos al mismo tiempo (`ulimit -n`, normalmente 1024). Si los pipes de todas las tareas se mantuvieran abiertos hasta el final, un plan grande podría agotar ese límite y provocar el error `EMFILE: Too many open files`. Para evitarlo, cada pipe se cierra apenas deja de ser necesario:

- En el hijo: apenas nace con `fork()`, cierra el extremo de lectura `out_pipe[0]`. Cuando termina de escribir `"LISTO:"`, cierra el extremo de escritura `out_pipe[1]` antes del `exit()`.
- En el padre: apenas hace el `fork()`, cierra su copia del extremo de escritura `out_pipe[1]`. Cuando `waitpid()` avisa que el hijo terminó, lee el insumo con `read()` y cierra de inmediato el extremo de lectura `out_pipe[0]`.

De esta forma, la cantidad de descriptores abiertos depende de K y no del número total de tareas.

### 4. Aislamiento de fallas con propagación recursiva

Si una actividad falla internamente, el programa no debe detenerse por completo, sino cancelar únicamente la rama que depende de ella.

Cuando `waitpid()` detecta que un proceso hijo terminó con error (`WEXITSTATUS != 0`), la tarea se marca como `estado_fallida` y se llama a `propagar_fallo()`. Como cada nodo del DAG conoce a sus dependientes, esta función recorre recursivamente toda la descendencia marcándola como fallida. Así, las tareas sucesoras no quedan esperando insumos que nunca llegarán, y las ramas independientes siguen ejecutándose normalmente.

### 5. Manejo de la Seremi (SIGINT)

Para simular la llegada de la autoridad sanitaria, al presionar Ctrl+C el planificador debe abortar todas las actividades en curso.

Como la terminal envía la señal a todo el grupo de procesos en primer plano, en los hijos se restaura el comportamiento por defecto (`SIG_DFL`) justo después del `fork()`, para que no ejecuten el manejador del padre. En el padre, `manejar_seremi()` recorre la tabla `TaskControl` y envía `SIGKILL` a los PIDs de los hijos que estén en `estado_ejecutandose`, para que no queden procesos en ejecución. Luego el programa termina con código 130.

### 6. Corrección en el parser para dependencias vacías

En las tareas iniciales, que no dependen de ninguna otra, el final de la línea podía contener espacios residuales. Tras aplicar `trim()`, la cadena quedaba vacía (`""`), pero el parser la contaba igual y sumaba 1 a `dep_count`. Esa "dependencia fantasma" dejaba a la tarea esperando indefinidamente a una tarea que no existía.

Se solucionó agregando un `if (strlen(dep_limpia) > 0)` antes de guardar la dependencia, de modo que solo se registren dependencias con texto real.

## Limitaciones actuales

- El programa asume que `plan.txt` representa un grafo sin ciclos, tal como lo establece el modelo del enunciado. Si el archivo contiene dependencias circulares (por ejemplo, la tarea 1 depende de la 2 y la 2 depende de la 1), el `grado_entrada` de las tareas involucradas nunca llegará a 0 y quedarán esperando indefinidamente.
- Los límites definidos en `parser.h` (`MAX_TASKS = 20000`, `MAX_DEPS = 32`, `MAX_LINE_LENGTH = 256`) permiten planes de hasta 20.000 tareas, lo que cubre el caso de 10.000 actividades del enunciado. Para cargar planes más grandes habría que aumentar esas constantes en el header.