/*
 * esh - the 'pluggable' shell.
 *
 * Developed by Godmar Back for CS 3214 Fall 2009
 * Virginia Tech.
 */
#include <stdio.h>
#include <readline/readline.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>

#include "esh-sys-utils.h"
#include "esh.h"

//#define DEBUG 1

static void
usage(char *progname)
{
    printf("Usage: %s -h\n"
        " -h            print this help\n"
        " -p  plugindir directory from which to load plug-ins\n",
        progname);

    exit(EXIT_SUCCESS);
}

/* Build a prompt by assembling fragments from loaded plugins that 
 * implement 'make_prompt.'
 *
 * This function demonstrates how to iterate over all loaded plugins.
 */
static char *
build_prompt_from_plugins(void)
{
    char *prompt = NULL;
    struct list_elem * e = list_begin(&esh_plugin_list);

    for (; e != list_end(&esh_plugin_list); e = list_next(e)) {
        struct esh_plugin *plugin = list_entry(e, struct esh_plugin, elem);

        if (plugin->make_prompt == NULL)
            continue;

        /* append prompt fragment created by plug-in */
        char * p = plugin->make_prompt();
        if (prompt == NULL) {
            prompt = p;
        } else {
            prompt = realloc(prompt, strlen(prompt) + strlen(p) + 1);
            strcat(prompt, p);
            free(p);
        }
    }

    /* default prompt */
    if (prompt == NULL)
        prompt = strdup("esh> ");

    return prompt;
}

/* Return the list of tthe current jobs
 * Implementing the function in esh.h */
static struct list * get_jobs(void)
{
	return &joblist;
}

/* Return job corresponding to jid
 * Implementing the function in esh.h */
static struct esh_pipeline * get_job_from_jid(int jobid)
{
    /* list traversal to look for by jobid */
    struct list_elem *e;
    for(e = list_begin(&joblist); list_end(&joblist); e = list_next(e))
    {
	struct esh_pipeline *job = list_entry(e, struct esh_pipeline, elem);
	if (job->jid == jobid)
	{
		return job;
	}
    }

    /* not found return NULL */
    return NULL;	
}

/* Return job corresponding to pgrp
 * Implementing the function in esh.h */
static struct esh_pipeline * get_job_from_pgrp(pid_t pgrp)
{
    /* list traversal to look for by pgrp */
    struct list_elem *e;
    for(e = list_begin(&joblist); list_end(&joblist); e = list_next(e))
    {
	struct esh_pipeline *job = list_entry(e, struct esh_pipeline, elem);
	if (job->pgrp == pgrp)
	{
		return job;
	}
    }
	
    /* if not found */
    return NULL;
}


/* The shell object plugins use.
 * Some methods are set to defaults.
 */
struct esh_shell shell =
{
    .build_prompt = build_prompt_from_plugins,
    .readline = readline,       /* GNU readline(3) */ 
    .parse_command_line = esh_parse_command_line, /* Default parser */
    .get_jobs = get_jobs,
    .get_job_from_jid = get_job_from_jid,
    .get_job_from_pgrp = get_job_from_pgrp    
};

/**
 *  * Assign ownership of ther terminal to process group
 *   * pgrp, restoring its terminal state if provided.
 *    *
 *     * Before printing a new prompt, the shell should
 *      * invoke this function with its own process group
 *       * id (obtained on startup via getpgrp()) and a
 *        * sane terminal state (obtained on startup via
 *         * esh_sys_tty_init()).
 *          */
static void
give_terminal_to(pid_t pgrp, struct termios *pg_tty_state)
{
    esh_signal_block(SIGTTOU);
    int rc = tcsetpgrp(esh_sys_tty_getfd(), pgrp);
    if (rc == -1)
        esh_sys_fatal_error("tcsetpgrp: ");

    if (pg_tty_state)
        esh_sys_tty_restore(pg_tty_state);
    esh_signal_unblock(SIGTTOU);
}


