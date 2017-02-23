/*
 * the biggest prime number
 * invocation: bpn [positive number]
 * calculates biggest prime number under the given number
 * Authors: zshuai8 + ptian94
 */
 
#include <stdbool.h>
#include <stdio.h>
#include "../esh.h"
#include "../esh-sys-utils.h"
static bool checkForPrime(int num);
static bool 
init_plugin(struct esh_shell *shell)
{
    printf("Plugin 'bpn' initialized...\n");
    return true;
}

/* Implement the calculations 
 * Returns true if handled correctly, false otherwise. */
static bool
bpn (struct esh_command *cmd)
{
    if (strcmp(cmd->argv[0], "bpn"))
        return false;

    int r;
    char *argument = cmd->argv[1];
    // have to provide the correct input
    if (argument == NULL) {
		printf("You have to provide a valid input as an argument.\n");
		return true;		
    }
    else if (cmd->argv[2] != NULL) {
		printf("You have too many inputs.\n");
		return true;
	}
    else if (atoi(cmd->argv[1]) >= 2) {
        r = atoi(cmd->argv[1]);
        while (r != 1) {
			
			if (checkForPrime(r)) {
				break;
			}
            r--;
		}
        printf("the biggest prime under the give number is %d\n", r);
		return true;
    }
    else {
		printf("Invalid number, please provide a number bigger than 1.");
		return true;
    }

	return true;
}

static bool checkForPrime(int num) {
	
	int i = 2;
	while (i < num) {
		
		if (num % i == 0) {
			return false;
		}
		i++;
	}
	return true;
}

struct esh_plugin esh_module = {
  .rank = 1,
  .init = init_plugin,
  .process_builtin = bpn
};
