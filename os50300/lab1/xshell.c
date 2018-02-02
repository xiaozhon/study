/***********************************************************************
 * CSCI 503 Operating System Lab 1: a simple shell
 * 
 * Use a structure to store every user commands, then parse the command,
 * then execute it.
 *
 * @author: Xiaoyang Zhong
 ***********************************************************************/

// header files
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <unistd.h>


// input buffer size
#define BUFFER_SIZE 12000	
// max number of arguments, when the value is 30 (original value)
// Pegasus cannot even start the application
// change it to 20, so that the program runs well
#define MAX_ARGS 200
// max char length for each argument
#define MAX_ARG_LEN 300
// max number of commands (max number of pipes)
#define MAX_CMD_NUMBER 3000

// bool values
#define TRUE 1
#define FALSE 0

struct command_t {
	int argc;	// argument counter
	char* name;	// command name, i.e. ls, cd
	char* argv[MAX_ARGS];	// arguments
};

struct command_t command[MAX_CMD_NUMBER];	// store all commands
int pipeCounter;							// count the number of pipes

// functions, see function details below
int read_command(char buf[], int maxSize);		// read input
int parse_command(char buf[], int maxSize);	// parse input into commands
void exe_pipe_command(struct command_t cmd[], int cmdCount);	// execution of a long piped command
void exe_redirect(struct command_t cmd, int pipeTypes, int pipe_write, int pipe_read);	// handle pipes
int is_background(struct command_t *cmd);	// check whether a simple command is background
void do_cd(struct command_t cmd);			// for "cd" command
int exe_simple_command(struct command_t cmd);	// execution of each simple command

/** 
 * Read uses' input command line
 * 
 * @arguments: 
 *		char buf[]: read buffer
 *		int maxSize: max buffer size
 * @return TRUE, if the input size can fit in the buffer, valid input
 * @return FALSE, if the input cannot fit in the buffer, invalid input
**/
int read_command(char buf[], int maxSize) {
	char c;			// get input char
	int len = 0;

	c = getchar();

	while(c != '\n'){	// continue to get the next char
		buf[len] = c;
		c = getchar();
		len++;
	}
	if (len >= maxSize) {		// command too long, ignore it
		printf("Command too long! Please reEnter your command!\n");
		len = 0;
		return FALSE;
	} else {
		buf[len] = '\0';
		return TRUE;
	}
}

/** 
 * Parse the input into commands, and store the commands into command_t structure
 * 
 * @arguments: 
 *		char buf[]: input contents
 *		int maxSize: max buffer size
**/
int parse_command(char buf[], int maxSize){
	int i = 0;
	char* token;

	// initialize pipe counter
	pipeCounter = 0;		
	
	// get each simple command, partitioned by '|' 
	for(token = strtok(buf, " "); token != NULL; token = strtok(NULL, " ")){
//		printf("%s\n", token);
		if (*token == '|') {	// encounter '|', increase pipe counter
			command[pipeCounter].argv[i] = 0;
			if(command[pipeCounter].argc == 0) {	// no arguments before "|", wrong command
				printf("xshell: error near %c \n", '|');
				return 0;
			}
			pipeCounter++;		// the real number of pipes is pipeCounter + 1
			i = 0;			// reset argument index
			continue;
		}
		command[pipeCounter].argv[i] = token;
		command[pipeCounter].argc++;
		if (i == 0) {
			command[pipeCounter].name = command[pipeCounter].argv[0];
		}
		i++;
	}
	return 1;
	
}




