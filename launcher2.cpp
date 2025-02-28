#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cctype> // For isspace()
#include <stack>
#include <sstream>
#include <unistd.h>  // for sleep()
using namespace std;


class File {
public:
	string name;
	bool isDir;
	vector<File*> contents;
	File* parent;
	bool deleted;

    // Method to display values
    void display() {
        cout << "isDir: " << isDir << ", deleted: " << deleted << ", name: " << name <<  endl;
    }
};



File* cwd;
File* homePointer;
File* currentlyRunningExecutable = nullptr;

///////////// FILE SYSTEM INITIALIZATION STUFF IS BELOW ////////////////////


// Function to remove all whitespace characters from the string
string removeWhitespace(const string& input) {
	string result;

	// Loop through each character in the input string
	for (char ch : input) {
		// If the character is not a whitespace character, add it to the result string
		if (!isspace(static_cast<unsigned char>(ch))) {
			result += ch;
		}
	}

	return result;
}


void createFilesRecursively(File* parentFilePtr, string& str) {
	File* root = new File;
	int firstBracket;

	for (size_t i=1; i<str.size(); ++i) {
		if (str[i] == '[') {
			firstBracket = i;
			break;
		}
		root->name += str[i];

		if (i == str.size() - 2) {		// -2 because of the ending `}`
			//then this is not a directory
			root->isDir = false;

			//connect this file to its parent
			root->parent = parentFilePtr;
			parentFilePtr->contents.push_back(root);

			return;

		}
	}

	root->isDir = true;



	
	//connect this file to its parent
	root->parent = parentFilePtr;
	parentFilePtr->contents.push_back(root);




	//firstBracket points to first bracket in str

	if (str[firstBracket + 1] == ']') {
		root->isDir = true;
		return;
	}

	//now create all children
	int bracketDepth = 0;
	
	vector<string> children;
	string temp;
	for (size_t i = firstBracket + 1; i<str.size() - 2; ++i) {		// -2 because of the end `]}`
		
		temp += str[i];

		if (str[i] == '{')
			++bracketDepth;

		if (str[i] == '}')
			--bracketDepth;

		if (bracketDepth == 0) {
			//this is the last char of the child's string
			children.push_back(temp);
			temp.clear();
			continue;
		}


	}

	for (size_t i=0; i<children.size(); ++i) {
		createFilesRecursively(root, children[i]);
	}




}




//REQUIRES: `str` must have correct syntax. createFiles will not check the syntax.
//RETURNS: a pointer to the root node
File* createFiles(string& str) {
	File* rootPtr = new File;
	int firstBracket;

	for (size_t i=1; i<str.size(); ++i) {
		if (str[i] == '[') {
			firstBracket = i;
			break;
		}
		rootPtr->name += str[i];

		if (i == str.size() - 1) {
			cout << "couldn't find root file" << endl;
			exit(1);
		}
	}

	//root is always a directory
	rootPtr->isDir = true;


	//firstBracket points to first bracket in str

	//now create all children
	int bracketDepth = 0;
	
	vector<string> children;
	string temp;
	for (size_t i = firstBracket + 1; i<str.size() - 2; ++i) {		// -2 because of the end `]}`
		
		temp += str[i];

		if (str[i] == '{')
			++bracketDepth;

		if (str[i] == '}')
			--bracketDepth;

		if (bracketDepth == 0) {
			//this is the last char of the child's string
			children.push_back(temp);
			temp.clear();
			continue;
		}

	}


	for (size_t i=0; i<children.size(); ++i) {
		createFilesRecursively(rootPtr, children[i]);
	}

	return rootPtr;

}





///////////// FILE SYSTEM INITIALIZATION STUFF IS ABOVE ////////////////////






///////////// SHELL STUFF IS BELOW ////////////////////

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



//finds the file named `str` in cwd and returns a pointer to it
File* getFileFromNameInCWD(string& str) {
	for (size_t i=0; i<cwd->contents.size(); ++i) {
		if (cwd->contents[i]->name == str)
			return cwd->contents[i];
	}
	return nullptr;
}




