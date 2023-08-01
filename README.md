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
