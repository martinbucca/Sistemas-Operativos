# FISOP-FS: Documentacion y detalles del diseño

## Estructuras de memoria que almacenan archivos, directorios y sus metadatos.

Decidimos implementar nuestro File System basandonos en la estrucura de un Very Simple File System (VSFS) utilizando inodos para representar archivos y directorios y teniendo un superbloque con bitmaps para ir marcando que bloques/inodos se encuentran en uso.

### Block
Decidimos guardar todos los datos propiamente dichos, correspondientes a los archivos en bloques de 4kb. Cada `struct block` va a contener hasta 4kb de informacion. Decidimos tener una cantidad de bloques proporcional a la cantidad de inodos que ibamos a tener. Como elegimos tener 250 inodos, y por el tamaño de cada bloque en el cual suponiendo que entran 16 inodos por bloque como en el VSFS (no es nuestro caso ya que no guardamos los inodos en bloques pero queriamos asimilarlo) la cantidad de bloques para guardar datos es de `4000`. Esto quiere decir que el tamaño de nuestro file system, por cuestiones de que no necesitamos que pueda guardar muchos archivos ni tampoco de gran tamaño, es de 4kb * 4000.

En un arreglo global de blocks `struct block data_region_blocks[AMOUNT_OF_BLOCKS]` se guardan todos los blqoues, los cuales son inicializados sin contenido y a medida que se van necesitando se guardan en estos la informacion de cada archivo. 

### Superblock
Para poder organizar y llevar de una manera mas simple el conteo de cuantos bloques/inodos tenemos en uso y que este toda la metadata que define al filesystem en una sola estructura, decidimos utilizar un superbloque. En este guardamos la cantidad de inodos y bloques que tiene nuestro filesystem. Tambien llevamos el registro de los inodos y bloques en uso en una estructura bitmap. Es simplemente un arreglo de `int` en donde en cada pocision puede haber un `FREE` (0) que representa que no esta en uso, o un `USED` (1) que representa que la posicion en el arreglo de inodos/bloques correspondiente a esa posicion esta en uso. Esta estructura es inicializada cuando abrimos el file system y leemos los datos persistidos de disco. 

### Inode
Esta estructura es fundamental ya que en nuestro filesystem representa a un archivo o a un directorio y contiene toda su metadata. Cuando se crea o borra un archivo/directorio en realidad se esta creando un inodo y se guarda en la lista de inodos `struct inode inodes[AMOUNT_OF_INODES]`. Decidimos que nuestro filesystem soporte un total de 250 inodos/archivos/directorios totales porque nos parecio un numero razonable, ni muy bajo ni tampoco muy alto. Esta estructura guarda en uno de sus campos cuantos bloques esta utilizando el archivo `blkcnt_t number_of_blocks`, en otro campo tiene un arreglo con 16 posiciones (maxima cantidad de bloques por inodo) en donde cada posicion representa un bloque del archivo. Empiezan todos vacios y a medida que se va escribiendo se van completando. Tambien nos vimos ante la necesidad de llevar otra lista con el indice en la lista de bloques de cada uno de los bloques guardados. Quizimos hacer un hashmap indice: bloque o una tupla, pero despues decidimos que lo mas facil y simple de implementar en C era otra lista del mismo tamaño y cada posicion se corresponderia a su poscion en la lista de punteros a bloques. Si en la lista de puntero a bloques no hubiera nada en una de las 16 posiciones, en la lista de indices tendria un 0. Es por eso que en la lista de bloques global `struct block data_region_blocks[AMOUNT_OF_BLOCKS]` nunca se usa el indice 0, ya que se usa para indicar en los inodos que un bloque de la lista de 16 posibles que puede tener el indodo, esta vacio.
Tambien decidimos que los indodos guarden el nombre del archivo o directorio que representan, para poder acceder a estos mas facil y decidimos poner una constante con el maximo largo de nombre que soporta nuestro filesystem. `MAX_NAME_LARGE 100`. Para diferenciar archivo regular de directorio, los inodos tienen un campo `type` que puede ser `DIR_T` (0) si representa un directorio o `REG_T` (1) si representa a un archivo regular.

## Cómo el sistema de archivos encuentra un archivo específico dado un path

Para eso decidimos implementar la funcion `get_inode_from_path()` la cual recorre cada elemento del bitmap de inodos, y si se encuentra ocupado se encarga de comparar el path con el inodo correspondiente a esa posicion. Si son iguales se devuelve el indice en la lista de inodos (mismo indice que en el bitmap) que se encuentra dicho inodo que represneta al archivo con ese path pasado por parametro. Si bien cada vez que queremos buscar un inodo dado un path tenemos que recorrer todo el bitmap (`O(n)`), nos parecio la manera mas sencilla de resolverlo.

## El formato de serialización del sistema de archivos en disco

En cuanto a la serializacion del filesystem decidimos escribir en crudo (bytes) toda la informacion relevante del estado del filesystem al momento de hacerlo. Primero escribimos el superbloque entero en bytes, despues los dos bitmaps, la lista de inodos y por ultimo la lista de bloques. A la hora de levantar el archivo se cargan las distintas estructuras en el orden especificado anteriormente (tiene que ser si o si en el orden en el que se escriben tambien) y se tiene el estado de las estructuras que manejan el filesystem previo. El archivo por defecto del que se va a a leer la informacion es `group26fs.fisopfs` y en caso de no existir se va a crear y escribir la informacion. Siempre que se presiste el filesystem se pisa todo el contenido anterior, de esta manera nos parecio mas facil implementarlo. 


# TESTS

Para correr los tests se necesitan **dos terminales**.

En la **primer terminal** se debe ejecutar:

`./mount.sh`: monta el filesystem en una carpeta llamada testsdir

en la **segunda terminal** se debe ejecutar:

`./fisopfs_tests.sh`: corre y muestra por terminal todos los tests de las funcionalidades pedidas del filesystem

**Tests para persistencia** luego de correr los tests anteriores:

En la **primer terminal** se debe ejecutar:

`./mount.sh -p`: monta el filesystem en una carpeta llamada testsdir y levanta el archivo que persiste los datos

en la **segunda terminal** se debe ejecutar:

`./fisopfs_tests.sh -p`: corre y muestra por terminal que los datos de la anterior corrida (en los tests anteriores al final se agregan coasas para este test) siguen estando.


