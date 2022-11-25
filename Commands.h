#ifndef SMASH_COMMAND_H_
#define SMASH_COMMAND_H_

#include <vector>
#include <ctime>
#include <sys/types.h>

#define COMMAND_ARGS_MAX_LENGTH (200)
#define COMMAND_MAX_ARGS (20)

class Command {
    char* cmd_line;
public:
    Command(const char* cmd_line): cmd_line(cmd_line) {}

    virtual ~Command();

    virtual void execute() = 0;

    //virtual void prepare();
    //virtual void cleanup();
    // TODO: Add your extra methods if needed
    const char* get_cmd_line() {
        return cmd_line;
    }
};

class BuiltInCommand : public Command {
public:
    BuiltInCommand(const char *cmd_line): Command(cmd_line) {}

    virtual ~BuiltInCommand() {}
};

class ExternalCommand : public Command {
public:
    ExternalCommand(const char *cmd_line);

    virtual ~ExternalCommand() {}

    void execute() override;
};

class PipeCommand : public Command {
    // TODO: Add your data members
public:
    PipeCommand(const char *cmd_line);

    virtual ~PipeCommand() {}

    void execute() override;
};

class RedirectionCommand : public Command {
    // TODO: Add your data members
public:
    explicit RedirectionCommand(const char *cmd_line);

    virtual ~RedirectionCommand() {}

    void execute() override;
    //void prepare() override;
    //void cleanup() override;
};

class JobsList;

/*
 * 1. chprompt
 */
class ChangePromptCommand : public BuiltInCommand {
public:
    ChangePromptCommand(const char *cmd_line);

    virtual ~ChangePromptCommand() {}

    void execute() override;
};

/*
 * 2. showpid
 */
class ShowPidCommand : public BuiltInCommand {
public:
    ShowPidCommand(const char *cmd_line);

    virtual ~ShowPidCommand() {}

    void execute() override;
};

/*
 * 3. pwd
 */
class GetCurrDirCommand : public BuiltInCommand {
public:
    GetCurrDirCommand(const char *cmd_line);

    virtual ~GetCurrDirCommand() {}

    void execute() override;
};

/*
 * 4. cd
 */
class ChangeDirCommand : public BuiltInCommand {
public:
    ChangeDirCommand(const char *cmd_line, char **plastPwd);

    virtual ~ChangeDirCommand() {}

    void execute() override;
};

/*
 * 5. jobs
 */
class JobsCommand : public BuiltInCommand {
    // TODO: Add your data members
public:
    JobsList* jobs;

    JobsCommand(const char *cmd_line, JobsList *jobs);

    virtual ~JobsCommand() {}

    void execute() override;
};

/*
 * 6. fg
 */
class ForegroundCommand : public BuiltInCommand {
    // TODO: Add your data members
public:
    JobsList *jobs;
    ForegroundCommand(const char *cmd_line, JobsList *jobs);

    virtual ~ForegroundCommand() {}

    void execute() override;
};

/*
 * 7. bg
 */
class BackgroundCommand : public BuiltInCommand {
    // TODO: Add your data members
public:
    JobsList *jobs;
    BackgroundCommand(const char *cmd_line, JobsList *jobs);

    virtual ~BackgroundCommand() {}

    void execute() override;
};

/*
 * 8. quit
 */
class QuitCommand : public BuiltInCommand {
public:
    char *cmd_line;
    JobsList *jobs;

    QuitCommand(const char *cmd_line, JobsList *jobs);

    virtual ~QuitCommand() {}

    void execute() override;
};

class TimeoutCommand : public BuiltInCommand {
/* Optional */
// TODO: Add your data members
public:
    explicit TimeoutCommand(const char *cmd_line);

    virtual ~TimeoutCommand() {}

    void execute() override;
};

class FareCommand : public BuiltInCommand {
    /* Optional */
    // TODO: Add your data members
public:
    FareCommand(const char *cmd_line);

    virtual ~FareCommand() {}

    void execute() override;
};

class SetcoreCommand : public BuiltInCommand {
    /* Optional */
    // TODO: Add your data members
public:
    SetcoreCommand(const char *cmd_line);

    virtual ~SetcoreCommand() {}

    void execute() override;
};

/*
 * 10. kill
 */
class KillCommand : public BuiltInCommand {
    /* Bonus */
    // TODO: Add your data members
public:
    KillCommand(const char *cmd_line, JobsList *jobs);

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

    time_t time;
    std::string command;
    pid_t pid;
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

    void addJob(Command *cmd, bool last_fg = false, bool isStopped = false);

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
    int fg_job_id; // Stores the job id of the job we removed out of jobList using 'fg'
    JobEntry *current_job;
    JobsList smash_jobs_list;

    Command *CreateCommand(const char *cmd_line);

    SmallShell(SmallShell const &) = delete; // disable copy c'tor
    void operator=(SmallShell const &) = delete; // disable '=' operator
    static SmallShell &getInstance() // make SmallShell singleton
    {
        static SmallShell instance; // Guaranteed to be destroyed
        // Instantiated on first use
        return instance;
    }

    ~SmallShell();

    void executeCommand(const char *cmd_line);
    // TODO: add extra methods as needed

    void updateCurrentJob(JobEntry* j) {
        current_job = j;
    }
};

#endif //SMASH_COMMAND_H_