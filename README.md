# OS-HW1
### In this assignment we've implemented a simplistic bash-like shell called 'smash'

'smash' has the ability to run builtin and external commands, as listed below.  
A job in the shell can run in the background (using the `&` operator), foreground or be stopped by pressing ctrl+z.  
The shell supports pipe and redirection operators (`>, >>, |, |&`), but only up to one operator in a command,  
and without the option to run it in background.  
The shell also supports sending SIGINT and SIGTSTP signals, by pressing ctrl+c and ctrl+z, respectively.

## Builtins:
The builtins were implemented by us, without `exec()`ing to a different program.
They can not be run in the background.
1. `chprompt <new_prompt>` - changes the prompt. If no parameters were sent, the prompt will be reset to 'smash'.
2. `showpid` - prints the shell's pid
3. `pwd` - prints the full path of the current working directory
4. `cd <new_path>` - changes to absolute or relative path given.  
If '-' is given as a parameter, it changes to last working directory
5. `jobs` - prints the jobs list, which contains background and stopped jobs.
6. `fg <job_id>` - brings to the foreground a job from the jobs list.  
If 'job_id' is specified, it brings the specified job.
7. `bg <job_id>` - resumes, in the background, one of the stopped processes.  
If 'job_id' is specified, it resumes the specified job.
8. `exit <kill>` - exits the shell. If 'kill' was specified, it kills and prints all unfinished and stopped jobs.
9. `kill -<signum> <job_id>` - sends a signal corresponding to <signum> to a corresponding to <job_id>.
10. `fare <file-name> <source> <destination>` - finds and replaces every instance of the word 'source' to the word 'destination',
and then prints how many instances of 'source' were replaced.

## External:
Commands which are not builtins would be regarded as externals, and would be run by using `exec()`.
