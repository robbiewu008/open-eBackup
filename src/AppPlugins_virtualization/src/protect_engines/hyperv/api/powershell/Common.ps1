<#PSScriptInfo
. This file is a part of the open-eBackup project.
. This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
. If a copy of the MPL was not distributed with this file, You can obtain one at
. http://mozilla.org/MPL/2.0/.
.
. Copyright (c) [2024] Huawei Technologies Co.,Ltd.
. THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
. EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
. MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
#>

$ENABLE_LOG = $True
$LOGLEVEL = $LOGLEVEL_ERROR
$LOGLEVEL_DEBUG = 'DEBUG'
$LOGLEVEL_INFO = 'INFO'
$LOGLEVEL_WARN = 'WARN'
$LOGLEVEL_ERROR = 'ERROR'

$SUCCESS = 0
$FAILED = -1
$INVALID_PARAM = 1
$INVALID_SERVICE = 200

$TYPE_UNKNOWN = 0
$TYPE_SCVMM = 1
$TYPE_FO_CLUSTER = 2
$TYPE_HOST = 3
 
$PAGE_NUM_MIN = 0
$PAGE_SIZE_MIN = 1
$PAGE_SIZE_MAX = 200

$CACHE_NAME_CLUSTER_LIST = "clusterlist"
$CACHE_NAME_HOST_LIST = "hostlist"
$CACHE_NAME_VM_LIST = "vmlist"
$CACHE_NAME_DISK_LIST = "disklist"
$CACHE_NAME_IP_LIST = "iplist"
$CACHE_NAME_VM_IP_LIST = "vmIpList"

$LOGFILE = "C:\\DataBackup\\ProtectClient\\ProtectClient-E\\log\\Plugins\\VirtualizationPlugin\\powershell.log"
$PARAM_PATH = "C:\\DataBackup\\ProtectClient\\Plugins\\VirtualizationPlugin\\tmp\\"
$RESULT_PATH = "C:\\DataBackup\\ProtectClient\\Plugins\\VirtualizationPlugin\\stmp\\"

## log functions ##
function DBGLOG($msg) {
    if ($LOGLEVEL -le $LOGLEVEL_DEBUG) {
        LOGGER "[DEBUG] $msg"
    }
}

function INFOLOG($msg) {
    if ($LOGLEVEL -le $LOGLEVEL_INFO) {
        LOGGER "[INFO] $msg"
    }
}

function WARNLOG($msg) {
    if ($LOGLEVEL -le $LOGLEVEL_WARN) {
        LOGGER "[WARN] $msg"
    }
}

function ERRLOG($msg) {
    if ($LOGLEVEL -le $LOGLEVEL_ERROR) {
        LOGGER "[ERROR] $msg"
    }
}

function LOGGER() {
    Param ([string]$msg)
    if ($ENABLE_LOG) {
        $timestamp = (Get-Date).toString("yyyy-MM-dd HH:mm:ss")
        $log_message = "$timestamp $msg"
        Add-content $LOGFILE -value $log_message
        Write-Host $log_message
    }
}

## session functions ##
function IsUserNamePasswordValid() {
    $SenInfo = Read-Host
    $SenJson = $SenInfo | ConvertFrom-Json
    $UserName = $SenJson.UserName
    $UserPwd = $SenJson.Password

    $Password = ConvertTo-SecureString -String $UserPwd -AsPlainText -Force
    $Credentials = New-Object -TypeName System.Management.Automation.PSCredential -ArgumentList $UserName, $Password
    $CredCheck = $Credentials | test_credentials
    Clear-Variable SenJson
    Clear-Variable UserPwd
    return $CredCheck
}

function test_credentials {
    [CmdletBinding()]
    [OutputType([bool])]

    param (
        [Parameter(
            Mandatory = $false,
            ValueFromPipeLine = $true,
            ValueFromPipelineByPropertyName = $true
        )]
        [Alias(
            'PSCredential'
        )]
        [ValidateNotNull()]
        [System.Management.Automation.PSCredential]
        [System.Management.Automation.Credential()]
        $Credentials
    )
    $Domain = $null
    $Root = $null
    $Username = $null
    $Password = $null

    # Checking module
    # Split username and password
    $Username = $credentials.username
    $Password = $credentials.GetNetworkCredential().password
    # Get Domain
    $Root = "LDAP://" + ([ADSI]'').distinguishedName
    $Domain = New-Object System.DirectoryServices.DirectoryEntry($Root, $UserName, $Password)
    Clear-Variable Password
    if (!$Domain) {
        WARNLOG("Something went wrong when credential validation.")
        return $False
    } else {
        if ($Domain.Name -ne $null) {
            DBGLOG("Credential validation success.")
            return $True
        } else {
            ERRLOG("Credential validation failed.")
            return $False
        }
    }
}

function NeedSession() {
    param ([string]$TargetHost, [string]$TargetType)
    # otherwise, if it's local command, no need to create session.
    if (($TargetHost -eq "localhost" -or $TargetHost -eq "127.0.0.1" -or $TargetHost -eq "::1" -or $TargetHost -eq '') -and ($TargetType -eq $TYPE_HOST)) {
        return $False
    }
    return $True
}

