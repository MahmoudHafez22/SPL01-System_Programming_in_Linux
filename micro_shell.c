#include <stdio.h>
#include <stdbool.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>

#define IN_SIZE 16000
#define DIR_SIZE 8000

void dolla_sign_op(char* input, char** local_vars)
{
    char* dolla_pos = strchr(input, '$');
    while (dolla_pos != NULL)
    {
        char* dolla_arg = dolla_pos + 1;
        bool found = false;

        for (int local_vars_ptr = 0; local_vars[local_vars_ptr] != NULL; local_vars_ptr++)
        {
            char* eq_pos = strchr(local_vars[local_vars_ptr], '=');
            if (eq_pos != NULL)
            {
                char lso[eq_pos - local_vars[local_vars_ptr] + 1];
                memcpy(lso, local_vars[local_vars_ptr], eq_pos - local_vars[local_vars_ptr]);
                lso[eq_pos - local_vars[local_vars_ptr]] = '\0';

                if (!strncmp(dolla_arg, lso, strlen(lso)) && 
                    (dolla_arg[strlen(lso)] == ' ' || dolla_arg[strlen(lso)] == '\0'))
                {
                    found = true;
                    char* rso = eq_pos + 1;

                    char temp_input[IN_SIZE];
                    strcpy(temp_input, dolla_pos + strlen(lso) + 1);
                    strcpy(dolla_pos, rso);
                    strcat(input, temp_input);
                    break;
                }
            }
        }

        if (!found)
        {
            char* end_of_var = dolla_arg;
            while (*end_of_var != ' ' && *end_of_var != '\0')
                end_of_var++;

            memmove(dolla_pos, end_of_var, strlen(end_of_var) + 1);
        }

        dolla_pos = strchr(input, '$');
    }
}


