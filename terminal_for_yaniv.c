#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/wait.h>
//socket stuff import
#include <sys/types.h> 
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netinet/tcp.h> 
#include <sys/stat.h>
#include <fcntl.h>

char* split_command(int start, int end, char* command){
    //gets the sub string in the command from start to end
    char* sub_command = malloc(sizeof(char) * (end - start + 1));
    int i;
    for(i = start; i <= end; i++){
        sub_command[i - start] = command[i];
    }
    sub_command[i - start] = '\0';
    return sub_command;
}



void run_command_default(char* input){
    // using fork exec and wait to run the command
    int pid = fork();
    int status;
    if(pid == 0){
        //child process
        char* args[4096];
        int i = 0;
        int runner = 0;
        int last_space = 0;
        while(runner < strlen(input)){
            if(input[runner] == ' '){
                args[i] = split_command(last_space, runner - 1, input);
                last_space = runner + 1;
                i++;
            }
            else if(runner == strlen(input)-1){
                args[i] = split_command(last_space, runner-1, input);
                last_space = runner + 1;
                i++;
            }
            runner++;
        }
        args[i] = NULL;
        execvp(args[0], args);  
        exit(0);
    }
    else{
        //parent process
        wait(&status);
    }
}

char* get_cwd()
{
    char* cwd = getcwd(NULL, 0);
    return cwd;
}
void change_dir(char* path){
    char temp_path[4096];
    strcpy(temp_path, get_cwd());
    strcat(temp_path, "/");
    strcat(temp_path, path);
    if(chdir(path) == -1){
        printf("coudn't change directory, %s - %s\n", strerror(errno), temp_path);
    }
}
int main(int argc, char *argv[]){
    char prompt = '$';
    char command[2048];
    char all_commands[150][2048] = {};
    int command_count = 0;
    while (1)
    {

        printf("%s", get_cwd());
        printf("%c ", prompt);
        fflush(stdout);
        fgets(command, 2048, stdin);
        strcpy(all_commands[command_count],command);
        if (strcmp(command, "exit\n") == 0)
        {
           exit(0);
        }
        else if (strcmp(command, "history\n") == 0){
            //print out all_commands array
            for(int i = 0; i < command_count; i++){
                printf("%d: %s", i+1, all_commands[i]);
            }
        }
        else if (strcmp(split_command(0, 1, command), "cd") == 0){
           if (strlen(command) > 4){
               change_dir(split_command(3,strlen(command)-2, command));
           }
           else{
                printf("%s\n", "the len of dir name shood be more than 0");
           }
        }
        else{
            run_command_default(command);
        }
        command_count++;
    }
    
    
}