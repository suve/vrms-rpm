**vrms-rpm**
----------
A clone of *vrms* ("*virtual Richard M. Stallman*") for
RPM-based GNU/Linux distributions.

This program analyses the list of currently installed RPM packages and reports
found non-free packages to stdout. 


**Why a clone instead of a fork?**
----------
Because my only experience with Perl (which the original *vrms* is written in)
has been one semester of Operating Systems class, and that was a long time ago.

Also, seeing how much the resulting program differs from the original, it's
quite possible that making a fork instead of starting fresh would only
complicate matters. After all, removing code is hard.


**Installing from a repository**
----------
Fedora users can use the *copr* repository 
[suve/vrms-rpm](https://copr.fedorainfracloud.org/coprs/suve/vrms-rpm/).
```
$ dnf copr enable suve/vrms-rpm
$ dnf install vrms-rpm
```


**Building it on your own: dependencies**
----------
- bash, since 'tis a shell script

- grep, for all that nifty pattern matching

- gettext, for handling multiple languages

- make, for simplifying the build & install process


**Building**
----------
Right now you're probably wondering why the hell do you need to build a 
bash script. Well, basically we need to process the gettext translation
files and set up data paths.

At this point you should've already decided where do you want to install
the program. As stated before, some paths are set in the code during build,
so if these don't match during build and install, the whole thing will
probably fail spectacularly in runtime.

To build, use `make` with the `build` target. The `PREFIX` variable can be
used for controlling the install-dir. The default is `/usr/local`.
```
$ make build [PREFIX=/usr/local]
```


**Installing**
----------
To install, perform your usual `make install`.
```
$ [sudo] make install [PREFIX=/usr/local]
```
Remember that to install stuff inside `/usr` you will need root capabilities.


**Getting rid of it**
----------
Should you decide to remove the program, the Makefile has got you covered.
```
$ [sudo] make remove [PREFIX=/usr/local]
```
Once again, remember that messing inside `/usr` requires extra privileges.


**Licensing**
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
