# `umask` (as an external command)

A small command-line program for running a program with a given umask.

# Why?

It nicely fills the gaps where the `umask` builtin of your shell cannot
reach, and where invoking a new shell or other interpreter would be
needlessly verbose and inconvenient - for example right after `sudo`.
