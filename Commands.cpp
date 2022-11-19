#include <unistd.h>
#include <string.h>
#include <iostream>
#include <vector>
#include <sstream>
#include <sys/wait.h>
#include <iomanip>
#include "Commands.h"

using namespace std;

string prompt = "smash"; // For chprompt
string prev_dir = ""; // For cd

const std::string WHITESPACE = " \n\r\t\f\v";

#if 0
#define FUNC_ENTRY()  \
  cout << __PRETTY_FUNCTION__ << " --> " << endl;

#define FUNC_EXIT()  \
  cout << __PRETTY_FUNCTION__ << " <-- " << endl;
#else
#define FUNC_ENTRY()
#define FUNC_EXIT()
#endif

string _ltrim(const std::string& s)
{
  size_t start = s.find_first_not_of(WHITESPACE);
  return (start == std::string::npos) ? "" : s.substr(start);
}

string _rtrim(const std::string& s)
{
  size_t end = s.find_last_not_of(WHITESPACE);
  return (end == std::string::npos) ? "" : s.substr(0, end + 1);
}

string _trim(const std::string& s)
{
  return _rtrim(_ltrim(s));
}

int _parseCommandLine(const char* cmd_line, char** args) {
  FUNC_ENTRY()
  int i = 0;
  std::istringstream iss(_trim(string(cmd_line)).c_str());
  for(std::string s; iss >> s; ) {
    args[i] = (char*)malloc(s.length()+1);
    memset(args[i], 0, s.length()+1);
    strcpy(args[i], s.c_str());
    args[++i] = NULL;
  }
  return i;

  FUNC_EXIT()
}

bool _isBackgroundComamnd(const char* cmd_line) {
  const string str(cmd_line);
  return str[str.find_last_not_of(WHITESPACE)] == '&';
}

void _removeBackgroundSign(char* cmd_line) {
  const string str(cmd_line);
  // find last character other than spaces
  unsigned int idx = str.find_last_not_of(WHITESPACE);
  // if all characters are spaces then return
  if (idx == string::npos) {
    return;
  }
  // if the command line does not end with & then return
  if (cmd_line[idx] != '&') {
    return;
  }
  // replace the & (background sign) with space and then remove all tailing spaces.
  cmd_line[idx] = ' ';
  // truncate the command line string up to the last non-space character
  cmd_line[str.find_last_not_of(WHITESPACE, idx) + 1] = 0;
}




// Helper functions:

/* // This function returns a vector containing the words in 'cmd_line'
vector<char*> getArgs(const char* cmd_line){
    vector<char*> args = {};
    char* token = strtok(cmd_line, " "); // This function runs on the string until it sees " ", and returns what it scanned so far

    while (token){
        args.push_back(toekn);
        token = strtok(NULL, " "); // It preserves internal state, so now it runs from where it stopped until it sees " " again
    }

    return args;
} */


// TODO: Add your implementation for classes in Commands.h 

SmallShell::SmallShell() {
// TODO: add your implementation
}

SmallShell::~SmallShell() {
// TODO: add your implementation
}

/**
* Creates and returns a pointer to Command class which matches the given command line (cmd_line)
 *  This is the factory method
*/
Command * SmallShell::CreateCommand(const char* cmd_line) {
	// For example:
/*
  string cmd_s = _trim(string(cmd_line));
  string firstWord = cmd_s.substr(0, cmd_s.find_first_of(" \n"));

  if (firstWord.compare("pwd") == 0) {
    return new GetCurrDirCommand(cmd_line);
  }
  else if (firstWord.compare("showpid") == 0) {
    return new ShowPidCommand(cmd_line);
  }
  else if ...
  .....
  else {
    return new ExternalCommand(cmd_line);
  }
  */
  return nullptr;
}

void SmallShell::executeCommand(const char *cmd_line) {
  // TODO: Add your implementation here
  // for example:
  // Command* cmd = CreateCommand(cmd_line);
  // cmd->execute();
  // Please note that you must fork smash process for some commands (e.g., external commands....)
}

/*
 * BUILT-IN COMMANDS
*/

/*
 * chprompt command
 * ChangePromptCommand
 * 
 * format:
 *    chprompt <new-prompt>
 * description:
 *    allow the user to change the prompt displayed by the smash while waiting for next command
 *    no param - prompt is reset to smash
 *    more than one param - rest is ignored
 */

ChangePromptCommand::ChangePromptCommand(const char* cmd_line):BuiltIncommand(cmd_line) {
    _removeBackgroundSign(cmd_line);
}

void ChangePromptCommand::execute(){
    char** args;
    size_t size = _parseCommandLine(this->cmd_line, args);

    if (!size) prompt = "smash";
    else prompt = args[0];
}

/*
 * showpid command
 * ShowPidCommand
 * 
 * format:
 *    showpid
 * description:
 *    prints the smash pid
 * error handling:
 *    params are ignored
 */

ShowPidCommand::ShowPidCommand(const char *cmd_line):BuiltIncommand(cmd_line) {}

void ShowPidCommand::execute(){
    pid_t pid = getpid();
    cout << "smash pid is " << pid << endl;
}

