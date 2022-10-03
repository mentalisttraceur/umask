# `umaskexec`

`umaskexec` fills the gaps where `umask` is awkward or cannot reach.

It executes a command with the given umask.
If no umask is given, it shows the current umask.
If no command is given, it shows what the new umask would be.
