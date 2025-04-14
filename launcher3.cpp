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
namespace fs = std::filesystem;

class Executable {
public:
	string name;
	vector<string> confinedFiles;
};

string rootPath;
string cwd;
vector<Executable*> executables;





void updateCWD() {
	cwd = std::filesystem::current_path().string();

	string dir;
	for (size_t i=rootPath.size(); i<cwd.size(); ++i) 
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


vector<string> splitStringByChar(const string& str, char c) {
    vector<string> result;
    stringstream ss(str);
    string token;

    // Split the string by commas
    while (getline(ss, token, c)) {
        result.push_back(token);
    }

    return result;
}


bool hasSlash(string& s) {
	for (size_t i=0; i<s.size(); ++i) {
		if (s[i] == '/')
			return true;
	}
	return false;
}

bool containsDotDot(string& s) {
	return (s.find("..") != string::npos);
}


bool isExecutable(string& exeName) {
	vector<string> fileNames;

	// Iterate over the current directory (".")
    for (const auto& entry : fs::directory_iterator(".")) {
		if (entry.is_regular_file()) {
			fileNames.push_back(entry.path().filename().string());
		}
	}

	for (size_t i=0; i<fileNames.size(); ++i) {
		if (fileNames[i] == exeName)
			return true;
	}

	//must end in .exe
	if (exeName.substr(exeName.size() - 4, 4) != ".exe")
		return false;

	return false;
}

bool isValidPath(string& path) {
	return fs::exists(rootPath + path);
}


void addPermisionToExecutable(string& exeName, string& confinedFile) {
	for (size_t i=0; i<executables.size(); ++i) {
		if (executables[i]->name == exeName) {

			//if there already is a confined file with the same path, then replace it
			for (size_t j=0; j<executables[i]->confinedFiles.size(); ++j) {
				if (executables[i]->confinedFiles[j].substr(0, executables[i]->confinedFiles[j].size() - 4) == confinedFile.substr(0, confinedFile.size() - 4)) {
					executables[i]->confinedFiles[j] = confinedFile;
					return;
				}
			}
			executables[i]->confinedFiles.push_back(confinedFile);
			return;
		}
	}

	//if you reach here then there is no such exe, so add it
	Executable* newExe = new Executable;
	newExe->name = exeName;
	newExe->confinedFiles.push_back(confinedFile);
	executables.push_back(newExe);
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
		if (containsDotDot(directory)) {
			cout << "cannot use .. when specifying absolute path" << endl;
			return;
		}
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
	string command = "rm " + args[1];
	system(command.c_str());
}

void executeRMDIR(vector<string>& args) {
	if (args.size() != 2) {
		cout << "usage rmdir: `rm [file]`" << endl;
		return;
	}
	string command = "rm " + args[1] + " -r";
	system(command.c_str());
}

void executeCPFILE(vector<string>& args) {
	if (args.size() != 3) {
		cout << "usage cpfile: `cpfile [file] [destination]`" << endl;
		return;
	}

	string file = args[1];
	string dest = args[2];

	if (hasSlash(dest)) {
		if (containsDotDot(dest)) {
			cout << "cannot use .. when specifying absolute path" << endl;
			return;
		}
		//add the rootPath
		dest = rootPath + dest;
	}

	if (hasSlash(file)) {
		if (containsDotDot(file)) {
			cout << "cannot use .. when specifying absolute path" << endl;
			return;
		}
		//add the rootPath
		file = rootPath + file;
	}

	string command = "cp " + file + " " + dest;
	system(command.c_str());
}

void executeCPDIR(vector<string>& args) {
	if (args.size() != 3) {
		cout << "usage cpdir: `cpdir [directory] [destination]`" << endl;
		return;
	}

	string file = args[1];
	string dest = args[2];

	if (hasSlash(dest)) {
		if (containsDotDot(dest)) {
			cout << "cannot use .. when specifying absolute path" << endl;
			return;
		}
		//add the rootPath
		dest = rootPath + dest;
	}

	if (hasSlash(file)) {
		if (containsDotDot(file)) {
			cout << "cannot use .. when specifying absolute path" << endl;
			return;
		}
		//add the rootPath
		file = rootPath + file;
	}

	string command = "cp " + file + " " + dest + " -r";
	system(command.c_str());
}

void executeMV(vector<string>& args) {
	if (args.size() != 3) {
		cout << "usage mv: `mv [file] [directory]`" << endl;
		return;
	}

	string file = args[1];
	string directory = args[2];

	if (hasSlash(directory)) {
		if (containsDotDot(directory)) {
			cout << "cannot use .. when specifying absolute path" << endl;
			return;
		}
		//add the rootPath
		directory = rootPath + directory;
	}

	if (hasSlash(file)) {
		if (containsDotDot(file)) {
			cout << "cannot use .. when specifying absolute path" << endl;
			return;
		}
		//add the rootPath
		file = rootPath + file;
	}

	string command = "mv " + file + " " + directory;
	system(command.c_str());
}

void executeMKDIR(vector<string>& args) {
	if (args.size() != 2) {
		cout << "usage mkdir: `mkdir [directory]`" << endl;
		return;
	}

	//cannot make a directory called home.dir
	if (args[1] == "home.dir") {
		cout << "cannot make a directory called home.dir" << endl;
		return;
	}

	string newDir = args[1];
	string command = "mkdir " + newDir;
	system(command.c_str());
}

void executeRUN(vector<string>& args) {
	if (args.size() < 2) {
		cout << "usage run: `run [executable name] file1(xx),file2(xx),file3(xx)...` or `run [executable]`" << endl;
		return;
	}

	string exeName = args[1];

	if (!isExecutable(exeName)) {
		cout << args[1] << " is not an executable in the current working directory" << endl;
		return;
	}

	

	//if only two args, then we just run without setting any permissions
	if (args.size() == 2) {
		cout << "sending " << exeName << " from guard to UM..." << endl;
		sleep(5);
		cout << "done" << endl;
		cout << "running " << exeName << "!" << endl;
		return;
	}

	//this contains the file names of the files to which we are restring the executable
	vector<string> confinedFiles = splitStringByChar(args[2], ',');
	
	//all file names in confinedFiles should be absolute paths (for now), and must end in the permissions (ie (01), (10), (11))
	for (size_t i=0; i<confinedFiles.size(); ++i) {
		string confinedFile = confinedFiles[i];

		//it must contain (xx)
		if (confinedFile.size() <= 4) {
			cout << confinedFile << " is too short; confined files must contain (01), (10), (11)" << endl;
			return;
		}

		string path = confinedFile.substr(0, confinedFile.size() - 4);
		string permission = confinedFile.substr(confinedFile.size() - 4, 4);

		if (!isValidPath(path)) {
			cout << confinedFile << " has an invalid path; paths must be absolute" << endl;
			return;
		}

		if (permission != "(01)" && permission != "(10)" && permission != "(11)") {
			cout << confinedFile << " has an invalid permission; permissions are either (01), (10), (11)" << endl;
			return;
		}
	}

	cout << "sending " << exeName << " from guard to UM..." << endl;
	sleep(5);
	cout << "done" << endl;
	cout << "running " << exeName << "!" << endl;
}

void executeDISABLE(vector<string>& args) {
	if (args.size() != 2) {
		cout << "usage disable: `disable [peripheral]`" << endl;
		return;
	}

	if (args[1] != "monitor" && args[1] != "camera" && args[1] != "microphone" && args[1] != "speaker" && args[1] != "headphonejack" && args[1] != "keyboard" && args[1] != "trackpad" && args[1] != "network" && args[1] != "gps") {
		cout << args[1] << " is not a known peripheral" << endl;
		return;
	}

	cout << args[1] << " disabled" << endl;
}

void executeSETPERMS(vector<string>& args) {
	if (args.size() != 3) {
		cout << "usage setperms: `setperms [executable] file1(xx),file2(xx),file3(xx)..." << endl;
		return;
	}

	string exeName = args[1];

	//this contains the file names of the files to which we are restring the executable
	vector<string> confinedFiles = splitStringByChar(args[2], ',');

	//all file names in confinedFiles should be absolute paths (for now), and must end in the permissions (ie (01), (10), (11))
	for (size_t i=0; i<confinedFiles.size(); ++i) {
		string confinedFile = confinedFiles[i];

		//it must contain (xx)
		if (confinedFile.size() <= 4) {
			cout << confinedFile << " is too short; confined files must contain (01), (10), (11)" << endl;
			return;
		}

		string path = confinedFile.substr(0, confinedFile.size() - 4);
		string permission = confinedFile.substr(confinedFile.size() - 4, 4);

		if (!isValidPath(path)) {
			cout << confinedFile << " has an invalid path; paths must be absolute" << endl;
			return;
		}

		if (permission != "(01)" && permission != "(10)" && permission != "(11)") {
			cout << confinedFile << " has an invalid permission; permissions are either (01), (10), (11)" << endl;
			return;
		}

		addPermisionToExecutable(exeName, confinedFile);
	}
}

void executeSHOWPERMS(vector<string>& args) {
	if (args.size() != 2) {
		cout << "usage showperms: `showperms [executable]`" << endl;
		return;
	}

	string exeName = args[1];

	if (!isExecutable(exeName)) {
		cout << exeName << " is not an executable in the current working directory" << endl;
		return; 
	}

	//find it in `executables`
	for (size_t i=0; i<executables.size(); ++i) {
		if (executables[i]->name == exeName) {
			//print out the confined files
			for (size_t j=0; j<executables[i]->confinedFiles.size(); ++j) {
				cout << executables[i]->confinedFiles[j] << endl;
			}
			return;
		}
	}

	cout << exeName << " has no restrictions" << endl;
}

void executeCLEARPERMS(vector<string>& args) {
	if (args.size() != 2) {
		cout << "usage clearperms: `clearperms [executable]`" << endl;
		return;
	}

	string exeName = args[1];

	for (int i=0; i<executables.size(); ++i) {
		if (executables[i]->name == exeName) {
			executables[i]->confinedFiles.clear();
			return;
		}
	}

	cout << exeName << " is not an executable" << endl;
}

void parseCommand(const string& command) {
	vector<string> args;
	split(command, args);

	if (args.size() == 0)
		return;

	if (args[0] == "help") {
		cout << "cd\npwd\nls\nrm\nrmdir\ncp\nmv\nmkdir\nrun\ndisable\nsetperms\nshowperms\n";
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
	
	if (args[0] == "cpfile") {
		executeCPFILE(args);
		return;
	}

	if (args[0] == "cpdir") {
		executeCPDIR(args);
		return;
	}

	if (args[0] == "mv") {
		executeMV(args);
		return;		
	}
	
	if (args[0] == "mkdir") {
		//cannot make a directory called `home`
		executeMKDIR(args);
		return;		
	}

	if (args[0] == "run") {
		executeRUN(args);
		return;
	}
	
	if (args[0] == "disable") {
		executeDISABLE(args);
		return;
	}

	if (args[0] == "clearperms") {
		executeCLEARPERMS(args);
		return;
	}

	if (args[0] == "setperms") {
		executeSETPERMS(args);
		return;
	}

	if (args[0] == "showperms") {
		executeSHOWPERMS(args);
		return;
	}

	

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