/* Wait for all processes in this pipeline to complete, or for
 * the pipeline's process group to no longer be the foreground 
 * process group. 
 * You should call this function from a) where you wait for
 * jobs started without the &; and b) where you implement the
 * 'fg' command.
 * 
 * Implement child_status_change such that it records the 
 * information obtained from waitpid() for pid 'child.'
 * If a child has exited or terminated (but not stopped!)
 * it should be removed from the list of commands of its
 * pipeline data structure so that an empty list is obtained
 * if all processes that are part of a pipeline have 
 * terminated.  If you use a different approach to keep
 * track of commands, adjust the code accordingly.
 */
static void
wait_for_job(struct esh_pipeline *pipeline, struct termios *shell_state)
{
    assert(esh_signal_is_blocked(SIGCHLD));
    while (pipeline->status == FOREGROUND && !list_empty(&pipeline->commands)) {
        int status;
	#ifdef DEBUG
	    printf("[DEBUG]status: %d is list empty? %d function waitfor job\n", pipeline->status,list_empty(&pipeline->commands));
	#endif
        pid_t child = waitpid(-1, &status, WUNTRACED);
	#ifdef DEBUG
	    printf("[DEBUG]reap child in waitforjob\n");
	#endif
        if (child != -1)
	{
	    child_status_change(child, status);
	    #ifdef DEBUG
	        printf("[DEBUG](wait_for_job)getpgrp(): %d, childid: %d\n", getpgrp(), child);
	    #endif
	}
    }
}

/* test if current job is done */
static int job_completed(struct esh_pipeline* pipe) {
    struct list_elem* cmdlink = list_begin(&pipe->commands);
    for (; cmdlink != list_end(&pipe->commands); cmdlink = list_next(cmdlink))
    {
	struct esh_command * cmd = list_entry(cmdlink, struct esh_command, elem);
		
	if (cmd->complete == 0)
	{
	    return 0;
	}
    }
    return 1;
}

//put job to the foreground and wait for it to ger completed
static void put_job_to_foreground(struct esh_pipeline* pipe, int cont, pid_t shell_group, struct termios* shell_state)
{
    struct list_elem *e;
    for(e = list_begin(&pipe->commands); e != list_end(&pipe->commands); e = list_next(e))
    {
	struct esh_command *command = list_entry(e, struct esh_command, elem);
	char **argv = command->argv;
	while(*argv)
	{
	    printf("%s ", *argv);
	    argv++;	
	}
	printf("\n");
    }

    /* first to give terminal */
    give_terminal_to(pipe->pgrp, shell_state);
    if (cont)
    {
	fflush(stdout);

	if (kill(-pipe->pgrp, SIGCONT) < 0)
	{
	    perror("put job to foreground error\n");
	}
		
    }

    /* start waiting for job */
    wait_for_job(pipe, &(pipe->saved_tty_state));

    /* ses if there are other commands in the pipeline */
    if (!job_completed(pipe))
    {
	esh_sys_tty_save(&pipe->saved_tty_state);
	pipe->status = STOPPED;
    }
    else
    {	
	list_remove(&pipe->elem);
	esh_pipeline_free(pipe);
    }

    give_terminal_to(shell_group, shell_state);
}

/* put job to the background */
static void put_job_to_background(struct esh_pipeline* pipe, int cont)
{	
    pipe->bg_job = true;
    #ifdef DEBUG
	printf("[DEBUG]we are in put_job_to_background\n");
    #endif
    if (cont)
    {
	if (kill(-pipe->pgrp, SIGCONT) < 0)
	{
	    perror("something went wrong with kill sigcont");
	}
	else
	{
	    pipe->status = BACKGROUND;
	}
    }
}

