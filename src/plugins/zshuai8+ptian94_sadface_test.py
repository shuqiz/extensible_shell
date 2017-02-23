#!/usr/bin/python
#
# Tests the functionality of gback's glob module
# Also serves as example of how to write plugin tests.
# You can copy the first part of this file.
#
import sys

# the script will be invoked with two arguments:
# argv[1]: the name of the hosting shell's eshoutput.py file
# argv[2]: a string " -p dirname" where dirname is passed to stdriver.py
#          this will be passed on to the shell verbatim.
esh_output_filename = sys.argv[1]
shell_arguments = sys.argv[2]

import imp, atexit
sys.path.append("/home/courses/cs3214/software/pexpect-dpty/");
import pexpect, shellio, signal, time, os, re, proc_check

#Ensure the shell process is terminated
def force_shell_termination(shell_process):
	c.close(force=True)

#pulling in the regular expression and other definitions
def_module = imp.load_source('', esh_output_filename)
logfile = None
if hasattr(def_module, 'logfile'):
    logfile = def_module.logfile

#spawn an instance of the shell
c = pexpect.spawn(def_module.shell + shell_arguments,  drainpty=True, logfile=logfile)

atexit.register(force_shell_termination, shell_process=c)

# set timeout for all following 'expect*' calls to 2 seconds
c.timeout = 2 

# ensure that shell prints expected prompt
assert c.expect(def_module.prompt) == 0, "Shell did not print expected prompt (1)" 

#################################################################
# 
# Boilerplate ends here, now write your specific test.
#
#################################################################
# Step 1. Create a temporary directory and put a few files in it
# 
#
import tempfile, shutil
tmpdir = tempfile.mkdtemp("esh")
testfiles = ['aa', 'ab', 'c']
for file in testfiles:
    open(tmpdir + "/" + file, "w")

# make sure it gets cleaned up
def cleanup():
    shutil.rmtree(tmpdir)

atexit.register(cleanup)

expectedoutput = " ".join(tmpdir + "/" + f for f in testfiles)


#############################################################################
# Our tests: 


# no arguments
c.sendline("sadface")
assert c.expect(":<\(") == 0, \
	"Error: expected output \"You have to put valid inputs inside.\" "
# invalid argument
c.sendline("sadface -3")
assert c.expect("Invalid number, please provide a number bigger than 1.") == 0, \
"Error: expected output \"You have to put valid inputs inside.\" "

# valid argument
c.sendline("sadface 2")
assert c.expect(":<\(:<\(") == 0, \
	"Error: expected output \"You have to put valid inputs inside.\" "

# invalid argument
c.sendline("sadface -1 2")
assert c.expect("You have too many inputs.") == 0, \
	"Error: expected output \"You have to put valid inputs inside.\" "

shellio.success()

