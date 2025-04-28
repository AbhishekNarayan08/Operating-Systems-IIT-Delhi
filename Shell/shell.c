#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<sys/wait.h>
#include<signal.h>
#include<errno.h>

#define buffer_length 500

// cmd_flag indicates type of command passed
// 1: run the executable
// 2: run the executable as bg process
// 3: piped
// 4: set environment variable
int cmd_flag=0;

int status;
extern int errno;

void SIGINThandler(int);
char *read_input(void);
char **parse_args(char*);
char **replace_env_vars(char**);
char **parse_args_env(char*);
char **parse_pipe(char*);
void execute(char*);
void print_hist(void);
void print_proc(void);

// char **h=malloc(sizeof(char)*5);
char* h[5]={'\0'};
int hnum=0;
int pidhist[10000];
int pidnum=0;

void add_hist(char* st){
    h[4]=h[3];
    h[3]=h[2];
    h[2]=h[1];
    h[1]=h[0];
    h[0]=strdup(st);
    if(hnum<5) hnum++;
}


int main(){
    signal(SIGINT, SIGINThandler);
    int cont=1;

    // printf("Hello, welcome to my shell\n");
    while(cont){

        char cwd[1000];
        getcwd(cwd, 1000);
        printf("%s~$ ",cwd);

        // readline takes input from cmd line
        char *line=read_input();
        if(line==NULL){
            continue;
        }

        if(strcmp(line, "cmd_history")==0){
            print_hist();
            continue;
        }
        
        if(strcmp(line, "ps_history")==0){
            print_proc();
            continue;
        }

        if(cmd_flag==4){
            // implement environment variables using setenv()
            char** env_args=parse_args_env(line);
            if(env_args[1]==NULL || *(env_args[1])=='\0'){
                fprintf(stderr, "please provide variable value\n");
                continue;
            }
            if(env_args[0]==NULL || *(env_args[0])=='\0'){
                fprintf(stderr, "please provide variable name\n");
                continue;
            }

            errno=setenv(env_args[0], env_args[1],0);
            if(errno!=0){
                int err=errno;
                fprintf(stderr, "could not set environment variable: %s\n", strerror(err));
            }

        }
        else if(cmd_flag==3){
            // implement piping
            char **pipe_cmd=parse_pipe(line);
            char *cmd1=pipe_cmd[0];
            char *cmd2=pipe_cmd[1];
            char **cmd_args1=parse_args(cmd1);
            char **cmd_args2=parse_args(cmd2);
            if(cmd_args1==NULL) continue;

            int fd1[2];

            if(pipe(fd1)<0){
                fprintf(stderr, "error creating pipe, try again\n");
                continue;
            }

            int cpid=fork();
            if(cpid<0){
                printf("failed to fork\n");
                continue;
            }
            else if(cpid==0){
                close(fd1[0]);
                dup2(fd1[1], 1);
                execvp(cmd_args1[0], cmd_args1);
            }
            else{
                waitpid(cpid, NULL, 0);
                // printf("%d\n",cpid);
                pidhist[pidnum]=cpid;
                pidnum++;
                close(fd1[1]);
                
                if(cmd_args2==NULL) continue;

                int cpid2=fork();
                if(cpid2<0){
                    printf("failed to fork\n");
                    continue;
                }
                else if(cpid2==0){
                    close(fd1[1]);
                    dup2(fd1[0], 0);
                    execvp(cmd_args2[0], cmd_args2);
                }
                else{
                    waitpid(cpid2, NULL, 0);
                    // printf("%d\n",cpid2);
                    pidhist[pidnum]=cpid2;
                    pidnum++;

                }
            }
        }
        else{
            execute(line);
        }
    }

    return 0;
}

void SIGINThandler(int signum){
    printf("\n");
    exit(1);
}

char *read_input(void){
    // printf("%d", cmd_flag);
    cmd_flag=1;
    // printf("%d", cmd_flag);
    char *input_buffer=malloc(sizeof(char)*buffer_length);
    int c;
    int pos=0;
    int last_char=' ';

    if(!input_buffer){
        printf("memory allocation error\n");
    }

    while(1){
        c=getchar();

        if(c=='&') cmd_flag=2;
        else if(c=='|') cmd_flag=3;
        else if(c=='=') cmd_flag=4;

        if(c==EOF || c=='\n'){
            input_buffer[pos]='\0';
            // printf("%s asdfasdfasdfasdfad\n",input_buffer+1);
            add_hist(input_buffer);
            return input_buffer;
        }
        else if(c==' '){
            if(last_char!=' '){
                input_buffer[pos]=c;
                // printf("%c\n",c);
                pos++;
            }
        }
        else{
            input_buffer[pos]=c;
            // printf("%c\n",c);
            pos++;
        }
        last_char=c;
        
        if(pos==buffer_length){
            printf("input length too long to parse, please enter shorter commands\n");
            while(1){
                c=getchar();
                if(c=='\n') break;
            }
            return NULL;
            // break;
        }
    }
}