/**
 * Execution of pipe commands 
 * There are four types of pipe commands:
 *		0. no pipe
 *		1. the first command, only has writing end
 *		2. the mid commands, has two pipe ends
 * 		3. the last command, only has reading end
 * @arguments: 
 *		command_t cmd[]: all parsed commands
 *		int cmdCount:	 the number of total commands
**/
void exe_pipe_command(struct command_t cmd[], int cmdCount){
	void exe_redirect(struct command_t, int, int, int);	// redirection
	
	int i;
	int pipeType;	// mark the type of commands
	
	int fd[pipeCounter][2];	// pipes
	
	for(i=0; i < pipeCounter; i++) {
		if(pipe(fd[i]) == -1) 	// initialize pipes
		{
			perror("pipe error");
			exit(1);
		}
//		printf("Create PIPE IDs: in -- %d, out -- %d \n", fd[i][0], fd[i][1]);
	}
	
	if(cmdCount == 1) {		// no pipe, execution the command directly
							// with pipe id 0, 0
		pipeType = 0;
		exe_redirect(cmd[0], pipeType, 0, 0);
	} else {
		for(i = 0; i < cmdCount; i++) {	// there are pipes here
			if(i == 0) {	// the first command
				pipeType = 1;	
				// write end is fd[0][1], read end is fd[0][0]
				exe_redirect(cmd[i], pipeType, fd[0][1], fd[0][0]);
			} else if (i == (cmdCount-1)) {
				pipeType = 3;	// the last command
				
				// the number of pipeCounter =  cmdCount - 1, 
				// but the max pipe index is pipeCounter - 1 = cmdCount - 2 = i - 1
				// the write end is fd[i - 1][1], the read end is fd[i - 1][0]
				exe_redirect(cmd[i], pipeType, fd[i - 1][1], fd[i - 1][0]);
			} else {
				pipeType = 2;	// the middle commands
				// the write end is fd[i][1], the read end is fd[i-1][0]
				exe_redirect(cmd[i], pipeType, fd[i][1], fd[i-1][0]);
			}

		}
	}
}

/**
 * Redirect pipes
 * There are four types of pipe commands redirection:
 *		0. no pipe
 *		1. the first command, only has writing end
 *		2. the mid commands, has two pipe ends
 * 		3. the last command, only has reading end
 * @arguments:
 *		command_t cmd:	each simple command
 *		int pipeTypes: 	the type of the command, and its pipe
 *		int pipe_write:	the write end of the pipe of this cmd
 *		int pipe_read:	the read end of the pipe of this cmd
**/
void exe_redirect(struct command_t cmd, int pipeTypes, int pipe_write, int pipe_read) {
	int exe_simple_command(struct command_t);
	int is_background(struct command_t*);	// check wether the command is background
	
	int childPid;
	int childStatus;	// for wait
	int background = 0;	
	
//	if(cmd.argc == 0) {
//		printf("xshell: wrong command\n");
//		return;
//	}

	switch (pipeTypes) {
		case 0:
			// no redirection is needed, execute the command directly
		//	printf("\n ######## NO PIPE! ######## \n \n");
			exe_simple_command(cmd);
			break;
		case 1:		// first child
//			printf("execute the first command in the pipe. The command is: %s \n", cmd.name);
			background = is_background(&cmd);
			if ((childPid = fork()) == 0) {
				if (pipe_write != STDOUT_FILENO) {	// first child, pipe[1] redirect to STDOUT
					
					if (dup2(pipe_write, STDOUT_FILENO) != STDOUT_FILENO) {
						perror("dup2 OUT error, first child");
						exit(1);
					}
				}
				close(pipe_write);// close pipe
				close(pipe_read);
				
				exe_simple_command(cmd);	// execute the command
				exit(1);
			} else if (background == 0){
				close(pipe_write);
				wait(&childStatus);
			//	childPid = waitpid(childPid, &childStatus, 0);
			} 
			break;
		case 2:		// middle children
//			printf("execute the mid commands in the pipe, the command is: %s \n", cmd.name);
			background = is_background(&cmd);
			if ((childPid = fork()) == 0) {
				if (pipe_read != STDIN_FILENO) {	// mid child, redirect pipe[0] to STDIN
					if (dup2(pipe_read, STDIN_FILENO) != STDIN_FILENO) {
						perror("dup2 IN error");
						exit(1);
					}
					close(pipe_read);
					if (dup2(pipe_write, STDOUT_FILENO) != STDOUT_FILENO) {	// redirect pipe[1] to STDOUT
						perror("dup2 OUT error");
						exit(1);
					}
					close(pipe_write);
				}
				
				exe_simple_command(cmd);	// execute the command
				exit(1);
			} else if (background == 0){
				close(pipe_write);
				wait(&childStatus);
			//	childPid = waitpid(childPid, &childStatus, 0);
			} 
			break;
		case 3:
//			printf("execute the last commands in the pipe, the command is: %s \n", cmd.name);
			
			background = is_background(&cmd);
			
			if ((childPid = fork()) == 0) {
				if (pipe_read!= STDIN_FILENO) {	// last child, pipe[0] redirect to STDIN
					if (dup2(pipe_read, STDIN_FILENO) != STDIN_FILENO) {
						perror("dup2 IN error, last child");
						exit(1);
					}
				}
				close(pipe_write);// close pipe
				close(pipe_read);
				
				exe_simple_command(cmd);	// execute the command
				exit(1);
			} else if (background == 0){	// not backgound command
				close(pipe_write);
				wait(&childStatus);
			//	childPid = waitpid(childPid, &childStatus, 0);
			} 
			break;
		default:
			break;
	}

}

