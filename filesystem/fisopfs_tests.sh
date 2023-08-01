#!/bin/bash


# Si se proporciona el flag -p se corre test de persistencia.
if [ "$1" == "-p" ]; then
    chmod +x tests/persistence.sh
    ./tests/persistence.sh
else
    chmod +x tests/files_creations.sh
    ./tests/files_creations.sh
    chmod +x tests/dir_creations.sh
    ./tests/dir_creations.sh
    chmod +x tests/readdir.sh
    ./tests/readdir.sh
    chmod +x tests/write_files.sh
    ./tests/write_files.sh
    chmod +x tests/stat.sh
    ./tests/stat.sh
    chmod +x tests/remove_files.sh
    ./tests/remove_files.sh
    chmod +x tests/remove_dir.sh
    ./tests/remove_dir.sh
    cd testsdir
    echo -e "\nCREO ALGUNOS ARCHIVOS Y DIRECTORIOS PARA VERIFICAR LA PERSISTENCIA"
    echo creo directorio1, directorio2, archivo1, archivo2 y dentro de directorio1 creo archivo3 y directorio3
    mkdir directorio1
    mkdir directorio2
    touch archivo1
    touch archivo2
    cd directorio1
    mkdir directorio3
    touch archivo3
    echo "--------------------------------------------------------------------------"
    echo "ls muestra los mecionados arriba (de directorio1: archivo3 y directorio3) "
    echo "--------------------------------------------------------------------------"
    ls
    cd ..
    echo "---------------------------------------------------------------------------------"
    echo "ls muestra los mecionados arriba (directorio1, directorio2, archivo1 y archivo2) "
    echo "---------------------------------------------------------------------------------"
    ls
    cd ..
fi
sudo umount testsdir
rmdir testsdir