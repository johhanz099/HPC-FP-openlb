#!/usr/bin/env bash
PROG=./cylinder2d
ARGS="--use-hwthread-cpus" # Procesadores logicos cómo slots
CORES=$(nproc) 

echo "Detectados $CORES procesadores lógicos."

#Crear carpeta de salida
mkdir -p data

# Cabecera de CSV
echo "cores,rt_s" > data/timings.csv

for p in $(seq 1 $CORES); do
  LOG=run_p${p}.log
  echo "Ejecutando con $p procesos..."
  mpirun $ARGS -np $p $PROG > data/$LOG 2>&1
  rt=$(grep "measured time (rt)" data/$LOG \
       | awk '{ print $6 }' \
       | tr -d 's')
  echo "$p,$rt" >> data/timings.csv
done