/* put stopped job to run again */
static void continue_job(struct esh_pipeline * pipe, pid_t pgid, int bg, struct termios * shell_state)
{
    #ifdef DEBUG
	printf("[DEBUG]we are in continue job\n");
    #endif
    struct list_elem* cmdline = list_begin(&pipe->commands);
    for (; cmdline != list_end(&pipe->commands); cmdline = list_next(cmdline))
    {
	struct esh_command * cmd = list_entry(cmdline, struct esh_command, elem);
	cmd->stop = 0;
    }
	
    if (!bg)
    {
	#ifdef DEBUG
	    printf("[DEBUF]putting job to fg\n");
	#endif
	put_job_to_foreground(pipe, 1, pgid, shell_state);
    }
    else
    {
	#ifdef DEBUG
	    printf("[DEBUF]putting job to bg\n");
	#endif
	put_job_to_background(pipe, 1);
    }
}

/* get foreground jobid */
struct list_elem * getForegroundJob()
{
    struct list_elem * job;
    for (job = list_begin(&joblist); job != list_end(&joblist); job = list_next(job))
    {
	struct esh_pipeline *pipeline = list_entry(job, struct esh_pipeline, elem);
	if (pipeline->status == FOREGROUND)
	{
	    return job;
	}
    }
    return NULL;
}

/*
 * SIGCHLD handler.
 * Call waitpid() to learn about any child processes that
 * have exited or changed status (been stopped, needed the
 * terminal, etc.)
 * Just record the information by updating the job list
 * data structures.  Since the call may be spurious (e.g.
 * an already pending SIGCHLD is delivered even though
 * a foreground process was already reaped), ignore when
 * waitpid returns -1.
 * Use a loop with WNOHANG since only a single SIGCHLD 
 * signal may be delivered for multiple children that have 
 * exited.
 */
static void
sigchld_handler(int sig, siginfo_t *info, void *_ctxt)
{
    #ifdef DEBUG
        printf("[DEBUG]caught sigchld function sighandler\n");
    #endif
    pid_t child;
    int status;

    assert(sig == SIGCHLD);

    while ((child = waitpid(-1, &status, WUNTRACED|WNOHANG)) > 0)
    {
	child_status_change(child, status);
	#ifdef DEBUG
	    printf("[DEBUG]signal change end ppid:%d, pid:%d\n", getppid(), child);
	#endif
    }
}



/* change the status of a child */
void child_status_change(pid_t childpid, int status)
{
    if (childpid > 0)
    {
	struct list_elem *job;
	for (job = list_begin(&joblist); job != list_end(&joblist); job = list_next(job))
	{
	    struct esh_pipeline *pipeline = list_entry(job, struct esh_pipeline, elem);
    	    struct list_elem * cmdlist = list_begin(&pipeline->commands);
	    while (cmdlist != list_end(&pipeline->commands) && !list_empty(&pipeline->commands))
	    {
		struct esh_command *cmd = list_entry(cmdlist, struct esh_command, elem);
		if (cmd->pid == childpid)
		{
		    if (WIFSTOPPED(status))
		    {
			pipeline->status = STOPPED;
			if (WSTOPSIG(status) == SIGTSTP)
			{
			    cmd->complete = 0;
		            cmd->stop = 1;
			    printf("\n");
			    // print stop statement
			    printf("[%d] Stopped\t(", pipeline->jid);
			    char ** p = cmd->argv;
			    while (*p)
			    {
				printf("%s ", *p++);
		  	    }
							
			    printf(")\n");
		 	    esh_sys_tty_save(shell_state);
			}
			cmdlist =  list_next(cmdlist);
		    }
		    else if (WIFEXITED(status))
		    {
			cmdlist = list_remove(cmdlist);
		    }
		    else if (WIFSIGNALED(status))
		    {
			if (WTERMSIG(status) == SIGINT)
			{
		            printf("\n");
			    cmdlist = list_remove(cmdlist);
			}
		    }
		}
		else
		{
	   	    cmdlist = list_next(cmdlist);
		}
	    }
	}
    }
    else if (childpid < 0)
    {
	esh_sys_fatal_error("Wait ERROR in child_status_change");
    }
}



