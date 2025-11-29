#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include "parser.h"

#define MAX_LINE_BUFFER 1024

void aux_redirect_file(char *filename, int target_fd, int flags);
void redirect(tline * line, int index);
int check_if_internal(tline *line);
void execute_internal_cd(tline *line);
void execute_internal_umask(tline *line);
void execute_internal_exit();

int main(void) {
	char buf[MAX_LINE_BUFFER];
	tline * line;
    pid_t pid;
    int status;
    int i;
    int last_pipe_fd;
    int p[2];

	printf("msh> ");	
	while (fgets(buf, MAX_LINE_BUFFER, stdin)) {
		
		line = tokenize(buf);
		if (line==NULL) {
            printf("msh> ");
			continue;
		}

        if (check_if_internal(line)) {
            printf("msh> ");
            continue;
        }
        
        last_pipe_fd = -1;
        for ( i = 0; i< line -> ncommands; i++){
            //Creacion de la pipe --> Solo necesario si no soy el ultimo mandato
            if( i < line-> ncommands-1 ) {
               if (pipe(p) < 0) {
                    fprintf(stderr, "Error al crear pipe: %s\n", strerror(errno));
                    exit(1);
                }
            }

            pid = fork();

            if ( pid < 0){
                 fprintf(stderr,"Error a la hora de hacer fork: %s \n",strerror(errno));
            }
            //Soy Proceso Hijo
            else if (pid == 0)
            {
                //Si last pipe tiene valor signifca que tengo que recibir la entrada desde la pipe
                //Apunto el extremo de lecuta del pipe que ha guardado el padre a mi entrada estandar
                if (last_pipe_fd != -1){
                    dup2(last_pipe_fd,STDIN_FILENO);
                    close(last_pipe_fd);
                }

                //Conectar la salida apuntando el lado de escrtiura de mi pipe a mi salida estandar: Si no soy el ultimo tengo que escribir en el pipe
                if( i < line -> ncommands-1){
                    close(p[0]);
                    dup2(p[1],STDOUT_FILENO);
                    close(p[1]);
                }
                
                //Gestiono las redirecciones si es el caso
                redirect(line,i);

                if (line->commands[i].filename == NULL) {
                    fprintf(stderr, "mandato: No se encuentra el mandato\n"); // 
                    exit(1);
                }
                //Ejecuto el comando
                execvp(line->commands[i].filename, line->commands[i].argv);
                
                // Si llegamos aquí, execvp falló
                fprintf(stderr, "mandato: No se encuentra el mandato\n");
                exit(1);
            }
            //PADRE
            else{
                //Si cuando entramos al padre ya hay asignado un pipe significa que ya estamos en el 2 hijo minimo
                //Por lo que me cierro el antiguo descriptor que tenia asociado para liberar lod fd
                //Si no lo cerrara como padre tendria muchisimos descriptores abiertos en mi tabla
                if (last_pipe_fd != -1){
                    close(last_pipe_fd);
                }

                //Si no estamos en el ultimo paso
                //tengo que guardar el extremo de la pipe de lectura para el siguiente hjo que se crerara en el i++
                //Y cierro el extremo de escritura para que cuando lea el hijosig sepa que ya no hay mas que leer
                //Porque tmb lo cerrara el hijo por su cuenta 
                if (i < line->ncommands - 1) {
                    last_pipe_fd = p[0];
                    close(p[1]);
                }
            }
        }
        // Esperar a todos los hijos creados en el bucle
        for (i = 0; i < line->ncommands; i++) {
            wait(&status);
        }
        printf("msh> ");			
    }
}



//Metodo para abstraer la logica de redirecciones y limpiar el codigo
void redirect(tline * line, int index) {

    //La redireccion de entrada solo se aplica si es el primer mandato
    if (index == 0 && line->redirect_input != NULL) {
        //Cogemos el fichero de redireccion de entrada y lo abrimos en modo solo Lecutura, esto devueleve un fd nuevo de la tabla de fd , que usaremos para asociar despues
        //Asociamos el descriptor otorgado al fichero de texxto abbierto a la entrada estandar para que los comando lo lean
        //Esto cierra el fileDescriptor de stdin
        aux_redirect_file(line->redirect_input, STDIN_FILENO, O_RDONLY);
    }

    //Stdout -- Salida estandar
    // Sólo puede realizarse sobre el último mandato, último mandato es aquel cuyo índice es (ncommands - 1)
    if (index == line->ncommands - 1 && line->redirect_output != NULL) {
        // O_CREAT: Si no existe, créalo.
        // O_TRUNC: Si existe, vacíalo.
        // O_WRONLY: Ábrelo para escribir.
        // 0666: Permisos por defecto si hay que creralo
        //Asociamos el descriptor del fichero de salida a salida estandar
        aux_redirect_file(line->redirect_output, STDOUT_FILENO, O_CREAT | O_TRUNC | O_WRONLY);
    }

    // Stderr - Salida estandar de error
    // Sólo puede realizarse sobre el último mandato
    if (index == line->ncommands - 1 && line->redirect_error != NULL) {
        aux_redirect_file(line->redirect_error, STDERR_FILENO, O_CREAT | O_TRUNC | O_WRONLY);
    }
}


