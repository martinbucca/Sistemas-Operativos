# shell

### Búsqueda en $PATH

La principal diferencia es que exec(3) es una familia de wrappers definida en la libreria estandar de C que se encargan de ejecutar la syscall execve(2) pero hacen otras tareas antes y despues de ejecutar esta syscall. Por ejemplo pueden expandir variables de entorno y luego llamar a execve(2) pasandole la lista de variables de entorno como tercer argumento, en cambio a execve(2) hay que pasarle un tercer argumento con las variables de entorno "a mano" y habria que expandir las varibles de entorno.

La llamada a exec(3) puede fallar ya sea porque la syscall execve(2) fallo o porque alguna de las otras tareas que realiza no haya sido exitosa. La implementacion de la shell en el caso de que la llamada a exec(3) falle muestra un mensaje de error "No se pudo ejecutar el comando" y termina el programa con exit(-1).

### Procesos en segundo plano

El mecanismo para implementar procesos en segundo plano fue el siguiente:
Si estoy en el proceso padre espero al proceso hijo para que no quede
zombie, pero le agrego el flag WNOHANG para que el proceso
padre no quede bloqueado y siga ejecutandose mientras espera al hijo. Esto lo que hace
es devolverle el prompt de la shell para poder ser utilizado en otro comando. De esta
forma se puede correr un proceso en segundo plano.

### Flujo estándar

El significado de 2>&1 es que el file descriptor 2 (stderr) se redirija al file descriptor
1 (stdout). En su forma general, si hay una salida por stderr, esta se va a mandar a el stdout pero sin afectar
el contenido que se manda a stodut.

En el ejemplo la salida de cat out.txt muestra lo que sale por stderr que es redirgido a stodut (a su vez es redirigido a out.txt). En nuestro caso el archivo out.txt muestra lo siguiente:
ls: /noexiste: No such file or directory
ls: home: Operation not permitted
/home:

Si el orden es invertido, no cambia en nada lo que se muestra en out.txt ya que stderr es redirigido a stodut y a su vez este a out.txt

En bash(1) el comportamiento es igual, teniendo que hacer >word 2>&1 para redirigir stderr y stdout a un archivo cuyo nombre es "word". Tiene dos maneras mas de hacerlo, que son: &>word y >&word (aunque la primera es preferible).

### Tuberías múltiples

Si cambia con respecto a los otros tipos de comando. En caso de que se ejecute un pipe el exit code reportado por la shell no sera mostrado al igual que cuando no se escribe ningun comando. La funcion print_status (printstatus.c) tiene una condicion que si el tipo de comando es PIPE, no muestra el status del programa.

Si hay un error en alguno de los comandos ya sea porque no existe o porque falla, se muestra el/los error/es y no se ejecuta ninguno de los comandos que esten bien. Como cada comando recibe por entrada la salida del comando anterior, si uno falla afectara al resto. En nuestra implementacion, funciona de la misma manera. La salida en la terminal bash es la siguiente:

ls -l | grep Doc | ls /noexiste
salida:
ls: cannot access '/noexiste': No such file or directory

sd | grep Doc | ls /noexiste
salida:
ls: cannot access '/noexiste': No such file or directory
sd: command not found

### Variables de entorno temporarias

Es necesario hacerlo despues de la llamada a fork(2) porque al ser variables de entorno temporales queremos que solo existan y puedan ser soportadas dentro del proceso en el que se esta ejecutando el comando y no en el proceso de la shell.

El comportamiento no necesariamente es el mismo. En el caso en que a cada variable se la setea usando la funcion setenv(), las variables de entorno temporales estaran disponibles durante todo el proceso mientras este se este ejecutando. En el segundo caso puede depender de como este implementada la funcion que se utiliza y a la que se le pasa por tercer argumento las variables de entorno, si es que se ocupa de setearlas para poder accederlas en ese mismo proceso o no. Una posible implementacion para que el comportamiento sea el mismo podria ser que por cada variable de entorno en la lista, sea seteada antes de ejecutar el proceso del comando. De esta manera van a poder ser accedidas por el proceso sin importar que funcion de la familia exec(3) se este usando.

### Pseudo-variables

$! --> Guarda el ID del ultimo proceso en segundo plano (BACK) que se haya ejecutado.

sleep 3 &
echo "$!"
99042

$$ --> Guarda el ID del proceso actual.

echo "$$"
98044

$0 --> Guarda el nombre del comando que se esta ejecutando.

echo "$0"
bash

### Comandos built-in

cd se podria implementar sin necesidad de que sea un builtin ya que se puede utilizar la funcion chdir() en el proceso en el que se esta ejecutando el comando, pero el motivo de hacerlo como builtin es la necesidad de actualizar el prompt con el nombre del directorio al que se accedio. Solo se actualizaria el directorio en el proceso en el que se ejecuto el comando y no en el proceso que ejecuta a la shell (no tendria efecto en esta). Tambien es mas rapido y facil, ya que no es necesaria la creacion de otro proceso para ejecutar el comando.

### Historial

El paraemtro MIN del modo no canonico es la cantidad minima de caracteres que se deben leer para que la entrada sea valida. El parametro TIME es el tiempo maximo en que read() va a esperar para leer mas caracteres despues de leer el numero minimo de caracteres. En el ejemplo dado al establecer MIN en 1 se logra que por lo menos se ingrese un caracter para que la entrada sea valida y al establecer TIME en 0 se logra que el tiempo de espera entre que lee un caracter ingresado y el otro es de 0.