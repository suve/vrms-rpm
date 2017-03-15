**vrms-rpm**
----------
A clone of *vrms* ("*virtual Richard M. Stallman*") for
RPM-based GNU/Linux distributions.

This program analyses the list of currently installed RPM packages and reports
found nonfree packages to stdout. 


**Why a clone instead of a fork?**
----------
Because my only experience with perl (which *vrms* is written in) was
one semester of Operating Systems class, and that was a long time ago.

Also, seeing how much the resulting program differs from the original, it's
quite possible that making a fork instead of starting fresh would only
complicate matters. After all, removing code is hard.


**Installing**
----------
No installation needed. Although if you insist on having the program in 
your PATH, you can just do the following:
```
sudo cp ./vrms-rpm.sh  /usr/bin/vrms-rpm
sudo cp ./vrms-rpm.man /usr/share/man/man1/vrms-rpm.1
```


**Licencing**
----------
This program is made available under the terms of the GNU
General Public License, version 3, as published by the
Free Software Foundation.


**Contributing**
----------
 1. Make a new branch for your changes.
 
 2. When modifying program options, remember to update the --help section
    and the manpage.
 
 3. Use British spelling, when appropriate.
 
 4. By contributing, you agree for your work to be included under
    the terms of the program licence.
