# Minishell en C

Este proyecto es una implementación de un intérprete de mandatos simple (minishell) desarrollado en C como práctica para la asignatura de **[Sistemas Operativos]** en la **[URJC]**.

El objetivo principal es crear un shell capaz de leer, analizar y ejecutar mandatos introducidos por el usuario, gestionando procesos, tuberías (pipes) y redirecciones.

---

## 🚀 Características Implementadas

Mi minishell (`msh>`) soporta las siguientes funcionalidades:

* **Ejecución de mandatos:** Ejecuta programas externos (como `ls`, `grep`, `cat`, etc.) con sus argumentos.
* **Tuberías (Pipes):** Permite encadenar múltiples mandatos con el operador `|`.
    * `ejemplo: ls -l | grep ".c" | wc -l`
* **Redirecciones:**
    * `< fichero`: Redirección de entrada (solo en el primer mandato).
    * `> fichero`: Redirección de salida (solo en el último mandato).
    * `>& fichero`: Redirección de la salida de error (solo en el último mandato).
* **Ejecución en Background:** Permite ejecutar trabajos en segundo plano usando el operador `&` al final de la línea.
* **Gestión de Señales:**
    * **SIGINT (Ctrl-C):** Los procesos en *foreground* terminan, pero el minishell y los procesos en *background* la ignoran.
    * **SIGTSTP (Ctrl-Z):** El minishell la ignora, pero los procesos en *foreground* se detienen.

---

## 🔧 Mandatos Internos (Built-ins)

Además de ejecutar programas externos, el minishell incluye los siguientes mandatos internos:

* `cd [directorio]`
    Cambia el directorio de trabajo actual. Si no se proporciona un argumento, cambia al directorio `HOME` del usuario. Muestra la nueva ruta absoluta.

* `exit`
    Termina la ejecución del minishell de forma ordenada.

* `jobs`
    Muestra la lista de trabajos que se están ejecutando en segundo plano o que han sido detenidos (por `Ctrl-Z`).

* `bg [job_id]`
    Reanuda la ejecución en *background* de un trabajo que estaba detenido. Si no se especifica `job_id`, se reanuda el último trabajo detenido.

* `umask [modo_octal]`
    Establece o consulta la máscara de permisos por defecto para la creación de nuevos ficheros. Si se ejecuta sin argumentos, muestra la máscara actual en formato octal.

---
