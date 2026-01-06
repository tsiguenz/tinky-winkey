# TODO

1. Create Vagrantfile to deploy Windows 11 VM
    - `gusztavvargadr/windows-11`
    - fix the file

2. Create Makefile who compile with CL (flags `/Wall /WX`):
    Need to make it better

3. `svc` who creates a service `tinky`:
    - If `tinky` is killed then `winkey` is killed
    - Check stop before delete
    - check behaviour when tinky.exe is killed via task manager

4. `winkey` (keylogger):
    - In a log file:
        - Timestamp
        - Foreground process information
        - Keystroke in human readable format accordingly to the current locale identifier

5. Refactor entire code
