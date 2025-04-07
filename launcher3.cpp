#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cctype> // For isspace()
#include <stack>
#include <sstream>
#include <unistd.h>  // for sleep()
#include <cstdlib>  // for system()
#include <filesystem>
using namespace std;



string rootPath;
string cwd;



void updateCWD() {
	cwd = std::filesystem::current_path().string();

	string dir;
	for (int i=rootPath.size(); i<cwd.size(); ++i) 
		dir += cwd[i];

	cwd = dir;
}


void split(const string& command, vector<string>& splitCommand) {
	string temp;

	//shave
	if (command[0] == ' ' || command[command.size() - 1] == ' ') {
		cout << "No spaces at the start or  end of command allowed." << endl;
		return;
	}


	for (size_t i=0; i<command.size(); ++i) {
		if (i == command.size() - 1) {
			temp += command[i];
			splitCommand.push_back(temp);
			break;
		}
		if (command[i] == ' ' && temp.size() != 0) {
			splitCommand.push_back(temp);
			temp.clear();
			continue;
		}
		if (command[i] == ' ' && temp.size() == 0) {
			continue;
		}

		temp += command[i];
	}

}


bool hasSlash(string s) {
	for (int i=0; i<s.size(); ++i) {
		if (s[i] == '/')
			return true;
	}
	return false;
}



void executeCD(vector<string>& args) {
	if (args.size() != 2) {
		cout << "usage cd: `cd [directory]`" << endl;
		return; 
	}

	//if user tried `cd ..` and cwd is home.dir, then error out
	if (args[1] == ".." && cwd == "/home.dir") {
		cout << "home.dir has no parent" << endl;
		return;
	}

	string directory = args[1];

	if (hasSlash(directory)) {
		//add the rootPath
		directory = rootPath + directory;
	}

	//just execute cd with this arg
	 if (chdir(directory.c_str()) != 0)
        std::cerr << "Failed to change directory to " << directory << std::endl;

    updateCWD();
}

void executePWD() {
	updateCWD();
	cout << cwd << endl;
}

void executeLS() {
	system("ls");
}

void executeRM(vector<string>& args) {
	if (args.size() != 2) {
		cout << "usage rm: `rm [file]`" << endl;
		return;
	}
	string temp = "rm " + args[1];
	system(temp.c_str());
}

void executeRMDIR(vector<string>& args) {
	if (args.size() != 2) {
		cout << "usage rmdir: `rm [file]`" << endl;
		return;
	}
	string temp = "rm " + args[1] + " -r";
	system(temp.c_str());
}


void parseCommand(const string& command) {
	vector<string> args;
	split(command, args);

	if (args.size() == 0)
		return;

	if (args[0] == "help") {
		cout << "cd\npwd\nls\nrm\nrmdir\ncp\nmv\nmkdir\nrun\ndisable\nsetperm\n";
		return;
	}

	if (args[0] == "cd") {
		executeCD(args);
		return;
	}

	if (args[0] == "pwd") {
		executePWD();
		return;
	}

	if (args[0] == "ls") {
		executeLS();
		return;	
	}

	if (args[0] == "rm") {
		executeRM(args);
		return;		
	}

	if (args[0] == "rmdir") {
		executeRMDIR(args);
		return;		
	}
	
	// if (args[0] == "cp") {
	// 	executeCP(args[1], args[2]);
	// 	return;
	// }

	// if (args[0] == "mv") {
	// 	executeMV(args);
	// 	return;		
	// }
	
	// if (args[0] == "mkdir") {
	// 	//cannot make a directory called `home`

	// 	return;		
	// }

	// if (args[0] == "run") {
	// 	executeRun(args);
	// 	return;
	// }
	
	// if (args[0] == "disable") {
	// 	executeDisable(args);
	// 	return;
	// }

	// if (args[0] == "setperm") {
	// 	executeSetperm(args[1]);
	// 	return;
	// }

	cout << "command not recognized" << endl;
}



int main() {
	string command;

	cout << "Welcome to the MoonCluster launcher. Written by Nicholas Belotserkovskiy." << endl << "\"One doesn't discover new lands without consenting to lose sight, for a very long time, of the shore.\" -Andre Gide" << endl;
	cout << "type `help` for a list of supported commands" << endl;

	rootPath = std::filesystem::current_path().string();

	cwd = std::filesystem::current_path().string();

	if (chdir("home.dir") != 0)
        std::cerr << "Failed to change directory to home.dir" << std::endl;

    updateCWD();


	while (true) {
		cout << ">";

		//read the command into `command`
		getline(cin, command);
		parseCommand(command);
	}
}