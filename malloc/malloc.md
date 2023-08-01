# CONSTANTES

MIN_SIZE: Como decia la consgina, decidimos que el tamaño minimo de una region sea de 256 bytes. Creemos que es un tamaño adecuado para el cual no se desperdicia memoria y al mismo tiempo no se hace splitting todo el tiempo no haya tanta fragmentacion.

SMALL_BLOCK_SIZE, MEDIUM_BLOCK_SIZE, BIG_BLOCK_SIZE: tamaño de los distintos bloques que se manejan en nuestra libreria malloc. El tamaño del bloque pequeño es de 16Kib, el del bloque mediano 1Mib y el del bloque grande 32Mib.

MAX_MEMORY: Decidimos que el tamaño maximo que se le puede pedir a la libreria es de 2GB en total, es decir sumando todos los pedidos de malloc hechos. Creemos que al ser una libreria malloc `estandar` es una cantidad adecuada de memoria que puede administrar. Ya que no tiene un uso especifico el valor no deberia ser ni muy grande ni muy poco.

MAX_AMOUNT_OF_SMALL_BLOCKS, MAX_AMOUNT_OF_MEDIUM_BLOCKS, MAX_AMOUNT_OF_BIG_BLOCKS: Dado que en la consgina se pide `soporte para mantener una cantidad arbitraria de cada tipo de bloque`, decidimos que en proporcion del tamaño de cada bloque se pueda tener la "misma cantidad" (la cantidad total dividido los tres tipos de bloques) de cada tipo de bloque. Al ser una libreria `estandar`, no se puede saber de que tamaño van a ser los pedidos de memoria de los usarios, por lo tanto lo mejor es dividirlo equitativamente. Si la memoria total de la libreria es `2 GB`, a cada tipo de bloque le va a corresponder `0.66 GB` por lo tanto, dividiendo esta cantidad por el tamaño de los bloques (redondeando para abajo) se obtiene la cantidad maxima de cada uno.

# ESTRUCTURAS Y REGION

Decidimos manejar todas las distintas estructuras como `regiones`. Cada Struct Region contiene los metadatos de la
region de memoria. Tiene informacion sobre si esta ocupada o no (`free`), el tamaño de la region (`size`), un puntero a la siguiente region (`next`), en caso de representar un bloque un puntero al proximo bloque (`next_block`) y el puntero a memoria que se le devuelve al usuario (`ptr`).
Los `bloques son regiones` que van a estar apuntando al proximo bloque en caso de tenerlo, para formar listas enlazadas de bloques. A su vez un bloque va a tener una siguiente region que va a tener otra y forma una lista enlazada de regiones dentro del bloque. De esta manera podemos abstraernos de la idea de bloque y pensarla como una region de gran tamaño que va a ser dividida a medida que se vayan haciendo mallocs (splitting) y formando las distintas regiones dentro de este.
A su vez mantenemos `3 listas que apuntan al primer bloque de cada tipo` (chico, mediano, grande) para poder recorrer los bloques y regiones desde este puntero que apunta al primer bloque.

# MALLOC

Al hacer un malloc lo primero que se chequea es si el tamaño de la memoria que se pide es menor al minimo tamaño de una region, si es asi, se actualiza este tamaño para devolverle al usurio este minimo tamaño (256B). Lo segundo que se chequea es si se pide un tamaño invalido (mas grande que el bloque grande o 0) y si es valido se calcula a que tamaño de bloque correspondera en caso de necesitar pedir uno nuevo porque no hay regiones libres. Lo siguiente que se hace es buscar una region libre utilizando FF o BF. Si no se encuentran regiones libres se pide un nuevo bloque con `mmap` (funcion que devuelve NULL si no se puede pedir un nuevo bloque porque se supera la cantidad maxima de bloques de ese tipo o de memoria total). Luego se fija si la region es muy grande en comparacion a lo que pidio el usuario y la divide en dos regiones (`splitting`), devolviendole así una region al usuario acorde a lo que pidio y evitando dar memoria de más al usuario que no pidio. Se actualizan las estadisticas (`amount_of_mallocs` y `requested_size`) y se devuelve el puntero al usuario.

# FREE

Si se le pasa NULL a free se espera que no haga nada (comportamiento de la libreria real malloc). Se obtiene la region y se busca a que bloque y que lista de bloque se encuentra el puntero pasado a liberar. Si no se encuentra en ningun bloque la region, no hace nada. Se lo marca como libre y se fija si en ese bloque hay dos regiones continuas libres y las une (`coalescing`). Por ultimo se fija si el bloque entero esta libre (contiene una unica region libre que engloba al bloque entero) y devuelve la memoria con `munmap` y actualiza la lista de bloques del bloque que se libero y actuliza las estadisticas (`amount_of_frees` y `requested_size`). Si se intenta liberar un puntero invalido o que ya fue liberado, al igual que la libreria real de malloc, el comportamiento no esta definido y es posible que se devuelva un `segmentation fault` (o algun otro error), dado que se puede estar accediendo a memoria que ya fue previamente devuelta con  `munmap`. Por eso el usuario debe tener cuidado de
no estar liberando un puntero que no es valido o que ya fue liberado previamente.

# CALLOC

Llama a malloc con el tamaño total de memoria a reservar (`nmemb * size`) y malloc se encarga de validar que este tamaño se pueda reservar y le devuelve la region correspondiente. En caso de ser NULL, se devuelve NULL (el `errno = ENOMEM` se actualiza dentro de malloc) y en caso contrario se settean todos los bytes en 0 con `memset` y se devuelve el puntero.

# REALLOC

Si el puntero que se le pasa para realocar es NULL, su comportamiento va a ser igual al `malloc` y por lo tanto va a devolver lo que haga malloc con el tamaño pasado. En el caso de que el size sea igual a 0 y el puntero no sea NULL, va a liberar el puntero llamando a `free`. Se verifica que la region suministrada fue previamente pedida con malloc y no es un puntero invalido y siempre que falle el realloc, la region original no es modificada ni liberada (se setea el `errno = ENOMEM`). Si la region es mas pequeña que la region actual, se actualiza el tamaño de la region y se splittea si es necesario. Se verifica si con la nueva region splitteada no se puede unir a otra siguiente a esta que este libre (coealescing) y se devuelve el mismo puntero. En caso de que el tamaño pedido sea mayor al tamaño de la region actual se verifica que no haya una region siguiente a esta que esta vacia y uniendola con esta satisfaga el pedido, en dicho caso se unen ambas regiones y se devuelve el mismo puntero a una a la misma region pero con un nuevo tamaño adecuado. En caso de que el tamaño pedido sea mayor a la region actual y no haya una region siguiente a esta que este libre y uniendolas satisfagan el pedido, se devuelve una nueva region que cumpla con el tamaño, llamando a `malloc` y copiando el contenido con  `memcpy` en esta nueva region. Se libera la region de donde se copio el contenido y se devuelve el nuevo puntero.