int microshell_main(int argc, char *argv[]) {

    int exit_code = 0;
	if (argc > 1)
	{
		printf("Usage: Nano Shell doesn't have arguments, please run it again without arguments\n");
		exit(-1);
	}

	int stdin_fd = dup(STDIN_FILENO);
	int stdout_fd = dup(STDOUT_FILENO);
	int stderr_fd = dup(STDERR_FILENO);

	char input[IN_SIZE];
	char** local_vars = (char**)malloc(sizeof(char*) * 1);
	local_vars[0] = NULL;
	while (1)
	{
		dup2(stdin_fd, STDIN_FILENO);
		dup2(stdout_fd, STDOUT_FILENO);
		dup2(stderr_fd, STDERR_FILENO);

		int local_vars_ptr = 0;
		input[0] = 0;
		printf("MicroSha >> ");
		fflush(stdout);
		if (fgets(input, IN_SIZE, stdin) == NULL)
		{
			break;
		}
		input[strlen(input) - 1] = 0;
		if (strlen(input) == 0) continue;
		dolla_sign_op(input, local_vars);
		int new_argc = strlen(input);
		char temp_input[IN_SIZE];
		strcpy(temp_input, input);
		char **new_argv = (char **)malloc(sizeof(char *) * new_argc + 1);
		char *temp = strtok(temp_input, " ");
		int k = 0;
		while (temp != NULL)
		{
			new_argv[k] = (char *)malloc(strlen(temp) + 1);
			strcpy(new_argv[k], temp);
			k++;
			temp = strtok(NULL, " ");
		}
		new_argv[k] = NULL;
		new_argc = k;
		// I/O redirection
		bool redirection_success = 1;
		for (int i = 0; i < new_argc; i++)
		{
			int fd;
			if(!strcmp(new_argv[i], "<"))
			{
				fd = open(new_argv[i + 1], O_RDONLY);
				if (fd < 0)
				{
					fprintf(stderr, "cannot access %s: No such file or directory\n", new_argv[i + 1]);
					exit_code = -1;
					redirection_success = 0;
					close(fd);
					break;
				}
				if (dup2(fd, STDIN_FILENO) < 0)
				{
					printf("Error: Failed to redirect the input\n");
					exit_code = -1;
					redirection_success = 0;
					close(fd);
					break;
				}
				close(fd);
			}
			else if(!strcmp(new_argv[i], ">"))
			{
				fd = open(new_argv[i + 1], O_WRONLY | O_CREAT | O_TRUNC, 0644);
				if (fd < 0)
				{
					fprintf(stderr, "%s: Permission denied\n", new_argv[i + 1]);
					exit_code = -1;
					redirection_success = 0;
					close(fd);
					break;
				}
				if (dup2(fd, STDOUT_FILENO) < 0)
				{
					printf("Error: Failed to redirect the output\n");
					exit_code = -1;
					redirection_success = 0;
					close(fd);
					break;
				}
				close(fd);
			}
			else if(!strcmp(new_argv[i], "2>"))
			{
				fd = open(new_argv[i + 1], O_WRONLY | O_CREAT | O_TRUNC, 0644);
				if (fd < 0)
				{
					fprintf(stderr, "%s: Permission denied\n", new_argv[i + 1]);
					exit_code = -1;
					redirection_success = 0;
					close(fd);
					break;
				}
				if (dup2(fd, STDERR_FILENO) < 0)
				{
					printf("Error: Failed to redirect the error\n");
					exit_code = -1;
					redirection_success = 0;
					close(fd);
					break;
				}
				close(fd);
			}
		}
		if (!redirection_success)
		{
			for (int i = 0; i < new_argc; i++)
				free(new_argv[i]);
			free(new_argv);
			continue;
		}
		int j = 0;
		for (int i = 0; i < new_argc; i++)
			if (!strcmp(new_argv[i], "<") || !strcmp(new_argv[i], ">") || !strcmp(new_argv[i], "2>"))
				i++;
			else
				new_argv[j++] = new_argv[i];
		new_argc = j;
		new_argv[j] = NULL;
		// End of I/O redirection
		char* eq_pos = strchr(input, '=');
		if (eq_pos != NULL)
		{
			int local_vars_ptr = 0;
			if ( (*(eq_pos + 1) == ' ' || *(eq_pos - 1) == ' ') || (new_argc > 1 && strchr(new_argv[0], '=') != NULL) )
			{
				printf("Error: The assignment have to be in this literal format: key=value\n");
				exit_code = -1;
				continue;
			}
			else
			{
				char var[strlen(input) + 1];
				var[0] = 0;
				for (int i = 0; i < new_argc; i++)
				{
					if (strchr(new_argv[i], '=') != NULL)
					{
						strcpy(var, new_argv[i]);
						break;
					}
				}
				eq_pos = strchr(var, '=');
				char lso[strlen(var)], rso[strlen(var)];
				lso[0] = 0; rso[0] = 0;
				memcpy(lso, var, eq_pos - var);
				lso[eq_pos - var] = 0;
				strcpy(rso, eq_pos + 1);
				bool added = 0;
				for (local_vars_ptr = 0; local_vars[local_vars_ptr] != NULL; local_vars_ptr++)
				{
					char new_lso[strlen(local_vars[local_vars_ptr])];
					char* new_eq_pos = strchr(local_vars[local_vars_ptr], '=');
					new_lso[0] = 0;
					memcpy(new_lso, local_vars[local_vars_ptr], new_eq_pos - local_vars[local_vars_ptr]);
					new_lso[new_eq_pos - local_vars[local_vars_ptr]] = 0;
					if (!strcmp(lso, new_lso))
					{
						added = 1;
						strcpy(local_vars[local_vars_ptr], lso);
						strcat(local_vars[local_vars_ptr], rso);
						break;
					}
				}
				if (!added)
				{
					local_vars = (char**)realloc(local_vars, sizeof(char*) * (local_vars_ptr + 2));
					local_vars[local_vars_ptr] = (char*)malloc(strlen(var) + 1);
					local_vars[local_vars_ptr + 1] = NULL;
					local_vars[local_vars_ptr][0] = 0;
					strcpy(local_vars[local_vars_ptr], var);
					local_vars[local_vars_ptr][strlen(var)] = 0;
					local_vars[local_vars_ptr + 1] = NULL;
				}
				exit_code = 0;
				if (!strcmp(new_argv[0], var)) continue;
			}
		}
		if (!strcmp(new_argv[0], "export"))
		{
			if (new_argc != 2)
				printf("Usage: export 'key=value' or 'key' (but key has to be defined)\n");
			else
			{
				if (strchr(new_argv[1], '=') == NULL)
				{
					char* var = NULL;
					bool found = 0;
					for (local_vars_ptr = 0; local_vars[local_vars_ptr] != NULL; local_vars_ptr++)
					{
						char new_lso[strlen(local_vars[local_vars_ptr])];
						char* new_eq_pos = strchr(local_vars[local_vars_ptr], '=');
						new_lso[0] = 0;
						memcpy(new_lso, local_vars[local_vars_ptr], new_eq_pos - local_vars[local_vars_ptr]);
						new_lso[new_eq_pos - local_vars[local_vars_ptr]] = 0;
						if (!strcmp(new_argv[1], new_lso))
						{
							found = 1;
							var = (char*)malloc(strlen(local_vars[local_vars_ptr]) + 1);
                			strcpy(var, local_vars[local_vars_ptr]);
							break;
						}
					}
					if (!found)
					{
						printf("export: %s is not defined, please define it first.\n", new_argv[1]);
						exit_code = -1;
						free(var);
					}
					else
					{
						if (putenv(var) != 0)
						{
							printf("export: couldn't add the variable to environ because there isn't enough memory space\n");
							exit_code = -1;
							free(var);
						}
						else
							exit_code = 0;
					}
					
				}
				else
				{
					char* var = (char*)malloc(strlen(new_argv[1]) + 1);
					strcpy(var, new_argv[1]);
					if (putenv(var) != 0)
					{
						printf("export: couldn't add the variable to environ because there isn't enough memory space\n");
						exit_code = -1;
						free(var);
					}
					else
						exit_code = 0;
				}
			}
		}
		else if (!strcmp(new_argv[0], "echo"))
		{
			for (int i = 1; i < new_argc; i++)
			{
				printf("%s", new_argv[i]);
				if (i < new_argc - 1)
					printf(" ");
			}
			printf("\n");
		}
		else if (!strcmp(new_argv[0], "pwd"))
		{
			if (new_argc > 1)
				printf("Usage: 'pwd' command doesn't have arguments. Arguments are ignored.\n");
			char cwd[DIR_SIZE];
			if (getcwd(cwd, DIR_SIZE) == NULL)
			{
				printf("Error: Failed to get the pathname (pathname is too long)\n");
				exit_code = 1;
			}
			else
			{
				printf("%s\n", cwd);
				exit_code = 0;
			}
		}
		else if (!strcmp(new_argv[0], "cd"))
		{
			if (new_argc != 2)
				printf("Usage: cd [new_directory_path]\n");
			else if (chdir(new_argv[1]) < 0)
			{
				printf("cd: %s: No such file or directory\n", new_argv[1]);
				exit_code = 1;
			}
			else
				exit_code = 0;
		}
		else if (!strcmp(new_argv[0], "exit"))
		{
			if (new_argc > 1)
				printf("Usage: 'exit' command doesn't have arguments. Arguments are ignored.\n");
			printf("Good Bye\n");
			for (int i = 0; i < new_argc; i++)
				free(new_argv[i]);
			free(new_argv);
			break;
		}
		else
		{
			pid_t child_pid = fork();
			if (child_pid > 0)	//This is parent
			{
				int status;
				wait(&status);
				if (WIFEXITED(status))
					exit_code = WEXITSTATUS(status);
				else
					exit_code = 1;
			}
			else if (child_pid == 0)	//This is child
			{
				execvp(new_argv[0], new_argv);
				printf("%s: command not found\n", new_argv[0]);
				exit(-2);
			}
			else
				printf("UNIX CMD: Failed to fork\n");
		}
		for (int i = 0; i < new_argc; i++)
			free(new_argv[i]);
		free(new_argv);
	}

	for (int i = 0; local_vars[i] != NULL; i++)
	       	free(local_vars[i]);
	free(local_vars);

	close(stdin_fd);
	close(stdout_fd);
	close(stderr_fd);

	return exit_code;
}