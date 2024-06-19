# TP Juego: PokéRush

## Repositorio de Francisco Russo - 107480 - [frrusso@fi.uba.ar](mailto:frrusso@fi.uba.ar)

- Para compilar y ejecutar con Valgrind las pruebas de `tp.c`:

```bash
make valgrind-alumno 
```

- Para compilar y ejecutar con Valgrind el juego:

```bash
make valgrind-juego
```

Nota sobre la organización del directorio `/src`: Al tener un makefile fijo, no queda otra opción que tener todos los archivos juntitos, así como están. De haber más libertad se hubieran creado dos subdirectorios: `/src/pokerush` y `/src/motor`, dejando los TDA's creados durante la materia (incluido `TP`) en `/src` o en otro subdirectorio.

---

## Trabajo práctico

Lo principal de este trabajo práctico es el archivo [`tp.c`](src/tp.c), el cual presenta un TDA para
poder jugar un juego de carreras de pokémon. Al ser lo más importante, es lo que en más detalle se
explica en este informe.

### Archivo csv de pokémon

Un aspecto fundamental del TDA TP es poder cargar y retener información de pokémon, la cual se encuentra
originalmente en un archivo CSV con el siguiente formato:

```csv
Nombre,Fuerza,Destreza,Inteligencia
```

La función principal utilizada para leer el archivo CSV es `fscanf`. No es una función muy segura, por lo
cual requiere precauciones. En particular, se limitó la cantidad de caracteres de un nombre a 127,
resultando en el siguiente format string: `fscanf(archivo, "%127[^, \t\r\n],%d,%d,%d ", ...)`. De esta manera,
se lee línea a línea, skippeando whitespace si lo hubiera al final de la línea. Además, se evita que un pokémon
tenga un nombre con whitespace en él. Frenamos una vez que leemos EOF (-1) 😀 o leemos menos que 4 variables 😢 (archivo inválido).

### Estructuras internas

Bueno, dónde y cómo se guardan los pokémon dentro del TP? Y los obstáculos? Por un lado tenemos el `struct pokemon_info`
que debemos respetar al ser parte del contrato (está en header file). La idea es que el usuario del TP acceda a los campos de este struct.

```c
struct pokemon_info {
    char *nombre;
    int fuerza, destreza, inteligencia;
};
```

Nada fuera de lo extraordinario. Un comentario es que el nombre tiene que ser `malloc`'eado y `free`'ado en algún momento.

Luego tenemos los structs internos a la implementación. Se crearon los siguientes dos:

```c
struct jugador {
    lista_t *obstaculos;
    struct pokemon_info *pokemon;
}

struct tp {
    hash_t *pokemones;
    struct jugador jugador_1;
    struct jugador jugador_2;
};
```

Acá se ve que se están reutilizando los TDA's de trabajos prácticos anteriores! Más adelante se explica
por qué se utilizó la lista para los obstáculos. También está
la selección de pokémon, la cual es un simple puntero a el struct ya contenido dentro de `tp.pokemones`.

Los pokémon están contenidos en un hash ya que se requiere poder buscarlos por nombre, y un hash tiene una
complejidad computacional de tiempo constante para dicha operación. El tema difícil del hash es el rehash. Pero
en este TP, la tabla se puebla una sola vez durante la creación del TDA y luego no se agregan más pokémon. Es
decir, `pokemones` sería principalmente de lectura.

### Selección de obstáculos

Un jugador puede insertar obstáculos en su propia pista, y hay 3 variantes para elegir (fuerza, destreza e
inteligencia). Además, se puede insertar en cualquier posición que el usuario desee. Por esta razón es
natural elegir el TDA lista como representación de los obstáculos. Básicamente tenemos todo el trabajo
ya hecho.

También está la lógica de remover obstáculos, y como se verá más adelante, de recorrerlos.
En general, todas estas operaciones son de orden O(N), donde N es la cantidad de obstáculos, simplemente
por el funcionamiento interno de una lista de nodos enlazados.

Finalmente, está la posibilidad de crear un string que represente la pista. Por ejemplo, podría tener
un string `FFDIF` que representa 5 obstáculos (fuerza, fuerza, destreza, inteligencia, fuerza). La lógica para
crear este string es muy sencilla. 5 obstáculos significaría 6 chars (uno extra para el `\0`). Hacemos un
switch para convertir cada tipo de obstáculo a un char ya definido en el header file.

### Selección de pokémon