void executeCD(string& fileName) {

	if (fileName == "..") {

		if (cwd->parent == NULL) {
			cout << "home.dir has no parent." << endl;
			return;
		}
		cwd = cwd->parent;
		return;
	}

	for (size_t i=0; i<cwd->contents.size(); ++i) {
		if (fileName == cwd->contents[i]->name) {
			if (!cwd->contents[i]->isDir) {
				cout << fileName << " is not a directory" << endl;
				return;
			}
			cwd = cwd->contents[i];
			return;
		}
	}
	cout << fileName << " not found" << endl;
}

void executePWD() {
	File* temp = cwd;

	vector<string> path;

	while (true) {
		if (temp->parent == NULL && temp->name == "home.dir") { 
			
			path.push_back(temp->name);
			break;
		}

		path.push_back(temp->name);
		temp = temp->parent;
	} 

	for (int i=path.size() - 1; i>=0; --i) {
		cout << "/" << path[i];
	}
	cout << endl;
}

void executeLS() {
	for (size_t i=0; i<cwd->contents.size(); ++i) {
		cout << cwd->contents[i]->name << endl;
	}
}

void executeRM(string& fileName) {
	for (size_t i=0; i<cwd->contents.size(); ++i) {
		if (fileName == cwd->contents[i]->name) {
			if (cwd->contents[i]->isDir) {
				cout << fileName << " is a directory--rm only removes files, use rmdir to remove directories" << endl;
				return;
			}
			cwd->contents.erase(cwd->contents.begin() + i);
			return;
		}
	}
	cout << "file not found" << endl;
}




void deleteFile(string& fileName, File* temp) {
	for (size_t i=0; i<temp->contents.size(); ++i) {
		if (fileName == temp->contents[i]->name) {
			temp->contents[i]->deleted = true;

			temp->contents.erase(temp->contents.begin() + i);
			return;
		}
	}
	cout << "file not found" << endl;
}




void executeRMDIR(string& fileName) {
	for (size_t i=0; i<cwd->contents.size(); ++i) {
		if (fileName == cwd->contents[i]->name) {
			if (!cwd->contents[i]->isDir) {
				cout << fileName << " is not a directory" << endl;
				return;
			}

			cwd->contents.erase(cwd->contents.begin() + i);
			return;
		}
	}
	cout << "file not found" << endl;
}


