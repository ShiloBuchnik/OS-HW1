#include <unistd.h>
#include <cstring>
#include <iostream>
#include <map>
#include <sstream>
#include <sys/wait.h>
#include <iomanip>
#include "Commands.h"
#include <stdexcept>
#include <fcntl.h>
#include <fstream>
#include <vector>

using namespace std;

#define MAX_LINE_LEN 80
#define MAX_ARG_NUM 21

#define SYSCALL_FAILED (-1)

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

string _ltrim(const std::string &s) {
    size_t start = s.find_first_not_of(WHITESPACE);
    return (start == std::string::npos) ? "" : s.substr(start);
}

string _rtrim(const std::string &s) {
    size_t end = s.find_last_not_of(WHITESPACE);
    return (end == std::string::npos) ? "" : s.substr(0, end + 1);
}

// trims whitespace from left and right of command
string _trim(const std::string &s){
    return _rtrim(_ltrim(s));
}

// Note to self: this returns an array of allocated strings, each of them is null-terminated
int _parseCommandLine(char *cmd_line, char **args) {
    FUNC_ENTRY()
    int i = 0;
    std::istringstream iss(_trim(string(cmd_line)).c_str());
    for (std::string s; iss >> s;) {
        args[i] = (char *) malloc(s.length() + 1);
        memset(args[i], 0, s.length() + 1);
        strcpy(args[i], s.c_str());
        args[++i] = nullptr;
    }
    return i;

    FUNC_EXIT()
}

bool _isBackgroundComamnd(char *cmd_line) {
    const string str(cmd_line);
    return str[str.find_last_not_of(WHITESPACE)] == '&';
}

