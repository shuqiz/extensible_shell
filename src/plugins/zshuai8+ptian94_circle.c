/*
 * the biggest common divisor
 * invocation: circle [positive number]
 * calculates the area and circumsference of a circle with a given radius
 * Authors: zshuai8 + ptian94
 */
 
#include <stdbool.h>
#include <stdio.h>
#include "../esh.h"
#include "../esh-sys-utils.h"
#define PI 3.1416

static bool 
init_plugin(struct esh_shell *shell)
{
    printf("Plugin 'circle' initialized...\n");
    return true;
}

/* Implement the calculations 
 * Returns true if handled correctly, false otherwise. */
static bool
circle (struct esh_command *cmd)
{
    if (strcmp(cmd->argv[0], "circle"))
        return false;

    int r;
    char *argument = cmd->argv[1];
    // have to provide the correct input
    if (argument == NULL) {
		printf("You have to provide valid raduis as an argument.\n");
		return true;		
    }
    else if (cmd->argv[2] != NULL) {
		printf("You have too many inputs.\n");
		return true;
	}
    else if (atoi(cmd->argv[1]) >= 0) {
        r = atoi(cmd->argv[1]);
        printf("The circle area = %.2f , The circle circumference = %.2f \n", (r * PI * r) , (2 * PI * r));
		return true;
    }
    else {
		printf("Invalid raduis, please provide with a valid input");
		return true;
    }

	return true;
}

struct esh_plugin esh_module = {
  .rank = 1,
  .init = init_plugin,
  .process_builtin = circle
};
