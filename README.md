# Sistemas-Operativos
TPs realizados para la materia Sistemas Operativos - Facultad de Ingenieria de Buenos Aires
## Lab Fork
El objetivo de este lab es familiarizarse con las llamadas al sistema `fork(2)` (que crea una copia del proceso actual) y `pipe(2)` (que proporciona un mecanismo de comunicación unidireccional entre dos procesos).
## TP Shell
El objetivo de este tp es desarrollar la funcionalidad mínima que caracteriza a un intérprete de comandos shell similar a lo que realizan bash, zsh, fish.
1. Invocacion de comandos:
   * Busqueda en $PATH
   * Procesos en segundo plano
2. Redirecciones:
   * Flujo estandar
   * Tuberías simples
   * Tuberías múltiples
3. Variables de entorno:
   * Expansion de variables
   * Variables de entorno temporales
   * Pseudo-variables
4. Comandos built-in:
   * cd
   * exit
   * pwd
5. Historial
   * Built-in `history`
   * opcion `n` para mostrar ultimos `n` comandos
## TP Malloc
El objetivo de este tp es desarrollar una librería de usuario que implemente las funciones `malloc(3)`, `calloc(3)`, `realloc(3)` y `free(3)`. La librería se encargará de solicitar la memoria que requiera, y la administrará de forma transparente para el usuario. Además debería poder utilizarse de forma normal en cualquier programa de C.
## TP Sched
El objetivo de este tp es implementar el mecanismo de cambio de contexto para procesos y el scheduler (i.e. planificador) sobre un sistema operativo preexistente. El kernel a utilizar será una modificación de `JOS`, un exokernel educativo con licencia libre del grupo de Sistemas Operativos Distribuidos del MIT.
`JOS` está diseñado para correr en la arquitectura Intel x86, y para poder ejecutarlo utilizaremos `QEMU` que emula dicha arquitectura.
1. Cambio de contexto:
   * Modo Usario a Modo Kernel
   * Modo Kernel a Modo Uuario
2. Scheduler Round-Robin
3. Scheduler con prioridades
## TP Filesystem
El obejetivo de este tp es implementar un sistema de archivos propio (o filesystem) para `Linux`. El sistema de archivos utilizará el mecanismo de `FUSE` (Filesystem in USErspace) provisto por el kernel, que permitirá definir en modo usuario la implementación de un filesystem. Gracias a ello, el mismo tendrá la interfaz VFS y podrá ser accedido con las syscalls y programas habituales (read, open, ls, etc). La implementación del filesystem es enteramente en memoria: tanto archivos como directorios son representados mediante estructuras que viven en memoria dinámica. Por esta razón, es un sistema de archivos que apunta a la velocidad de acceso, y no al volumen de datos o a la persistencia. Aún así, los datos de nuestro filesystem estarán representados en disco por un archivo.
1. Representacion del sistema de archivos (inodos, bloques)
2. Persistencia en disco
3. Tests y salidas de ejemplo

### Consignas
* [Fork](https://fisop.github.io/website/labs/fork/)
* [Shell](https://fisop.github.io/website/tps/shell/)
* [Malloc](https://fisop.github.io/website/tps/malloc/)
* [Sched](https://fisop.github.io/website/tps/sched/)
* [Filesystem](https://fisop.github.io/website/tps/filesystem/)
