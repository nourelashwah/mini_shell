
/*
 * 
 *
 * Template file.
 * You will need to add more code here to execute the command table.
 *
 * NOTE: You are responsible for fixing any bugs this code may have!
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <signal.h>
#include <fcntl.h>
#include <time.h>
#include <glob.h>
#include <wordexp.h>
#include <vector>
#include <string>
#include <cctype>
#include <iostream>
#include <sstream>
#include "command.h"
#include "tokenizer.h"


void parse(std::vector<Token> &tokens)
{
    Command currentCommand;
    SimpleCommand *currentSimpleCommand = new SimpleCommand();

    for (size_t i = 0; i < tokens.size(); i++)
    {
        if (tokens[i].type == TOKEN_EOF)
            break;

        if (tokens[i].type == TOKEN_COMMAND || tokens[i].type == TOKEN_ARGUMENT)
        {
            currentSimpleCommand->insertArgument(strdup(tokens[i].value.c_str()));
        }
        else if (tokens[i].type == TOKEN_REDIRECT)
        {
            i++;
	if (tokens[i].type != TOKEN_COMMAND && tokens[i].type != TOKEN_ARGUMENT)
	{
        printf("Error: missing file name after '>'\n");
        return;
	 }
            currentCommand._outFile = strdup(tokens[i].value.c_str());
        }
	else if (tokens[i].type==TOKEN_INPUT)
	{
	i++;
 	if (tokens[i].type != TOKEN_COMMAND && tokens[i].type != TOKEN_ARGUMENT)
        {
        printf("Error: missing file name after '<'\n");
        return;
         }
           
	currentCommand._inputFile = strdup(tokens[i].value.c_str());
	}
	 else if (tokens[i].type==TOKEN_ERROR)
        {
        i++;
	 if (tokens[i].type != TOKEN_COMMAND && tokens[i].type != TOKEN_ARGUMENT)
        {
        printf("Error: missing file name after '2>'\n");
        return;
         }
           
        currentCommand._errFile = strdup(tokens[i].value.c_str());
        }
	 else if (tokens[i].type==TOKEN_APPEND)
        {
        i++;
	 if (tokens[i].type != TOKEN_COMMAND && tokens[i].type != TOKEN_ARGUMENT)
        {
        printf("Error: missing file name after '>>'\n");
        return;
         }
           
        currentCommand._outFile = strdup(tokens[i].value.c_str());
	currentCommand._append = 1;
        }
	else if (tokens[i].type == TOKEN_BACKGROUND)
	{
    	currentCommand._background = 1;
	}
	else if (tokens[i].type == TOKEN_PIPE)
	{
	if (currentSimpleCommand->_numberOfArguments == 0)
    {
        printf("Error: missing command before '|'\n");
        return;
    }
   	 currentCommand.insertSimpleCommand(currentSimpleCommand);
    	 currentSimpleCommand = new SimpleCommand();
	}
	else if (tokens[i].type == TOKEN_REDIRECT_AND_ERROR)
	{
    	 i++;
	 if (tokens[i].type != TOKEN_COMMAND && tokens[i].type != TOKEN_ARGUMENT)
        {
        printf("Error: missing file name after '>>&'\n");
        return;
         }
           
    	 currentCommand._outFile = strdup(tokens[i].value.c_str());
    	 currentCommand._errFile= strdup(tokens[i].value.c_str());
    	 currentCommand._append = 1;
    	 currentCommand._out_error= 1;
	}
    }
	if (tokens[0].type == TOKEN_EOF)     // <-- NEW: empty line
        return;

    if (currentSimpleCommand->_numberOfArguments == 0)   // <-- NEW: "ls |" or "> out.txt"
    {
        printf("Error: missing command\n");
        return;
    }
    currentCommand.insertSimpleCommand(currentSimpleCommand);
    currentCommand.execute();
}

SimpleCommand::SimpleCommand()
{
    _numberOfAvailableArguments = 5;
    _numberOfArguments = 0;
    _arguments = (char **)malloc(_numberOfAvailableArguments * sizeof(char *));
}

void SimpleCommand::insertArgument(char *argument)
{
    if (_numberOfAvailableArguments == _numberOfArguments + 1)
    {
        _numberOfAvailableArguments *= 2;
        _arguments = (char **)realloc(_arguments,
                                      _numberOfAvailableArguments * sizeof(char *));
    }

    _arguments[_numberOfArguments] = argument;
    _arguments[_numberOfArguments + 1] = NULL;

    _numberOfArguments++;
}

Command::Command()
{
    _numberOfAvailableSimpleCommands = 1;
    _simpleCommands = (SimpleCommand **)
        malloc(_numberOfAvailableSimpleCommands * sizeof(SimpleCommand *));

    _numberOfSimpleCommands = 0;
    _outFile = 0;
    _inputFile = 0;
    _errFile = 0;
    _background = 0;
    _append = 0;
    _out_error = 0;
}

void Command::insertSimpleCommand(SimpleCommand *simpleCommand)
{
    if (_numberOfAvailableSimpleCommands == _numberOfSimpleCommands)
    {
        _numberOfAvailableSimpleCommands *= 2;
        _simpleCommands = (SimpleCommand **)realloc(_simpleCommands,
                                                    _numberOfAvailableSimpleCommands * sizeof(SimpleCommand *));
    }

    _simpleCommands[_numberOfSimpleCommands] = simpleCommand;
    _numberOfSimpleCommands++;
}

void Command::clear()
{
    for (int i = 0; i < _numberOfSimpleCommands; i++)
    {
        for (int j = 0; j < _simpleCommands[i]->_numberOfArguments; j++)
        {
            free(_simpleCommands[i]->_arguments[j]);
        }

        free(_simpleCommands[i]->_arguments);
        free(_simpleCommands[i]);
    }

    if (_outFile)
    {
        free(_outFile);
    }

    if (_inputFile)
    {
        free(_inputFile);
    }

    if (_errFile)
    {
        free(_errFile);
    }

    _numberOfSimpleCommands = 0;
    _outFile = 0;
    _inputFile = 0;
    _errFile = 0;
    _background = 0;
    _append = 0;
    _out_error = 0;
}

void Command::print()
{
    printf("\n\n");
    printf("              COMMAND TABLE                \n");
    printf("\n");
    printf("  #   Simple Commands\n");
    printf("  --- ----------------------------------------------------------\n");

    for (int i = 0; i < _numberOfSimpleCommands; i++)
    {
        printf("  %-3d ", i);
        for (int j = 0; j < _simpleCommands[i]->_numberOfArguments; j++)
        {
            printf("\"%s\" \t", _simpleCommands[i]->_arguments[j]);
        }
	printf("\n");
    }

    printf("\n\n");
    printf("  Output       Input        Error        Err&Out       Background\n");
    printf("  ------------ ------------ ------------ ------------ ------------\n");
    printf("  %-12s %-12s %-12s %-12s %-12s\n", _outFile ? _outFile : "default",
           _inputFile ? _inputFile : "default", _errFile ? _errFile : "default", _out_error == 1 ? _errFile : "default"
           ,_background ? "YES" : "NO");
    printf("\n\n");
}

void Command::execute()
{
    print();

    int n = _numberOfSimpleCommands;
    std::vector<int> pipes(2 * (n - 1));        // one pipe = 2 fds

    for (int i = 0; i < n - 1; i++)
    {
        if (pipe(&pipes[2 * i]) < 0)
        {
            perror("pipe");
            clear();
            return;
        }
    }

    pid_t pid = -1;
    for (int i = 0; i < n; i++)
    {
        pid = fork();

        if (pid < 0)
        {
            perror("fork");
            break;
        }
        else if (pid == 0)                      // child 
        {
            
            if (i > 0)
                dup2(pipes[2 * (i - 1)], 0);
            if (i < n - 1)
                dup2(pipes[2 * i + 1], 1);

            
            if (_inputFile && i == 0)
            {
                int fdin = open(_inputFile, O_RDONLY);
                if (fdin < 0)
                {
                    perror(_inputFile);
                    _exit(1);
                }
                dup2(fdin, 0);
                close(fdin);
            }

            if (_outFile && i == n - 1)
            {
                int flags = O_WRONLY | O_CREAT;
                if (_append)
                    flags |= O_APPEND;
                else
                    flags |= O_TRUNC;

                int fd = open(_outFile, flags, 0644);
                if (fd < 0)
                {
                    perror(_outFile);
                    _exit(1);
                }
                dup2(fd, 1);
                if (_out_error)               
                    dup2(fd, 2);
                close(fd);
            }

            if (_errFile && !_out_error && i == n - 1)
            {
                int fderr = open(_errFile, O_WRONLY | O_CREAT | O_TRUNC, 0644);
                if (fderr < 0)
                {
                    perror(_errFile);
                    _exit(1);
                }
                dup2(fderr, 2);
                close(fderr);
            }

          
            for (size_t k = 0; k < pipes.size(); k++)
                close(pipes[k]);

            execvp(_simpleCommands[i]->_arguments[0], _simpleCommands[i]->_arguments);
            perror("execvp");
            _exit(1);
        }
    }                                           

    // parent
    for (size_t k = 0; k < pipes.size(); k++)
        close(pipes[k]);

    if (_background == 0)
        waitpid(pid, NULL, 0);

    clear();
}

void Command::prompt()
{
    std::string input;
    while (true)
    {
        printf("myshell>");
        fflush(stdout);

        if (!std::getline(std::cin, input))   // Ctrl-D
        {
            printf("\n");
            break;
        }

        std::vector<Token> tokens = tokenize(input);
        parse(tokens);
    }
}

Command Command::_currentCommand;
SimpleCommand *Command::_currentSimpleCommand;

int main()
{
    Command::_currentCommand.prompt();
    return 0;
}
