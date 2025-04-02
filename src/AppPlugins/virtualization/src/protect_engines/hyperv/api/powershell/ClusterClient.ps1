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

function check_cluster_connection() {
    param ([string]$action, [string]$uuid)
    DBGLOG("Run CheckHostConnection")

    $valid = IsUserNamePasswordValid
    if (!$valid) {
        $errResult = '{"code": -1, "bodyErr": -1, "message": "Auth unavailable"}'
        $write_res = write_result_file $uuid $errResult
        ERRLOG("Valid cridential failed.")
        return $FAILED
    }
    try {
        Get-ClusterNode | Out-Null
        if (-not $?) {
            ERRLOG("Run CheckClusterConnection Failed")
            $result = '{"code": 200, "bodyErr": 200, "message": "Cluster service error"}'
            $write_result = write_result_file $uuid $result
            return $INVALID_SERVICE
        } else {
            $result = '{"code": 0, "bodyErr": 0, "message": ""}'
        }
    } catch {
        ERRLOG("Run CheckHostConnection catch Failed")
        $result = '{"code": 200, "bodyErr": 200, "message": "Cluster service error"}'
        $write_result = write_result_file $uuid $result
        return $INVALID_SERVICE
    }
    $write_result = write_result_file $uuid $result
    INFOLOG("Run CheckClusterConnection Success")
    return $SUCCESS
}

function get_host_list_from_cluster_fromcache() {
    param ([string]$action, [string]$uuid)
    $cached = target_cached $CACHE_NAME_CLUSTER_LIST
    if (!$cached) {
        # write cache
        $res = get_host_list_from_cluster
        if (!$res) {
            ERRLOG("Write cache failed.")
            return $FAILED
        }
    }
    # read cache
    $result = read_cache $CACHE_NAME_CLUSTER_LIST
    return write_result $uuid $result
}

function get_host_list_from_cluster() {
    DBGLOG("Run GetHostList")
    $result = (
        Get-ClusterNode |
        Select-Object -Property Name,SerialNumber,State,MajorVersion |
        ConvertTo-Csv | ConvertFrom-Csv
    )
    $ipInfo = Get-ClusterNetworkInterface | Select-Object Node,Address
    $domainName = (Get-WmiObject Win32_ComputerSystem).Domain
    $domain = $domainName.ToString()
    foreach ($node in $result) {
        $nodeName = $node.Name + "." + $domain
        $node | Add-Member -MemberType NoteProperty -Name 'NodeName' -Value $nodeName
        $node | Add-Member -MemberType NoteProperty -Name 'FQDN' -Value $nodeName
        $nodeIp = $ipInfo | Where-Object {$_.Node -eq $node.Name} | Select-Object Address
        $nodeAdd = ""
        foreach ($nIp in $nodeIp) {
            $nodeAdd = $nodeAdd + $nIp.Address + ","
        }
        $node | Add-Member -MemberType NoteProperty -Name 'IPAddress' -Value $nodeAdd
    }
    return write_cache $CACHE_NAME_CLUSTER_LIST $result
}

function add_cluster {
    param ([string]$action, [string]$uuid)
    $result = '{"code": 0, "bodyErr": 0, "message": ""}'
    $resultCode = $SUCCESS
    try {
        $jsonParm = read_parm_file $uuid
        $response = Add-ClusterVirtualMachineRole -VMId $jsonParm.VMId
        INFOLOG("$response AddCluster success!")
    } catch {
        $expInfo = $_.Exception.Message
        ERRLOG("Run AddCluster failed, err: $expInfo")
        $result = '{"code": 200, "bodyErr": 200, "message": "Add Cluster error"}'
        $resultCode = $FAILED
    }
    $write_result = write_result_file $uuid $result
    return $resultCode
}

function get_cluster_shared_volume {
    param ([string]$action, [string]$uuid)
    try {
        $result = Get-ClusterSharedVolume | Select-Object -ExpandProperty SharedVolumeInfo | Select-Object FriendlyVolumeName
    } catch {
        ERRLOG("Run get_cluster_shared_volume failed")
        $result = {}
    }
    $result = format_result $result
    $write_result = write_result_file $uuid $result
    return $write_result
}