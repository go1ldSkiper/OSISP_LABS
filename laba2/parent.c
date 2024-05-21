#define _XOPEN_SOURCE 700
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

extern char **environ; 

int compare(const void *a, const void *b) {
    return strcmp(*(const char **)a, *(const char **)b);
}


int plus_process(char * argv[], char *envp[], int * child_counter) {
    pid_t pid = fork();

    char * path;
    int path_size = 1;
    path = (char *)malloc(path_size*sizeof(char));
    path[0] = '\0';
    for(int i = 0; argv[1][i] != '\0'; i++){
        path[i] = argv[1][i];
        path_size++;
        path = (char *)realloc(path, path_size*sizeof(char));
        path[path_size-1] = '\0';
    }

    if (pid < 0) {
        perror("fork failed");
        return 1;
    }

    if (pid == 0) {
        char *child_path = getenv("CHILD_PATH");
        if (child_path == NULL) {
            fprintf(stderr, "CHILD_PATH not set\n");
            return 1;
        }

        char child_name[256];
        sprintf(child_name, "child_%02d", (*child_counter));
        char *child_args[] = {child_name, path, NULL};
        execve(child_path, child_args, envp);
        perror("execve failed");
        return 1;
    } 
    else {
        waitpid(pid, NULL, 0);
    }

    return 0;
}

int multiplication_process(int argc, char *argv[], char *envp[], int * child_counter) {
    pid_t pid = fork();

    char * path;
    int path_size = 1;
    path = (char *)malloc(path_size*sizeof(char));
    path[0] = '\0';
    for(int i = 0; argv[1][i] != '\0'; i++){
        path[i] = argv[1][i];
        path_size++;
        path = (char *)realloc(path, path_size*sizeof(char));
        path[path_size-1] = '\0';
    }

    if (pid == 0) { 
        char *child_path = getenv("CHILD_PATH");
        if (child_path == NULL) {
            fprintf(stderr, "CHILD_PATH not found in environment.\n");
            exit(EXIT_FAILURE);
        }

        char child_name[256];
        sprintf(child_name, "child_%02d", (*child_counter));
        char *child_args[] = {child_name, path, NULL};
        execve(child_path, child_args, envp); 
        perror("execve");
        exit(EXIT_FAILURE);
    } 
    else if (pid > 0) { 
        waitpid(pid, NULL, 0); 
    } 
    else {
        perror("fork"); 
        exit(EXIT_FAILURE);
    }
    return 0;
}

int ampersand_process(char * argv[], int * child_counter) {
    pid_t pid = fork();
    if (pid == 0) {
        char *child_path = getenv("CHILD_PATH");
        if (child_path == NULL) {
            fprintf(stderr, "CHILD_PATH not found in environment.\n");
            exit(EXIT_FAILURE);
        }

        char * path;
        int path_size = 1;
        path = (char *)malloc(path_size*sizeof(char));
        path[0] = '\0';
        for(int i = 0; argv[1][i] != '\0'; i++){
            path[i] = argv[1][i];
            path_size++;
            path = (char *)realloc(path, path_size*sizeof(char));
            path[path_size-1] = '\0';
        }


        char child_name[256];
        sprintf(child_name, "child_%02d", (*child_counter));
        char *child_args[] = {child_name, path, NULL};
        execve(child_path, child_args, environ); 
        perror("execve");
        exit(EXIT_FAILURE);
    } 
    else if (pid < 0) { 
        perror("fork");
        exit(EXIT_FAILURE);
    }
    else if (pid > 0) { 
        waitpid(pid, NULL, 0); 
    }
    return 0;
}


int main(int argc, char * argv[], char * envp[]){
    if(argc != 2){
        printf("No parameters. Try again.\n");
        return 1;
    }

    int child_counter = 0;

    char **env = environ;
    int count = 0;
    while (env[count] != NULL) {
        count++;
    }


    setenv("LC_COLLATE", "C", 1);
    qsort(env, count, sizeof(char *), compare);

    for (int i = 0; i < count; i++) {
        printf("%s\n", env[i]);
    }


    while (1) {
        char c[2] = {'\0', '\0'}, gc;
        printf("Введите '+', '*', '&', или 'q'(выход): ");
        for(int i = 0; (gc = getchar()) != '\n'; i++){
            if(i == 1 && gc != '\n'){
                break;
            }
            c[i] = gc;
        }
        c[1] = '\0';

        if (c[0] == '+') {
            plus_process(argv, envp, &child_counter);
            child_counter++;
        } 
        else if (c[0] == '*') {
            multiplication_process(argc, argv, envp, &child_counter);
            child_counter++;
        } 
        else if (c[0] == '&') {
            ampersand_process(argv, &child_counter);
            child_counter++;
        } 
        else if (c[0] == 'q') {
            break;
        } 
        else {
            printf("Неверный ввод. Попробуйте еще раз.\n");
        }
    }

    return 0;
}