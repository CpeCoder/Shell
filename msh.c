// The MIT License (MIT)
// 
// Copyright (c) 2016 Trevor Bakker 
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
// 
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.

#define _GNU_SOURCE

#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <signal.h>

#define WHITESPACE " \t\n"      // We want to split our command line up into tokens
                                // so we need to define what delimits our tokens.
                                // In this case  white space
                                // will separate the tokens on our command line

#define MAX_COMMAND_SIZE 255    // The maximum command-line size

#define MAX_NUM_ARGUMENTS 11     // Mav shell only supports ten arguments

int main()
{
  char hist_arr[15][MAX_COMMAND_SIZE];   // stores strings of history to 15
  char * command_string = (char*) malloc( MAX_COMMAND_SIZE );
  int hist_p[15];                        // used for storing pid_id on inputs
  int hist_count=0;                      // counting inputs and managing hist_arr

  while( 1 )
  {
    // Print out the msh prompt
    printf ("msh> ");

    // Read the command from the commandline.  The
    // maximum command that will be read is MAX_COMMAND_SIZE
    // This while command will wait here until the user
    // inputs something since fgets returns NULL when there
    // is no input
    while( !fgets (command_string, MAX_COMMAND_SIZE, stdin) );
    
    /* Parse input */
    char *token[MAX_NUM_ARGUMENTS];

    for( int i = 0; i < MAX_NUM_ARGUMENTS; i++ )
    {
      token[i] = NULL;
    }

    int   token_count = 0;                                 
                                                           
    // Pointer to point to the token
    // parsed by strsep
    char *argument_ptr = NULL;                                         
                                                           
    char *working_string  = strdup( command_string );                

    // we are going to move the working_string pointer so
    // keep track of its original value so we can deallocate
    // the correct amount at the end
    char *head_ptr = working_string;

    // Tokenize the input strings with whitespace used as the delimiter
    while ( ( (argument_ptr = strsep(&working_string, WHITESPACE ) ) != NULL) && 
              (token_count<MAX_NUM_ARGUMENTS))
    {
      token[token_count] = strndup( argument_ptr, MAX_COMMAND_SIZE );
      if( strlen( token[token_count] ) == 0 )
      {
        token[token_count] = NULL;
      }
        token_count++;
    }

    // condition checks for blank entry
    if(token[0] != NULL)
    {
      // need to implement '!' first to avoid error as #history changes
      if(token[0][0] == '!')
      {
        char cpy[10];
        strcpy(cpy, strtok(token[0], "!"));
        int x = atoi(cpy);             // making char to int
        strcpy(command_string, hist_arr[x]);
        // re-assigning variables to spread string from history (104-112)
        working_string  = strdup( command_string );  
        argument_ptr = NULL;
        token_count = 0;

        for( int i = 0; i < MAX_NUM_ARGUMENTS; i++ )
        {
          token[i] = NULL;
        }
        
        // as the whole input string was displayed in the history we need to spread them
        // by using given code by professor
        while ( ( (argument_ptr = strsep(&working_string, WHITESPACE ) ) != NULL) && 
                (token_count<MAX_NUM_ARGUMENTS))
        {
          token[token_count] = strndup( argument_ptr, MAX_COMMAND_SIZE );
          if( strlen( token[token_count] ) == 0 )
          {
            token[token_count] = NULL;
          }
            token_count++;
        }
      }
      // changes/adds strings in history as per the # of inputs 
      if(hist_count == 15)
      {
        for(int i=0; i<14; i++)
        {
          strcpy(hist_arr[i], hist_arr[i+1]);
          hist_p[i] = hist_p[i+1];
        }
        strcpy(hist_arr[14], command_string);
        hist_p[14] = -1;
        hist_count = 15;
      }
      else
      {
        strcpy(hist_arr[hist_count], command_string);
        hist_count++;
      }
    
      // functions
      if(!strcmp(token[0], "exit") || !strcmp(token[0], "quit"))
      {
        exit(0);
      }
      else if(!strcmp(token[0], "cd"))
      {   
        chdir(token[1]);
        hist_p[hist_count-1] = -1;    // need to decreament the count as hist_arr increases it
      }
      else if(!strcmp(token[0], "history"))
      {
        hist_p[hist_count-1] = -1;
        if(token[1] == NULL)
        {
          for (int i = 0; i < hist_count; i++)
          {
            printf("%d: %s", i, hist_arr[i]);
          }
        }
        else if(!strcmp(token[1], "-p"))
        {
          for (int i = 0; i < hist_count; i++)
          {
            printf("%d: %d  %s", i, hist_p[i], hist_arr[i]);
          }
        }
        else
        {
          printf("%s: Command not found.", command_string);
        }
      }
      else
      {
        pid_t pid = fork( );
        if( pid == 0 )          // executes all functions using execvp as it doesn't require path
        {
          // Notice you can add as many NULLs on the end as you want
          int ret = execvp( token[0], &token[0] );  
          if( ret == -1 )
          {
            printf("%s: Command not found.\n", token[0]);
            return 0;
          }
        }
        else                    // parent waits and add pid_id to array
        {
          int status;
          wait( & status );
          hist_p[hist_count-1] = (int)pid;
        }
      }

      // Cleanup allocated memory
      for( int i = 0; i < MAX_NUM_ARGUMENTS; i++ )
      {
        if( token[i] != NULL )
        {
          free( token[i] );
        }
      }

      free( head_ptr );
    }
  }

  free( command_string );

  return 0;
  // e2520ca2-76f3-90d6-0242ac120003
}
