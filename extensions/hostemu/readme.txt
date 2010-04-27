The autotools do NOT copmile the cmdparse.y into cmdparse.cpp. That has to be done by hand.
The command for doing that is

	yacc -o cmdparse.cpp cmdparse.y

Be sure the current sub dir is extensions/hostemu when you run that command.

