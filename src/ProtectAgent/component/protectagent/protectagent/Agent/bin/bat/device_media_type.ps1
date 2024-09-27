# This file is a part of the open-eBackup project.
# This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
# If a copy of the MPL was not distributed with this file, You can obtain one at
# http://mozilla.org/MPL/2.0/.
#
# Copyright (c) [2024] Huawei Technologies Co.,Ltd.
#
# THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
# EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
# MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
<#

 *******************************************************
 get device(disks) media type from wmi
 *******************************************************

#>

$gResultFile = $args[0]

$ERR_SCRIPT_EXEC_FAILED = 5

$NeedLog = 1

$CurrentDir = Split-Path -Parent $MyInvocation.MyCommand.Definition
$AgentRoot = Split-Path -Parent $CurrentDir
$AgentLogPath = join-path -path $AgentRoot -childpath log
$LogFilePath = join-path -path $AgentLogPath -childpath query_device_media_type.log

function Log($InputLog)
{
    if (1 -eq $NeedLog)
    {
        "[$(Get-Date)] [$env:username] $InputLog" | Out-File $LogFilePath -Encoding UTF8 -Append
    }

    . "$CurrentDir\agent_func.ps1" $LogFilePath
}

function GetDeviceMediaType($ResultFile)
{
    Log "begin to get device media type."

	$MediaResult = Get-PhysicalDisk | select-object *
    foreach ($Result in $MediaResult)
    {
        $Result | Out-File $ResultFile -Encoding ASCII -Append
    }
    Log "Get device media type succ."
}

try
{
    Log "######################################"
 	GetDeviceMediaType $gResultFile
}
catch
{
    $lineNumber = $Error[0].InvocationInfo.scriptlinenumber
    $ErrorInfos = $Error[0]
    Log "ERROR LINE NUMBER $lineNumber, DESCRIPTION $ErrorInfos"
    exit($ERR_SCRIPT_EXEC_FAILED)
}

Log "exit script disk_media_type.ps1 with code 0"
exit(0)
