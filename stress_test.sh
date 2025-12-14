#!/bin/bash

# Nombre del ejecutable y archivos auxiliares
MYSHELL="./myshell"
INPUT_FILE="input_stress.txt"

# 1. Compilación (por si acaso)
echo "🔧 Compilando myshell..."
gcc -Wall -Wextra myshell.c libparser.a -o myshell -static
if [ $? -ne 0 ]; then
    echo "❌ Error de compilación. Abortando."
    exit 1
fi

echo "✅ Compilación correcta."
echo "📝 Generando archivo de pruebas masivas ($INPUT_FILE)..."

# --- GENERACIÓN DE COMANDOS PARA MYSHELL ---

# Limpiar archivo anterior
> $INPUT_FILE

# PRUEBA 1: LLENADO DE LA TABLA DE JOBS (STRESS TEST)
# Tu límite es 100. Vamos a intentar lanzar 105 para ver si los últimos 5 dan error.
echo "echo '--- INICIO PRUEBA 1: SATURACION DE JOBS (105 intentos) ---'" >> $INPUT_FILE
for i in {1..105}; do
    # Lanzamos sleeps largos en background
    echo "sleep 10 &" >> $INPUT_FILE
done

# Listamos para ver si la tabla está llena (Debería haber 100 Running)
echo "echo '--- Verificando tabla llena (Deberia haber 100 jobs) ---'" >> $INPUT_FILE
echo "jobs" >> $INPUT_FILE

# PRUEBA 2: LIMPIEZA AUTOMÁTICA
# Esperamos un poco para que terminen (usamos un sleep en foreground para bloquear la shell)
echo "echo '--- Esperando 12 segundos para que terminen los jobs... ---'" >> $INPUT_FILE
echo "sleep 12" >> $INPUT_FILE

# Al pulsar Enter (o siguiente comando), el recolector debería limpiar todo
echo "echo '--- Verificando limpieza (La tabla deberia estar vacia) ---'" >> $INPUT_FILE
echo "jobs" >> $INPUT_FILE

# PRUEBA 3: TUBERÍA GIGANTE (PIPES STRESS)
# Probamos si tu gestión de descriptores aguanta un pipe de 50 comandos
echo "echo '--- INICIO PRUEBA 3: MEGA PIPE (50 procesos) ---'" >> $INPUT_FILE
PIPE_CMD="ls -l"
for i in {1..50}; do
    PIPE_CMD="$PIPE_CMD | cat"
done
# Al final contamos líneas para verificar que el flujo de datos no se rompió
PIPE_CMD="$PIPE_CMD | wc -l"

echo "$PIPE_CMD" >> $INPUT_FILE

# PRUEBA 4: INTEGRIDAD DE FD Y REDIRECCIONES
# Creamos un archivo, escribimos, leemos con pipe y guardamos
echo "echo '--- INICIO PRUEBA 4: REDIRECCIONES + PIPE ---'" >> $INPUT_FILE
echo "ls -l > stress_test_file.txt" >> $INPUT_FILE
echo "grep myshell < stress_test_file.txt | wc -l" >> $INPUT_FILE

# Salir
echo "exit" >> $INPUT_FILE

# --- EJECUCIÓN ---

echo "🚀 Lanzando myshell con el set de pruebas..."
echo "---------------------------------------------------"
# Redirigimos el archivo como entrada estándar de tu shell
$MYSHELL < $INPUT_FILE
echo "---------------------------------------------------"

# Limpieza
rm $INPUT_FILE stress_test_file.txt
echo "🏁 Prueba finalizada."