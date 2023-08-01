#!/bin/bash

cd testsdir
echo -e "\n\n"
echo "INICIO PRUEBAS LECTURA DE DIRECTORIOS"
cd directorio1
echo se cambia a directorio1: cd directorio1
echo "--------------------------------------------------------"
echo "               ls muestra: directorio1.1                "
echo "--------------------------------------------------------"
ls
echo se hace ls ..
echo "--------------------------------------------------------"
echo "               ls muestra: archivo1, directorio1        "
echo "--------------------------------------------------------"
ls ..
echo se hace ls .
echo "--------------------------------------------------------"
echo "               ls muestra: directorio1.1                "
echo "--------------------------------------------------------"
ls . 
cd ..
echo se cambia a directorio padre: cd ..
echo "--------------------------------------------------------"
echo "               ls muestra: archivo1, directorio1        "
echo "--------------------------------------------------------"
ls
