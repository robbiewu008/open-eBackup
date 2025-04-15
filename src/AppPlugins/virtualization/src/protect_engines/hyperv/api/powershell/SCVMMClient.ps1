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


. C:\DataBackup\ProtectClient\Plugins\VirtualizationPlugin\bin\Common.ps1

function check_sys_center_connection() {
    param ([string]$action, [string]$uuid)
    $jsonParm = read_parm_file $uuid
    $TargetHost = $jsonParm.TargetHost

    $valid = IsUserNamePasswordValid
    if (!$valid) {
        $errResult = '{"code": -1, "bodyErr": -1, "message": "Auth unavailable"}'
        $write_res = write_result_file $uuid $errResult
        ERRLOG("Valid cridential failed.")
        return $FAILED
    }

    $result = '{"code": 0, "bodyErr": 0, "message": ""}'
    $errResult = '{"code": -1, "bodyErr": 200, "message": "SCVMMService unavailable"}'
    $SCS = Get-Service -ComputerName $TargetHost -Name SCVMMService
    if (-not $?) {
        ERRLOG("Checking SysCenter Connection Failed")
        $write_res = write_result_file $uuid $errResult
        return $INVALID_SERVICE
    }
    foreach ($Status in $SCS.Status) {
        if ($Status -ne 'Running') {
            ERRLOG("Checking SysCenter Connection Status Failed")
            $write_res = write_result_file $uuid $errResult
            return $INVALID_SERVICE
        }
    }
    $write_res = write_result_file $uuid $result
    return $SUCCESS
}

function get_cluster_list_fromcache() {
    param ([string]$action, [string]$uuid)
    $cached = target_cached $CACHE_NAME_CLUSTER_LIST
    if (!$cached) {
        # write cache
        $res = get_cluster_list
        if (!$res) {
            ERRLOG("Write cache failed.")
            return $FAILED
        }
    }
    # read cache
    $result = read_cache $CACHE_NAME_CLUSTER_LIST
    return write_result $uuid $result
}

function get_cluster_list() {
    DBGLOG("Run GetClusterList")
    $UniqIdSession = Get-PSSession | Where-Object { $_.Name -eq $uuid }
    Invoke-Command  -Session $UniqIdSession -ScriptBlock { Get-SCVMMServer localhost | Out-Null }
    $result = Invoke-Command  -Session $UniqIdSession -ScriptBlock {(
        Get-SCVMHostCluster |
        Where-Object { $_.VirtualizationPlatform -eq "HyperV" } |
        Select-Object -Property Name,ID,IPAddresses
    )}
    return write_cache $CACHE_NAME_CLUSTER_LIST $result
}

function get_hostlist_SCVMM_fromcache() {
    param ([string]$action, [string]$uuid)
    $cached = target_cached $CACHE_NAME_HOST_LIST
    if (!$cached) {
        # write cache
        $res = get_host_list_from_SCVMM
        if (!$res) {
            ERRLOG("Write cache failed.")
            return $FAILED
        }
    }
    # read cache
    $result = read_cache $CACHE_NAME_HOST_LIST
    return write_result $uuid $result
}

function get_host_list_from_SCVMM() {
    DBGLOG("Run GetHostList")
    $UniqIdSession = Get-PSSession | Where-Object { $_.Name -eq $uuid }
    $localRes = Invoke-Command -Session $UniqIdSession -ScriptBlock { Get-SCVMMServer localhost | Out-Null }
    # Convert the output to CSV which will force the string output,
    # then re-convert it back from CSV to PS Object, then finally back to Json.
    # Otherwise, for example, the OverallState will be convert the enum integer in the output json string.
    $iPAddresses = Invoke-Command  -Session $UniqIdSession -ScriptBlock { (
        Get-SCVMHostNetworkAdapter
    )}
    $hIpsMap = @{}
    foreach ($ips in $iPAddresses) {
        if ($ips.VMHost.Name -eq $null) {
            $hostName = $ips.VMHost
        } else {
            $hostName = $ips.VMHost.Name
        }
        
        if ($hIpsMap.($hostName) -eq $null) {
            $hIpsMap.($hostName) = @()
        }
        $hIpsMap.($hostName) += $ips.IPAddresses.IPAddressToString
    }
    $result = Invoke-Command -Session $UniqIdSession -ScriptBlock {(
        Get-SCVMHost |
        Select-Object -Property Name,ID,OverallState,VirtualizationPlatform,HyperVVersion,HostCluster,FQDN |
        Where-Object {$_.VirtualizationPlatform -eq "HyperV"} |
        ConvertTo-Csv | ConvertFrom-Csv
    )}
    foreach ($hostInfo in $result) {
        $hostInfo | Add-Member -MemberType NoteProperty -Name 'IPAddress' -Value $hIpsMap.($hostInfo.Name)
    }
    return write_cache $CACHE_NAME_HOST_LIST $result
}

function get_iplist_fromcache() {
    param ([string]$action, [string]$uuid)
    $cached = target_cached $CACHE_NAME_IP_LIST
    if (!$cached) {
        # write cache
        $res = get_iplist
        if (!$res) {
            ERRLOG("Write cache failed.")
            return $FAILED
        }
    }
    # read cache
    $result = read_cache $CACHE_NAME_IP_LIST
    return write_result $uuid $result
}

function get_iplist() {
    $paramJson = read_parm_file $uuid
    [string]$hostName = $ParamJson.HostName
    $UniqIdSession = Get-PSSession | Where-Object { $_.Name -eq $uuid }
    Invoke-Command  -Session $UniqIdSession -ScriptBlock { Get-SCVMMServer localhost | Out-Null }

    DBGLOG("HostName: $hostName")
    $iPAddresses = Invoke-Command  -Session $UniqIdSession -ScriptBlock { (
        Get-SCVMHostNetworkAdapter -VMHost $Using:hostName |
        Select-Object -property IPAddresses
    )}
    $result = (
        $IPAddresses.IPAddresses | ConvertTo-Csv | ConvertFrom-Csv
    )
    return write_cache $CACHE_NAME_IP_LIST $result
}