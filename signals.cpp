#include <iostream>
#include <signal.h>
#include "signals.h"
#include "Commands.h"

using namespace std;

void ctrlZHandler(int sig_num){
    cout <<  "smash: got ctrl-Z" << endl;
    SmallShell &instance = SmallShell::getInstance();
    JobEntry* current_job = instance.getCurrentJob();

    //add the foreground process to the jobs list. If no process is running in the foreground, then nothing will be added to the jobs list.
    //send SIGSTOP to the process in the foreground. If no process is running in the foreground, then no signal will be sent. (page 16)
    if (instance.current_pid != -1){
        Command* command = instance.CreateCommand(instance.current_command.c_str());
        if (instance.last_fg) instance.smash_jobs_list.addJob(command, instance.current_pid, true, true); // If last was foreground
        else instance.smash_jobs_list.addJob(command, instance.current_pid, true, true);

        kill(current_pid, SIGSTOP);
        cout << "smash: process " << current_pid << " was stopped" << endl;

        instance.current_pid = -1;
        instance.current_command = "";
        //delete command; Depends on allocation with new in factory method. brb
    }
}

void ctrlCHandler(int sig_num) {
  cout << "smash: got ctrl-C" << endl;
  SmallShell &instance = SmallShell::getInstance();
  if (instance.current_pid != -1){
      int pid = instance.current_pid;
      instance.current_pid = -1;
      instance.current_command = "";
      kill(pid, SIGINT);
      cout << "smash: process " << pid << " was killed"
  }
}

void alarmHandler(int sig_num) {
  // TODO: Add your implementation
}
