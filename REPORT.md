****

# SShell: Simple Command Line interpreter 



## Summary
This program, we developed a simple shell that could take commands from users and execute them. It could support simple bash commands: echo, ls, date etc. Also, it supports any options for the commands. 

## Implementation 
The implementation of the program follows steps:
1. Parse the command by using strtok( ), then split it into three cases : command without pipe or output redirect; command with output direct; command with pipe
2.  If the command is built in, execute it using provided function from standard library( chdir..etc). Otherwise, execute it using execvp( ).
3. Manipulating the STDOUT_FILENO and STDIN_FILENO if we encounter output redirection or piping. 
4. Build some help functions to parse the command more easily ( implemented substring..etc).

## Transformation Assignment to Code
1. Rewrite the system call by combining fork, wait, and execvp. We chose execvp because it could support automatically search for executable in the $PATH environment. 
2. In order to make executing commands more convieniently, we designed a data structure that consists of an array of strings, a pointer to next command( also the same type data structure), and a string that consist of output direction. Because the execvp takes two parameters, when we parse the command, we would store the original command to the first index of array.

```c
struct command {
        char *argv[17];
        struct command * next_command;
        char write_to[CMDLINE_MAX];
};
```
3. From inspired https://stackoverflow.com/a/4214350, I implemented substring() to manually split the commands with strtok.
4. REDIRECTION: We parsed commands by detecting the occurence of '>', I implemented a function called "count_char()" which would return the number of occurence of specific character in the string using a for loop. Whenever, it detected the ">", it would split the string into two parts. The right hand side would be fed into the write-to field in the structure.

## Limitation
Our project could handle pipe, but when there is an output redirect at the end of the pipe, there will be some memory issues with structures.





















> Written with [StackEdit](https://stackedit.io/). `

    

## Limitation




















> Written with [StackEdit](https://stackedit.io/).