function new_pssession {
    param (
        [string]$PSSessionName,
        [string]$TargetHost = 'localhost',
        [string]$TargetType = $TYPE_UNKNOWN
    )

    $needSession = NeedSession $TargetHost $TargetType
    if (!$needSession) {
        DBGLOG("No need to create session.")
        return $SUCCESS
    }

    if (($TargetType -eq $TYPE_SCVMM) -or ($TargetType -eq $TYPE_FO_CLUSTER)) {
        $TargetHost = "localhost"
    }
    $SenInfo = Read-Host
    $SenJson = $SenInfo | ConvertFrom-Json
    $UserName = $SenJson.UserName
    $UserPwd = $SenJson.Password

    if ($TargetHost -eq 'localhost' -or $TargetHost -eq '') {
        $CurrentHostName = hostname
    } else {
        $CurrentHostName = $targetHost
    }
    INFOLOG("TargetHost: $TargetHost, CurrentHostName: $CurrentHostName")
    try {
        $Password = ConvertTo-SecureString -String $UserPwd -AsPlainText -Force
        $Credentials = New-Object -TypeName System.Management.Automation.PSCredential -ArgumentList $UserName, $Password
        $Session = New-PSSession -Name $PSSessionName -ComputerName $CurrentHostName -Credential $Credentials
        if ($TargetType -eq $TYPE_HOST) {
            Import-Module -Name Hyper-V -PSSession $Session
        }
        # clear sensitive date #
        Clear-Variable SenInfo
        Clear-Variable SenJson
        Clear-Variable UserPwd
    } catch {
        ERRLOG("Create new session exception occures.")
        close_pssession $PSSessionName
        # clear sensitive date #
        Clear-Variable SenInfo
        Clear-Variable SenJson
        Clear-Variable UserPwd
        return $FAILED
    }
    INFOLOG("Checking ps session available.")
    $checkSession = Get-PSSession | Where-Object { $_.Name -eq $PSSessionName }
    if ($checkSession.Availability -ne "Available") {
        close_pssession $PSSessionName
        return $FAILED
    }
    return $SUCCESS
}

function close_pssession {
    param($sessionName)
    Remove-PSSession -Name $sessionName -Confirm:$false -ErrorAction SilentlyContinue
}

## param functions ##
function read_parm_file() {
    param ([string]$uuid)
    $param_file = $PARAM_PATH + "param" + $uuid
    INFOLOG("Read parameter file: $param_file")
    $params = (Get-Content $param_file) | ConvertFrom-Json
    return $params
}

## write result functions ##
function write_result_file() {
    param ([string]$uuid, [string]$result)
    $dir_exists = (Test-Path $RESULT_PATH)
    if ($dir_exists -ne "True") {
        return $FAILED
    }
    $result_file = $RESULT_PATH + "result" + $uuid
    INFOLOG("Write result to file: $result_file")
    $file_exists = (Test-Path $result_file)
    if ($file_exists -eq "True") {
        Clear-Content $result_file
    }
    Add-content $result_file -value $result -Encoding utf8
    DBGLOG("Result: $result")
    return $SUCCESS
}

function format_result() {
    param ($str)
    $arrayResult = ConvertTo-Json -depth 1 -Compress @($str)
    $result = '{"result":' + $arrayResult + '}'
    return $result
}

function paging_result() {
    param ($array)
    [int]$page = $global:pageNo
    [int]$size = $global:pageSize
    DBGLOG("Page: $page, Size: $size")

    if ($size -eq 0) {
        $size = $PAGE_SIZE_MAX
    }

    $result = @()
    if ($page -lt $PAGE_NUM_MIN -or $size -lt $PAGE_SIZE_MIN -or $size -gt $PAGE_SIZE_MAX) {
        ERRLOG("page info is invalid. page([0,-]): $page, size([1, 200]): $size")
        return $result
    }
    [int]$start = $page * $size
    [int]$scope = ($page + 1) * $size
    for ($i = $start; $i -lt $scope -and $i -lt $array.Count; $i++) {
        $result += $array[$i]
        if ($i + 1 -ge $array.Count) {
            # clear cache when last item return
            clear_cache
        }
    }
    return $result
}

function write_result() {
    param ([string]$uuid, $result)
    $result = paging_result @($result)
    $result = format_result $result
    $res = write_result_file $uuid $result
    return $res
}

## cache functions ##
function target_cached() {
    param ([string]$target)
    $cacheFile = $RESULT_PATH + $target + "_" + $global:requestId + ".cache"
    $exists = (Test-Path $cacheFile)
    return $exists
}

function write_cache {
    param ([string]$target, $result)
    $cacheFile = $RESULT_PATH + $target + "_" + $global:requestId + ".cache"
    $exists = (Test-Path $cacheFile)
    if ($exists) {
        Clear-Content $cacheFile
    }
    $result = $result | ConvertTo-Json -depth 1
    Add-content $cacheFile -value $result -Encoding utf8
    return $True
}

function read_cache() {
    param ([string]$target)
    INFOLOG("Read from cache.")
    $cacheFile = $RESULT_PATH + $target + "_" + $global:requestId + ".cache"
    $result = (Get-Content $cacheFile -Raw) | ConvertFrom-Json
    return $result
}

function clear_cache() {
    param ([string]$target)
    $cacheFile = $RESULT_PATH + "*" + $global:requestId + ".cache"
    DBGLOG("Clear cache: $cacheFile")
    if (Test-Path $cacheFile) {
        Remove-Item -Path $cacheFile -Force
    }
}