// Función auxiliar para no repetir código
void aux_redirect_file(char *filename, int target_fd, int flags) {
    int fdaux;
    
    // 0666 se usa si hay O_CREAT, si no, se ignora.
    fdaux = open(filename, flags, 0666);
    
    if (fdaux < 0) 
    // si es <0 da error
    {
        fprintf(stderr, "%s: Error. %s\n", filename, strerror(errno)); // 
        exit(1);
    }
    
    dup2(fdaux, target_fd);
    close(fdaux); // Necesario cerrarlo ya que no se va a usar
}

int check_if_internal(tline *line)
{
    if (line->ncommands == 0 || line->commands[0].argv[0] == NULL) 
        return 0;

    if (strcmp(line -> commands[0].argv[0], "cd") == 0)
    {
        execute_internal_cd(line);
        return 1;

    }

    if (strcmp(line -> commands[0].argv[0],"exit") == 0)
    {
        execute_internal_exit();
        return 1;

    }

   if (strcmp(line -> commands[0].argv[0],"umask") == 0)
    {
        execute_internal_umask(line);
        return 1;

    }

    return 0;
}

void execute_internal_cd(tline *line)
{
    char cwd_buff[MAX_LINE_BUFFER];
    char *target; //puntero para almacenar la ruta de destino
    int available = 1; //para llevar la lógica de si se podrá ejecutar o no

    if (line -> ncommands > 1 )
    {
        fprintf(stderr,"Error no se puede usar CD con mas comandos\n");
        return;
    }      
    //caso 1: no argumentos así que vamos a home
    if(line -> commands[0].argc == 1){
        target = getenv("HOME");
        if(target == NULL)
        {
            fprintf(stderr,"cd: HOME no existe la variable\n");
            available = 0;
        }
    }else if(line->commands[0].argc == 2)
    { //caso 2: que me venga un argumento que sería la ruta relativa o absoluta
        target = line->commands[0].argv[1];
                
    }else
    { //si no es ningún caso de los anteriores el mandato está mal llamado
        fprintf(stderr,"cd: demasiados argumentos\n");
        available = 0;
            
    }
    if(available && target != NULL)
    { //pasamos a ejecutar el cd
        if(chdir(target) == -1)
        { //caso error
            fprintf(stderr,"Error en el cd: %s \n", strerror(errno));
        }else{
            //almacenamos la cadena que representa la ruta absoluta del directorio actual
            if(getcwd(cwd_buff, sizeof(cwd_buff)) != NULL)
            {
                printf("%s\n", cwd_buff);
            }else
            { //marcamos el error con perror pq es el estándar para reportar un error dee función deel sistema (getcwd)
                perror("cd: error al obtener el directorio actual");
            }    
        }
    }

}
void execute_internal_exit()
{
    exit(0);
}

void execute_internal_umask(tline *line)
{   
    mode_t new_mask;
    long value;
    char *endptr;

    if(line -> ncommands > 1){
        fprintf(stderr, "Error no se puede ejecutar umask con mas comandos usando pipes\n");
        return;
    }
    //Caso 1 Umask sin argumentos -> muestra el valor actual de umask sin modificar nada
    if(line -> commands[0].argc == 1)
    {
        //umask siempre cambia la mascara al llamarlo y devuelve la anterior por lo que tenemos que cambiarla guadar la nueva
        //mostrar la actual y volverla a cambiar
        new_mask = umask(0);
        umask(new_mask);
        printf("%04o\n",new_mask);

    }
    //Nos especifican la mascara que queremos usar
    else if (line -> commands[0].argc == 2)
    {
        // Convertimos directamente el argumento 1 a Base 8 (Octal).
        value = strtol(line -> commands[0].argv[1],&endptr,8);
        //
        if (*endptr != '\0' || endptr == line->commands[0].argv[1]) {
            fprintf(stderr, "umask: Error. Numero octal invalido\n");
            return;
        }
        //Comprobar si la mascara esta en el rango de valores de mascaras permitidos
        if (value < 0 || value > 0777) {
            fprintf(stderr, "umask: Error. Mascara fuera de rango (0000-0777)\n");
            return;
        }
        new_mask = (mode_t) value;
        umask(new_mask);
    }
    else
    {
        fprintf(stderr,"Error umask no permite tantos argumentos\n");
        return;
    }

}
