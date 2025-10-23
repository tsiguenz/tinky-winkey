# TODO

- svc = base program (binary)
- tinky = service
- winkey = keylogger (binary)

1. Create Vagrantfile to deploy Windows 11 VM
    - `gusztavvargadr/windows-11`

2. Create Makefile who compile with CL (flags `/Wall /WX`):
    - `svc`
    - `winkey`

3. `svc` who creates a service `tinky`:
    1. Manage `tinky` with:
        - install
        - start
        - stop
        - delete
    2. Mandatory functions:
        - OpenSCManager
        - CreateService
        - OpenService
        - StartService
        - ControlService
        - CloseServiceHandle
        - DuplicateTokenEx
    3. Must impersonate Windows service with [system token](https://learn.microsoft.com/en-us/windows/win32/secauthz/access-tokens)
    4. If `tinky` is killed then `winkey` is killed

4. `winkey` (keylogger):
    - Started by `tinky` in the background
    - Only one instance can run
    - Must detect foreground processes
    - Must save every keystroke (mandatory) of the current foreground process
    - Low level hook
    - In a log file:
        - Timestamp
        - Foreground process information
        - Keystroke in human readable format accordingly to the current locale identifier
