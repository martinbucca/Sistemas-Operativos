#!/bin/bash

cd testsdir
echo -e "\n\n"
echo "INICIO PRUEBAS CREACION DE DIRECTORIOS"
mkdir directorio1
echo se crea directorio1: mkdir directorio1
echo "--------------------------------------------------------"
echo "               ls muestra: archivo1, directorio1        "
echo "--------------------------------------------------------"
ls
cd directorio1
echo se cambia a directorio1: cd directorio1
mkdir directorio1.1
echo se crea directorio1.1: mkdir directorio1.1
echo "--------------------------------------------------------"
echo "               ls muestra: directorio1.1                "
echo "--------------------------------------------------------"
ls
