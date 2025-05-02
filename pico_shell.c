#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#define BUF_SIZE 1000

int main(int argc, char *argv[]) {
    char buf[BUF_SIZE];

    while (1) {
        printf("Picoshell > ");
        if (fgets(buf, BUF_SIZE, stdin) == NULL) {
            break;
        }

        buf[strcspn(buf, "\n")] = '\0';
        if (strlen(buf) == 0) continue;

        // Tokenize input
        char **args = NULL;
        int arg_count = 0;
        char *token = strtok(buf, " ");
        while (token != NULL) {
            args = (char **)realloc(args, sizeof(char *) * (arg_count + 2));
            args[arg_count] = strdup(token);
            arg_count++;
            token = strtok(NULL, " ");
        }

        if (arg_count == 0) {
            free(args);
            continue;
        }

        args[arg_count] = NULL;

        // Built-in commands
        if (strcmp(args[0], "exit") == 0) {
            printf("Good Bye :)\n");
            for (int i = 0; i < arg_count; i++) free(args[i]);
            free(args);
            break;
        } else if (strcmp(args[0], "echo") == 0) {
            for (int i = 1; i < arg_count; i++) {
                printf("%s", args[i]);
                if (i < arg_count - 1) printf(" ");
            }
            printf("\n");
        } else if (strcmp(args[0], "pwd") == 0) {
            char cwd[1024];
            if (getcwd(cwd, sizeof(cwd)) != NULL) {
                printf("%s\n", cwd);
            } else {
                perror("pwd");
            }
        } else if (strcmp(args[0], "cd") == 0) {
            const char *path = (arg_count > 1) ? args[1] : getenv("HOME");
            if (chdir(path) != 0) {
                perror("cd");
            }
        } else {
            // External command
            pid_t pid = fork();
            if (pid == 0) {
                execvp(args[0], args);
                perror("exec");
                exit(EXIT_FAILURE);
            } else if (pid > 0) {
                int status;
                waitpid(pid, &status, 0);
            } else {
                perror("fork");
            }
        }

        // Cleanup
        for (int i = 0; i < arg_count; i++) {
            free(args[i]);
        }
        free(args);
    }

    return 0;
}