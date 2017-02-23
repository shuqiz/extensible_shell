/*
 * the time converter
 * invocation: tc < positive number > < positive number >
 * convert given hours and given mins to seconds
 * Authors: zshuai8 + ptian94
 */
 
#include <stdbool.h>
#include <stdio.h>
#include "../esh.h"
#include "../esh-sys-utils.h"

static bool 
init_plugin(struct esh_shell *shell)
{
    printf("Plugin 'time converter' initialized...\n");
    return true;
}

/* Implement the calculations 
 * Returns true if handled correctly, false otherwise. */
static bool
tc(struct esh_command *cmd)
{
    if (strcmp(cmd->argv[0], "tc"))
        return false;

    char *argument = cmd->argv[1];
    // if no argument is given doesn't work
    if (argument == NULL || cmd->argv[2] == NULL || cmd->argv[3] == NULL) {
		printf("You have to provide enough inputs.\n");
		return true;		
    }
    else if (cmd->argv[4] != NULL) {
		
		printf("You have too many inputs.\n");
		return true;
	} 
    else if (atoi(cmd->argv[1]) >= 0 
			&& atoi(cmd->argv[2]) >= 0
			&& atoi(cmd->argv[3]) >= 0) {
				
		int result = (int) (atoi(cmd->argv[1]) * 3600 + atoi(cmd->argv[2]) * 60 + atoi(cmd->argv[3]));
        printf("After conversion, the time is %d seconds.\n", result);
		return true;
    }
    else {
		printf("You have to put valid inputs inside.\n");
		return true;
    }

	return true;
}

struct esh_plugin esh_module = {
  .rank = 1,
  .init = init_plugin,
  .process_builtin = tc
};
