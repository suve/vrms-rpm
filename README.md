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


**How does it work?**
----------
*vrms-rpm* works by processing the list of installed packages (as reported by *rpm*)
and analysing their licence strings. The program breaks down compound expressions
using AND / OR operators and parentheses, and then checks individual licences against
a list of acceptable licences.

To make a simple example, the licence string `(AGPL-3.0-or-later OR SSPL) AND CC-BY-3.0`
will be broken down in the following way:
```
AGPL-3.0-or-later      SSPL      CC-BY-3.0
     [Free]         [Non-free]     [Free]
        |               |            |
        \----- OR ------/            |
             [Free]                  |
                |                    |
                \-------- AND -------/
                         [Free]
```


**Installing from a repository**
----------
[![Packaging status](https://repology.org/badge/vertical-allrepos/vrms-rpm.svg?exclude_unsupported=true&minversion=2.2)](https://repology.org/project/vrms-rpm/)    
Fedora users can install *vrms-rpm* from the official distro repository.
RHEL/CentOS users can get the package from the Fedora EPEL repositories.
```
$ dnf install vrms-rpm
```


**Building it on your own: dependencies**
----------
- `bash`, as there are some shell scripts used during the build process

- `sed` - used in the aforementioned shell scripts

- `make`, for simplifying the build & install process

- `gcc`, although clang should work too

- `gettext` (`libintl`), for handling multiple languages

- `librpm` (**optional**), for accessing RPM's functions for comparing package versions

- `cmocka` (**optional**), for running the test suite


**Picking the licence list**
----------
As mentioned earlier, *vrms-rpm* works by checking package licences against a list of known good licences.
Since distributions can have differing opinions on what constitutes as free software,
and what the licence tags should be written as (e.g. "GPLv2" vs "GPL-2.0"),
*vrms-rpm* comes packaged with several different licence lists.

These are:

- `fedora` - a list created in accordance to the [Fedora Allowed License list](https://docs.fedoraproject.org/en-US/legal/allowed-licenses/).

- `spdx-fsf-and-osi` - a list created in accordance to the [SPDX licence list](https://spdx.org/licenses/). Includes only licences marked as both "FSF free/libre" and "OSI approved".

- `spdx-fsf-or-osi` - a list created in accordance to the [SPDX licence list](https://spdx.org/licenses/). Includes licences marked as either "FSF free/libre" or "OSI approved".

- `spdx-only-fsf` - a list created in accordance to the [SPDX licence list](https://spdx.org/licenses/). Includes only licences marked as "FSF free/libre".

- `spdx-only-osi` - a list created in accordance to the [SPDX licence list](https://spdx.org/licenses/). Includes only licences marked as "OSI approved".

- `suse` - a list created in accordance to [openSUSE packaging guidelines](https://en.opensuse.org/openSUSE:Packaging_guidelines#Licensing).

- `tweaked` - a list that combines all of the above and also includes many non-standard licence tags spotted in the wild.
This list has the smallest chance of generating false-positives (free packages being marked as non-free)

When building the program, one of those lists has to be selected as default.
This can be done by providing the `DEFAULT_LICENCE_LIST` variable to *make*.


**Picking the grammar rules**
----------
Licensing can be a complicated subject, with many pieces of software utilising various
multiple-licensing schemes. These can be written down in various ways - using combining words
such as "and" / "or", but also listing individual licences separated by commas or semicolons.
This results in a myriad of possible ways to describe the licensing situation.

In recent years, the SPDX standard - offering a formalized way of building
licence expressions - has gained a lot of traction, with more and more distributions
adopting SPDX as the preferred way of specifying package licences.

To reflect this, *vrms-rpm* comes with multiple licence-string parsers.
Currently, the supported options are:

- `spdx-strict` - a parser implementing the [SPDX license expressions](https://spdx.github.io/spdx-spec/v2.3/SPDX-license-expressions/)
  specification.

- `spdx-lenient` - a variant of the SPDX parser, which relaxes some rules regarding case-sensitivity
  and whitespace.

- `loose` - an ad-hoc, informal parser that tries its best to make sense of licence strings
  that do not follow any formal grammar. This is the original parser used in *vrms-rpm* v2.2 and older.

When building the program, one of the parsers has to be selected as default.
This can be done by providing the `DEFAULT_GRAMMAR` variable to *make*.


**Building**
----------
To build, use `make` with the `build` target. The `PREFIX` variable can be
used for controlling program data paths (e.g. where to look for the good licence list).
The default is `/usr/local`.

By default, *vrms-rpm* links against `librpm` and `librpmio`, to make use of RPM's version comparison functions.
To disable this feature, you can set the `WITH_LIBRPM` variable to `0`. In this case, *vrms-rpm* will use a fallback,
simplified algorithm when comparing package versions.
```
$ make build [PREFIX=/usr/local] [DEFAULT_GRAMMAR=loose] [DEFAULT_LICENCE_LIST=tweaked] [WITH_LIBRPM=1]
```


**Testing**
----------
To test the program, use `make` with the `test` target. This will build the test suite and immediately execute it.
If you want to just build the test suite, use the `build/test-suite` target.
```
# Shorthand for: make build/test-suite && ./build/test-suite
$ make test
```
The test suite uses [*cmocka*](https://cmocka.org/), so make sure you have it installed.
If you build against `librpm`, you may notice that some tests are skipped;
these check *vrms-rpms's* fallback behaviour, so it makes no sense to run them when that's disabled.


**Installing**
----------
To install, perform your usual `make install`.
You can use the `DESTDIR` variable if you need to install the files
somewhere else than `PREFIX` dictates (e.g. for packaging).
```
$ [sudo] make install [PREFIX=/usr/local] [DESTDIR=]
```
Remember that to install stuff inside `/usr` you will need root capabilities.


**Getting rid of it**
----------
Should you decide to remove the program, the Makefile has got you covered.
```
$ [sudo] make remove [PREFIX=/usr/local] [DESTDIR=]
```
Once again, remember that messing inside `/usr` requires extra privileges.


**Licensing**
----------
This program is made available under the terms of the GNU
General Public License, version 3, as published by the
Free Software Foundation.

This program uses photos courtesy of other people;
see `IMAGE-CREDITS.txt` for details.


**Contributing**
----------
Whether you'd like to report an issue with the program or submit a patch,
you can do so on [GitHub](https://github.com/suve/vrms-rpm/) or via e-mail.

When sending patches, please keep in mind the following:

 1. When modifying program options, remember to update the `--help`
    section and the man page(s).
 
 2. Use British spelling, when appropriate.
 
 3. By contributing, you agree for your work to be included under
    the terms of the program licence.


**Donating**
----------
If you'd like to support the development of this program financially,
you can use the following methods:

- [GitHub Sponsors](https://github.com/sponsors/suve/)

- [Liberapay](https://liberapay.com/suve)

Much appreciated!
