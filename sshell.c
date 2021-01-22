#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <ctype.h>
#include <fcntl.h>
#include <sys/stat.h>


#define CMDLINE_MAX 512
#define MAX_ARGVS 16


enum error_flags {
        ERR_TOOMANY_ARG,
        ERR_MISSING_CMD,
        ERR_NO_OUTPUT,
        ERR_CANT_OPEN,
        ERR_MISLOCATED_RED,
        ERR_CANT_CHDIR,
        ERR_CMD_NOTFOUND,
        NO_REDIRECT_NO_PIPE,
        SUCCESS_FLAG,
        WITH_ONLY_REDIRECT
};


enum parsing_state {
        STARTING_MODE,
        LOOKING_FOR_CHARS,
        WAITING_FOR_DEST,
        FINISHING_DEST,
        WAITING_FOR_NEXT_TOKEN
};


struct command {
        char *argv[17];
        struct command *next_command;
        char read_from[CMDLINE_MAX];
        char write_to[CMDLINE_MAX];
};


int first_index(char check, char cmd[CMDLINE_MAX]) {
        int index = -1;
        for (int i = 0; cmd[i] != '\0'; i++) {
                if (cmd[i] == check) {
                        index = i;
                        break;
                }
        }
        return index;
}


void substring(char original[CMDLINE_MAX], char substring[CMDLINE_MAX], int start, int end)
{
        memcpy(substring, &original[start], end-start);
        substring[end-start] = '\0';
}


int count_char(char check, char cmd[CMDLINE_MAX]) {
        int counter = 0;
        for (int i = 0; cmd[i] != '\0'; i++) {
                if (cmd[i] == check) {
                        counter++;
                }
        }
        return counter;
}


int command_len(char cmd[CMDLINE_MAX]) {
        int len = 0;
        for (int i = 0; cmd[i] != '\0'; i++) {
                len++;
        }
        return len;
}


int parse_command(struct command *obj, char cmd[CMDLINE_MAX]) {

        const char pipe = '|';
        const char redirect = '>';

        struct command *current_command = obj;

        if (strchr(cmd, pipe) == NULL && strchr(cmd, redirect) == NULL) {
        
                int counter = 0;
                char * current_str;
                // the usage of strtok is inspired from 
                // http://www.cplusplus.com/reference/cstring/strtok/
                current_str = strtok(cmd, " ");
                while (current_str != NULL)
                {
                        if (counter == 16) {
                                return ERR_TOOMANY_ARG;
                        }
                        current_command->argv[counter] = current_str;
                        counter++;
                        current_str = strtok(NULL, " ");
                }
                current_command->argv[counter] = NULL;
                return NO_REDIRECT_NO_PIPE;
        }

        if (strchr(cmd, redirect) != NULL) {

                int redirect_sign = first_index('>', cmd);
                char command_before[CMDLINE_MAX];
                char dest_after[CMDLINE_MAX];
                int start = 0;
                int command_length;
                command_length = command_len(cmd);
                substring(cmd, command_before, start, redirect_sign);
                substring(cmd, dest_after, redirect_sign+1, command_length);

                if (count_char('|', dest_after)) {
                        return ERR_MISLOCATED_RED;
                }

                if (dest_after[0] == '\0') {
                        return ERR_MISSING_CMD;
                } else {
                        char * dest_after_no_space;
                        dest_after_no_space = strtok(dest_after, " ");
                        if (!access(dest_after_no_space, F_OK) && access(dest_after_no_space, W_OK)) {
                                return ERR_CANT_OPEN;
                        } else {
                                strcpy(current_command->write_to, dest_after_no_space);
                                return WITH_ONLY_REDIRECT;
                        }
                }
        }
        return SUCCESS_FLAG;
}


void error_message(int error_code) 
{
        switch(error_code) {
                case ERR_TOOMANY_ARG:
                        fprintf(stderr,
                                "Error: too many process arguments\n");
                        break;
                case ERR_MISSING_CMD:
                        fprintf(stderr,
                                "Error: missing command\n");
                        break;
                case ERR_NO_OUTPUT:
                        fprintf(stderr,
                                "Error: no output file\n");
                        break;
                case ERR_CANT_OPEN:
                        fprintf(stderr,
                                "Error: cannot open output file\n");
                        break;
                case ERR_MISLOCATED_RED:
                        fprintf(stderr,
                                "Error: mislocated output redirection\n");
                        break;
                case ERR_CANT_CHDIR:
                        fprintf(stderr,
                                "Error: cannot cd into directory\n");
                        break;
                case ERR_CMD_NOTFOUND:
                        fprintf(stderr,
                                "Error: command not found\n");
                        break;
        }
}


