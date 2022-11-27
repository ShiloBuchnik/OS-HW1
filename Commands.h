#ifndef SMASH_COMMAND_H_
#define SMASH_COMMAND_H_

#include <map>
#include <ctime>
#include <sys/types.h>

#define COMMAND_ARGS_MAX_LENGTH (200)
#define COMMAND_MAX_ARGS (20)

class Command {
public:
    char* cmd_line;
    Command(char* cmd_line): cmd_line(cmd_line) {}

    virtual ~Command() {}

    virtual void execute() = 0;

    /*virtual void prepare();
    virtual void cleanup();
    char* get_cmd_line() {
        return cmd_line;
    }*/
};

class BuiltInCommand: public Command {
public:
    BuiltInCommand(char *cmd_line);

    virtual ~BuiltInCommand() {}
};

class ExternalCommand: public Command {
public:
    ExternalCommand(char *cmd_line);

    virtual ~ExternalCommand() {}

    void execute() override;
};

class PipeCommand: public Command {
    std::string com1;
    std::string com2;
    std::string bar;

public:
    PipeCommand(char *cmd_line);

    virtual ~PipeCommand() {}

    void execute() override;

    void pipeErrorHandle(int pipe0, int pipe1);
};

class RedirectionCommand: public Command {
public:
    char* cmd;
    char* filename;
    bool is_append;
    bool is_success; // redirect_success, delete this comment later
    int fd_copy_of_stdout;
    int fd;

    explicit RedirectionCommand(char *cmd_line);

    virtual ~RedirectionCommand() {}

    void execute() override;
    void prepare();
    void cleanup();
};

class JobsList;

/*
 * 1. chprompt
 */
class ChangePromptCommand: public BuiltInCommand {
public:
    ChangePromptCommand(char *cmd_line);

    virtual ~ChangePromptCommand() {}

    void execute() override;
};

/*
 * 2. showpid
 */
class ShowPidCommand: public BuiltInCommand {
public:
    ShowPidCommand(char *cmd_line);

    virtual ~ShowPidCommand() {}

    void execute() override;
};

/*
 * 3. pwd
 */
class GetCurrDirCommand: public BuiltInCommand {
public:
    GetCurrDirCommand(char *cmd_line);

    virtual ~GetCurrDirCommand() {}

    void execute() override;
};

/*
 * 4. cd
 */
class ChangeDirCommand: public BuiltInCommand {
public:
    //ChangeDirCommand(char *cmd_line, char **plastPwd);
    ChangeDirCommand(char *cmd_line);

    virtual ~ChangeDirCommand() {}

    void execute() override;
};

/*
 * 5. jobs
 */
class JobsCommand: public BuiltInCommand {
    // TODO: Add your data members
public:
    JobsList* jobs;

    JobsCommand(char *cmd_line, JobsList* jobs);

    virtual ~JobsCommand() {}

    void execute() override;
};

/*
 * 6. fg
 */
class ForegroundCommand: public BuiltInCommand {
    // TODO: Add your data members
public:
    ForegroundCommand(char *cmd_line);

    virtual ~ForegroundCommand() {}

    void execute() override;
};

/*
 * 7. bg
 */
class BackgroundCommand: public BuiltInCommand {
    // TODO: Add your data members
public:
    BackgroundCommand(char *cmd_line);

    virtual ~BackgroundCommand() {}

    void execute() override;
};

/*
 * 8. quit
 */
class QuitCommand: public BuiltInCommand {
public:
    QuitCommand(char *cmd_line);

    virtual ~QuitCommand() {}

    void execute() override;
};

class TimeoutCommand: public BuiltInCommand {
/* Optional */
// TODO: Add your data members
public:
    explicit TimeoutCommand(char *cmd_line);

    virtual ~TimeoutCommand() {}

    void execute() override;
};

class FareCommand: public BuiltInCommand {
    /* Optional */
    // TODO: Add your data members
public:
    FareCommand(char *cmd_line);

    virtual ~FareCommand() {}

    void execute() override;
};

class SetcoreCommand: public BuiltInCommand {
    /* Optional */
    // TODO: Add your data members
public:
    SetcoreCommand(char *cmd_line);

    virtual ~SetcoreCommand() {}

    void execute() override;
};

/*
 * 10. kill
 */
class KillCommand: public BuiltInCommand {
    /* Bonus */
    // TODO: Add your data members
public:
    KillCommand(char *cmd_line, JobsList *jobs);

    virtual ~KillCommand() {}

    void execute() override;
};



class JobEntry {
public:
    /*
     * job id is assigned by the shell once it's inserted to the jobs list
     * time is seconds elapsed since job was inserted to the jobs list (?)
     * name is the job's command
     * pid is assigned to the process by the kernel
     */

    pid_t pid;
    time_t time;
    std::string command;
    bool stopped;

    JobEntry(pid_t pid, time_t time, std::string command, bool stopped);
};

class JobsList {
public:
    /*
     * jobs list is a vector made of jobEntry elements
     */
    std::map <int, JobEntry> jobs_map;

    // TODO: Add your data members
public:
    JobsList();

    ~JobsList() = default;

    void addJob(Command *cmd, pid_t pid, bool last_fg = false, bool isStopped = false);
    void printJobsList();

    void killAllJobs();

    void removeFinishedJobs();
    JobEntry *getJobById(int jobId);

    void removeJobById(int jobId);

    JobEntry *getLastJob(int *lastJobId);

    JobEntry *getLastStoppedJob(int *jobId);
    // TODO: Add extra methods or modify exisitng ones as needed
};

class SmallShell {
private:
    SmallShell();

public:
    std::string prompt; // For chprompt
    std::string prev_dir; // For cd
    bool last_fg; // True if last command was 'fg'
    int fg_job_id; // Stores the job id of the last job we removed out of jobList using 'fg'
    JobEntry* current_job;
    JobsList smash_jobs_list;
    pid_t current_pid;
    std::string current_command;
    bool is_pipe;

    SmallShell(SmallShell const &) = delete; // disable copy c'tor
    void operator=(SmallShell const &) = delete; // disable '=' operator
    static SmallShell &getInstance() // make SmallShell singleton
    {
        static SmallShell instance; // Guaranteed to be destroyed
        // Instantiated on first use
        return instance;
    }

    ~SmallShell();

    Command* CreateCommand(char *cmd_line);

    void executeCommand(const char *cmd_line);
    // TODO: add extra methods as needed
};

#endif //SMASH_COMMAND_H_