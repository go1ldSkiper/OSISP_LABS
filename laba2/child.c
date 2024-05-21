#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <environment_file>\n", argv[0]);
        return 1;
    }
    
    printf("Process name: %s\n", argv[0]);
    printf("PID: %d\n", getpid());
    printf("PPID: %d\n", getppid());

    FILE *env_file = fopen(argv[1], "r");
    if (env_file == NULL) {
        perror("Error opening environment file");
        return 1;
    }

    char var_name[256];
    while (fscanf(env_file, "%255s", var_name) != EOF) {
        char *var_value = getenv(var_name);
        if (var_value != NULL) {
            printf("%s=%s\n", var_name, var_value);
        } else {
            printf("%s not found\n", var_name);
        }
    }

    fclose(env_file);

    return 0;
}
