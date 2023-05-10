/*
	German Ramirez
	CS461 - Operating Systems
	Simple Shell Project to recieve and execute commands
*/

#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <cstring>
#include <sstream>
#include <iomanip>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <fcntl.h>
using namespace std;

#define MAX_CMD_LINE 2000

int main(void) {
    char *args[MAX_CMD_LINE/2 +1];
    int run = 1;
    vector<string> entered_input;
    vector<string> command_history;
    
    while (run == 1) {
        int status = 1;             //int variable
        int argc = 0;
        bool input_redirect = false; //bool variable for input redirection
        bool output_redirect = false; //bool variable for output redirection
        bool append_redirect = false; //bool variable for append output redirection
        string input_file;  //string variable for input file
        string output_file; //string variable for output file
        int pipefd[2];
        bool is_piping = false;
        string input;   //variable initialize
        
        cout << "Enter Command:>";
        getline(cin, input);    //user input line
        istringstream iss(input);
        string InputAfterSpace;     //second input after space
        
        while (getline(iss, InputAfterSpace, ' ')) {        //getline with space delimiter
            if (InputAfterSpace == "exit") {                //if user input exit, program stops
                cout << "Exiting the program..." << endl;
                run = 0;
                break;
            } else if (InputAfterSpace == "cd") {
                getline(iss, InputAfterSpace, ' '); // get the directory path after "cd"
                chdir(InputAfterSpace.c_str());
                continue;
            } else if (InputAfterSpace == "pwd") {  // get the directory path after "pwd"
                char cwd[1024];
                getcwd(cwd, sizeof(cwd));
                cout << cwd << endl;
                continue;
            } else if (InputAfterSpace == "echo") { // get the directory path after "echo"
                string output;
                getline(iss, output);
                cout << output << endl;
                continue;
            } else if (InputAfterSpace == "mkdir") {
                getline(iss, InputAfterSpace, ' '); // get the directory path after "mkdir"
                mkdir(InputAfterSpace.c_str(), 0777);
                continue;
            }

            if (!InputAfterSpace.empty() && (InputAfterSpace != "history")) {
                if (input[0] == '!') {
                    continue;
                } else {
                    entered_input.push_back(InputAfterSpace);
                    command_history.push_back(InputAfterSpace);
                    argc++;
                }
            }
            // I/O redirection support
            if (InputAfterSpace == "<") {
                input_redirect = true;
                getline(iss, input_file, ' ');
            } else if (InputAfterSpace == ">") {
                output_redirect = true;
                getline(iss, output_file, ' ');
            } else if (InputAfterSpace == ">>") {
                append_redirect = true;
                getline(iss, output_file, ' ');
            } else if (InputAfterSpace == "|") {
                is_piping = true;
            }
        }
        
        string com_arr[argc];           //allocate array memory
        copy(command_history.begin(), command_history.end(), com_arr); // copy into array of strings
        
        if (input == "history") {   //get history of commands
            int num = 0;
            if (command_history.empty()) {
                cout << "No history.." << endl;
            }
            if (num > command_history.size()) {
                for (int i = argc; i > (command_history.size() - 10); i--) {
                    cout << i << ": " << com_arr[i - 1] << endl;;
                }
            } else {
                for (int i = argc; i > 0; i--) {
                    cout << i << ": " << com_arr[i - 1] << endl;
                }
            }
        }
        
        string arr[argc];      //array for argument count
        copy(entered_input.begin(), entered_input.end(), arr);  //vector into array

        if (input != "history") {       //get and execute commands
            if (input[0] == '!' && input[1] != '!') {
                int num = 0;
                string hold = &input[1];
                num = atoi(hold.c_str());
                if (num > command_history.size()) {     //conversion from string to int
                    cout << "No such command in history..." << endl;
                    status = 0;
                } else {
                    num--;      //decrease num by 1
                    args[0] = const_cast<char*>(command_history[num].c_str());
                }
            } else if (input == "!!") {
                if (command_history.empty()) {  //check command history
                    cout << "No history.." << endl;
                } else {
                    args[0] = const_cast<char*>(entered_input.end()[-1].c_str());
                }
            } else {
                args[0] = const_cast<char*>(arr[0].c_str());
            }
        }
 
        // Check for input redirection
        if (input_redirect) {
            int fd = open(input_file.c_str(), O_RDONLY);
            if (fd < 0) {
                cout << "Error opening input file: " << input_file << endl;
                return 1;
            }
            dup2(fd, 0); // Redirect standard input to file descriptor
            close(fd);
        }
        // Check for output redirection
        if (output_redirect) {
            int fd = open(output_file.c_str(), O_WRONLY|O_CREAT|O_TRUNC, 0666);
            if (fd < 0) {
                cout << "Error opening output file: " << output_file << endl;
                return 1;
            }
            dup2(fd, 1); // Redirect standard output to file descriptor
            close(fd);
        }
        // Check for append redirection
        if (append_redirect) {
            int fd = open(output_file.c_str(), O_WRONLY|O_CREAT|O_APPEND, 0666);
            if (fd < 0) {
                cout << "Error opening output file: " << output_file << endl;
                return 1;
            }
            dup2(fd, 1); // Redirect standard output to file descriptor
            close(fd);
        }
        // Check for piping
        if (is_piping) {
            if (pipe(pipefd) == -1) {
                cout << "Error creating pipe" << endl;
                return 1;
            }
            pid_t pid2 = fork();
            if (pid2 == -1) {
                cout << "Error forking process" << endl;
                return 1;
            }
            else if (pid2 == 0) { // Child process to handle piping output
                close(pipefd[0]); // Close read end of pipe
                dup2(pipefd[1], 1); // Redirect standard output to write end of pipe
                close(pipefd[1]); // Close write end of pipe
                // Execute command before pipe
                if (execvp(args[0], args) == -1) {
                    cout << "Error executing command: " << args[0] << endl;
                    return 1;
                }
            }
            else { // Parent process to handle piping input
                wait(NULL); // Wait for child process to complete
                args[0] = const_cast<char*>(arr[argc-1].c_str()); // Get command after pipe
                args[1] = NULL;
                close(pipefd[1]); // Close write end of pipe
                dup2(pipefd[0], 0); // Redirect standard input to read end of pipe
                close(pipefd[0]); // Close read end of pipe
            }
        }
        // Execute command
        if (execvp(args[0], args) == -1) {
            cout << "Error executing command: " << args[0] << endl;
            return 1;
        }
    }
    return 0;
}