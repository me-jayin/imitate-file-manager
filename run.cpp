#include<stdio.h>
#include<stdlib.h>
#include<iostream>
#include<fstream>
#include "stringu.cpp" 
#include "user.cpp"
#include "bitmap.cpp" 
#include "file.cpp"
#include "file-manager.cpp"
#include "command.cpp"
using namespace std;

int main() {
	Command *cmd = new Command(loginForm(), initFileManager(false));
	cmd->command();
} 
