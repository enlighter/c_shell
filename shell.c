#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <unistd.h>
#include <errno.h>
#include <pwd.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <readline/readline.h>
#include <readline/history.h>
#include "parse.h"   /*include declarations for parse-related structs*/


enum
BUILTIN_COMMANDS { NO_SUCH_BUILTIN=0, EXIT,JOBS};

	char *
buildPrompt()
{
	return  "";
}

int
isBuiltInCommand(char * cmd){

	if( strncmp(cmd, "exit") == 0){
		return EXIT;
	}
	if( strncmp(cmd, "history") == 0){
		return HISTORY;
	}
	if( strncmp(cmd, "jobs") == 0){
		return JOBS;
	}
	if( strncmp(cmd, "cd") == 0){
		return CD;
	}
	if( strncmp(cmd, "kill") == 0){
		return KILL;
	}
	if( strncmp(cmd, "help") == 0){
		return HELP;
	}
	return NO_SUCH_BUILTIN;
}


int main (int argc, char **argv)
{

	char * history[10];
	int history_index = 0;
	int jobs[10];
	int child_pid;
	int jobs_running = 0;
	int status;
	char * cmdLine;
	int i = 0;
	parseInfo *info; /*info stores all the information returned by parser.*/
	struct commandType *com; /*com stores command name and Arg list for one command.*/

#ifdef UNIX

	fprintf(stdout, "This is the UNIX version\n");
#endif

#ifdef WINDOWS
	fprintf(stdout, "This is the WINDOWS version\n");
#endif

	while(1){
		char cwd[1024];
		char hostname[1024];
		char username[1024];

		if(getcwd(cwd, sizeof(cwd)) == NULL)
			perror("getcwd() error");

		if(gethostname(hostname, sizeof(hostname)) != 0)
			perror("gethostname() error");

		if(getlogin_r(username, sizeof(username)) != 0)
			perror("getlogin_r() error");

		fprintf(stdout, "%s@%s[%s]:", username,hostname,cwd);

#ifdef UNIX
		cmdLine = readline(buildPrompt());
		if(cmdLine == NULL) {
			fprintf(stderr, "Unable to read command\n");
			continue;
		}
#endif	
		/*calls the parser*/
		info = parse(cmdLine);
		if(info == NULL){
			free(cmdLine);
			continue;
		}
		/*prints the info struct*/
		/*print_info(info);*/

		/*com contains the info. of the command before the first "|"*/
		com=&info->CommArray[0];
		if((com == NULL)  || (com->command == NULL)) {
			free_info(info);
			free(cmdLine);
			continue;
		}

		if(history_index < 10 ){
			history[history_index] = cmdLine;
			history_index ++;
		}
		else{
			free(history[0]);

			for(i = 0; i < 9; i++){
				history[i] = history[i+1];
			}
			history[history_index - 1] = cmdLine;
		}

		/*com->command tells the command name of com*/
		if(isBuiltInCommand(com->command) == EXIT){
			exit(1);
		}
		else if(isBuiltInCommand(com->command) == HISTORY){
			for(i = 0; i < history_index; i++){
				fprintf(stdout,"%d: %s\n", i+1,history[i]);
			}
		}
		else if(isBuiltInCommand(com->command) == JOBS){
			for(i = 0; i < 10; i++){
				if(jobs[i] !0){
					fprintf(stdout,"%d: %d\n", i+1,jobs[i]);
				}
				else{
					fprintf(stdout,"No Jobs Running");
				}
			}
		}
		else if(isBuiltInCommand(com->command) == CD){
			if (chdir(com->VarList[1] == -1 )
				fprintf(stderr,"no such directory\n");
			
		}
		else if(isBuiltInCommand(com->command) == KILL){
			if (kill(atoi(com->VarList[1] == -1 )))
				fprintf(stderr,"kill not sucessful\n");
			
		}
		else if(isBuiltInCommand(com->command) == HELP){
			fprintf(stderr,"kill job_number\n"
					"history\n"
					"jobs\n"
					"exit\n"
					"cd <relitave or absolute directory>\n");
		}
		else{
			/*redirect*/
			child_pid = fork();
			if(child_pid == 0){
				if(info->boolInfile){
					dup2(open(info->inFile, O_RDONLY),fileno(stdin));
				}
				if(info->boolOutfile){
					dup2(open(info->outFile,O_CREAT | O_WRONLY,S_IRUSR | S_IWUSR), fileno(stdout));
				}
				if(execvp(com->command,com->VarList) == -1){
					fprintf(stderr, "command not found.\n");
					exit(EXIT_FAILURE);
				}
			}

			/*run commands in background*/
			if(!(info->boolBackground)){
				do{
					waitpid(child_pid, &status, 0);
				}while(WIFEXITED(status) == 0 && !WIFSIGNALED(status));
			}
			else{
				for(i = 0; i < 10; i++){
					if(jobs[i] != 0){
						jobs[i] = child_pid;
						break;
					}
				}
			}
		
			free_info(info);
		}
		free(cmdLine);
	
	}/* while(1) */
	
	/*check for background jobs*/
	for(i = 0; i < 10; i++){
		if(jobs[i] != 0 && WIFEXITED(status) == 0 && WIFSIGNALED(status)){
			jobs[i] = 0;
		}
	}
}






