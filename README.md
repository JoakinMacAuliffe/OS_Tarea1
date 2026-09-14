# Planificador Dieciochero 

## Descripción

WIP

## Modo de uso

El programa utiliza como entrada un archivo de texto con las distintas tareas (`plan.txt`). Para ejecutar el programa, utilizar el ejecutable ya compilado:

```
./planificador plan.txt [Límite de concurrencia]
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
4 : asar_longaniza : 800 : 1, 2
```

Los campos de cada tarea son:

- `ID_Actividad`: identificador de la tarea, almacenado como texto para
	permitir identificadores alfanuméricos.
- `Nombre_Actividad`: nombre de la tarea.
- `Duración`: duración en milisegundos, almacenada como un entero.
- `Dependencias`: identificadores de las tareas que deben ejecutarse antes.
	Puede estar vacío.

## Archivos

### parser.c

Es el archivo encargado de parsear el contenido de `plan.txt` al programa. Dentro del archivo se implementa la función `readFile()`, que almacena las tareas parseadas en un arreglo de structs llamado tasks[].

## Compilación y ejecución

Desde la raíz del repositorio, compilar con GCC:

WIP

## Decisiones de diseño

WIP

## Limitaciones actuales

WIP
