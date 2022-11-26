#include <unistd.h>
#include <string.h>
#include <iostream>
#include <map>
#include <sstream>
#include <sys/wait.h>
#include <iomanip>
#include "Commands.h"
#include <stdexcept>

using namespace std;

#define MAX_LINE_LEN 80
#define MAX_ARG_NUM 21

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
        args[++i] = NULL;
    }
    return i;

    FUNC_EXIT()
}

bool _isBackgroundComamnd(char *cmd_line) {
    const string str(cmd_line);
    return str[str.find_last_not_of(WHITESPACE)] == '&';
}

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

// Args is an array of pointers. This function frees said pointers, and then frees the array itself
void freeArgs(char **args) {
    for (int i = 0; i < MAX_ARG_NUM; i++) free(args[i]);
    free(args);
}

// Checks if a command contains '*' or '?'
bool isComplexCommand(char **args) {
    for (int i = 0; i < MAX_ARG_NUM; i++) {
        if (strchr(args[i], '*') || strchr(args[i], '?')) return true;
    }

    return false;
}


SmallShell::SmallShell(): prompt("smash"), prev_dir(""), last_fg(false), fg_job_id(-1), current_job(nullptr), smash_jobs_list() {
// TODO: add your implementation
}

SmallShell::~SmallShell() {
// TODO: add your implementation
}

