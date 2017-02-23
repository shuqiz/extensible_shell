/*
 * the biggest common divisor
 * invocation: bcd < positive number > < positive number >
 * calculates the biggest common disvor of 2 numbers.
 * decimal numbers automatically round up to integer
 * Authors: zshuai8 + ptian94
 */
 
#include <stdbool.h>
#include <stdio.h>
#include "../esh.h"
#include "../esh-sys-utils.h"
static int calculate(int, int);

static bool 
init_plugin(struct esh_shell *shell)
{
    printf("Plugin 'biggest common disvor' initialized...\n");
    return true;
}

/* Implement the calculations 
 * Returns true if handled correctly, false otherwise. */
static bool
bcd(struct esh_command *cmd)
{
    if (strcmp(cmd->argv[0], "bcd"))
        return false;

    char *argument = cmd->argv[1];
    // if no argument is given doesn't work
    if (argument == NULL || cmd->argv[2] == NULL) {
		printf("You have to provide enough inputs.\n");
		return true;		
    }
    else if (cmd->argv[3] != NULL) {
		
		printf("You have too many inputs.\n");
		return true;
	} 
    else if (atoi(cmd->argv[1]) > 0 
			&& atoi(cmd->argv[2]) >= 0) {
				
        int returned = calculate(atoi(cmd->argv[1]), atoi(cmd->argv[2]));
        printf("The biggest common divisor of two input numbers is %d\n", returned);
		return true;
    }
    else {
		printf("You have to put valid inputs inside\n");
		return true;
    }

	return true;
}

static int calculate(int number1, int number2) {
	
	int i = number1;
	int j = number2;
	while (i != j) {
		if (i > j) {
			i = i - j;
		}
		else {
			j = j - i;
		}
	}
	return i;
}

struct esh_plugin esh_module = {
  .rank = 1,
  .init = init_plugin,
  .process_builtin = bcd
};
