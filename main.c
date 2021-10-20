#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
#include <limits.h>
#include <readline/readline.h>

#define ASH_READLINE_BUFFER_SIZE 1024

#define ASH_TOKEN_BUFFER_SIZE 64
#define ASH_TOKEN_DELIMETER " \t\r\n\a"

#define CYN  	"\x1B[36m"
#define GRN     "\x1B[32m"
#define RESET   "\x1B[0m"
#define BOLD	"\033[1m"
#define UNBOLD  "\033[22m"


int ash_cd(char**);
int ash_help(char**);
int ash_exit(char**);


void ash_loop(void);
char* ash_readline(void);
char** ash_split_line(char*);
int ash_launch(char**);
int ash_execute(char**);


char* builtin_str[] = { 
	"cd",
	"help",
	"exit"
};


int (*builtin_func[]) ( char** ) = {
	&ash_cd,
	&ash_help,
	&ash_exit
};

int ash_size_builtins(void) {
	return sizeof(builtin_str) / sizeof(char *);
}

int ash_cd(char** args) {
	if ( args[1] == NULL ) {
		fprintf(stderr, "ash: expected argument to \"cd\"\n");
	}
	else {
		if ( chdir(args[1]) != 0 ) {
			perror("ash");
		}
	}
	return 1;
}

int ash_help(char** args) {
	printf("Welcome to ash\n");
	printf("Type program names and arguments, and hit enter.\n");
	printf("The following are built in:\n");

	for ( int i=0; i<ash_size_builtins(); ++i ) {
		printf("%s\n", builtin_str[i]);
	}

	printf("Use the man command for information on other programs.\n");
	return 1;
}

int ash_exit(char** args) {
	return 0;
}

void ash_prompt() {
	char* user_name = getlogin();

	size_t size = 100;
	char working_dir[size];
	char* current_dir = getcwd(working_dir, size);

	printf(CYN BOLD "%s@" RESET UNBOLD, user_name);
	printf(GRN BOLD "%s" RESET UNBOLD, current_dir);
}

void ash_loop(void) {
	char* line;
	char** args;
	int status;

	do {
		ash_prompt();
		line = readline(BOLD"$ "UNBOLD);
		args = ash_split_line(line);
		status = ash_execute(args);
	
		free(line);
		free(args);

	} while ( status );
}

char** ash_split_line(char* line) {
	int buffer_size = ASH_TOKEN_BUFFER_SIZE;
	int position = 0;

	char** tokens = malloc( sizeof(char *) * buffer_size );
	char* token;

	if ( !tokens ) {
		fprintf(stderr, "ash: allocation error\n");
		exit(EXIT_FAILURE);
	}

	token = strtok(line, ASH_TOKEN_DELIMETER);
	while ( token != NULL ) {
		tokens[position] = token;
		position++;

		if ( position >= buffer_size ) {
			buffer_size += ASH_TOKEN_BUFFER_SIZE;
			tokens = realloc( tokens, buffer_size * sizeof(char *) );

			if ( !tokens ) {
				fprintf(stderr, "ash: allocation error\n");
				exit(EXIT_FAILURE);
			}
		}
		token = strtok(NULL, ASH_TOKEN_DELIMETER);
	}
	tokens[position] = NULL;

	return tokens;
}

int ash_launch(char** args) {
	pid_t pid, wpid;
	int status;

	pid = fork();
	if ( pid == 0 ) {
		if ( execvp(args[0], args) == -1 ) {
			perror("ash");
		}
		exit(EXIT_FAILURE);
	}
	else if ( pid < 0 ) {
		perror("ash");
	}
	else {
		do {
			wpid = waitpid(pid, &status, WUNTRACED);
		} while ( !WIFEXITED(status) && !WIFSIGNALED(status) );
	}

	return 1;
}

int ash_execute(char** args) {
	if ( args[0] == NULL ) {
		return 1;
	}

	for ( int i = 0; i < ash_size_builtins(); ++i ) {
		if ( strcmp(args[0], builtin_str[i]) == 0 ) {
			return (*builtin_func[i])(args);
		}
	}
	return ash_launch(args);
}


int main(int argc, char** argv) {

	system("clear");
	ash_loop();

	return EXIT_SUCCESS;
}