#include <iostream>
#include <signal.h>
#include "signals.h"
#include "Commands.h"

using namespace std;

void ctrlZHandler(int sig_num) {
	// TODO: Add your implementation

    cout <<  "smash: got ctrl-Z" << endl;
    SmallShell &instance = SmallShell::getInstance();
    JobEntry* current_job = instance.getCurrentJob();

    //add the foreground process to the jobs list. If no process is running in the foreground, then nothing will be added to the jobs list.
    //send SIGSTOP to the process in the foreground. If no process is running in the foreground, then no signal will be sent. (page 16)
    if (current_job){
        pid_t current_pid = current_job->pid;
        std::string current_command = current_job->command;

        if (instance.last_fg) {
            //if the last command was in fg
            kill(current_pid, SIGSTOP);
            current_job->stopped = true;
            cout << "smash: process " << current_pid << " was stopped" << endl;
            instance.smash_jobs_list.addJob(current_command, current_pid, true, true);
        }

    }

}

void ctrlCHandler(int sig_num) {
  // TODO: Add your implementation

  cout << ": smash: got ctrl-C" << endl;
  SmallShell &instance = SmallShell::getInstance();
  if (instance.current_pid != -1){
      int pid = instance.current_pid;
      instance.current_pid = -1;
      instance.current_cmd = "";
      kill (pid, SIGINT);
      cout << "smash: process " << pid << " was killed"
  }
}

void alarmHandler(int sig_num) {
  // TODO: Add your implementation
}
