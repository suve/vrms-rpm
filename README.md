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


**Dependencies**
----------
- bash, obviously

- grep, for all that nifty pattern matching

- gettext, for handling multiple languages


**Building**
----------
Right now You're probably wondering why the hell do you need to build a 
bash script. Well, basically we need to process the gettext translation
files and set up data paths.

At this point you should've already decided whether you'll be performing
a local install (into `/usr/local`) or a global one (into `/usr`).
Use the `--local` or `--global` options to specify your choice.

To build, simply use `build.sh` with the `build` argument. 
```
$ ./build.sh build (--global|--local)
```


**Installing**
----------
To install, simply use `build.sh` with the `install` argument.
```
$ sudo ./build.sh install (--global|--local) [--prefix ARG]
```
Should you want to install to a chroot (for example, while packaging),
you can use the `--prefix ARG` option to specify a prefix to go before `/usr`.


Fedora users can use the *copr* repository 
[suve/vrms-rpm](https://copr.fedorainfracloud.org/coprs/suve/vrms-rpm/).
```
$ dnf copr enable suve/vrms-rpm
$ dnf install vrms-rpm
```

**Getting rid of it**
----------
Should you decide to uninstall the program `build.sh` has got you covered.
```
$ sudo ./build.sh remove (--global|--local) [--prefix ARG]
```
The `--local` and `--prefix` options can be used here, too.


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
