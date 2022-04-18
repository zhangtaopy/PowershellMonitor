# PowershellMonitor
PowershellMonitor

# Introduction
monit powershell behaviour by hooking powershell script engine.

## Features
* detect obfuscated script
* log powershell behaviour use lua
* intercept behaviour by lua(by return non zero)

# Usage
* Execute powershellexec.exe, it will start a powershell process and inject powershellmonitor.dll into the process, then use this powershell session
execute poweshell command, the behavior(format to json struct) of the powershellscript will be pass to the luascript(luascript.lua) , then according to your lua script, you can log the behavior

## Behavior list
### Native api
 * VirtualAlloc
 * VirtualAllocEx
 * WriteProcessMemory
 * CreateRemoteThread
 * CreateThread
 
### Normal method
 * DownloadString
 * DownloadFile
 * DownloadData
 * UploadData
 * UploadString
 * UploadFile
 * Shell.Application.Open
 * WScript.Shell.Exec
 * WScript.ShellRegDelete
 * WScript.ShellRegWrite
 * CallByname
 * Assembly Load
 * OpenRead(url)
 
### Inner command
 * Invoke-Expression
 * Invoke-WebRequest
 
# Use lua to monit

```lua
--[[
powershell json define:
functionname
argcount
arg1
arg2
arg3
arg4
arg5
arg6
arg7
---]]
function powershellcheck(jsonstring)	
	local params = cjson.decode(jsonstring)
	
	local funcname = params.functionname
	
	local logpath = "e:\\code\\PowershellMonitor\\Release\\log.dat"
	
	local binarypath = "e:\\code\\PowershellMonitor\\Release\\binary.dat"
	
	local scriptpath = "e:\\code\\PowershellMonitor\\Release\\script.dat"	
	
	if funcname == 'downloadstring' or funcname == 'openread' then
		Log.write(logpath, string.format('[%s] url -> {%s}', funcname, params.arg1))
		return 0
	elseif funcname == 'invoke-expression' then
		Log.write(scriptpath, params.arg1)
		Log.write(logpath, string.format('[%s] scriptcontent -> {%s}', funcname, scriptpath))
		return 0
	elseif funcname == 'load' then
		Log.writebinary(binarypath, params.arg1)
		Log.write(logpath, string.format('[%s] binary -> {%s}', funcname, binarypath))
		return 0
	elseif funcname == 'virtualalloc' or funcname == 'virtualallocex' or funcname == 'virtualprotect' then
		Log.write(logpath, string.format('[%s] notify -> {native api called}', funcname))
		return 1
	end
	
	return 0
end
```

#  References
* [.NET Internals and Code Injection](https://ntcore.com/files/netint_injection.htm)
