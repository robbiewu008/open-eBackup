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
<#########################################################
agent common powershell function
#######################################################x#>

$CurrentDir = Split-Path $MyInvocation.MyCommand.Definition
$CurrentPath = Split-Path -Parent $MyInvocation.MyCommand.Definition
$AgentRoot = Split-Path -Parent $CurrentPath
$AgentLogPath = join-path -path $AgentRoot -childpath log
$LogFilePath = join-path -path $AgentLogPath -childpath agentcom.log

# define function name, compatible with batch calls
$funcName = $args[0]

function Log($InputLog) {
    "[$(Get-Date)] [$env:username] $InputLog" | Out-File $LogFilePath -Encoding UTF8 -Append
    . "$CurrentDir\agent_func.ps1" $LogFilePath
}

function ModifyPfile {
    param (
        $pfileName,
        $ctlfileName,
        $recoverPath
    )
    
    if ([String]::IsNullOrEmpty($recoverPath)) {
        # if recoverPath is null, create control file directory
        foreach ($item in Get-Content. $ctlfileName) {
            if ([String]::IsNullOrEmpty($item)) {
                continue
            }

            $fileDir = Split-Path $item
            mkdir $fileDir
        }
    } else {
        # if recoverPath isn't null, replace pfile control file list
        $content = get-content $pfileName.pspath
        clear-content $pfileName.pspath
        foreach ($line in $content) {
            if ($line -match "control_files=") {
                $newCtlList = "*.control_files='$recoverPath\control01.ctl'"
                Add-content $pfileName.pspath -Value $newCtlList
            } else {
                Add-content $pfileName.pspath -Value $line
            }
        }
    }
}

function SplitVar {
    param (
        $ArgInput,
        $SplitSym,
        $OutFile
    )
    $ArgInput.Split($SplitSym, [StringSplitOptions]::RemoveEmptyEntries) | Out-File $OutFile -Encoding ASCII
}

# this is for compatible with batch calls
if ([String]::IsNullOrEmpty($funcName)) {
    return 0
}

if ($funcName -eq "ModifyPfile") {
    $ret = ModifyPfile $args[1] $args[2] $args[3]
    exit $ret
} elseif ($funcName -eq "SplitVar") {
    $ret = SplitVar $args[1] $args[2] $args[3]
    exit $ret
} elseif ($funcName -eq "GetVolByPath") {
    $volPath=$args[1]
    $OutFile=$args[2]
    if ($volPath[-1] -ne "\") {
        $volPath = $volPath + "\"
    }
    $volPath = $volPath.Replace("\", "\\")
    Get-WmiObject -class Win32_Volume -filter "Name='$volPath'" | ft -hide DeviceID | Out-File $OutFile -Encoding ASCII
}

exit 0
