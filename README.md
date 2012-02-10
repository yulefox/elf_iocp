About
=====
`elf_iocp` is mainly a simple IOCP demo.

There're not many locks on recv/send packets. Thus, some known or unknown bugs
lie there.

Projects
========

public
------
 * PROJECT: static library
 * DEPENDENCY: log4c

`public` contains:
 * log
 * file parser(`*.ini` and `*.xml` now)

public/net
----------
 * PROJECT: static library
 * DEPENDENCY: public

sandbox
-------
 * PROJECT: static library

`sandbox` do NOTHING but an empty project with common project configuration.

