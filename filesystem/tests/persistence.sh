#!/bin/bash

cd testsdir
echo -e "\n\n"
echo "INICIO PRUEBAS CREACION DE ARCHIVOS"
echo "-------------------------------------------------------------"
echo "  ls muestra: directorio1, directorio2, archivo1, archivo2   "
echo "-------------------------------------------------------------"
ls
echo se cambia a directorio1: cd directorio1
cd directorio1
echo "-------------------------------------------------------------"
echo "  ls muestra: directorio3, archivo3                          "
echo "-------------------------------------------------------------"
ls
cd ..
cd ..