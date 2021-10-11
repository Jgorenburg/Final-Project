NAME:
=====
	Li Fan, xx, xxx, (fill in your full names!) 

File directory:
===================================
    README.txt;
    makefile;

    main.c - main function;
    main.h - header file for main function;
    shared.c - c file that contains shared global variables;
    shared.h - header file for shared.c;
    parser.c - parser function;
    parser.h - header file for parser.c;
    jobs.c - c file that defines the jobs struct and struct functions;
    jobs.h - header file for jobs.c;
    linkedlist.c - c file that defines the linkedlist struct and struct functions;
    linkedlist.h - header file for linkedlist.c;
    executor.c - c file that containes functions that execute commands;
    executor.h - header file for executor.c;


How to Compile:
===============
    make
    make clean (to remove temp files)
    
How to Run:
===========
    Suppose the we are currently under the same directory as the executable file, i.e. 
    the executable file is "./shell", then use './shell' to run the program.
	

Basic Structures:
(!!!pls edit if you think anything needs change!!!)
=================
    In general, every time user input command line arguments, we have a parser that preprocess user input 
    and generates a global 2d array that can be refernced by other functions. Nect we use the executor to 
    analyze input arguments and to 1) detect and process special symbols ("&%;|><"); 2) detect and process 
    built-in commands; and 3) directly use execvp() to run commands. 

    To be more specific, each time we execute a command, we fork a child process to do it, and creates a "job" 
    structure to store the information (pid, groupid, state: fg/bg, status: suspended/running, temios settings), 
    and add it to a linked list. We use linkedlist to maintain a job list and both foreground and background 
    jobs are in the list. In particular, we adopt the asynchronous apporach to communite between child processes 
    and parent processes, and we avoid concurrency issues by masking signals when one process is updating the 
    job list.


Features, Known Bugs and Limitations:
!!!this part needs final update and double check!!!
=====================================
Fully implemented:  
    Backgrounding with &;
    Process suspension with Control-Z;
    Process groups;
    Ability to parse special symbols;
    Special symbols - ';' ;
    

Partially implemented:
    Attemped to implement pipe but did not succeed, but the main idea is correct, we think. See pipe_cmd().

Not implemented:
    Extra credits other than pipe.


Tests for Robustness and Correctness:
=====================================
    !!!!Need final comments on how the testing goes!!!!


Special Instructions to the grader:
===================================
N/A