int main(void)
{
        char cmd[CMDLINE_MAX];

        while(1) {
                char *nl;

                /* Pring prompt */
                printf("sshell@ucd$ ");
                fflush(stdout);

                /* Get command line */
                fgets(cmd, CMDLINE_MAX, stdin);

                /* Print command line if stdin is not provided by terminal */
                if (!isatty(STDIN_FILENO)) {
                        printf("%s", cmd);
                        fflush(stdout);
                }

                /* Detect if the command is empty, ignore the \n characters */
                if (strlen(cmd) < 2) {
                        continue;
                }

                /* Remove trailing newline from command line */
                nl = strchr(cmd, '\n');
                if (nl)
                        *nl = '\0';

                /* Builtin commands */
                if (!strcmp(cmd, "exit")) {
                        fprintf(stderr,
                                "Bye...\n");
                        fprintf(stderr,
                                "+ completed '%s' [0]\n",
                                cmd);
                        exit(0);
                }

                if (!strcmp(cmd, "pwd")) {
                        char current_dir[CMDLINE_MAX];
                        getcwd(current_dir, CMDLINE_MAX);
                        fprintf(stdout,
                                "%s\n",
                                current_dir);
                        fprintf(stderr,
                                "+ completed '%s' [0]\n",
                                cmd);
                        continue;
                }

                struct command command_obj_one;
                struct command command_obj_two;
                struct command command_obj_three;
                struct command command_obj_four;
                command_obj_one.next_command = &command_obj_two;
                command_obj_two.next_command = &command_obj_three;
                command_obj_three.next_command = &command_obj_four;
                char command[CMDLINE_MAX];
                strcpy(command, cmd);
                int return_state = parse_command(&command_obj_one, command);

                switch(return_state) {
                        case ERR_TOOMANY_ARG:
                                error_message(ERR_TOOMANY_ARG);
                                continue;
                        case ERR_MISLOCATED_RED:
                                error_message(ERR_MISLOCATED_RED);
                                continue;
                        case ERR_MISSING_CMD:
                                error_message(ERR_MISSING_CMD);
                                continue;
                        case ERR_CANT_OPEN:
                                error_message(ERR_CANT_OPEN);
                                continue;
                }

                if (!strcmp(command_obj_one.argv[0], "cd")) {
                        // if failed
                        if (chdir(command_obj_one.argv[1]) == -1) {
                                error_message(ERR_CANT_CHDIR);
                                fprintf(stderr,
                                        "+ completed '%s' [1]\n",
                                        cmd);
                        } else {
                                fprintf(stderr,
                                        "+ completed '%s' [0]\n",
                                        cmd);
                        }
                        continue;
                }

                if (return_state == NO_REDIRECT_NO_PIPE) {
                
                        /* Regular command with no output redirection or pipe*/
                        int pid = fork();

                        if (pid != 0) {
                        
                                int status;
                                wait(&status);
                                if (status != 0) {
                                        error_message(ERR_CMD_NOTFOUND);
                                        continue;
                                }
                                fprintf(stderr,
                                        "+ completed '%s' [%d]\n",
                                        cmd, WEXITSTATUS(status));
                        } else {

                                if (execvp(command_obj_one.argv[0], command_obj_one.argv) == -1) {
                                        exit(1);
                                }
                                exit(0);
                        }
                }

                if (return_state == WITH_ONLY_REDIRECT) {
                        /* Regular command with only redirection */
                        int pid = fork();

                        if (pid != 0) {
                        
                                int status;
                                wait(&status);
                                if (status != 0) {
                                        error_message(ERR_CMD_NOTFOUND);
                                        continue;
                                }
                                fprintf(stderr,
                                        "+ completed '%s' [%d]\n",
                                        cmd, WEXITSTATUS(status));
                        } else {
                                int fd1;
                                fd1 = open(command_obj_one.write_to, O_WRONLY | O_RDWR, 0644);
                                dup2(fd1, STDOUT_FILENO);
                                close(fd1);

                                if (execvp(command_obj_one.argv[0], command_obj_one.argv) == -1) {
                                        exit(1);
                                }
                                exit(0);
                        }
                }
        }

        return EXIT_SUCCESS;
}