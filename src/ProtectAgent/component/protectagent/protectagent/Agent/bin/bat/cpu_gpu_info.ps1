# 
#  This file is a part of the open-eBackup project.
#  This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
#  If a copy of the MPL was not distributed with this file, You can obtain one at
#  http://mozilla.org/MPL/2.0/.
# 
#  Copyright (c) [2024] Huawei Technologies Co.,Ltd.
# 
#  THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
#  EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
#  MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
# 
<#
 get cpu and gpu info from wmi
 *******************************************************
 cpu info include : Architecture, CurrentClockSpeed, Name, NumberOfCores, L3CacheSize
 *******************************************************
 gpu info include : AdapterRAM, Name
 *******************************************************
#>

$gCpuGpuType = $args[0]
$gResultFile = $args[1]

$ERR_SCRIPT_EXEC_FAILED = 5

$NeedLog = 1

$CurrentDir = Split-Path -Parent $MyInvocation.MyCommand.Definition
$AgentRoot = Split-Path -Parent $CurrentDir
$AgentLogPath = join-path -path $AgentRoot -childpath log
$LogFilePath = join-path -path $AgentLogPath -childpath query_cpu_gpu_info.log

function Log($InputLog)
{
    if (1 -eq $NeedLog)
    {
        "[$(Get-Date)] [$env:username] $InputLog" | Out-File $LogFilePath -Encoding UTF8 -Append
    }

    . "$CurrentDir\agent_func.ps1" $LogFilePath
}

function GetCpuGpuInfo($CpuGpuType, $ResultFile)
{
    Log "begin to get $CpuGpuType info."
    if ("cpu" -eq $CpuGpuType)
    {
		$ComputerCpu = Get-WmiObject -Class Win32_Processor | select-object *
	    foreach ($CpuItem in $ComputerCpu)
	    {
	        "Architecture : " + $CpuItem.Architecture | Out-File $ResultFile -Encoding ASCII -Append
	        #"Architecture : " + $CpuItem.Architecture | Out-File $LogFilePath -Encoding ASCII -Append

	        "CurrentClockSpeed : " + $CpuItem.CurrentClockSpeed | Out-File $ResultFile -Encoding ASCII -Append
	        #"CurrentClockSpeed : " + $CpuItem.CurrentClockSpeed | Out-File $LogFilePath -Encoding ASCII -Append

	        "Name : " + $CpuItem.Name | Out-File $ResultFile -Encoding ASCII -Append
	        #"Name : " + $CpuItem.Name | Out-File $LogFilePath -Encoding ASCII -Append

	        "NumberOfCores : " + $CpuItem.NumberOfCores | Out-File $ResultFile -Encoding ASCII -Append
	        #"NumberOfCores : " + $CpuItem.NumberOfCores | Out-File $LogFilePath -Encoding ASCII -Append

	        "L3CacheSize : " + $CpuItem.L3CacheSize | Out-File $ResultFile -Encoding ASCII -Append
	        #"L3CacheSize : " + $CpuItem.L3CacheSize | Out-File $LogFilePath -Encoding ASCII -Append

	        #"Container" is a separative sign for different cpus
	        "Container : " | Out-File $ResultFile -Encoding ASCII -Append
	        #"Container : " | Out-File $LogFilePath -Encoding ASCII -Append
	    }
	    Log "Get cpu info succ."
    }
    else
    {
		$ComputerGpu = Get-WmiObject -Class Win32_VideoController | select-object *
		foreach ($GpuItem in $ComputerGpu)
		{
			"AdapterRAM : " + $GpuItem.AdapterRAM | Out-File $ResultFile -Encoding ASCII -Append
			#"AdapterRAM : " + $GpuItem.AdapterRAM | Out-File $LogFilePath -Encoding ASCII -Append

	       	"Name : " + $GpuItem.Name | Out-File $ResultFile -Encoding ASCII -Append
	       	#"Name : " + $GpuItem.Name | Out-File $LogFilePath -Encoding ASCII -Append

	       	#"Container" is a separative sign for different gpus
	        "Container : " | Out-File $ResultFile -Encoding ASCII -Append
	        #"Container : " | Out-File $LogFilePath -Encoding ASCII -Append
		}
		Log "Get gpu info succ."
    }
}


try
{
    #get cpu or gpu info
    Log "######################################"
    Log "get $gCpuGpuType info."

 	GetCpuGpuInfo $gCpuGpuType $gResultFile
}
catch
{
    $lineNumber = $Error[0].InvocationInfo.scriptlinenumber
    $ErrorInfos = $Error[0]
    Log "ERROR LINE NUMBER $lineNumber, DESCRIPTION $ErrorInfos"
    exit($ERR_SCRIPT_EXEC_FAILED)
}

exit(0)
