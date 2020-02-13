#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>

extern char **environ;


void help(){
    printf("################################# PSEUDO-SHELL #################################\n\n");
    printf("Liste des commandes intégrées:\n");
    printf("\tlist\t\t\tliste les variables d'environnement\n");
    printf("\tgetenv <var>\t\taffiche la valeur de la variable <var>\n");
    printf("\tsetenv <var> <valeur>\tchange la variable <var> en <valeur>\n");
    printf("\thelp\t\t\taffiche ce message\n");
    printf("\texit\t\t\tpour quitter\n\n");
    printf("Les commandes systemes sont aussi disponibles\n");
    printf("*Pour les pipes, mettre des espaces autour du symbole \"|\"\n");
    printf("*Les modifications apportées par setenv ne sont valables que pour cette session\n\n");
    printf("################################################################################\n");
}

void cd(){
    char *dir = strtok(NULL, "ø");
    chdir(dir);
}


void exec_cmd(char *args){
    char *argv[100];
    char *argv2[100];
    char *token;
    int i = 0;
    int needsPipe = 0;

    pid_t pid;

    token = strtok(args, " ");
    while (token != NULL){
        argv[i] = token;
        if (strcmp("|", argv[i]) == 0){
            needsPipe = 1;
            break;
        }
        token = strtok(NULL, " ");
        i++;
    }
    argv[i] = NULL;

    if (needsPipe == 1){
        i = 0;
        while (token != NULL){
            argv2[i] = token;
            if(strcmp(argv2[i],"|")!=0){
                i++;
            }
            token = strtok(NULL, " ");
        }
        argv2[i] = NULL;
    }
    

    pid = fork();
    switch (pid)
    {
        case -1:
            printf("\n/!\\ ERREUR /!\\");
            exit(-1);
            break;
        case 0:
            if (needsPipe == 1){
                int pipefd[2];
                pipe(pipefd);
                needsPipe = 0; //pour éviter des forks infinis

                pid_t p;
                p = fork();
                switch (p){
                    case -1:
                        printf("\n/!\\ ERREUR /!\\");
                        exit(-1);
                        break;

                    case 0:
                        //printf(argv[0]);
                        close(1); //fermer le stdout standard
                        dup(pipefd[1]); //faire du coté 'write' du tube le stdout
                        close(pipefd[0]); //fermer le coté 'read' du tube
                        execvp(argv[0], argv);
                        break;

                    default:
                        wait();
                        //printf(argv2[0]);
                        close(0); //fermer le stdin standard
                        dup(pipefd[0]); //faire du coté 'read' du tube le stdin
                        close(pipefd[1]); //fermer le coté 'write' du tube
                        execvp(argv2[0], argv2); 
                        break;

                }

            }
            else{
                execvp(argv[0], argv);
                break;
            }
            break;
        default:
            wait();
            break;
    }

}

void list(){
    int i ;
    for(i=0 ; environ[i] != NULL ; i++){
        puts(environ[i]);
    }
}

void get(){
    char *var = strtok(NULL, "ø");
    char *res = getenv(var);
    printf("[%s] = %s\n", var, res);
}

void set(){
    char *argv[10];
    char *token;
    int i = 0;

    token = strtok(NULL, " ");
    while (token != NULL){
        argv[i] = token;
        i++;
        token = strtok(NULL, " ");
    }
    setenv(argv[0], argv[1], 1);
}

void main(){
    char buf[2048];
    char *user;
    char cwd[2048];
    char *cmd;

    exec_cmd("clear");
    help();
    
    while(1){
        user = getenv("USER");
        getcwd(cwd, 1024);
        
        printf("\n<%s> [%s]: $ ", user, cwd);
        fgets(buf, 1024, stdin);
        
        if (strcmp("\n", buf) != 0){
            buf[strlen(buf)-1] = NULL;
        }

        char *copy_cmd;
        strcpy(copy_cmd, buf);
        
        cmd = strtok(buf, " ");
        if (strcmp("exit", cmd) == 0){
            printf("\n\nFin du programme\n");
            exit(0);
        }else if (strcmp("help", cmd) == 0){
            help();
        }else if (strcmp("list", cmd) == 0){
            list();
        }else if (strcmp("getenv", cmd) == 0){
            get();
        }else if (strcmp("setenv", cmd) == 0){
            set();
        }else if (strcmp("cd", cmd) == 0){
            cd();
        }else{
            exec_cmd(copy_cmd);
        }

    }

}
