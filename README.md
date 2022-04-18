# PowershellMonitor
PowershellMonitor


# use lua to monit

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

<code>
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
		return 0
	end
	
	return 0
end
</code>
