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
Log Oper
########################################################>

$CURRENTPATH = Split-Path -Parent $MyInvocation.MyCommand.Definition
$AGENTROOT = Split-Path -Parent $CURRENTPATH
$AGENTCONFPATH = join-path -path $AGENTROOT -childpath conf
$MAXLOGSIZE  = 52428800
$LOGFILE_SUFFIX = "zip"
$LOGFILENAME = $args[0]
$TEMPLOGNAME = Split-Path -Leaf $args[0]
$BACKLOGCOUNT = 5
$BACKLOGNAME = $LOGFILENAME, $BACKLOGCOUNT -join "."
$CONFFILE = join-path -path $AGENTCONFPATH -childpath agent_cfg.xml
$EXEC7ZIP = join-path -path $CURRENTPATH -childpath 7z.exe

function BackLog($NUMBER, $BACKLOGNAME)
{
    if (0 -le $NUMBER)
    {
        if (0 -eq $NUMBER)
        {
            $BACKLOGNAME = $LOGFILENAME, $LOGFILE_SUFFIX -join "."
            & $EXEC7ZIP a -y -tzip $BACKLOGNAME $LOGFILENAME -mx=9 | Out-Null
            remove-item $LOGFILENAME
        }
        else
        {
            $BACKLOGNAME = $LOGFILENAME, $NUMBER, $LOGFILE_SUFFIX -join "."
        }
        
        if (Test-Path $BACKLOGNAME)
        {
            $NUMBER_TEMP = $NUMBER + 1
            $DESTLOGNAME = $TEMPLOGNAME, $NUMBER_TEMP, $LOGFILE_SUFFIX -join "."
            Rename-Item $BACKLOGNAME $DESTLOGNAME
        }
        
        $NUMBER = $NUMBER - 1
        
        BackLog $NUMBER $BACKLOGNAME
    }
}

#read logfile size
$filesize = (get-item $LOGFILENAME).length

$content = select-string -Path $CONFFILE -pattern "log_count"
if ("" -ne $content)
{
    $BACKLOGCOUNT = [int](("$content").Split("`""))[1]
}

$BACKLOGNAME = $LOGFILENAME, $BACKLOGCOUNT, $LOGFILE_SUFFIX -join "."

if ($filesize -ge $MAXLOGSIZE)
{
    if (Test-Path $BACKLOGNAME)
    {
        remove-item $BACKLOGNAME
    }
    
    $NUMBER = $BACKLOGCOUNT - 1
    
    BackLog $NUMBER $BACKLOGNAME
    
    New-Item "$LOGFILENAME" -type file -force
    
    #set file access
    icacls "$LOGFILENAME" /deny Users:F
}