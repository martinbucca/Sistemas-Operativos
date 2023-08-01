#!/bin/bash

cd testsdir
echo -e "\n\n"
echo "INICIO PRUEBAS BORRADO DE DIRECTORIOS"
cd directorio1
echo se cambia a directorio1: cd directorio1
echo "-----------------------------------------------------------"
echo "           ls muestra: directorio1.1                       "
echo "-----------------------------------------------------------"
ls
rmdir directorio1.1
echo se borra direcotrio1.1: rmdir directorio1.1
echo "-----------------------------------------------------------"
echo "           ls muestra:                                     "
echo "-----------------------------------------------------------"
ls
cd ..
echo se cambia a diretorio padre: cd ..
echo "-----------------------------------------------------------"
echo "           ls muestra: directorio1                         "
echo "-----------------------------------------------------------"
ls
rmdir directorio1
echo se borra directorio1: rmdir directorio1
echo "-----------------------------------------------------------"
echo "           ls muestra:                                     "
echo "-----------------------------------------------------------"
ls