/**
 * Check whether the command is backgournd (check for '&')
 * @arguments:
 *		command_t *cmd: the command
 *
 * @return 1, if it is a background command
 * @return 0, if it is not a backgound command
 **/
int is_background(struct command_t *cmd){	
	if (*cmd->argv[cmd->argc - 1] == '&') {
//		printf("this is a backgound command \n");
		cmd->argv[cmd->argc - 1] = NULL;
		cmd->argc = cmd->argc - 1;

		return 1;
	}	
	else 
		return 0;
 }
 
 /**
  * cd command
  *
  * @arguments:
  *		command_t cmd: a simple command
 **/
 void do_cd (struct command_t cmd){
	if(cmd.argv[1] != NULL) {
		if (chdir(cmd.argv[1]) < 0)
			printf("cd Command Error! \n");
	}
 }
 
/**
 * Execute each simple command
 *
 * @arguments:
 * 		command_t cmd: the simple command
 **/
int exe_simple_command(struct command_t cmd){
	int i;
	int childPid;
	int childStatus;
	int background;
	int re_in = 0;		// mark whether we need redirect in ('<')
	int re_out = 0;		// mark whether we need redirect out ('>')
	char *in_file;
	char *out_file;
	
	int fd_in;
	int fd_out;
	
	background = is_background(&cmd);

	if(cmd.argc == 0){
		printf("xshell: command error!\n");
		return 0;
	}

	in_file = (char *)malloc((MAX_ARG_LEN)*(sizeof(char)));
	out_file = (char *)malloc((MAX_ARG_LEN)*(sizeof(char)));
	
//	printf("now the command is: %s \n", cmd.argv[0]);
	
	if(strcmp(cmd.argv[0], "cd") == 0) {	// internal command "cd"
		do_cd(cmd);
		return 1;
	}
	
	for (i = 0; i < cmd.argc; i++) {
		if (*cmd.argv[i] == '<') {	// check for '<'
//			printf("there is a '<' here\n");
			re_in = 1;
			cmd.argv[i] = 0;
			if (cmd.argv[i + 1] != NULL) {	// the file name is the argv after '<'
				
				in_file = cmd.argv[i + 1];
			//	printf("in file is: %s \n ", in_file);
			} else {
			//	perror("Input file missed!");
				in_file = '\0';
			}
			
			break;
		}	
		if (*cmd.argv[i] == '>') {	// check for '>'
			re_out = 1;
			cmd.argv[i] = 0;
			if (cmd.argv[i + 1] != NULL) {	// the file name is the argv after '>'
				out_file = cmd.argv[i + 1];
			} else {
			//	perror("Output file missed!");
				out_file = '\0';
			}
			break;
		}	
	}
	
	if((childPid = fork()) == 0) {	// create child process to execute the command
		if(re_in == 1) {	// redirectin put
			fd_in = open(in_file, O_RDONLY, S_IRUSR|S_IWUSR );
			if(fd_in == -1) {	// open in file error
				perror("Cannot open input file!");
				exit(1);
			}
			if (dup2(fd_in, STDIN_FILENO) != STDIN_FILENO)
			{
				perror("dup2-fd_in");
				exit(1);
			}
		}
		if(re_out == 1) {	// redirect output
			fd_out = open(out_file, O_CREAT|O_TRUNC|O_WRONLY, 0644);
			if(fd_out == -1) {	// open in file error
				perror("Cannot open output file!");
				exit(1);
			}
			if (dup2(fd_out, STDOUT_FILENO) != STDOUT_FILENO)
			{
				perror("dup2-fd_out");
				exit(1);
			}
		}

		if(execvp(cmd.name, cmd.argv) < 0) {	// actual execution
			perror("execvp-simple_command"); 
			exit(1);
		}
		
		
	} else if(background == 0) {	// no background command
		wait(&childStatus);
	//	childPid = waitpid(childPid, &childStatus, 0);
	}
	// if there is background command, there will be an zombie if parent terminates,
	// kill the zombie child at main function
	return 1;
}
/**
 *	Main function
**/
int main(int argc, char* argv[]) { 
	int i,j;
	int zombiePid;
	// variables
	char* currentPath;		// get the current working path
	char inBuffer[BUFFER_SIZE];	// input buffer	
	int inputValid = FALSE;		// the input line is valid or not (TRUE or FALSE)
//	printf("I am here 1\n");
	// initilization of the command structure
	for ( i = 0; i < MAX_CMD_NUMBER; i++) {
                command[i].argc = 0;
                command[i].name = (char*)malloc(MAX_ARG_LEN);
//              printf("I am here 2, Max is: %d, i is: %d\n", MAX_CMD_NUMBER, i);
		for(j = 0; j < MAX_ARGS; j++){
			command[i].argv[j]=(char *)malloc(MAX_ARG_LEN);
//			printf("I am here 3, MAX_ARGS is: %d, j is: %d \n", MAX_ARGS, j);
		}
	}
//	printf("I am here 3\n");
	while(1) {
		currentPath = getcwd(NULL, 0);		// print prompt
		printf("XSHELL:%s> ", currentPath);
		
		// clear commands buffer
		for(i = 0; i < MAX_CMD_NUMBER; i++) {
			command[i].argc = 0;
			command[i].name = '\0';
			for(j = 0; j < MAX_ARGS; j++){
				command[i].argv[j]='\0';
			}
		}
		
		//read input command
		inputValid = read_command(inBuffer, BUFFER_SIZE);

		// we don't process invalid command
		if (inputValid == FALSE) {			
			continue;
		} else {
			if (inBuffer[0] == '\0') 
				continue;
				
	//		printf("input line is: %s \n", inBuffer);
			
			// quit shell
			if ((strcmp(inBuffer,"exit")==0) || (strcmp(inBuffer,"quit")==0)) 
				break;
			
			// kill all zombies
			zombiePid = wait(NULL);
			if(zombiePid >= 0) {
				printf("There is a Zombie in last command, zombie pid is: %d \n", zombiePid);
			}
			
			// parse the command 
			int parseResult = parse_command(inBuffer, BUFFER_SIZE);
			
	/*		// print each command and its arguments
			for(i = 0; i <= pipeCounter; i++) {
				for(j = 0; j < command[i].argc; j++){
					printf("%d th command, name is: %s, %d th argument: %s \n", i, command[i].name, j, command[i].argv[j]);
				}
			}
	*/		
			// execute the command
			// the real number of pipes is pipeCounter + 1
			if(parseResult) {
				exe_pipe_command(command, pipeCounter + 1);
			}

		}

	}
 	for( i = 0; i < MAX_CMD_NUMBER; i++) {
                free( command[i].name);
                for(j = 0; j < MAX_ARGS; j++){
  	             free(command[i].argv[j]);
		}
	}
	return 0;
}
