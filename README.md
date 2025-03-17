## PsExec
A CLI tool used to launch applications under different users and different integrity levels. Additionally, it logs token information about the created processes. Inspired by the Sysinternals tool by the same name.
```
C:\Users\Dima\Desktop\PsExec\x64\Release>PsExec.exe
Usage:
        PsExec.exe <application>
        PsExec.exe -u <username> -p <password> <application>
        PsExec.exe -tp <target_pid> <application>
        PsExec.exe -tn <target_name> <application>
```

## Capabilities
### CreateProcess (Standard or Elevated)
- Create a standard user or elevated process using the `CreateProcess` API by starting a Command Prompt as a stardard user or as an administator. Any process spawned using this method will inherit the token of the parent. For example, if the Command Prompt was launched as a standard user then the child process will be spawned as a standard user as well and vise versa.
```
C:\Users\Dima\Desktop\PsExec\x64\Release>PsExec.exe notepad.exe
================================================================
PsExec Start:
    Command Line: notepad.exe
================================================================
================================================================
PsExec Summary:
        Process ID: 0x0000164C
        User: WIN10VM\Dima
        Elevated: Yes
        Elevation Type: Full (Elevated)
        Integrity Level: High
        Image Path: C:\Windows\System32\notepad.exe
        Return Code: 0x00000000
================================================================

```
### CreateProcessWithLogon (Standard)
- Create a process as any standard user using the `CreateProcessWithLogonW` API by starting a Command Prompt as standard or as an administrator. Any process spawned using this method will inherit the token of the user provided credentials.
```
C:\Users\Dima\Desktop\PsExec\x64\Release>PsExec.exe -u Dima -p 1234 notepad.exe
================================================================
PsExec Start:
    Command Line: notepad.exe
================================================================
================================================================
PsExec Summary:
        Process ID: 0x0000338C
        User: WIN10VM\Dima
        Elevated: No
        Elevation Type: Limited (Limited User)
        Integrity Level: Medium
        Image Path: C:\Windows\System32\notepad.exe
        Return Code: 0x00000000
================================================================

```
### CreateProcessWithToken (Standard or Elevated)
- Create a process as a standard user or elevated using the `CreateProcessWithTokenW` API by starting a Command Prompt as an administrator. This method opens a handle to a target process whose token is to be duplicated before calling `CreateProcessWithTokenW`. Therefore, starting processes with higher integrity levels(High or System) requires that `PsExec.exe` be run as Administrator. This method also can start processes under different users as long as a handle can be opened to the processes under that particular user.
```
C:\Users\Dima\Desktop\PsExec\x64\Release>PsExec.exe -tn lsass.exe notepad.exe
================================================================
PsExec Start:
    Command Line: notepad.exe
================================================================
================================================================
PsExec Summary:
        Process ID: 0x00003164
        User: NT AUTHORITY\SYSTEM
        Elevated: Yes
        Elevation Type: Default (Standard User)
        Integrity Level: System
        Image Path: C:\Windows\System32\notepad.exe
        Return Code: 0x00000000
================================================================

```

## Project Goal
My goal for this project was to understand Windows process creation, process token information, strip C/C++ runtime, implement own runtime functionalities, and use the Windows Native API.

## Help
This project could not have been built without the excellent tutorials provided by Pavel Yosifovich.
- [YouTube](https://www.youtube.com/@zodiacon)
- [Blog](https://scorpiosoftware.net/)
- [Windows 10 System Programming](https://leanpub.com/b/windows10systemprogrammingP1and2)
- [Windows Native API Programming](https://leanpub.com/windowsnativeapiprogramming)
