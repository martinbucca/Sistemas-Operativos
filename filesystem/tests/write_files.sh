#!/bin/bash

cd testsdir
echo -e "\n\n"
echo "INICIO PRUEBAS ESCRITURA DE ARCHIVOS"
echo "--------------------------------------------------------"
echo "stat de archivo 1 muestra que el tamaño del archivo es 0"
echo "--------------------------------------------------------"
stat archivo1
echo hola mundo! >> archivo1
echo se escibe el archivo1: con echo y dirigiendo hacia archivo1
echo "-----------------------------------------------------------"
echo "stat de archivo 1 muestra que el tamaño del archivo cambio "
echo "-----------------------------------------------------------"
stat archivo1
echo "-----------------------------------------------------------"
echo "           ls muestra: archivo1, directorio1               "
echo "-----------------------------------------------------------"
ls
echo creo nuevo archivo > archivo2
echo se crea y escirbe nuevo archivo: archivo2
echo "-----------------------------------------------------------"
echo "           ls muestra: archivo1, directorio1, archivo2     "
echo "-----------------------------------------------------------"
ls
echo "--------------------------------------------------------------------"
echo "stat de archivo 2 muestra que el archivo fue escrito cuando se creo "
echo "--------------------------------------------------------------------"
stat archivo2
