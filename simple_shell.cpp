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
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
using namespace std;

#define MAX_CMD_LINE 80

int main(void){
    
    char *args[MAX_CMD_LINE/2 +1];
    int run =1;
    int argc = 0;               //argument start
    vector<string>entered_input;
    vector<string>command_history;
    
    while(run == 1){
        int status = 1;             //int variable
        
        string input;              //variable initialize
        cout << "Enter Command:>";
        getline(cin, input);  //user input line
        
        istringstream iss(input);
        string InputAfterSpace;                  //second input after space
        while ( getline( iss, InputAfterSpace, ' ' ) ) {    //getline with space delimiter
            
            if(InputAfterSpace == "exit"){              //if user input exit, program stops
                cout << "Exiting the program..." << endl;
                run = 0; 
            }
            
            if(!InputAfterSpace.empty() && (InputAfterSpace != "history")){ 
                if(input[0] == '!'){
                    continue;
                }
                else{
                    entered_input.push_back(InputAfterSpace);
                    command_history.push_back(InputAfterSpace);
                    argc++;
                }
            } 
        }
        string com_arr[argc];                      //allocate array memory
        copy(command_history.begin(), command_history.end(), com_arr); //copy into array of strings
        if(input == "history"){ //get history of commands            
            if(command_history.empty()){                
                cout << "No history.." << endl;
            }
            if(command_history.size() > 10)
            {
                for(int i=argc; i> (command_history.size() -10); i--){
                    cout << i << ": " << com_arr[i-1] << endl;
                }
            }
            else
            {
                for(int i = argc; i>0; i--)
                {
                    cout<<i<< ": "<<com_arr[i-1] <<endl;
                }
            }
        }        
        
        string arr[argc];          //array for argument count
        copy(entered_input.begin(),  entered_input.end(), arr); //vector into array
        
        if(input != "history") {  //get and execute commands
            if(input[0] == '!' && input[1] != '!'){                
                int num = 0;                    
                string hold = &input[1]; 
                num = atoi(hold.c_str());  //conversion from string to int and
                if (num > command_history.size()) //error 
                {
                    cout << "No such command in history..." << endl;
                    status =0;                     
                }
                else{
                    num--;                     // decrease num by 1
                    args[0] = const_cast<char*>(entered_input.begin()[num].c_str()); 
                }
            }
            else  if(input == "!!"){
                if(command_history.empty())  //check command history
                {
                    cout << "No history.." << endl;  
                }
                else{
                    args[0] = const_cast<char*>(entered_input.end()[-1].c_str());
                }
            }
            
            else{
                args[0]= const_cast<char*>(entered_input.back().c_str());
            }
            // execute command
            if(status == 1 && (run == 1)){
                args[argc] = NULL; //The last element of args is always set to be NULL
                pid_t pid = fork();  //create child process
                
                if (pid < 0) {
                    cout << "Error" << endl;
                    return 1;
                }
                
                // Child process
                else if (pid == 0) {
                    execvp(args[0], args);
                    
                    return 0;
                }
                
                // Parent process
                else {
                    int childStatus;
                    waitpid(pid, &childStatus, 0);
                    run = 1;
                }
            }            
        }
    }    
    return 0;
}