char **parse_args(char *cmdline){
    char **cmd_args=malloc(sizeof(char)*buffer_length);
    int pos=0;
    char *ptr=cmdline;
    char *start=cmdline;
    if (cmd_flag==2){
        ptr++;
        start++;
    }
    while(ptr!=NULL){
        if(*ptr==' '){
            *ptr='\0';
            cmd_args[pos]=strdup(start);
            // printf("%s\n",cmd_args[pos]);
            pos++;
            ptr++;
            start=ptr;
        }
        else if(*ptr=='\0'){ 
            cmd_args[pos]=strdup(start);
            // printf("%s\n",cmd_args[pos]);
            break;
        }
        else ptr++;
    }

    cmd_args=replace_env_vars(cmd_args);
    free(cmdline);
    // printf("2\n");
    return cmd_args;
}

char **replace_env_vars(char **args){
    // char **ptr=args;
    int pos=0;
    // while(ptr!=NULL){
    while(pos<buffer_length){
        // printf("%d\n", pos);
        if(args[pos]==NULL) break;
        if(args[pos][0]=='$'){
            char *ptr=args[pos];
            ptr++;
            char *val=getenv(ptr);
            if(val==NULL){
                fprintf(stderr, "no variable found by name %s\n",ptr);
                return NULL;
            }
            args[pos]=strdup(val);
        }
        pos++;
    }
    return args;
}

char **parse_args_env(char *cmdline){
    char **cmd_args=malloc(sizeof(char)*2);
    int pos=0;
    char *ptr=cmdline;
    char *start=cmdline;
    while(*ptr!='='){
        ptr++;
    }
    *ptr='\0';
    ptr++;
    cmd_args[0]=strdup(start);
    cmd_args[1]=strdup(ptr);

    free(cmdline);
    // printf("2\n");
    return cmd_args;
}

char **parse_pipe(char *cmdline){
    char **cmd_args=malloc(sizeof(char)*2);
    int pos=0;
    char *ptr=cmdline;
    char *start=cmdline;
    while(*ptr!='|'){
        ptr++;
    }

    ptr--;
    *ptr='\0';
    ptr++;
    ptr++;
    ptr++;
    // while(*ptr==' ') ptr++;
    cmd_args[0]=strdup(start);
    cmd_args[1]=strdup(ptr);
    printf("pipe cmd \n%s\n", cmd_args[0]);
    printf("%s\n", cmd_args[1]);

    free(cmdline);
    // printf("2\n");
    return cmd_args;
}

void execute(char *cmdline){
    char **args=parse_args(cmdline);
    if(args==NULL) return ;

    // implementing exit cmd
    if(strcmp(args[0],"exit")==0) exit(1);

    //implementing cd cmd
    if(strcmp(args[0],"cd")==0){
        if(args[2]!=NULL){
            fprintf(stderr, "too many arguments\n");
            // continue;
            return;
        }
        int cd=chdir(args[1]);
        if(cd!=0){
            int err=errno;
            // printf("error number %d\n", err);
            fprintf(stderr, "cd failed: %s\n", strerror(err));
        }
    }

    //implementing all other commands with execvp
    int cpid=fork();
    if(cpid<0){
        printf("failed to fork\n");
        return;
    }
    else if(cpid==0){
        // printf("\n%s %s\n",args[0],args[1]); 
        execvp(args[0], args);
        // cont=0;
        return ;
    }
    else{
        if (cmd_flag!=2)
            waitpid(cpid,NULL,0);
        // printf("%d\n",cpid);
        pidhist[pidnum]=cpid;
        pidnum++;
    }
}

void print_hist(void){
    for(int i=0;i<hnum;i++){
        printf("%s\n", h[i]);
    }
}

void print_proc(void){
    for(int i=0;i<pidnum;i++){
        int pid=pidhist[i];
        // printf("%d\n",pid);
        if(waitpid(pid, &status, WNOHANG) !=0){
            printf("%d %s\n",pid, "STOPPED");
        }
        else{
            printf("%d %s\n",pid, "RUNNING");
        }
    }
}