/**
* Creates and returns a pointer to Command class which matches the given command line (cmd_line)
 *  This is the factory method
*/
/* Things to do:
1) set 'last_fg=false' before everything, and then if 'fg' is chosen, set 'last_fg=true' */
Command *SmallShell::CreateCommand(char *cmd_line) {
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
    char **args = (char **) malloc(MAX_ARG_NUM * sizeof(char *));
    size_t size = _parseCommandLine(this->cmd_line, args);
    SmallShell &instance = SmallShell::getInstance();

    if (size == 1) instance.prompt = "smash";
    else instance.prompt = args[1];

    freeArgs(args);
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

ChangeDirCommand::ChangeDirCommand(char *cmd_line): BuiltInCommand(cmd_line) {}

void ChangeDirCommand::execute() {
    char **args = (char **) malloc(MAX_ARG_NUM * sizeof(char *));
    size_t size = _parseCommandLine(this->cmd_line, args);
    SmallShell &instance = SmallShell::getInstance();

    if (size == 1) { // No arguments
        cerr << "smash error:> " << cmd_line << endl;
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

        if (chdir(instance.prev_dir.c_str()))
            perror("smash error: chdir failed"); // Previous directory is invalid - could it be?
    }
    else { // User entered a path
        char cur_path[MAX_LINE_LEN];
        getcwd(cur_path, MAX_LINE_LEN);

        if (chdir(args[1])) perror("smash error: chdir failed"); // Path is invalid
        else instance.prev_dir = cur_path;
    }

    freeArgs(args);
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

JobsCommand::JobsCommand(char* cmd_line, JobsList* jobs): BuiltInCommand(cmd_line), jobs(jobs) {}

void JobsCommand::execute(){
    jobs->printJobsList();
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
    JobsList jobs = instance.smash_jobs_list;

    char **args = (char **) malloc(MAX_ARG_NUM * sizeof(char *));
    size_t size = _parseCommandLine(this->cmd_line, args);

    int job_id = 0;
    if (size > 2) {
        cerr << "smash error: fg: invalid arguments" << endl;
        freeArgs(args);
        return;
    }
    //job_id is specified
    else if (size == 2) {
        try {
            job_id = atoi(args[1]);
        } catch (invalid_argument &e) {
            cerr << "smash error: fg: invalid arguments" << endl;
            freeArgs(args);
            return;
        } catch (out_of_range &e) {
            cerr << "smash error: fg: invalid arguments" << endl;
            freeArgs(args);
            return;
        }
    }
    jobs.removeFinishedJobs();
    int last_job_id = 0;

    JobEntry *job = jobs.getLastJob(&last_job_id);
    //if no job_id specified, the job to be moved to the fg is the last job
    //if job_id is specified, we will change job to the job by id

    //no job_id && jobs list is empty
    if (!job && size == 1){
        cerr << "smash error: fg: jobs list is empty" << endl;
        freeArgs(args);
        return;
    }

    //job_id && jobs list is empty
    if (!job && size == 2){
        cerr << "smash error: fg: job-id " << args[1] << " does not exist" << endl;
        freeArgs(args);
        return;
    }

    if (job_id > 0){
        job = jobs.getJobById(job_id);

        //job_id && the job with that id doesn't  exist
        if (!job){
            cerr << "smash error: fg: job-id " << job_id << " does not exist" << endl;
            freeArgs(args);
            return;
        }
    }

    /*
    JobEntry *last_job = jobs.getLastJob(&last_job_id);

    //if no job id is specified, and last_job is null -> jobs list is empty
    if (!last_job && size == 1) {
        cerr << "smash error: fg: jobs list is empty" << endl;
        freeArgs(args);
        return;
    }

    //if job id is specified and jobs list is empty, different error (page 8)
    if (!last_job && size == 2) {
        cerr << "smash error: fg: job-id " << args[1] << " does not exist" << endl;
        freeArgs(args);
        return;
    }

    JobEntry *job = nullptr;
    if (0 < job_id) {
        //job_id>0 if job_id is specified
        job = jobs.getJobById(job_id);

        if (!job) {
            cerr << "smash error: fg: job-id " << job_id << " does not exist" << endl;
            freeArgs(args);
            return;
        }
    }
    else {
        cerr << "smash error: fg: invalid arguments" << endl;
        freeArgs(args);
        return;
    }
     */

    pid_t pid = job->pid;
    cout << job->command << " : " << pid << endl;

    if (job->stopped) {
        job->stopped = false;
        kill(pid, SIGCONT);
    }

    jobs.removeJobById(job_id);
    instance.updateCurrentJob(job);
    waitpid( pid, nullptr, WUNTRACED);
    instance.updateCurrentJob(nullptr);

    freeArgs(args);
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
    JobsList jobs = instance.smash_jobs_list;

    if (size > 2) {
        cerr << "smash error: bg: invalid arguments" << endl;
        freeArgs(args);
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
            freeArgs(args);
            return;
        } catch (out_of_range &e) {
            cerr << "smash error: bg: invalid arguments" << endl;
            freeArgs(args);
            return;
        }
        job = jobs.getJobById(job_id);
        if (!job) {
            cerr << "smash error: bg: job-id " << job_id << " does not exist" << endl;
            freeArgs(args);
            return;
        }
        if (!job->stopped) {
            cerr << "smash error: bg: job-id " << job_id << " is already running in the background" << endl;
            freeArgs(args);
            return;
        }
    } else {
        //no job id specified - last stopped job in jobs list is to continue running in bg
        job = jobs.getLastStoppedJob(&job_id);
        if (!job) {
            cerr << "smash error: bg: there is no stopped jobs to resume" << endl;
            freeArgs(args);
            return;
        }
    }

    pid_t pid = job->pid;
    cout << job->command << " : " << pid << endl;
    job->stopped = false;
    kill(pid, SIGCONT);

    freeArgs(args);
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
    JobsList jobs = instance.smash_jobs_list;

    if (size == 2 && !strcmp(args[1], "kill")) {
        jobs.removeFinishedJobs();
        cout << "smash: sending SIGKILL signal to " << jobs.jobs_map.size() << " jobs:" << endl;
        jobs.killAllJobs();

        exit(0);
    }
    else exit(0);

    freeArgs(args);
}



/// External commands ///

ExternalCommand::ExternalCommand(char *cmd_line) : Command(cmd_line) {}

void ExternalCommand::execute(){
    _removeBackgroundSign(this->cmd_line);
    char* args[MAX_ARG_NUM];
    size_t size = _parseCommandLine(this->cmd_line, args);
    char* path = (char*) malloc(MAX_LINE_LEN * sizeof(char));
    strcpy(path, "/bin/\0");

    pid_t pid = fork();
    if (pid == 0) { // Son, this is the actual external command
        if (setpgrp()) { // Unrelated to the logic, ignore it
            perror("smash error: setpgrp failed");
            exit(-1);
        }

        if (isComplexCommand(args)) { // Complex command
            strcat(path, "bash"); // Setting the path to be "/bin/bash\0"

            char* new_args[MAX_ARG_NUM + 3];
            new_args[0] = (char*)"bash\0";
            new_args[1] = (char*)"-c\0";
            size_t i = 2;
            for (; i < size + 2; i++) new_args[i] = args[i];
            new_args[i] = NULL;

            if (execv(path, new_args)) { // Passing non-const to a const argument is an implicit conversion, all good
                perror("smash error: execv failed");
                exit(-1);
            }
        }
        else { // Simple command
            strcat(path, args[0]); // Setting the path to be "/bin/<command>\0"

            if (execv(path, args)) {
                perror("smash error: execv failed");
                exit(-1);
            }
        }
    }
    else { // Father, this is the smash shell
        if (waitpid(pid, NULL, 0) == -1) { // In this case the process is in the foreground, so we wait for it. *Need to add background*
            perror("smash error: waitpid failed");
            return;
        }
    }

    freeArgs(args);
    free(path);
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




/*
 *  JOBS LIST FUNCTIONS
 */

// JobEntry c'tor
//JobEntry::JobEntry(pid_t pid, size_t time, string command, bool stopped): pid(pid), time(time), command(command), stopped(stopped) {}

// JobsList c'tor
JobsList::JobsList(): jobs_map() {}

/*
 * add job to the jobs list; differentiate between jobs in background and jobs that have been stopped
 */
void JobsList::addJob(Command *cmd, pid_t pid, bool last_fg, bool isStopped) {
    //before adding new jobs to the list, finished jobs are deleted from list
    removeFinishedJobs(); // Get back to here when implementing external background commands
    string command(cmd->get_cmd_line());
    SmallShell &instance = SmallShell::getInstance();

    if (last_fg) jobs_map.emplace <int, JobEntry> ((int)instance.fg_job_id, {pid, time(nullptr), command, isStopped});
    else{ // Note for self (shilo): no need to worry about cases such as running an fg proc and adding a different job, since it's not possible. Also, smile more
        int job_id;
        if (jobs_map.empty()) job_id = 0;
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
void JobsList::removeFinishedJobs() { // Waiting for completion of pipe
    // TBC
}

/*
 * return job with job id
 */
JobEntry* JobsList::getJobById(int jobId) {
    auto found = jobs_map.find(jobId);

    if (found == jobs_map.end()) return nullptr;
    else return &(found->second);
}

// Assert given key always exists in the map
void JobsList::removeJobById(int jobId) {
    jobs_map.erase(jobId);
}

JobEntry* JobsList::getLastJob(int *lastJobId) {
    if (jobs_map.empty()) return nullptr;

    auto end = jobs_map.end();
    *lastJobId = (--end)->first;

    return getJobById(*lastJobId);
}

JobEntry *JobsList::getLastStoppedJob(int *jobId) {
    int max = -1;
    for (auto &j: jobs_map) {
        if (j.second.stopped == true) max = j.first;
    }

    if (max == -1){
        *jobId = -1;
        return nullptr;
    }
    else{
        *jobId = max;
        return getJobById(max);
    }
}
