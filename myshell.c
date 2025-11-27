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
    int p[2];

	printf("msh> ");	
	while (fgets(buf, 1024, stdin)) {
		
		line = tokenize(buf);
		if (line==NULL) {
            printf("msh> ");
			continue;
		}
        if (line -> ncommands == 1 ){
            pid = fork();

            if (pid < 0){
                fprintf(stderr,"Error a la hora de hacer fork: %s \n",strerror(errno));
            }

            else if (pid == 0)
            {
                //Control de todas las salidas pues es el unico mandato
                redirect(line,0);
                //Si devuleve NULL es que el comando no existe 
                if (line->commands[0].filename == NULL) {
                    fprintf(stderr, "mandato: No se encuentra el mandato\n"); // 
                    exit(1); // Matamos al hijo manualmente y de forma ordenada
                }

                execvp(line->commands[0].filename,line->commands[0].argv);
                fprintf(stderr, "mandato: No se encuentra el mandato\n");
                exit(1);
            }
            else{
                //Esperar al hijo
                wait(&status);
            }
    
        //PASO 2 DOS COMANDOS CORRIENDO 
        }else if (line -> ncommands == 2){
            //Creamos el pipe que envia la informacion entre los dos mandatos
            pipe(p);
            pid = fork();

            if (pid < 0){
                fprintf(stderr,"Error a la hora de hacer fork: %s \n",strerror(errno));

            }else if (pid == 0)
            {
                //No voy a leer
                close(p[0]);
                //Apunto el extremo escritura del pipe a la salida estanddar
                dup2(p[1],STDOUT_FILENO);
                close(p[1]);

                redirect(line,0);
                if (line->commands[0].filename == NULL) {
                    fprintf(stderr, "Mandato: No se encuentra el mandato\n"); // 
                    exit(1);
                }

                execvp(line->commands[0].filename, line->commands[0].argv);
                fprintf(stderr, "Mandato: No se encuentra el mandato\n"); // 
                exit(1);
            }

            pid2 = fork();
            if (pid2 < 0){
                fprintf(stderr,"Error a la hora de hacer fork: %s \n",strerror(errno));
            }
            else if (pid2 == 0){
                //No voy a escribir lo cierro
                close(p[1]);
                dup2(p[0],STDIN_FILENO);
                close(p[0]);

                redirect(line,1);

                if (line->commands[1].filename == NULL) {
                    fprintf(stderr, "Mandato: No se encuentra el mandato\n"); // 
                    exit(1);
                }

                execvp(line->commands[1].filename, line->commands[1].argv);
                fprintf(stderr, "Mandato: No se encuentra el mandato\n"); // 
                exit(1);

            }
            //Padre debe cerra los extremos de las pipe porque no los usa yy asi avisar a los hijos de el fin 
            close(p[0]);
            close(p[1]);

            // Esperar a los dos hijos
            wait(NULL);
            wait(NULL);
            

        }
        else{
            printf("Aun no implementados mas de do comandos");
        }
		
		printf("msh> ");	
	}
	return 0;
}