// Removes '&' from END of command
void _removeBackgroundSign(char *cmd_line) {
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




/// Helper functions:

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

// Args is an array of pointers. This function frees said pointers, and then frees the array itself
void freeArgs(char **args, size_t size) {
    for (size_t i = 0; i < size; i++) {
	  free(args[i]);
	}
    free(args);
}

// Checks if a command contains '*' or '?'
bool isComplexCommand(char **args, size_t size) {
    for (size_t i = 0; i < size; i++) {
        if (strchr(args[i], '*') || strchr(args[i], '?')){
		  return true;
		}
    }

    return false;
}

// To prevent boilerplate code
void PipeCommand::pipeErrorHandle(int pipe0, int pipe1){
    if (close(pipe0) == SYSCALL_FAILED) {
	  perror("smash error: close failed");
	}
    if (close(pipe1) == SYSCALL_FAILED) {
	  perror("smash error: close failed");
	}
}


SmallShell::SmallShell(): prompt("smash"), prev_dir(""), last_fg(false), fg_job_id(-1), current_job(nullptr), smash_jobs_list(),
                          current_pid(-1), current_command(""), is_pipe(false) {
// TODO: add your implementation
}

SmallShell::~SmallShell() {
// TODO: add your implementation
}

/**
* Creates and returns a pointer to Command class which matches the given command line (cmd_line)
* This is the factory method
*/
Command* SmallShell::CreateCommand(char *cmd_line) {
    string trim_cmd = _trim(string(cmd_line));

    /// Return if code doesn't work
    /*if (!checker(cmd_s) && _isBackgroundCommand(cmd_s.c_str())) {
        char cmd_line_copy[COMMAND_ARGS_MAX_LENGTH];
        strcpy(cmd_line_copy, cmd_s.c_str());
        _removeBackgroundSign(cmd_line_copy);
        cmd_s = cmd_line_copy;
    } */

    string first_word = trim_cmd.substr(0, trim_cmd.find_first_of("& \n")); // Finds first occurrence of '&', or ' ' or '\n'

    if (strstr(cmd_line, ">") || strstr(cmd_line, ">>")) {
	  return new RedirectionCommand(cmd_line);
	}
    if (strstr(cmd_line, "|") || strstr(cmd_line, "|&")) {
	  return new PipeCommand(cmd_line);
	}

    last_fg = false;
    if (first_word == "chprompt") {
	  return new ChangePromptCommand(cmd_line);
	}

    else if (first_word == "showpid") {
	  return new ShowPidCommand(cmd_line);
	}

    else if (first_word == "pwd") {
	  return new GetCurrDirCommand(cmd_line);
	}

    else if (first_word == "cd") {
	  return new ChangeDirCommand(cmd_line);
	}

    else if (first_word == "jobs") {
	  //return new JobsCommand(cmd_line, &(this->smash_jobs_list));				//CHANGED 18:05
	  return new JobsCommand(cmd_line);
	}

    else if (first_word == "fg"){
        last_fg = true;
        return new ForegroundCommand(cmd_line);
    }

    else if (first_word == "bg") {
	  return new BackgroundCommand(cmd_line);
	}

    else if (first_word == "quit") {
	  return new QuitCommand(cmd_line);
	}

    else if (first_word == "fare") {
	  return new FareCommand(cmd_line);
	}

    else if (first_word == "kill") {
	  return new KillCommand(cmd_line);
	}

    /// Get back to it if we have time
    //else if (first_word == "kill") return new KillCommand(cmd_line);

    else return new ExternalCommand(cmd_line);
}

void SmallShell::executeCommand(const char *cmd_line) {
    // for example:
    // Command* cmd = CreateCommand(cmd_line);
    // cmd->execute();
    // Please note that you must fork smash process for some commands (e.g., external commands....)

    if (strcmp(cmd_line, "") == 0) return;

    //SmallShell& instance = SmallShell::getInstance();
    //instance.smash_jobs_list.removeFinishedJobs();
	smash_jobs_list.removeFinishedJobs();

    Command* cmd = CreateCommand((char*)cmd_line);
    cmd->execute();
    delete cmd;

    this->current_command = "";
    this->current_pid = -1;
}


BuiltInCommand::BuiltInCommand(char *cmd_line): Command(cmd_line) {
    _removeBackgroundSign(cmd_line);
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

ChangePromptCommand::ChangePromptCommand(char *cmd_line): BuiltInCommand(cmd_line) {}

void ChangePromptCommand::execute() {
    char** args = (char**) malloc(MAX_ARG_NUM * sizeof(char*));
    size_t size = _parseCommandLine(this->cmd_line, args);
    SmallShell& instance = SmallShell::getInstance();

    if (size == 1) {
	  instance.prompt = "smash";
	}
    else {
	  instance.prompt = args[1];
	}

    freeArgs(args, size);
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

ShowPidCommand::ShowPidCommand(char *cmd_line): BuiltInCommand(cmd_line) {}

void ShowPidCommand::execute() {
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

GetCurrDirCommand::GetCurrDirCommand(char *cmd_line): BuiltInCommand(cmd_line) {}

void GetCurrDirCommand::execute() {
    size_t size = pathconf(".", _PC_PATH_MAX);
    char *buf = (char *) malloc((size_t) size);

    if (buf != nullptr) {
        getcwd(buf, size);
        cout << buf << endl;
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

ChangeDirCommand::ChangeDirCommand(char *cmd_line): BuiltInCommand(cmd_line) {}

void ChangeDirCommand::execute() {
    char **args = (char **) malloc(MAX_ARG_NUM * sizeof(char *));
    size_t size = _parseCommandLine(this->cmd_line, args);
    SmallShell &instance = SmallShell::getInstance();

    if (size == 1) { // No arguments
        cerr << "smash error:> " << cmd_line << endl;
        return;
    }
    else if (size > 2) {
        cerr << "smash error: cd: too many arguments" << endl;
        return;
    }

    if (!strcmp(args[1], "-")) { // User entered "-"
        if (!strcmp(instance.prev_dir.c_str(), "")) { // No previous working directory
            cerr << "smash error: cd: OLDPWD not set" << endl;
            return;
        }

        char cur_path[MAX_LINE_LEN];
        getcwd(cur_path, MAX_LINE_LEN);

        if (chdir(instance.prev_dir.c_str()))
            perror("smash error: chdir failed"); // Previous directory is invalid - could it be?

        instance.prev_dir = cur_path;
    }
    else { // User entered a path
        char cur_path[MAX_LINE_LEN];
        getcwd(cur_path, MAX_LINE_LEN);

        if (chdir(args[1])) {
		  perror("smash error: chdir failed"); // Path is invalid
		}
        else {
		  instance.prev_dir = cur_path;
		}
    }

    freeArgs(args, size);
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

/*				////CHANGED 18:05, MIGHT DELETE
JobsCommand::JobsCommand(char* cmd_line, JobsList* jobs): BuiltInCommand(cmd_line), jobs(jobs) {}

void JobsCommand::execute(){
    jobs->printJobsList();
}
 */

JobsCommand::JobsCommand(char* cmd_line): BuiltInCommand(cmd_line) {}

void JobsCommand::execute(){
  SmallShell &instance = SmallShell::getInstance();
  //before printing jobs list, finished jobs are deleted from list
  instance.smash_jobs_list.removeFinishedJobs();

  for (auto &j: instance.smash_jobs_list.jobs_map) {
	if (j.second.stopped) {
	  //[<job-id>] <command> : <process id> <seconds elapsed> (stopped)
	  cout << "[" << j.first << "] " << j.second.command << " : " << j.second.pid << " " <<
		   difftime(time(nullptr), j.second.time) << "secs (stopped)" << endl;
	}
	else{
	  //[<job-id>] <command> : <process id> <seconds elapsed>
	  cout << "[" << j.first << "] " << j.second.command << " : " << j.second.pid << " " <<
		   difftime(time(nullptr), j.second.time) << "secs" << endl;
	}
  }

}


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

ForegroundCommand::ForegroundCommand(char *cmd_line): BuiltInCommand(cmd_line) {}

void ForegroundCommand::execute() {
    SmallShell &instance = SmallShell::getInstance();
    //JobsList jobs = instance.smash_jobs_list;

    char** args = (char **) malloc(MAX_ARG_NUM * sizeof(char *));
    size_t size = _parseCommandLine(this->cmd_line, args);

    int job_id = 0;
    if (size > 2) {
        cerr << "smash error: fg: invalid arguments" << endl;
        freeArgs(args, size);
        return;
    }
    //job_id is specified
    else if (size == 2) {
        try {
            job_id = atoi(args[1]);
        } catch (invalid_argument &e) {
            cerr << "smash error: fg: invalid arguments" << endl;
            freeArgs(args, size);
            return;
        } catch (out_of_range &e) {
            cerr << "smash error: fg: invalid arguments" << endl;
            freeArgs(args, size);
            return;
        }
    }
    instance.smash_jobs_list.removeFinishedJobs();
    int last_job_id = 0;

    JobEntry *job = instance.smash_jobs_list.getLastJob(&last_job_id);
    //if no job_id specified, the job to be moved to the fg is the last job
    //if job_id is specified, we will change job to the job by id

    //no job_id && jobs list is empty
    if (!job && size == 1){
        cerr << "smash error: fg: jobs list is empty" << endl;
        freeArgs(args, size);
        return;
    }

    //job_id && jobs list is empty
    if (!job && size == 2){
        cerr << "smash error: fg: job-id " << args[1] << " does not exist" << endl;
        freeArgs(args, size);
        return;
    }

    if (job_id > 0){ // Assert size == 2
        job = instance.smash_jobs_list.getJobById(job_id);

        //job_id && the job with that id doesn't  exist
        if (!job){
            cerr << "smash error: fg: job-id " << job_id << " does not exist" << endl;
            freeArgs(args, size);
            return;
        }
    } else {
	  job_id = last_job_id;
	}

    pid_t pid = job->pid;
    cout << job->command << " : " << pid << endl;

    if (job->stopped) {
        job->stopped = false;
        kill(pid, SIGCONT);
    }

    instance.current_pid = pid;
    instance.current_command = job->command;
    instance.smash_jobs_list.removeJobById(job_id);
	instance.fg_job_id = job_id;

    instance.current_job = job;
    if (waitpid(pid, nullptr, WUNTRACED) == SYSCALL_FAILED){
        perror("smash error: waitpid failed");
        freeArgs(args, size);
        return;
    }
    instance.current_job = nullptr;

    freeArgs(args, size);
}

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

BackgroundCommand::BackgroundCommand(char *cmd_line): BuiltInCommand(cmd_line) {}

void BackgroundCommand::execute() {
    char **args = (char **) malloc(MAX_ARG_NUM * sizeof(char *));
    size_t size = _parseCommandLine(this->cmd_line, args);

    SmallShell &instance = SmallShell::getInstance();
    //JobsList jobs = instance.smash_jobs_list;

    if (size > 2) {
        cerr << "smash error: bg: invalid arguments" << endl;
        freeArgs(args, size);
        return;
    }

    int job_id = 0;
    JobEntry *job = nullptr;
    //if job id is specified
    if (size == 2) {
        try {
            job_id = atoi(args[1]);
        } catch (invalid_argument &e) {
            cerr << "smash error: bg: invalid arguments" << endl;
            freeArgs(args, size);
            return;
        } catch (out_of_range &e) {
            cerr << "smash error: bg: invalid arguments" << endl;
            freeArgs(args, size);
            return;
        }
        job = instance.smash_jobs_list.getJobById(job_id);
        if (!job) {
            cerr << "smash error: bg: job-id " << job_id << " does not exist" << endl;
            freeArgs(args, size);
            return;
        }
        if (!job->stopped) {
            cerr << "smash error: bg: job-id " << job_id << " is already running in the background" << endl;
            freeArgs(args, size);
            return;
        }
    } else {
        //no job id specified - last stopped job in jobs list is to continue running in bg
        job = instance.smash_jobs_list.getLastStoppedJob(&job_id);
        if (!job) {
            cerr << "smash error: bg: there is no stopped jobs to resume" << endl;
            freeArgs(args, size);
            return;
        }
    }

    pid_t pid = job->pid;
    cout << job->command << " : " << pid << endl;
    job->stopped = false;
    kill(pid, SIGCONT);

    freeArgs(args, size);
}

/*
 * quit command
 * QuitCommand
 * 
 * formant:
 *    quit [kill]
 * description:
 *    exists the smash
 *    if kill arg is specified:
 *        print the num of processes/jobs that are to be killed , pids, command-lines
 *        smash kills all unfinished and stopped jobs before exiting
 * error handling:
 *    params are ignored
 */

QuitCommand::QuitCommand(char *cmd_line): BuiltInCommand(cmd_line) {}

void QuitCommand::execute() {
    char **args = (char **) malloc(MAX_ARG_NUM * sizeof(char *));
    size_t size = _parseCommandLine(this->cmd_line, args);

    SmallShell &instance = SmallShell::getInstance();
    //JobsList jobs = instance.smash_jobs_list;

    if (size == 2 && !strcmp(args[1], "kill")) {
        instance.smash_jobs_list.removeFinishedJobs();
        cout << "smash: sending SIGKILL signal to " << instance.smash_jobs_list.jobs_map.size() << " jobs:" << endl;
        instance.smash_jobs_list.killAllJobs();

        exit(0);
    }
    else exit(0);

    freeArgs(args, size);
}


/// External commands ///

ExternalCommand::ExternalCommand(char *cmd_line): Command(cmd_line) {}

void ExternalCommand::execute(){ // Remember to update current_pid and current_cmd somewhere idgaf
    char** args = (char**) malloc(MAX_ARG_NUM * sizeof(char *));

    char cmd_line_copy[MAX_LINE_LEN];
    strcpy(cmd_line_copy, this->cmd_line);
    bool background = _isBackgroundComamnd(cmd_line_copy);
    _removeBackgroundSign(cmd_line_copy);

    size_t size = _parseCommandLine(cmd_line_copy, args);

    /*for (size_t i = 0; i < size; i++){
        cout << args[i] << endl;
    }
     ^Z
jobs
bg
jobs
     */

    pid_t pid = fork();
    if (pid == 0) { // Son, this is the actual external command
        if (setpgrp() == SYSCALL_FAILED) { // Unrelated to the logic, ignore it
            perror("smash error: setpgrp failed");
            exit(-1);
        }

        if (isComplexCommand(args, size)) { // Complex command
            char* path = (char*) malloc(MAX_LINE_LEN * sizeof(char));
            strcpy(path, "/bin/bash\0");

            char* new_args[MAX_ARG_NUM + 3];
            new_args[0] = (char*)"bash\0";
            new_args[1] = (char*)"-c\0";
            size_t i = 2;
            for (; i < size + 2; i++) new_args[i] = args[i - 2];
            new_args[i] = nullptr;

            if (execv(path, new_args)) { // Passing non-const to a const argument is an implicit conversion, all good
                perror("smash error: execv failed");
                free(path);
                exit(-1);
            }
            free(path);
        }
        else { // Simple command
		  //cerr << "gonna run execvp yay" << endl;
		  //cerr << "args[0] " << args[0] << " args " << *args << endl;
            if (execvp(args[0], args)) {
                perror("smash error: execvp failed");
                exit(-1);
            }
        }
    }
    else { // Father, this is the smash shell
        SmallShell& instance = SmallShell::getInstance();

        if (background) {
		  instance.smash_jobs_list.addJob(this, pid); // If background
		}
        else{ // If foreground
            instance.current_pid = pid;
            instance.current_command = cmd_line;

            if (waitpid(pid, nullptr, WUNTRACED) == SYSCALL_FAILED) {
                perror("smash error: waitpid failed");
                return;
            }
        }
    }

    freeArgs(args, size);
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

KillCommand::KillCommand(char *cmd_line) : BuiltInCommand(cmd_line) {}

void KillCommand::execute() {
    char** args = (char**) malloc(MAX_ARG_NUM * sizeof(char*));
    size_t size = _parseCommandLine(this->cmd_line, args);

    if (size != 3){
        cerr << "smash error: kill: invalid arguments" << endl;
        freeArgs(args, size);
        return;
    }
    //kill -sig jobid

    int signum, job_id;
    try{ // Validating job_id
        job_id = atoi(args[2]);
    } catch (invalid_argument &e){
        cerr << "smash error: kill: invalid arguments" << endl;
        freeArgs(args, size);
        return;
    } catch (out_of_range &e){
        cerr << "smash error: kill: invalid arguments" << endl;
        freeArgs(args, size);
        return;
    }

    char f = string(args[1]).at(0);
    if (f != '-'){ // Checking for hyphen
        cerr << "smash error: kill: invalid arguments" << endl;
        freeArgs(args, size);
        return;
    }

    try{ // Validating signum
        signum = stoi(string(args[1]).erase(0, 1));
    } catch (invalid_argument &e){
        cerr << "smash error: kill: invalid arguments" << endl;
        freeArgs(args, size);
        return;
    } catch (out_of_range &e){
        cerr << "smash error: kill: invalid arguments" << endl;
        freeArgs(args, size);
        return;
    }

    //both signum and job_id are numbers
    SmallShell& instance = SmallShell::getInstance();
    //JobsList jobs = instance.smash_jobs_list;

    JobEntry *job = instance.smash_jobs_list.getJobById(job_id);
    if (job){
        pid_t pid = job->pid;

        if (kill(pid, signum) == SYSCALL_FAILED){
            perror("smash error: kill failed");
            freeArgs(args, size);
            return;
        }

        cout << "signal number " << signum << " was sent to pid " << pid << endl;

        if (signum == SIGTSTP)
		{
		  job->stopped = true;
		}
        else if (signum == SIGCONT)
		{
		  job->stopped = false;
		}
    } else {
        cerr << "smash error: kill: job-id " << job_id << " does not exist" << endl;
        freeArgs(args, size);
        return;
    }
    freeArgs(args, size);
}


/*
 *  JOBS LIST FUNCTIONS
 */

// JobEntry c'tor
JobEntry::JobEntry(pid_t pid, time_t time, string command, bool stopped): pid(pid), time(time), command(command), stopped(stopped) {}

// JobsList c'tor
JobsList::JobsList(): jobs_map() {}

/*
 * add job to the jobs list; differentiate between jobs in background and jobs that have been stopped
 */
void JobsList::addJob(Command *cmd, pid_t pid, bool last_fg, bool isStopped) {
    //before adding new jobs to the list, finished jobs are deleted from list
    removeFinishedJobs(); // Get back to here when implementing external background commands
    string command(cmd->cmd_line);
    SmallShell &instance = SmallShell::getInstance();
	instance.smash_jobs_list.removeFinishedJobs();

    if (last_fg) {
	  jobs_map.emplace<int, JobEntry>((int) instance.fg_job_id, {pid, time(nullptr), command, isStopped});
	}
    else{ // Note for self (shilo): no need to worry about cases such as running an fg proc and adding a different job, since it's not possible. Also, smile more
        int job_id;
        if (jobs_map.empty()) {
		  job_id = 0;
		}
        else{
            map<int, JobEntry>::iterator it = jobs_map.end();
            job_id = (--it)->first;
        }

        jobs_map.emplace <int, JobEntry> (job_id + 1, {pid, time(nullptr), command, isStopped});
    }
}

/*
 * print jobs list in format (page 6)
 * used in jobs command
 */

/*					////CHANGED 18:05, MIGHT DELETE
void JobsList::printJobsList() {
    SmallShell &instance = SmallShell::getInstance();
    //before printing jobs list, finished jobs are deleted from list
    instance.smash_jobs_list.removeFinishedJobs();

    for (auto &j: jobs_map) {
        if (j.second.stopped) {
            //[<job-id>] <command> : <process id> <seconds elapsed> (stopped)
            cout << "[" << j.first << "] " << j.second.command << " : " << j.second.pid << " " <<
                 difftime(time(nullptr), j.second.time) << "secs (stopped)" << endl;
        }
        else{
            //[<job-id>] <command> : <process id> <seconds elapsed>
            cout << "[" << j.first << "] " << j.second.command << " : " << j.second.pid << " " <<
                 difftime(time(nullptr), j.second.time) << "secs" << endl;
        }
    }
}
 */

/*
 * kills all jobs. used in quit command if kill is specified and prints jobs killed in format (page 9)
 */
void JobsList::killAllJobs() {
    for (auto &j: jobs_map) {
        cout << j.second.pid << ": " << j.second.command << endl;
        kill(j.second.pid, SIGKILL);
    }
}

/*
 * removes finished jobs from list
 */
void JobsList::removeFinishedJobs(){
    if (jobs_map.empty()) {
	  return;
	}

    SmallShell& instance = SmallShell::getInstance();

    for (auto it = jobs_map.begin(); it != jobs_map.end();){
        if(!instance.is_pipe){
            JobEntry job = (*it).second;
            int ret_wait = waitpid(job.pid, nullptr, WNOHANG);

            if (ret_wait == job.pid || ret_wait == -1) {
			  it = jobs_map.erase(it); // 'erase' returns an iterator to the element after erased element
			}
            else it++;
        }
    }
}

/*
 * return job with job id
 */
JobEntry* JobsList::getJobById(int jobId) {
    auto found = jobs_map.find(jobId);

    if (found == jobs_map.end()) {
	  return nullptr;
	}
    else return &(found->second);
}

// Assert given key always exists in the map
void JobsList::removeJobById(int jobId) {
    jobs_map.erase(jobId);
}

JobEntry* JobsList::getLastJob(int *lastJobId) {
    if (jobs_map.empty()) {
	  return nullptr;
	}

    auto end = jobs_map.end();
    *lastJobId = (--end)->first;

    return getJobById(*lastJobId);
}

JobEntry *JobsList::getLastStoppedJob(int *jobId) {
    int last = -1;
    for (auto &j: jobs_map) {
        if (j.second.stopped) {
		  last = j.first;
		}
    }

    if (last == -1){
        *jobId = -1;
        return nullptr;
    }
    else{
        *jobId = last;
        return getJobById(last);
    }
}


/*
 * REDIRECTION COMMANDS
 */

RedirectionCommand::RedirectionCommand(char *cmd_line): Command(cmd_line){
    char** args = (char**) malloc(MAX_ARG_NUM * sizeof(char*));
    size_t size = _parseCommandLine(this->cmd_line, args);
    string str = string(cmd_line);
    string delim = str.find(">") != string::npos ? ">" : ">>";

    string trim_cmd = _trim(str.substr(0, str.find(delim))); // command to the left
    this->cmd = (char*) malloc((strlen(trim_cmd.c_str()) + 1) * sizeof(char));
    strcpy(cmd, trim_cmd.c_str()); // // command to the left
    string trim_filename = _trim(str.substr(str.find(delim) + delim.length(), str.length())); // filename to the right

    if (_isBackgroundComamnd((char*)trim_filename.c_str())) { // Background commands ('&') are not allowed with redirection, so we remove and trim
        char temp[MAX_LINE_LEN]; // Temporary trimmed copy of filename. We remove '&' from it, and reassign
        strcpy(temp, trim_filename.c_str());
        _removeBackgroundSign(temp);
        trim_filename = _trim(string(temp));
    }

    filename = (char*) malloc((strlen(trim_filename.c_str()) + 1) * sizeof(char));
    strcpy(filename, trim_filename.c_str()); // filename to the right

    this->is_append = (strcmp(delim.c_str(), ">>") == 0) ? true : false;
    this->is_success = false;
    this->fd_copy_of_stdout = 0;
    this->fd = 0;

    prepare();
    freeArgs(args, size);
}

void RedirectionCommand::prepare(){
    this->fd_copy_of_stdout = dup(1); // Duplicating stdout
    if (close(1) == SYSCALL_FAILED) { // Closing "old" stdout
        perror("smash error: close failed");
        return;
    }

    this->fd = (is_append) ? open(filename, O_WRONLY | O_APPEND | O_CREAT, 0655) : open(filename, O_WRONLY | O_TRUNC | O_CREAT , 0655);

    if (this->fd == SYSCALL_FAILED) {
        perror("smash error: open failed");
        this->is_success = false;
    }
    else this->is_success = true;
}

void RedirectionCommand::execute(){
    if (this->is_success){
        SmallShell& instance = SmallShell::getInstance();
        instance.executeCommand(this->cmd);
    }

    cleanup();
}

void RedirectionCommand::cleanup(){
    free(this->filename);
    free(this->cmd);

    // We check for success first, because if we failed to open in the first place - of course we won't close anything
    if (this->is_success && close(fd) == SYSCALL_FAILED) {
	  perror("smash error: close failed");
	}

    if (dup2(this->fd_copy_of_stdout, 1) == SYSCALL_FAILED) {
	  perror("smash error: dup2 failed"); // Moving copy of stdout back to stdout's index (1)
	}
    if (close(this->fd_copy_of_stdout) == SYSCALL_FAILED) {
	  perror("smash error: close failed"); // Closing copy of stdout
	}
}


/*
 * PIPE COMMANDS
 */

PipeCommand::PipeCommand(char *cmd_line): Command(cmd_line){
    string cmd = string(cmd_line);

    //determine if the pipe is | or |&
    if (cmd.find("|&") == string::npos) {
	  bar = "|";
	}
    else {
	  bar = "|&";
	}

    //cmd1 is from the start of the string to the position of | or |&
    com1 = cmd.substr(0, cmd.find(bar));

    //cmd2 is from after | or |& to end of string
    if (bar == "|") {
	  com2 = cmd.substr(cmd.find(bar) + 1, cmd.length());
	}
    else {
	  com2 = cmd.substr(cmd.find(bar) + 2, cmd.length());
	}
}

void PipeCommand::execute(){
    //pipefd array is used to return two file descriptors referring to the ends of the pipe
    //pipefd[0] is read end
    //pipefd[1] is write end
    int pipefd[2];
    pipe(pipefd);

    SmallShell &instance = SmallShell::getInstance();

    pid_t pid1 = fork();

    //if fork is successful, the pid of the child is returned in the parent and 0 in the child; on failure -1 is returned
    if (pid1 == SYSCALL_FAILED) {
        perror("smash error: fork failed");
        pipeErrorHandle(pipefd[0], pipefd[1]);
        return;
    }

    if (pid1 == 0) { //son no.1
        //not sure if needed - in the man there is no return value for setpgrp
        if (setpgrp() == SYSCALL_FAILED) {
            perror("smash error: setpgrp failed");
            pipeErrorHandle(pipefd[0], pipefd[1]);
            return;
        }
        if (bar == "|") {
            /*
             * using the pipe character “|” will produce a pipe which
             * redirects command1 stdout to its write channel and command2 stdin to its read channel.
             *
             * int dup2(int oldfd, int newfd);
             * the file descriptor newfd is adjusted so that it now
             * refers to the same open file description as oldfd.
             *
             * 1 is fd for stdout
             */
            if (dup2(pipefd[1], 1) == SYSCALL_FAILED) {
                perror("smash error: dup2 failed");
                pipeErrorHandle(pipefd[0], pipefd[1]);
                return;
            }
            else { //bar == "|&"
                /*
                 * using the pipe character “|&” will produce a pipe
                 * which redirects command1 stderr to the pipe’s write channel and command2 stdin
                 * to the pipe’s read channel.
                 *
                 * 2 is fd for stderr
                 */

                if (dup2(pipefd[1], 2) == SYSCALL_FAILED) {
                    perror("smash error: dup2 failed");
                    pipeErrorHandle(pipefd[0], pipefd[1]);
                    return;
                }
            }

            pipeErrorHandle(pipefd[0], pipefd[1]);
            instance.is_pipe = true;
            instance.executeCommand(com1.c_str());
            exit(0);
        }
    }

    pid_t pid2 = fork();
    if (pid2 == SYSCALL_FAILED) {
        perror("smash error: fork failed");
        pipeErrorHandle(pipefd[0], pipefd[1]);
        return;
    }

    if (pid2 == 0) { //son no.2
        if (setpgrp() == SYSCALL_FAILED) {
            perror("smash error: setpgrp failed");
            pipeErrorHandle(pipefd[0], pipefd[1]);
            return;
        }
        /*
         * both | and |& redirect command2 stdin to the pipe's read channel
         *
         * 0 is fd for stdin
         */
        if (dup2(pipefd[0], 0) == SYSCALL_FAILED) {
            perror("smash error: dup2 failed");
            pipeErrorHandle(pipefd[0], pipefd[1]);
            return;
        }
        pipeErrorHandle(pipefd[0], pipefd[1]);
        instance.is_pipe = true;
        instance.executeCommand(com2.c_str());
        exit(0);
    }

    pipeErrorHandle(pipefd[0], pipefd[1]);

    // Waiting for son no.1
    if (waitpid(pid1, nullptr, WUNTRACED) == SYSCALL_FAILED) {
        perror("smash error: waitpid failed");
        return;
    }
    // Waiting for son no.2
    if (waitpid(pid2, nullptr, WUNTRACED) == SYSCALL_FAILED) {
        perror("smash error: waitpid failed");
        return;
    }
}

FareCommand::FareCommand(char *cmd_line) : BuiltInCommand(cmd_line) {
    char** args = (char**) malloc(MAX_ARG_NUM * sizeof(char*));
    size_t size = _parseCommandLine(this->cmd_line, args);

    if (size != 4){
        cerr << "smash error: fare: invalid arguments" << endl;
        freeArgs(args, size);
        failed = true;
        return;
    }

    file_name = args[1];
    source = args[2];
    destination = args[3];

    freeArgs(args, size);
}

//"If the “fare” command files mid-way, the file should remain as it was before invoking
//“fare”." TODO!!
void FareCommand::execute() {
    if (failed) return;

    fstream file;
    file.open(file_name, ios::in);

    if (!file.is_open()){
        perror("smash error: open failed");
        return;
    }

    string l;
    vector<string> lines;

    while (getline(file, l))
	{
	  lines.push_back(l);
	}

    file.close();

    //up to here the file wasn't touched
    file.open(file_name, ios::out);
    int c = 0;
    for (string line : lines){
        if (line.find(source) != string::npos){
            size_t i = 0;
            while (true){
                i = line.find(source, i);
                if (i == string::npos) {
				  break;
				}
                line.erase(i, source.size());
                line.insert(i, destination);
                i += destination.size();
                c++;
            }
        }
        file << line << endl;
    }

    file.close();

	if (c == 1){
	  cout << "replaced " << c << " instance of the string \"" << source << "\"" << endl;
	}else {
	  cout << "replaced " << c << " instances of the string \"" << source << "\"" << endl;
	}
}