/*
 * pwd command
 * GetCurrDirCommand
 * 
 * format:
 *    pwd
 * description:
 *    prints full path of curr working directory
 *    can use getcwd syscall
 * error handling:
 *    params are ignored
 */

GetCurrDirCommand::GetCurrDirCommand(const char* cmd_line):BuiltInCommand(cmd_line) {}

void GetCurrDirCommand::execute()
{
  size_t size = pathconf(".", _PC_PATH_MAX);
  char* buf = (char*) malloc((size_t)size);

  if (buf != nullptr)
  {
    getcwd(buf, size);
    cout << buf;
    free(buf);
  }
}

/*
 * cd command
 * ChangeDirCommand
 * 
 * formant:
 *    cd <new-path>
 * description:
 *    changes directory to the new path, relative or full
 *    if argumant is "-" it will change the current working directory to the last working directory
 *    can use chdir syscall
 * error handling:
 *    more than 1 args - smash error: cd: too many arguments
 *    last working dir is empty and "cd -" was called - smash error: cd: OLDPWD not set
 *    chdir() syscall fails - perror used to print proper error message
 */

ChangeDirCommand::ChangeDirCommand(const char* cmd_line):BuiltInCommand(cmd_line){
    _removeBackgroundSign(cmd_line);
}

void ChangeDirCommand::execute(){
    char** args;
    size_t size = _parseCommandLine(this->cmd_line, args);

    if (size != 1) {
        cerr << "smash error: cd: too many arguments" << endl;
        return;
    }

    if (!strcmp(args[0], "-")){ // User entered "-"
        if (!strcmp(prev_dir, "")){ // No previous working directory
            cerr << "smash error: cd: OLDPWD not set" << endl;
            return;
        }

        if (chdir(prev_dir) perror("smash error: chdir failed"); // Previous directory is invalid - could it be?
    }
    else{ // User entered a single argument
        if (chdir(args[0])) perror("smash error: chdir failed"); // Path is invalid
        else prev_dir = args[0];
    }
}

/*
 * jobs command
 * JobsCommand
 * 
 * formant:
 *    jobs
 * description:
 *    prints the jobs list which contains:
 *      1. unfinished jobs running in background 
 *      2. stopped jobs that were stopped using Ctrl+Z
 *    sorted bt job-id
 * printing format:
 *    stopped jobs:
 *      [<job-id>] <command> : <process id> <seconds elapsed> (stopped)
 *    else:
 *      [<job-id>] <command> : <process id> <seconds elapsed>
 * error handling:
 *    params are ignored
 */

/*
 * fg command
 * ForegroundCommand
 * 
 * formant:
 *    fg [job-id]
 * description:
 *    brings a stopped process or a process that runs in the background to the foreground
 *    prints the command line of that job along with its pid
 *    if no arg is specified the job with the maximal job-id in jobs list is selected
 * side effects:
 *    job is removed from jobs list
 * error handling:
 *    job-id doesn't exist - smash error: fg: job-id <job-id> does not exist
 *    no args and jobs list is empty - smash error: fg: jobs list is empty
 *    invalid syntax - smash error: fg: invalid arguments
 */

/*
 * bg command
 * BackgroundCommand
 * 
 * formant:
 *    bg [job-id]
 * description:
 *    resumes one of the stopped processes in the background
 *    prints the command line of that job
 *    if no arg is specified the last stopped job is selected
 * side effects:
 *    job is removed from stopped jobs in list
 * error handling:
 *    job-id doesn't exist - smash error: bg: job-id <job-id> does not exist
 *    job is already running in background (not stopped) - smash error: bg: job-id <job-id> is already running in the background
 *    no args and no stopped jobs - smash error: bg: there is no stopped jobs to resume
 *    invalid syntax - smash error: bg: invalid arguments
 */

/*
 * quit command
 * QuitCommand
 * 
 * formant:
 *    quit [kill]
 * description:
 *    exists the smash
 *    if kill arg is specified:
 *        prit the num of processes/jobs that are to be killed , pids, command-lines
 *        smash kills all unfinished and stopped jobs before exiting
 * error handling:
 *    params are ignored
 */

QuitCommand::QuitCommand(const char* cmd_line, JobsList* jobs):BuiltInCommand(cmd_line), jobs(jobs){
    _removeBackgroundSign(cmd_line);
}

void QuitCommand::execute(){
    char** args;
    size_t size = _parseCommandLine(this->cmd_line, args);

    if (size == 1 && strcmp(args[0], "kill")){
        // TBC
    }
    else{
        exit(0);
    }

}


/* class ExternalCommand : public Command {
public:
    ExternalCommand(const char* cmd_line);
    virtual ~ExternalCommand() {}
    void execute() override;
};*/


ExternalCommand::ExternalCommand(const char* cmd_line):Command(cmd_line) {}

void ExternalCommand::execute(){

}


// BONUS
/*
 * kill command
 * KillCommand
 * 
 * formant:
 *    kill -<signum> <jobid>
 * description:
 *    sends signum to job-id and prints a message reporting that
 * error handling:
 *    job-id doesn't exist - smash error: kill: job-id <job-id> does not exist
 *    invalid syntax - smash error: kill: invalid arguments
 *    kill() syscall fails - perror used to print proper error message
 */