# Minishell 🚀

Un intérprete de mandatos simple tipo shell implementado en C para la asignatura de Sistemas Operativos en la URJC.

<p align="center">
  <img alt="Language" src="https://img.shields.io/badge/Language-C-blue?style=for-the-badge&logo=c">
  <img alt="Platform" src="https://img.shields.io/badge/Platform-Linux-yellow?style=for-the-badge&logo=linux">
  <img alt="License" src="https://img.shields.io/badge/License-Academic-brightgreen?style=for-the-badge">
</p>

---

## 📋 Tabla de Contenidos

* [Características Principales](#características-principales)
* [Mandatos Internos](#mandatos-internos)
* [Compilación y Ejecución](#compilación-y-ejecución)
* [Estructura del Proyecto](#estructura-del-proyecto)
* [Autores](#autores)

---

## ✨ Características Principales

Esta `minishell` soporta las funcionalidades clave de un intérprete de mandatos moderno:

* **Ejecución de Mandatos:** Ejecuta cualquier programa externo del sistema (como `ls`, `grep`, `awk`, `find`...) con sus argumentos.
* **Tuberías (Pipes):** Encadena múltiples mandatos. La salida de un mandato es la entrada del siguiente.
    ```bash
    msh> ls -l | grep ".c" | wc -l
    ```
    
* **Redirecciones:**
    * `< fichero`: Redirección de entrada estándar.
    * `> fichero`: Redirección de salida estándar.
    * `>& fichero`: Redirección de la salida de error estándar.
    ```bash
    msh> sort < fichero_in.txt > fichero_out.txt
    ```
    
* **Procesos en Background:** Ejecuta trabajos en segundo plano usando `&`, permitiendo al usuario seguir usando la shell.
    ```bash
    msh> find / -name "core" &
    [1] 12345
    ```
* **Gestión de Señales:**
    * `SIGINT (Ctrl+C)`: Es ignorada por la minishell y los procesos en background, pero termina los procesos en foreground.
    * `SIGTSTP (Ctrl+Z)`: Es ignorada por la minishell, pero detiene los procesos en foreground para su posterior gestión.

---

## 🔧 Mandatos Internos

He implementado varios mandatos *built-in* que se ejecutan directamente en la shell sin crear un nuevo proceso:

| Mandato | Descripción |
| :--- | :--- |
| **`cd [dir]`** | Cambia el directorio de trabajo actual. Si no se usa argumento, va a `$HOME`. |
| **`exit`** | Termina la ejecución del minishell de forma limpia. |
| **`jobs`** | Muestra la lista de trabajos en background o detenidos. |
| **`bg [job_id]`**| Reanuda un trabajo detenido (`Stopped`) en segundo plano. |
| **`umask [octal]`** | Establece o muestra la máscara de permisos para la creación de nuevos ficheros. |

---

## ⚙️ Compilación y Ejecución

### Requisitos
* Compilador `gcc`
* `make` (opcional, pero recomendado)
* Librería `parser` (proporcionada por la cátedra)

### Pasos de Instalación

1.  **Clona el repositorio:**
    ```bash
    git clone [https://github.com/](https://github.com/)[TuUsuario]/[TuRepo].git
    cd [TuRepo]
    ```

2.  **Compila el proyecto:**
    (Ajusta esta línea según cómo enlaces la librería `parser`. Si tienes un `Makefile`, ¡mejor!)
    ```bash
    gcc -o myshell myshell.c parser/parser.c -Wall -Wextra
    ```
    *Si usas un Makefile:*
    ```bash
    make
    ```

3.  **Ejecuta la Minishell:**
    ```bash
    ./myshell
    ```
    Y verás el prompt:
    ```
    msh>
    ```

---

## 📁 Estructura del Proyecto


---

## 👨‍💻 Autores

* **Pablo Ruiz Uroz**
    * GitHub: `[@TuUsuarioDeGitHub]` (Reemplaza esto)
    * Email: `p.ruizu.2023@alumnos.urjc.es`
* **Hugo Capa Mora**
    * GitHub: `[@UsuarioCompañero]` (Reemplaza esto)
    * Email: `h.capa.2023@alumnos.urjc.es`
(Esta es una estructura de ejemplo basada en tu comando de compilación. ¡Ajústala a tu proyecto real!)
