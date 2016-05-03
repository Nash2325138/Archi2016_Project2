Warning:
	1. If you cannot run this code or simply get all warning while running this code.
	   Please check your file execution permission by "ls -alt".
           If the permission of execution is denied, you may run "chmod 777 . -R" to get fully permission.
	2. Please check if you have a "testcases/" directory to store testcases.
	3. Usage of each file is writen below.

Usage:
	1. check massive testcases 
		After checking you have the correct directory structure, execute MassiveChecker by "./MassiveChecker" directly
	2. replace answer with new answer processed by new golden simulator
		Please check you have correct directory structure and have the correct golden simulator.
		Execute xxx by "./MassiveChecker (any word you like)". (EX: ./MassiveChecker 12345)
		After double checking you want to replace answer, the program will run.


Structure of this directory:

MassiveChecker/ --------- testcases/
                   |		Contain all testcases you want to judge.
                   |		You need to wrap each testcase with another directory like:
                   |			open_testcase1 -> iimage,bin, dimage.bin, ...
                   |
                   |----- golden_simulator/
                   |		Contain the golded simulator provided by TA. 
                   |		Please check the golden simulaotr is up-to-date before updating new answer to testcases.
                   |
                   |----- workspace/
                   |		The program will create automatically
                   |
                   |----- pipeline
                   |		Your simulator.
                   |
                   |----- ansCheck & ansCheck.c
                   |		The answer checking program writen by me. 
                   |		Use this program check answer by default.
                   |		If you think the massive output create by "diff" command is okay,
                   |			then you can get into MassiveChecker.c and:
                   |				1. comment out function "usingMyAnsCheckProgram()" in function checkAns()
                   |				2. un-comment fuction "usingDiff()" in next line
                   |				3. make file
                   |		This should work.
                   |
                   |----- MassiveChecker.c & MassiveChecker
                   |		The body of this program MassiveChecker.c will become MassiveChecker after "make" command
                   |
                   |----- result.log
                   |		Store the result of judge.(Same as the outputs on screen while execution)
                   |
                   |----- makefile
                   |		Only support compile MassiveChecker.c to MassiveChecker
                   |
                   |----- allpass
                   |		The ascii art you want to print out to congrat yourself if all pass!





