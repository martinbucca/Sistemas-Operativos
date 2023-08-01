#!/bin/bash

cd testsdir
echo -e "\n\n"
echo "INICIO PRUEBAS BORRADO DE ARCHIVOS"
echo "-----------------------------------------------------------"
echo "           ls muestra: archivo1, directorio1, archivo2     "
echo "-----------------------------------------------------------"
ls
unlink archivo1
echo "borrado de archivo1 con unlink"
echo "-----------------------------------------------------------"
echo "           ls muestra: directorio1, archivo2     "
echo "-----------------------------------------------------------"
ls
rm archivo2
echo "borrado de archivo2 con rm"
echo "-----------------------------------------------------------"
echo "           ls muestra: directorio1                         "
echo "-----------------------------------------------------------"
ls

