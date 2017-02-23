/*
 * smile face
 * invocation: smile [positive number]
 * prints out smile face with given number
 * Authors: zshuai8 + ptian94
 */
 
#include <stdbool.h>
#include <stdio.h>
#include "../esh.h"
#include "../esh-sys-utils.h"

static bool 
init_plugin(struct esh_shell *shell)
{
    printf("Plugin 'smile' initialized...\n");
    return true;
}

/* Implement the calculations 
 * Returns true if handled correctly, false otherwise. */
static bool
smile (struct esh_command *cmd)
{
    if (strcmp(cmd->argv[0], "smile"))
        return false;

    char *argument = cmd->argv[1];
    // have to provide the correct input
    if (argument == NULL) {
		printf(":)\n");
		return true;		
    }
    else if (cmd->argv[2] != NULL) {
		printf("You have too many inputs.\n");
		return true;
	}
    else if (atoi(cmd->argv[1]) >= 1) {
        int r = atoi(cmd->argv[1]);
        while (r > 1) {
			printf(":)");
			r--;
		}
		printf(":)\n");
		return true;
    }
    else {
		printf("Invalid number, please provide a number bigger than 1.\n");
		return true;
    }

	return true;
}

struct esh_plugin esh_module = {
  .rank = 1,
  .init = init_plugin,
  .process_builtin = smile
};