int
main(int ac, char *av[])
{
    esh_signal_sethandler(SIGCHLD, sigchld_handler);
    jobid = 0;	//intialize the number of jobs
    shell_pgrp = getpid();

    int opt;
    list_init(&esh_plugin_list);
    list_init(&joblist);

    /* Process command-line arguments. See getopt(3) */
    while ((opt = getopt(ac, av, "hp:")) > 0) {
        switch (opt) {
        case 'h':
            usage(av[0]);
            break;

        case 'p':
            esh_plugin_load_from_directory(optarg);
            break;
        }
    }

    esh_plugin_initialize(&shell);

    setpgid(0, 0);
    
    shell_state = esh_sys_tty_init();
    give_terminal_to(getpgrp(), shell_state);


    /* Read/eval loop. */
    for (;;) {
	//printf("in eval lopp");	
        /* Do not output a prompt unless shell's stdin is a terminal */
        char * prompt = isatty(0) ? shell.build_prompt() : NULL;
        char * cmdline = shell.readline(prompt);
        
        //To check if plugin wants to change the command lines
        struct list_elem *pluginCMD;
        for (pluginCMD=list_begin(&esh_plugin_list); pluginCMD!=list_end(&esh_plugin_list); pluginCMD=list_next(pluginCMD)) {
			
				struct esh_plugin * plugin = list_entry(pluginCMD,struct esh_plugin,elem);
				if(plugin->process_raw_cmdline) {
						plugin->process_raw_cmdline(&cmdline);
				}
		}
		
        free (prompt);

        if (cmdline == NULL)  /* User typed EOF */
            break;
		
		
        struct esh_command_line * cline = shell.parse_command_line(cmdline);
        free (cmdline);
        if (cline == NULL)                  /* Error in command line */
            continue;

        if (list_empty(&cline->pipes)) {    /* User hit enter */
            esh_command_line_free(cline);
            continue;
        }

	/* construct command and pipeline to pass to eval*/
	struct esh_pipeline *pipeline;
	pipeline = list_entry(list_begin(&cline->pipes), struct esh_pipeline, elem);
	struct esh_command *commands;
	commands = list_entry(list_begin(&pipeline->commands), struct esh_command, elem);
	
	//check if plugin wants to change the pipeline
	for (pluginCMD = list_begin(&esh_plugin_list); pluginCMD != list_end(&esh_plugin_list); pluginCMD = list_next(pluginCMD)) { 
		struct esh_plugin * plugin=list_entry(pluginCMD,struct esh_plugin,elem);
		if(plugin->process_pipeline) {
				
		plugin->process_pipeline(pipeline);
		}
	}
	bool isPlugin = false;
	struct esh_command *command=list_entry(list_begin(&pipeline->commands),struct esh_command,elem);
	for(pluginCMD = list_begin(&esh_plugin_list); pluginCMD != list_end(&esh_plugin_list); pluginCMD=list_next(pluginCMD)) {
			struct esh_plugin *plugin=list_entry(pluginCMD,struct esh_plugin,elem);
			if(plugin->process_builtin) {
					if(plugin->process_builtin(command)) {
							isPlugin = true;
							continue;
					}
			}
	}        //esh_command_line_print(cline);
	if (!isPlugin) {
		eval_command(cline, pipeline, commands, shell_state);
	}
		
	struct list_elem *e = list_begin(&joblist);
	while (e != list_end(&joblist) && !list_empty(&joblist))
	{
	    struct esh_pipeline *p = list_entry(e, struct esh_pipeline, elem);
	    if (list_empty(&p->commands))
	    {
		e = list_remove(&p->elem);
	    }
	    else
	    {
		e = list_next(e);
	    }
	}
		
        esh_command_line_free(cline);
	//printf("after freeing, go back to loop");
    }
    return 0;
}    

