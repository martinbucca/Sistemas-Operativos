#!/bin/bash

cd testsdir
echo -e "\n\n"
echo "INICIO PRUEBAS ESTADISTICAS DE ARCHIVOS (stat)"
echo se muestran las estadisticas del archivo1: stat archivo1
echo "--------------------------------------------------------"
echo "stat de archivo 1 muestra correctamente sus estadisticas"
echo "--------------------------------------------------------"
stat archivo1
echo se muestran las estadisticas del archivo2: stat archivo2
echo "--------------------------------------------------------"
echo "stat de archivo 2 muestra correctamente sus estadisticas"
echo "--------------------------------------------------------"
stat archivo2
