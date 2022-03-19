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


#define SERVER_PORT 5060
#define SERVER_IP_ADDRESS "0.0.0.0"

const char prompt = '>';
int org_stdout;
int org_stderr;

char* get_cwd()
{
    char* cwd = getcwd(NULL, 0);
    return cwd;
}

void help(){
    printf("Available commands:\n");
    printf("1. ECHO <message>\n");
    printf("2. TCP PORT\n");
    printf("3. CD <path>\n");
    printf("4. DIR\n");
    printf("5. LOCAL\n");
    printf("6. COPY <src> <dst>\n");
    printf("7. DELETE <name>\n");
    printf("6. EXIT\n");
    printf("7. HELP\n");
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

void print_dir(char* dir_name){
    //list all the items in the directory
    DIR *dir;
    struct dirent *dir_data;
    if ((dir = opendir (dir_name)) != NULL) {
      while ((dir_data = readdir (dir)) != NULL) {
        printf ("%s\n", dir_data->d_name);
      }
      closedir (dir);
    } else {
      printf("Directory is not exist\n");
    }
}

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

void unlink_a_file(char* file_name){
    //unlink the file
    if(unlink(file_name) == -1){
        printf("delete file faild, %s\n", strerror(errno));
    }
}

void copy_file_to_dest(char* input){
    //copy the file to the destination COPY SRC DST
    //assuming that the path doesnt have \n or spaces in it 
    char* src = malloc(sizeof(char) * (strlen(input) - 1));
    char* dst = malloc(sizeof(char) * (strlen(input) - 1));

    int space_count = 0;
    int space_runner = 0;
    while (input[space_runner] != '\n'){
        if (input[space_runner] == ' '){
            space_count++;
        }
        space_runner++;
    }
    
    if (space_count == 1){
        int i = 0;
        while(input[i] != ' '){
            src[i] = input[i];
            i++;
        }
        src[i] = '\0';
        i++;
        int j = 0;
        while(input[i+j] != '\n'){
            dst[j] = input[i+j];
            j++;
        }
        printf("%s %ld\n", src, strlen(src));
        printf("%s %ld\n", dst, strlen(dst));
        FILE* src_file = fopen(src, "rb");
    
        if(src_file){
            FILE* dst_file = fopen(dst, "wb+");
            char* buffer = malloc(sizeof(char) * 4096);
            int read_bytes = 0;
            while((read_bytes = fread(buffer, sizeof(char), 4096, src_file)) > 0){
                fwrite(buffer, sizeof(char), read_bytes, dst_file);
            }
            fclose(src_file);
            fclose(dst_file);
            free(buffer);
        }
        else{
            printf("Couldn't open the file\n");
        }
    }
    else{
        printf("Invalid input\n");
    }
    free(src);
    free(dst);
}

void run_command_default(char* input){
    // using fork exec and wait to run the command
    // used code from https://vitux.com/fork-exec-wait-and-exit-system-call-explained-in-linux/ for inspriation on how to use fork and exec and wait
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

void handle_send_error(int bytesSent, int messageLen){
     if (-1 == bytesSent){printf("send() failed with error code : %d" ,errno);}
     else if (0 == bytesSent){printf("peer has closed the TCP connection prior to send().\n");}
     else if (messageLen > bytesSent){printf("sent only %d bytes from the required %d.\n", messageLen, bytesSent);}
}

void standart_output_tcp(){
    org_stderr = dup(STDERR_FILENO);
    org_stdout = dup(STDOUT_FILENO);
    //changes the standart output to send by a tcp connectiom to server in port 8080 ip 127.0.0.1 Localhost
    //the socket and server part was taken from one of the assignment of last semester networking class 
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if(sock < 0){
        printf("Error with creating socket please try again\n");
    }
    struct sockaddr_in serverAddress;
    memset(&serverAddress, 0, sizeof(serverAddress));

    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(SERVER_PORT);
	int rval = inet_pton(AF_INET, (const char*)SERVER_IP_ADDRESS, &serverAddress.sin_addr);
	if (rval <= 0){
		printf("inet_pton() failed");
	}
    if (connect(sock, (struct sockaddr *) &serverAddress, sizeof(serverAddress)) == -1){
	   printf("connect() failed with error code : %d" ,errno);
    }
    dup2(sock, STDERR_FILENO);
    dup2(sock, STDOUT_FILENO);
    close(sock);
 
}
void standart_output_stdout(){
    //changes the standart output to stdout
    dup2(org_stderr, STDERR_FILENO);
    dup2(org_stdout, STDOUT_FILENO);
    close(org_stderr);
    close(org_stdout);
}

void main_loop(){
    char input[4096] = {};
    while (strcmp(input, "EXIT\n") != 0)
    {
        // Print prompt task 1
        // printf("%s", "yes master?>");
        
        printf("%s", get_cwd());//part od task 2
        printf("%c ", prompt);
        fgets(input, 1024, stdin);
        if (strcmp(input, "EXIT\n") == 0)
        {//elso part of task 1
            printf("Exiting Terminal...\n");
        }
        else if (strcmp(split_command(0, 3, input), "ECHO") == 0){
            //task 3
            if(strlen(input)>6){
                printf("%s\n", split_command(5, strlen(input)-2, input));
            }
            else{
                printf("%s\n", "the len of message shood be more than 0");
            }
        }
        else if (strcmp(input, "TCP PORT\n") == 0){//task 4 part 1 & also part 2(the server is in anouter file) used some help form https://stackoverflow.com/questions/8100817/redirect-stdout-and-stderr-to-socket-in-c
            standart_output_tcp();
            printf("%s\n", "standart output changed to TCP");
        }
        else if (strcmp(input, "LOCAL\n") == 0){
            //task 5 
            standart_output_stdout();
        }
        else if (strcmp(input, "DIR\n") == 0){
            //list the files in the current directory with open dir close dir and read dir
            print_dir(get_cwd());
        }
        else if (strcmp(split_command(0, 1, input), "CD") == 0){
            //move to a new working diirectory task 7
            //yes it is a system call
           if (strlen(input) > 4){
               change_dir(split_command(3,strlen(input)-2, input));
           }
           else{
                printf("%s\n", "the len of dir name shood be more than 0");
           }
        }
        else if (strcmp(split_command(0, 3, input), "COPY") == 0){
            //task 10 copy file from src to dst 
            //fopen and fread and fwrite are function calls and not system calls
            if(strlen(input)>6){
                copy_file_to_dest(split_command(5, strlen(input)-1, input));
            }
        }
        else if (strcmp(split_command(0, 5, input), "DELETE") == 0){
            //tsk 11 delete a file
            //and it is a system call
            if(strlen(input)>6){
                unlink_a_file(split_command(7, strlen(input)-2, input));
            }
        }
        else if (strcmp(split_command(0, 3, input), "HELP") == 0){
            //used for convinience to see all the commands in one place
            help();
        }
        // else{
        //     //task 8 - default run with system
        //     //system is a libery call
        //     system(input);
        // }
        else{
            //task 9 - default run with fork exec and wait
            run_command_default(input);
        }

    }
}

int main(int argc, char const *argv[])
{
    
    main_loop();
    return 0;
}