Cada jugador debe elegir un pokémon para que recorra su pista de obstáculos. La selección es simple y cómoda
ya que no hace falta pegarle a las mayúsculas y minúsculas del nombre original. En particular, todo nombre
de pokémon leído en el CSV se convierte a un formato estándar, con la primera letra mayúscula y las demás
minúsculas. Al hacer una búsqueda, se debe realizar un paso intermedio para convertir el string del usuario
a uno con dicho formato especial. Para lograr este objetivo se creó una función similar a `strcpy` pero
modificada:

```c
void strcpy_nombre_pokemon(char *destino, const char *origen)
{
    bool primer_caracter = true;

    while (*origen != '\0') {
        *destino = primer_caracter ? (char)toupper(*origen) :
        (char)tolower(*origen);

    primer_caracter = false;
    destino++;
    origen++;
    }

    *destino = '\0';
}
```

Ah, y no se puede elegir el mismo pokémon que el oponente. Si fuera necesario saber cuáles se pueden elegir,
existe la opción de crear un string con todos los pokémon en él, separados por coma. Para lograr esto
se utilizó el iterador interno del hash, el cual puede recorrer todos los elementos en él en orden O(N).
Sin embargo, nos piden que los nombres estén en orden alfabético, y el hash no nos garantiza nada sobre
el orden de sus elementos en el iterador! Así que tenemos que ordenar el arreglo que armamos nosotros. Se
creó un `quicksort` propio para reemplazar a `qsort`, con el comparador `strcmp`. Aunque en el peor caso
quicksort es O(N^2), normalmente podemos esperar una complejidad computacional del orden O(NlogN), y
de memoria constante (es in-place).

Una vez hecho todo ese lío podemos combinar todos los nombres en uno. Primero `malloc`'eamos la cantidad
de bytes exacta que necesitemos, incluyendo espacio para las comas y para el `\0`. Copy paste de a uno
y estamos.

### Carreras

Una vez seleccionado uno o más obstáculos, y teniendo un pokémon seleccionado también, el usuario del TP
puede calcular tanto el tiempo total del recorrido como los tiempos
individuales por obstáculo (esto útlimo nuevamente en formato string).

Para lograr este objetivo se utiliza el iterador interno de lista, y que vaya sumando uno por uno el tiempo que se toma el pokémon en atravesárlos. Se creo una estructura que actúa como contexto al recorrido:

```c
struct contexto_sumar_tiempo {
    struct pokemon_info *pokemon;
    size_t indice_obstaculo;
    unsigned *tiempos_por_obstaculo;
    unsigned tiempo_total;
    int valor_base;
    enum TP_OBSTACULO *obstaculo_anterior;
};
```

Lo único fijo durante el recorrido es el pokémon. Lo necesitamos saber porque el tiempo depende de sus atributos. Luego tenemos un índice, requerido para saber en qué posición de `tiempos_por_obstaculo` tenemos que escribir. A su vez, vamos sumando el tiempo total acá mismo (aunque se podría hacer más tarde tamién). Por último tenemos dos variables necesarias para el comportamiento de los pokémon cuando se encuentran muchos obstáculos seguidos del mismo tipo. Cada obstáculo suma un tiempo `valor base - valor atributo`, donde el valor base es 10 por defecto, y disminuye en 1 con cada obstáculo del mismo tipo seguido. Adicionalmente se verifica que este tiempo nunca sea negativo.

Si necesitamos el tiempo total, ya está calculado. Si necesitamos el string, debemos hacer un proceso muy similar al de los nombres disponibles. Todo este proceso es O(N), donde N es la cantidad de obstáculos.

---

## PokéRush

El TDA TP es solo una de las piezas necesarias para realizar un juego. El mismo en combinación con un conjunto de "escenas" y algún motor que permita realizar operaciones de input y output forman un ejecutable listo para ser jugado. El motor se explica más adelante.

PokéRush es un juego que se corre en la terminal y permite al jugador visualizar pokémon, elegir una dificultad, crear su pista de obstáculos y ver a los pokémon recorrer los obstáculos. Todo esto de una manera gráfica e interactiva mediante controles de teclado.

### Estructura principal

Pokerush tiene tres componentes principales en el código. Primero de todo está `pokerush.c` y `pokerush.h`, donde se define la estructura principal del juego:

```c
struct pokerush {
    void *escenario;
    struct pr_nombre_escena escena_actual;
    struct pr_escena escenas[PR_CANTIDAD_ESCENAS];
    struct pr_contexto contexto;
};
```

Parece simple, pero toda la complejidad está dentro de `contexto` y también en `escenario` (posiblemente, dependiendo de la escena).
