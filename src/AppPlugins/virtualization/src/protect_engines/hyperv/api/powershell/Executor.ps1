# requires -version 3.0
# requires -module Hyper-V

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

<#
.SYNOPSIS
Wrapper to call Hyper-V WMI Commands.

.DESCRIPTION
Wrapper to call Hyper-V WMI Commands.

.PARAMETER Command-Type
Command needs to be executed.

.PARAMETER UniqId
An uuid to identify parameter file and result file.

.EXAMPLE
PS C:\> Executor.ps1 -Command-Type CheckHostConnection -UniqId xxxx-xxxx-xxxx-xxxx
#>

param (
    [String]${Command-Type},
    [String]${UniqId}
)

. C:\DataBackup\ProtectClient\Plugins\VirtualizationPlugin\bin\Common.ps1
. C:\DataBackup\ProtectClient\Plugins\VirtualizationPlugin\bin\SCVMMClient.ps1
. C:\DataBackup\ProtectClient\Plugins\VirtualizationPlugin\bin\HostClient.ps1
. C:\DataBackup\ProtectClient\Plugins\VirtualizationPlugin\bin\ClusterClient.ps1
. C:\DataBackup\ProtectClient\Plugins\VirtualizationPlugin\bin\Snapshot.ps1

$COMMAND_MAP = @{
    CheckSCConnection = (Get-Item "function:check_sys_center_connection").ScriptBlock;
    CheckHostConnection = (Get-Item "function:check_host_connection").ScriptBlock;
    CheckClusterConnection = (Get-Item "function:check_cluster_connection").ScriptBlock;
    ListCluster = (Get-Item "function:get_cluster_list_fromcache").ScriptBlock;
    ListClusterHost = (Get-Item "function:get_host_list_from_cluster_fromcache").ScriptBlock;
    ListHost = (Get-Item "function:get_hostlist_SCVMM_fromcache").ScriptBlock;
    ListVM = (Get-Item "function:get_vmlist_fromcache").ScriptBlock;
    ListDisk = (Get-Item "function:get_disklist_fromcache").ScriptBlock;
    ListIpAddress = (Get-Item "function:get_iplist_fromcache").ScriptBlock;
    ListVMIpAddress = (Get-Item "function:get_vm_iplist_fromcache").ScriptBlock;
    ListDirectory = (Get-Item "function:get_Directory_list").ScriptBlock;
    CreateVMCheckPoint = (Get-Item "function:create_vm_checkpoint").ScriptBlock;
    ConvertVMCheckPoint = (Get-Item "function:convert_vm_checkpoint").ScriptBlock;
    GetVMCheckPoints = (Get-Item "function:get_vm_checkpoints").ScriptBlock;
    GetVMReferrencePoints = (Get-Item "function:get_vm_referrence_points").ScriptBlock;
    DeleteVMAllReferrencePoints = (Get-Item "function:delete_vm_all_referrence_points").ScriptBlock;
    DeleteVMReferrencePoint = (Get-Item "function:delete_vm_referrence_point").ScriptBlock;
    DeleteVMReferrencePointExcept = (Get-Item "function:delete_vm_referrence_point_except").ScriptBlock;
    DeleteVMAllCheckPoints = (Get-Item "function:delete_vm_all_checkpoints").ScriptBlock;
    DeleteVMCheckPoint = (Get-Item "function:delete_vm_checkpoint").ScriptBlock;
    GetVMInfo = (Get-Item "function:get_vm_info").ScriptBlock;
    GetVMDriver = (Get-Item "function:get_vm_harddisk_drive").ScriptBlock;
    CreateVHD = (Get-Item "function:create_vhd").ScriptBlock;
    AddCluster = (Get-Item "function:add_cluster").ScriptBlock;
    GetClusterSharedVolume = (Get-Item "function:get_cluster_shared_volume").ScriptBlock;
}

$RETURN_VALUT = @{
    code = 0;
    bodyErr = 0;
    message = "";
}

function check_params($cmd_type, $uuid) {
    INFOLOG("Checking input parameters...")
    if ([String]::IsNullOrEmpty($cmd_type) -or [String]::IsNullOrEmpty($uuid)) {
        ERRLOG("Invalid parameter. Command-Type: $cmd_type or UniqId: $uuid is empty.")
        return $INVALID_PARAM
    }

    if (-not $COMMAND_MAP.ContainsKey($cmd_type)) {
        ERRLOG("Invalid parameter. Command-Type: $cmd_type is invalid.")
        return $INVALID_PARAM
    }

    INFOLOG "Check params success."
    return $SUCCESS
}


function handle_invoke_method_failed() {
    param([int]$res, [string]$uuid)
    $errMsg = $Error[0].Exception.Message
    ERRLOG("Invoke method failed.Error message: $errMsg")

    $RETURN_VALUT["code"] = $res;
    $RETURN_VALUT["bodyErr"] = $res;
    $RETURN_VALUT["message"] = $errMsg;

    $Result = (
        $RETURN_VALUT | ConvertTo-Json -depth 1
    )

    write_result_file $uuid $Result
    exit $res
}

function main() {
    INFOLOG("----- START -----")

    $script:res = check_params ${Command-Type} ${UniqId}
    if ($res -ne $SUCCESS) {
        ERRLOG("Invalid parameter.")
        INFOLOG("Usage: Executor -Command-Type <CommadType> -UniqId <UniqId>")
        return $FAILED
    }

    $ParamJson = read_parm_file $UniqId
    DBGLOG("Read param file: $ParamJson")
    $TargetHost = $ParamJson.TargetHost
    if ([String]::IsNullOrEmpty($TargetHost)) {
        $TargetHost = 'localhost'
    }

    $TargetType = $ParamJson.TargetType
    if ([String]::IsNullOrEmpty($TargetHost)) {
        ERRLOG("Target type is invalid: $TargetType")
        return $FAILED
    }

    [int]$global:pageNo = $ParamJson.pageNo
    [int]$global:pageSize = $ParamJson.PageSize
    [string]$global:requestId = $ParamJson.RequestId

    INFOLOG("Running $COMMAND_MAP.${Command-Type}")

    if ("${Command-Type}" -eq "GetVMDriver" -or "${Command-Type}" -eq "CreateVHD" -or "${Command-Type}" -eq "CheckSCConnection" -or "${Command-Type}" -eq "CheckClusterConnection") {
        $resp = $COMMAND_MAP.${Command-Type}.Invoke("Call", $UniqId)
        if ($resp -ne $SUCCESS) {
            handle_invoke_method_failed $resp $UniqId
        }
        return $SUCCESS
    }
    
    try {
        $newSessionRes = new_pssession $UniqId $TargetHost $TargetType
        if ($newSessionRes -ne $SUCCESS) {
            ERRLOG("Create new session failed.")
            clear_cache
            handle_invoke_method_failed $newSessionRes $UniqId
        }
        $res = $COMMAND_MAP.${Command-Type}.Invoke("Call", $UniqId)
        if ($res -ne $SUCCESS) {
            clear_cache
            close_pssession $UniqId
            handle_invoke_method_failed $res $UniqId
        }
    } catch {
        clear_cache
        close_pssession $UniqId
        handle_invoke_method_failed $FAILED $UniqId
    }

    INFOLOG("----- END -----")
    return $SUCCESS
}

exit main