/*
 * U mad face
 * invocation: umd
 * display a u mad face
 * Authors: zshuai8 + ptian94
 */
 
#include <stdbool.h>
#include <stdio.h>
#include "../esh.h"
#include "../esh-sys-utils.h"
static bool 
init_plugin(struct esh_shell *shell)
{
    printf("Plugin 'umad' initialized...\n");
    return true;
}

/* Implement the calculations 
 * Returns true if handled correctly, false otherwise. */
static bool
umad (struct esh_command *cmd)
{
    if (strcmp(cmd->argv[0], "umad"))
        return false;

    char *argument = cmd->argv[1];
    // have to provide the correct input
    if (argument == NULL) {
		printf("You are mad bro.\n");
		return true;		
    }
    else if (cmd->argv[2] != NULL) {
		printf("You are way too mad bro.\n");
		return true;
	}
    else {
		printf("not mad enough.\n");
		return true;
    }

	return true;
}

struct esh_plugin esh_module = {
  .rank = 1,
  .init = init_plugin,
  .process_builtin = umad
};
