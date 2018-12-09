James Allender
Vincent Li
CISC-340 
Project 04
README

cacheSim.c - This file is the C source code for our cache simulator
    Compile using the command $make to build our project using the included make file
    Run with syntax $./sim - f "input file path" -b "block size in word" -s "number of set/line" -a "associativity"

Makefile - This is a makefile to be used with Make to build our project
	Run comand [$make] to create our project
	Run commad [$make clean] to clean the project for rebuilding

Project_4_Overview.pdf - Overview document describing our project.


README.me - This README file

Test Files (./testFiles/):
        test1.as.4.2.1: test example from the project description, to ensure program operates correctly
        test2.as.4.2.2: test to ensure the LRU functionality in the program operates currently.
        test3.as.4.2.2: test to ensure the write-back functionality operates correctly.
        test4.as.4.4.2: test to ensure the program "read hit" and "read miss" correctly.
        test5.as.4.4.2: test to ensure the program "write hit" and "write miss" correctly.
        test6.as.8.4.2: This test is verry simmilar to test 4 but it uses a block size of 8 to ensure that blocks are being placed correctly
        test7.as.4.1.8: This is again verry simmilar to test 4 but it tests a fully asoiative cache rather than a blend aprotch