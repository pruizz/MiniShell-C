#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include "parser.h"

//Metodo para abstraer la logica de redirecciones y limpiar el codigo
void redirect(tline * line, int index) {
    int fdaux;

    //La redireccion de entrada solo se aplica si es el primer mandato
    if (index == 0 && line->redirect_input != NULL) {
        //Cogemos el fichero de redireccion de entrada y lo abrimos en modo solo Lecutura, esto devueleve un fd nuevo de la tabla de fd , que usaremos para asociar despues
        fdaux = open(line->redirect_input, O_RDONLY);
        if (fdaux < 0) 
        // si es <0 da error
        {
            fprintf(stderr, "%s: Error. %s\n", line->redirect_input, strerror(errno)); // 
            exit(1);
        }
        //Asociamos el descriptor otorgado al fichero de texxto abbierto a la entrada estandar para que los comando lo lean
        //Esto cierra el fileDescriptor de stdin
        dup2(fdaux, STDIN_FILENO);
        close(fdaux);// Necesario cerrarlo ya que no se va a usar
    }

    //Stdout -- Salida estandar
    // Sólo puede realizarse sobre el último mandato, último mandato es aquel cuyo índice es (ncommands - 1)
    if (index == line->ncommands - 1 && line->redirect_output != NULL) {
        // O_CREAT: Si no existe, créalo.
        // O_TRUNC: Si existe, vacíalo.
        // O_WRONLY: Ábrelo para escribir.
        // 0666: Permisos por defecto si hay que creralo
        fdaux = open(line->redirect_output, O_CREAT | O_TRUNC | O_WRONLY, 0666);
        if (fdaux < 0) {
            fprintf(stderr, "%s: Error. %s\n", line->redirect_output, strerror(errno)); // 
            exit(1);
        }
        //Asociamos el descriptor del fichero de salida a salida estandar
        dup2(fdaux, STDOUT_FILENO);
        close(fdaux);
    }

    // Stderr - Salida estandar de error
    // Sólo puede realizarse sobre el último mandato
    if (index == line->ncommands - 1 && line->redirect_error != NULL) {
        fdaux = open(line->redirect_error, O_CREAT | O_TRUNC | O_WRONLY, 0666);
        if (fdaux < 0) {
            fprintf(stderr, "%s: Error. %s\n", line->redirect_error, strerror(errno)); // 
            exit(1);
        }
        dup2(fdaux, STDERR_FILENO);
        close(fdaux);
    }
}

int main(void) {
	char buf[1024];
	tline * line;
    pid_t pid,pid2;
    int status;
    int i;
    int last_pipe_fd;
    int p[2];

	printf("msh> ");	
	while (fgets(buf, 1024, stdin)) {
		
		line = tokenize(buf);
		if (line==NULL) {
            printf("msh> ");
			continue;
		}

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
            else if (pid == 0)
            {
                //Si last pipe tiene valor signifca que tengo que recibir la entrada desde la pipe
                if (last_pipe_fd != -1){
                    dup2(last_pipe_fd,STDIN_FILENO);
                    close(last_pipe_fd);
                }

                //Conectar la salida: SI no soy el ultimo tengo que escribir en el pipe
                if( i < line -> ncommands-1){
                    close(p[0]);
                    dup2(p[1],STDOUT_FILENO);
                    close(p[1]);
                }

                redirect(line,i);

                if (line->commands[i].filename == NULL) {
                    fprintf(stderr, "mandato: No se encuentra el mandato\n"); // 
                    exit(1);
                }

                execvp(line->commands[i].filename, line->commands[i].argv);
                
                // Si llegamos aquí, execvp falló
                fprintf(stderr, "mandato: No se encuentra el mandato\n");
                exit(1);
            }
            //PADRE
            else{
                //Si cuando entramos al padre ya hay asignado un pipe signfica que 
                if (last_pipe_fd != -1){
                    close(last_pipe_fd);
                }

                //No soy la ultima vuleta aun hay mandatos por lo que tengo que decir al siguiente por donde escuchar
                if (i < line->ncommands - 1) {
                    last_pipe_fd = p[0];
                    //Extremo de escritura no usado por el padre;
                    close(p[1]);
                }
            }
        }
        printf("msh> ");			
    }
}


