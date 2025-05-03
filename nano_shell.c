#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#define BUF_SIZE 1000

// Data structure for local variables
typedef struct {
    char *name;
    char *value;
} Variable;

Variable *local_vars = NULL;
int var_count = 0;

// Function to get value of a variable
const char* get_local_var(const char *name) {
    for (int i = 0; i < var_count; i++) {
        if (strcmp(local_vars[i].name, name) == 0) {
            return local_vars[i].value;
        }
    }
    return NULL;
}

// Function to set a local variable
void set_local_var(const char *name, const char *value) {
    for (int i = 0; i < var_count; i++) {
        if (strcmp(local_vars[i].name, name) == 0) {
            free(local_vars[i].value);
            local_vars[i].value = strdup(value);
            return;
        }
    }
    local_vars = realloc(local_vars, sizeof(Variable) * (var_count + 1));
    local_vars[var_count].name = strdup(name);
    local_vars[var_count].value = strdup(value);
    var_count++;
}

// Function to check for a valid assignment (no spaces, only var=value)
int is_valid_assignment(char *arg) {
    char *eq = strchr(arg, '=');
    if (!eq) return 0;
    // Ensure no space around '='
    for (char *p = arg; p < eq; p++) {
        if (*p == ' ') return 0;
    }
    for (char *p = eq + 1; *p; p++) {
        if (*p == ' ') return 0;
    }
    return (eq != arg && *(eq + 1) != '\0');
}

// Substitute variables like $var in arguments
void substitute_variables(char **args, int arg_count) {
    for (int i = 0; i < arg_count; i++) {
        if (args[i][0] == '$') {
            const char *value = get_local_var(args[i] + 1);
            free(args[i]);
            args[i] = strdup(value ? value : "");
        }
    }
}

int main(int argc, char *argv[]) {
    char buf[BUF_SIZE];

    while (1) {
        printf("Nano Shell Prompt > ");
        if (fgets(buf, BUF_SIZE, stdin) == NULL) break;

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

        // Variable assignment case (VAR=value)
        if (strchr(args[0], '=') != NULL) {
            if (arg_count > 1 || !is_valid_assignment(args[0])) {
                printf("Invalid command\n");
            } else {
                char *eq = strchr(args[0], '=');
                *eq = '\0';
                const char *name = args[0];
                const char *value = eq + 1;
                set_local_var(name, value);
            }
        }
        // Handle "export VAR"
        else if (strcmp(args[0], "export") == 0) {
            if (arg_count != 2) {
                printf("Usage: export varname\n");
            } else {
                const char *value = get_local_var(args[1]);
                if (value) {
                    setenv(args[1], value, 1);
                }
            }
        }
        // Built-in "exit"
        else if (strcmp(args[0], "exit") == 0) {
            printf("Good Bye :)\n");
            for (int i = 0; i < arg_count; i++) free(args[i]);
            free(args);
            break;
        }
        // Built-in "echo"
        else if (strcmp(args[0], "echo") == 0) {
            substitute_variables(args, arg_count);
            for (int i = 1; i < arg_count; i++) {
                printf("%s", args[i]);
                if (i < arg_count - 1) printf(" ");
            }
            printf("\n");
        }
        // Built-in "pwd"
        else if (strcmp(args[0], "pwd") == 0) {
            char cwd[1024];
            if (getcwd(cwd, sizeof(cwd)) != NULL) {
                printf("%s\n", cwd);
            } else {
                perror("pwd");
            }
        }
        // Built-in "cd"
        else if (strcmp(args[0], "cd") == 0) {
            const char *path = (arg_count > 1) ? args[1] : getenv("HOME");
            if (chdir(path) != 0) {
                perror("cd");
            }
        }
        // External commands
        else {
            substitute_variables(args, arg_count);
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

        // Cleanup args
        for (int i = 0; i < arg_count; i++) {
            free(args[i]);
        }
        free(args);
    }

    // Cleanup local variables
    for (int i = 0; i < var_count; i++) {
        free(local_vars[i].name);
        free(local_vars[i].value);
    }
    free(local_vars);

    return 0;
}