/* This function evaluates the command line
 */
void eval_command(struct esh_command_line *cline, struct esh_pipeline *pipeline, struct esh_command *cmdline, struct termios *shell_state)
{
    int builtin = builtin_command(cmdline->argv[0]);
		
    /* if it's not build-in command, fork new child */
    if(builtin == 0)
    {
	jobid += 1;
	if (list_empty(&joblist))
	{
		jobid = 1;
	}
	pipeline->jid = jobid;
	pipeline->pgrp = -1; // it's from terminal)
	
	esh_signal_block(SIGCHLD);
	pid_t pid;

	int oldPipe[2],newPipe[2];

	struct list_elem *commandElem;
	for(commandElem=list_begin(&pipeline->commands); commandElem!=list_end(&pipeline->commands); commandElem=list_next(commandElem))
	{
	    struct esh_command *command=list_entry(commandElem,struct esh_command,elem);

	    if(list_size(&pipeline->commands)>1 && commandElem!=list_back(&pipeline->commands))
	    {
		pipe(newPipe);
	    }

    	    pid = fork();
	    if (pid < 0)
	    {
		esh_sys_fatal_error("Fork Error ");
	    }
	    // this is child
	    else if(pid == 0)
	    {
		pid = getpid();
		command->pid = pid;

		if(pipeline->pgrp == -1)
		{
		    pipeline->pgrp = pid;
		}

		// Set the pgrp of the every command process as the pgrp of the pipeline
		if(setpgid(pid,pipeline->pgrp) < 0)
		{
	  	    esh_sys_fatal_error("setpgid error");
		}

		if(command->iored_input!= NULL)
		{
		    int fd_in = open(command->iored_input,O_RDONLY);
								
		    if (dup2(fd_in, 0) < 0)
		    {
			esh_sys_fatal_error("dup2 error");
		    }
		    close(fd_in);
		}

		if(command->iored_output != NULL)
		{
		    int fd_out;
								
		    if(command->append_to_output)
		    {
			fd_out = open(command->iored_output, O_WRONLY | O_APPEND | O_CREAT, S_IRUSR | S_IRGRP | S_IWGRP | S_IWUSR);
		    }
		    else
		    {
			fd_out = open(command->iored_output, O_WRONLY | O_TRUNC  | O_CREAT, S_IRUSR | S_IRGRP | S_IWGRP | S_IWUSR);
		    }
		    if((dup2(fd_out,1)) < 0)
		    {
			esh_sys_fatal_error("dup2 error");
		    };
		    close(fd_out);
		}

		// IO direction
		if(list_size(&pipeline->commands) > 1)
		{
		    // Commands that are not the head
	   	    if(commandElem!=list_begin(&pipeline->commands))
		    {
			dup2(oldPipe[0],0);
			close(oldPipe[0]);
			close(oldPipe[1]);
		    }

		    // Commands that are not the back
		    if(commandElem!=list_back(&pipeline->commands))
		    {
			dup2(newPipe[1],1);
			close(newPipe[1]);
			close(newPipe[0]);
		    }

		}// End of pipeline size > 1

		esh_signal_unblock(SIGCHLD);
		if(execvp(command->argv[0],command->argv)<0)
		{
		    esh_sys_fatal_error("");
		}
	    } // End of Child
	    // Parent
	    else
	    {
		// Make the pid of the first command in the command list the pgrp of the pipeline
		if(pipeline->pgrp == -1)
		{
		    pipeline->pgrp=pid;
		}
		command->pid=pid;
		// Set the pgrp of the every command process as the pgrp of the pipeline
		if(setpgid(pid,pipeline->pgrp) < 0)
		{
	   	    esh_sys_fatal_error("setpgid error");
		}
						
		if(list_size(&pipeline->commands) > 1)
		{
		    //close pipes for the old pipe when commandelem is not the first element in the list
		    if(commandElem != list_begin(&pipeline->commands))
		    {
			close(oldPipe[0]);
			close(oldPipe[1]);
		    }
		    //close pipes for the new pipe when commandelem is no the last element in the list
		    if(commandElem != list_back(&pipeline->commands))
		    {
			oldPipe[0] = newPipe[0];
			oldPipe[1] = newPipe[1];
		    }
								
	 	    //close all pipes when commandelem is the last element in the list
		    if(commandElem==list_back(&pipeline->commands))
		    {
			close(oldPipe[0]);
			close(oldPipe[1]);
			close(newPipe[0]);
			close(newPipe[1]);
		    }
		}
	    }
	}
		
		
	struct list_elem *e;
	for(e = list_begin(&esh_plugin_list); e != list_end(&esh_plugin_list); e = list_next(e))
	{		
	    struct esh_plugin * plugin = list_entry(e,struct esh_plugin,elem);
	    if(plugin->pipeline_forked)
	    {
		plugin->pipeline_forked(pipeline);
	    }
	}
	
	/* adding the job */	
	commandElem = list_pop_front(&cline->pipes);
	list_push_back(&joblist, commandElem);


	if (!pipeline->bg_job)
	{
	    pipeline->status = FOREGROUND;
	    //printf("parent about to wait\n");
	    give_terminal_to(pipeline->pgrp, shell_state);
	    wait_for_job(pipeline, shell_state);
	}
	else	
	{			
	    pipeline->status = BACKGROUND;
	    put_job_to_background(pipeline, 1);
	    printf("[%d] %d\n", pipeline->jid, pipeline->pgrp);
	 }
			
	give_terminal_to(getpgrp(), shell_state);
	esh_signal_unblock(SIGCHLD);

	/*
	if (list_empty(&pipeline->commands)) {
		
		list_remove(&pipeline->elem);
	}*/
	//give_terminal_to(getpgrp(), shell_state);
			
	return;
    }

    /* if it's a build-in command */
    // jobs
    if (builtin == 2)
    {
	char *status[] = {"Foreground", "Running", "Stopped", "Needs Terminal"};
	struct list_elem *e;
	for (e = list_begin(&joblist);e != list_end(&joblist);e = list_next(e))
	{
	    struct esh_pipeline *pipeline = list_entry(e, struct esh_pipeline, elem);
	    printf("[%d] %s\t", pipeline->jid, status[pipeline->status]);

	    printf("(");
	    /* print actual job name */
	    struct list_elem *commandElem;
	    for (commandElem = list_begin(&pipeline->commands); commandElem != list_end(&pipeline->commands); commandElem = list_next(commandElem))
	    {
		struct esh_command *command = list_entry(commandElem, struct esh_command, elem);
		int i = 0;
		while(command->argv[i])
		{
		    printf("%s", command->argv[i]);
		    i++;
		    if(command->argv[i])
			printf(" ");
		}
	    }
	    if (pipeline->bg_job)
		printf(" &");
	    printf(")\n");
	}
    }
    // fg
    else if (builtin == 3)
    {
	struct list_elem* pipeline = list_begin(&joblist);
	int found = 1;
	//esh_signal_sethandler(SIGCHLD, sigchld_handler);
	//esh_signal_sethandler(SIGTSTP, sigtstp_handler);
	//esh_signal_sethandler(SIGINT, sigint_handler);
	esh_signal_block(SIGCHLD);
		
	for (; pipeline != list_end(&joblist); pipeline = list_next(pipeline))
	{
	    //printf("in the loop now\n");
	    struct esh_pipeline* job = list_entry(pipeline, struct esh_pipeline, elem);
	    //printf("job id is %d\n", job->jid);
	    if (job->jid == atoi(cmdline->argv[1]))
	    {
		//printf("job status is %d\n", job->status);
		if (job->status == BACKGROUND)
		{
		    job->status = FOREGROUND;
		    //printf("job status background\n");
		    put_job_to_foreground(job, 0, shell_pgrp, shell_state);
		    break;
		}
		else if (job->status == FOREGROUND)
		{
		//printf("job status foreground\n");
		    break;
		}
		else
		{
		//printf("job status other\n");
		    job->status = FOREGROUND;
		    continue_job(job, shell_pgrp, 0, shell_state);
		    break;
		}
	    }
	}
	esh_signal_unblock(SIGCHLD);
	if (!found)
	{
	    printf("no such job");
	}
    }
    // bg
    else if (builtin == 4)
    {
	struct list_elem * job_link = list_head(&joblist);
	#ifdef DEBUG
	    printf("[DEBUG] bg processing\n");
	#endif
	bool found = false;
	if (cmdline->argv[1] == NULL)
	{
	    found = true;
   	    #ifdef DEBUG
		printf("[DEBUG] argv is %s\n", cmdline->argv[1]);
	    #endif
	    //run the most recently used job if no second argument
	    struct list_elem * jobtail = list_end(&joblist);
	    jobtail = list_prev(jobtail);

	    //check if there is any job in the pipeline
	    if (jobtail ==  list_head(&joblist))
	    {
		printf("no job is currently running\n");
		return;
	    }
	    struct esh_pipeline* job = list_entry(jobtail, struct esh_pipeline, elem);
	    kill(job->pgrp, SIGCONT);
	    return;
	}
			
	for (; job_link != list_end(&joblist); job_link = list_next(job_link))
	{
	    struct esh_pipeline* job = list_entry(job_link, struct esh_pipeline, elem);
			
	    if (job->jid == atoi(cmdline->argv[1]))
	    {
		found = true;
		#ifdef DEBUG
		    printf("[DEBUG] argv is %s\n", cmdline->argv[1]);
		#endif
		continue_job(job, shell_pgrp, 1, shell_state); 
		return;
	    }
	}
		
	if (!found)
	{		
	    printf("No job matched witht the given ID\n");
	}
    }
    // kill
    else if (builtin == 5)
    {
	struct list_elem * job_link = list_head(&joblist);
		
	if (cmdline->argv[1] == NULL)
	{
	    printf("Need the job id");
	}
	for (; job_link != list_end(&joblist); job_link = list_next(job_link))
	{	
	    struct esh_pipeline* job = list_entry(job_link, struct esh_pipeline, elem);
	    #ifdef DEBUG
		printf("[DEBUG] jid is %d", job->jid);
	    #endif
	    if (job->jid == atoi(cmdline->argv[1]))
	    {	
		kill(-job->pgrp, SIGKILL);
		list_remove(job_link);
		return;
	    }
	}
	printf("No job matched witht the given ID\n");
		
    }
    //stop
    else if (builtin == 6)  
    {
	struct list_elem * job_link = list_head(&joblist);
		
	if (cmdline->argv[1] == NULL)
	{
	    printf("Need the job id");
	}
	for (; job_link != list_end(&joblist); job_link = list_next(job_link))
	{	
	    struct esh_pipeline* job = list_entry(job_link, struct esh_pipeline, elem);
			
	    if (job->jid == atoi(cmdline->argv[1]))
	    {			
		kill(-job->pgrp, SIGSTOP);
		job->status = STOPPED;
		return;
	    }
	}
	printf("No job matched witht the given ID\n");
    }

    return;
}

/* this function decides whether argv[0] is a builtin_function
 * and return the number of the built-in command */
int builtin_command(char *argv)
{
    if (!strcmp(argv, "exit"))
	exit(0);
    if (!strcmp((argv), "&"))
	return 1;
    if (!strcmp(argv, "jobs"))
	return 2;
    if (!strcmp(argv, "fg"))
	return 3;
    if (!strcmp(argv, "bg"))
	return 4;
    if (!strcmp(argv, "kill"))
	return 5;
    if (!strcmp(argv, "stop"))
	return 6;
    return 0;
}
