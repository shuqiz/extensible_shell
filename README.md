# extensible_shell
Student Information
-------------------
<Student 1 Information>
Shuaicheng Zhang (zshuai8)

<Student 2 Information>
Tianchen Peng (ptian94)

Invocation
./esh
This shell supports buildin commands with implementation of job controls, 
foreground, background, kill stop

------------------------
input buildin commands or plugin commands, press enter

Important Notes
This shell supports external plugins with
./esh -p [directory of plugins]
---------------

Description of Base Functionality
---------------------------------

jobs: show current running or stopped jobs with their jid and status

fg [jid]: bring the job of jid from background(running / stopped) to foreground and continue runnning

bd [jid]: bring a job of jid to background and continue it in the background

kill [jid]: kill a job of jid

stop [jid]: stop a job of jid

\ˆC: terminate the foreground job

\ˆZ: stop the foreground job

Description of Extended Functionality
-------------------------------------

List of Plugins Implemented

This shell supports I/O redirection, pipes, exclusive access >, syntax is exactly the same as the regular shell.
---------------------------
Plugin 1:
bcd(Biggest-common-divisor)

functionality:
invocation: bcd [positive number] [positive number]
returns the biggest common divisor of two numbers


Plugin 2:
circle

functionality:
invocation: circle [positive number]
returns the circumsference and area of given radius(input number)

Plugin 3:
bpn(Biggest-prime-number)

functionality:
invocation: bpn [positive number]
returns the biggest prime number under the input number

Plugin 4:
umad(saying u are mad)

functionality:
invocation: umad [anthing] [anything]
returns sentences saying u are mad

Plugin 5:
smile(smile face)

functionality:
invocation: smile [positive number]
returns smile faces with given number

Plugin 6:
time converter

functionality:
invocation: tc [positive number] [positive number] [positive number]
returns seconds after conversion of hour, min and second.

Plugin 7:
sad face(smile face)

functionality:
invocation: sadface [positive number]
returns sad faces with given number