void executeCP(string& str1, string& str2) {
	// File* file = getFilePointerFromName(str1);
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


File* getFileFromNameInDir(string& fileName, File* ptr) {

	for (size_t i=0; i < ptr->contents.size(); ++i) {
		if (ptr->contents[i]->name == fileName) 
			return ptr->contents[i];
	}

	return nullptr;
}



bool isValidPath(string& str) {

	if (str.size() <= 3)
		return false;

	if (str.substr(str.size() - 3, 3) != "(W)" && str.substr(str.size() - 3, 3) != "(R)" && str.substr(str.size() - 3, 3) != "(B)") {
		cout << "paths must end in `(W)`, `(R)`, or `(B)`" << endl;
		return false;
	}

	str = str.substr(0, str.size() - 3);

	vector<string> path = splitStringByChar(str, '/');

	if (path[0] != "home.dir") {
		cout << "to confine directories, you must specify the absolute path, starting from home.dir" << endl;
		return false;
	}

	File* tempPtr = homePointer;

	for (size_t i=1; i<path.size(); ++i) {	
		tempPtr = getFileFromNameInDir(path[i], tempPtr);
		if (tempPtr == nullptr)
			return false;
	}

	return true;
	
}






bool parseConfine(string& str) {
	vector<string> directoryNames = splitStringByChar(str, ',');

	for (size_t i=0; i<directoryNames.size(); ++i) {
		if (!isValidPath(directoryNames[i])) {
			cout << "not a valid path" << endl;
			return false;
		}
	}

	return true;

}




//this function simulates launcher's messages as it executes a new process
void executeRun(string& str1, string& str2) {
	//str1 is the executable name, str2 is the confined directories

	//check if str1 is a valid executable name
	File* executable = getFileFromNameInCWD(str1);
	if (executable == nullptr) {
		cout << str1 << " is not in current working directory" << endl;
		return;
	}
	if (executable->isDir) {
		cout << "cannot execute a directory" << endl;
		return;
	}


	if (!parseConfine(str2))
		return;




	if (currentlyRunningExecutable == nullptr) {
		//then userModule is clear
		cout << "user module clear" << endl;
		cout << "sending " << executable->name << " from guard to UM..." << endl;
		sleep(5);
		cout << "done" << endl;
		cout << "running " << executable->name << "!" << endl;

		currentlyRunningExecutable = executable;
		return;
	}
	else {
		//then we need to clear user Module
		cout << "sending end signal to UM which is currently running " << currentlyRunningExecutable->name << "; process has five seconds to save its state" << endl;
		sleep(5);
		cout << "starting reload procedure" << endl;
		cout << "clearing UM's memory..." << endl;
		sleep(4);
		cout << "done" << endl;
		cout << "reinitializing UM's hardware..." << endl;
		sleep(5);
		cout << "done" << endl;
		cout << "sending " << executable->name << " from guard to UM..." << endl;
		sleep(5);
		cout << "done" << endl;
		cout << "running " << executable->name << "!" << endl;

		currentlyRunningExecutable = executable;

		return;
	}


}



void executeDisable(string& str) {
	if  (str != "monitor" && str != "camera" && str != "microphone" && str != "speaker" && str != "headphonejack" && str != "keyboard" && str != "trackpad" && str != "network" && str != "gps") {
		cout << str << " is not a known peripheral" << endl;
		return;
	}

	cout <<  str << " disabled" << endl;
}



void parseCommand(const string& command) {
	vector<string> splitCommand;
	split(command, splitCommand);

	if (splitCommand.size() == 0)
		return;

	if (splitCommand[0] == "cd") {
		if (splitCommand.size() != 2) {
			cout << "usage error for cd" << endl;
			return;
		}

		executeCD(splitCommand[1]);
		return;
	}
	if (splitCommand[0] == "pwd") {
		if (splitCommand.size() != 1) {
			cout << "usage error for pwd" << endl;
			return;
		}

		executePWD();
		return;
	}
	if (splitCommand[0] == "ls") {
		if (splitCommand.size() != 1) {
			cout << "usage error for ls" << endl;
			return;
		}

		executeLS();
		return;	
	}
	if (splitCommand[0] == "rm") {
		if (splitCommand.size() != 2) {
			cout << "usage rm" << endl;
			return;
		}
		executeRM(splitCommand[1]);
		return;		
	}
	if (splitCommand[0] == "rmdir") {
		if (splitCommand.size() != 2) {
			cout << "usage rmdir" << endl;
			return;
		}
		executeRMDIR(splitCommand[1]);
		return;		
	}
	if (splitCommand[0] == "cp") {
		if (splitCommand.size() != 3) {
			cout << "usage cp: `cp [file name] [directory name]`" << endl;
			return;
		}
		executeCP(splitCommand[1], splitCommand[2]);
		return;		
	}
	if (splitCommand[0] == "mkdir") {
		//cannot make a directory called `home`

		return;		
	}
	if (splitCommand[0] == "run") {
		if (splitCommand.size() != 3) {
			cout << "usage run: `run [executable name] confine([files])`" << endl;
			return;
		}
		executeRun(splitCommand[1], splitCommand[2]);
		return;
	}
	if (splitCommand[0] == "disable") {
		if (splitCommand.size() != 2) {
			cout << "usage disable: `disable [peripheral]`" << endl;
			return;
		}
		executeDisable(splitCommand[1]);
		return;
	}
	cout << "command not recognized" << endl;
}

///////////// SHELL STUFF IS ABOVE ////////////////////





int main() {

	//initialize file system
	ifstream file("fileSystem.txt"); // Open the file
    stringstream buffer;          // Stringstream to hold the file contents
    if (file.is_open()) {
    	buffer << file.rdbuf();        // Read the file into the stringstream
		file.close();                  // Close the file
	} else {
		cerr << "Could not open the file!" << endl;
		return 1;
	}

    // Convert the stringstream contents to a string
    string fileContents = buffer.str();


    //remove all whitespace
    fileContents = removeWhitespace(fileContents);

    // cout << fileContents << endl;

	File* root = createFiles(fileContents);
	root->parent = NULL;

	cwd = root;
	homePointer = root;



	string command;

	cout << "Welcome to the MoonCluster launcher. Written by Nicholas Belotserkovskiy." << endl << "\"One doesn't discover new lands without consenting to lose sight, for a very long time, of the shore.\" -Andre Gide" << endl;
	
	while (true) {
		cout << ">";

		//read the command into `command`
		getline(cin, command);

		parseCommand(command);

	}
}
