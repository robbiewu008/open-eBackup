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

. C:\DataBackup\ProtectClient\Plugins\VirtualizationPlugin\bin\Common.ps1

function check_host_connection() {
    param ([string]$action, [string]$uuid)
    DBGLOG("Run CheckHostConnection")

    try {
        Get-VMHost | Out-Null
        if (-not $?) {
            ERRLOG("Run CheckHostConnection Failed")
            $result = '{"code": 200, "bodyErr": 200, "message": "Hyper service error"}'
        } else {
            $result = '{"code": 0, "bodyErr": 0, "message": ""}'
            INFOLOG("Run CheckHostConnection Success")
        }
    } catch {
        ERRLOG("Run CheckHostConnection catch Failed")
        $result = '{"code": 200, "bodyErr": 200, "message": "Hyper service error"}'
    }
    $write_result = write_result_file $uuid $result
    return $SUCCESS
}

function get_vmlist_fromcache() {
    param ([string]$action, [string]$uuid)
    $cached = target_cached $CACHE_NAME_VM_LIST
    if (!$cached) {
        # write cache
        $res = get_VM_list
        if (!$res) {
            ERRLOG("Write cache failed.")
            return $FAILED
        }
    }
    # read cache
    $result = read_cache $CACHE_NAME_VM_LIST
    return write_result $uuid $result
}

function get_VM_list() {
    DBGLOG("Run GetVMList")
    $vminfos = Get-VM
    $adapters = (
        Get-VMNetworkAdapter -all
    )
    $ipsMap = @{}
    foreach ($net in $adapters) {
        if ($net.VMId -eq $null) {
            continue
        }
        $Id = $net.VMId.ToString()
        if ($ipsMap[$Id] -eq $null) {
            $ipsMap[$Id] = @()
        }
        if ($net.IPAddresses -ne $null) {
            $ipsMap[$Id] = $ipsMap[$Id] + $net.IPAddresses
        }
    }
    $result = (
        $vminfos |
        Select-Object -Property Name,ID,State,Version,Generation,ConfigurationLocation,CheckpointFileLocation |
        ConvertTo-Csv | ConvertFrom-Csv
    )
    foreach ($vm in $result) {
        $vm | Add-Member -MemberType NoteProperty -Name 'IPAddress' -Value $ipsMap.($vm.Id)
    }
    return write_cache $CACHE_NAME_VM_LIST $result
}

function get_disklist_fromcache() {
    param ([string]$action, [string]$uuid)
    $cached = target_cached $CACHE_NAME_DISK_LIST
    if (!$cached) {
        # write cache
        $res = get_Disk_list
        if (!$res) {
            ERRLOG("Write cache failed.")
            return $FAILED
        }
    }

    #$ParamJson = read_parm_file $uuid
    #[string]$VMId = $ParamJson.VMId
    #DBGLOG("VMId: $VMId")

    # read cache
    $result = read_cache $CACHE_NAME_DISK_LIST #| Where-Object { $_.VMId -eq $VMId }
    return write_result $uuid $result
}

function get_Disk_list() {
    DBGLOG("Run GetDiskList")

    $ParamJson = read_parm_file $uuid
    [string]$VMId = $ParamJson.VMId
    DBGLOG("VMId: $VMId")

    if (![String]::IsNullOrEmpty($VMId)) {
        $vm = Get-VM -Id $VMId
    } else {
        $vm = Get-VM
    }

    $result = (
        $vm | Select-Object VMId | Get-VHD |
        Select-Object -Property DiskIdentifier,FileSize,Size,VhdFormat,VhdType,Path,ParentPath |
        ConvertTo-Csv | ConvertFrom-Csv
    )

    $driveMap = @{}
    $hdDrives = @()
    foreach ($vmInfo in $vm) {
        $hdDrives += $vmInfo.Name | Get-VMHardDiskDrive | Where-Object {$_.VMId -eq $vmInfo.VMId}
    }
    foreach ($drive in $hdDrives) {
        if (![String]::IsNullOrEmpty($drive.Path)) {
            $path = $drive.Path
            try {
                $driveMap.Add($path, $drive)
                $drive | Add-Member -MemberType NoteProperty -Name 'NeedAttension' -Value "false"
            } catch {
                ERRLOG("Hyper-V path has been added $_")
                $drive | Add-Member -MemberType NoteProperty -Name 'NeedAttension' -Value "true"
                $driveMap[$path] = $drive
            }
        }
    }

    foreach ($disk in $result) {
        $disk | Add-Member -MemberType NoteProperty -Name 'ControllerType' -Value $driveMap.($disk.Path).ControllerType
        $disk | Add-Member -MemberType NoteProperty -Name 'ControllerUUID' -Value ($driveMap.($disk.Path).Id -split "\\")[1]
        $disk | Add-Member -MemberType NoteProperty -Name 'ControllerIndex' -Value $driveMap.($disk.Path).ControllerNumber
        $disk | Add-Member -MemberType NoteProperty -Name 'ControllerPos' -Value $driveMap.($disk.Path).ControllerLocation
        $disk | Add-Member -MemberType NoteProperty -Name 'VMId' -Value $driveMap.($disk.Path).VMId
        $disk | Add-Member -MemberType NoteProperty -Name 'NeedAttension' -Value $driveMap.($disk.Path).NeedAttension
        $disk | Add-Member -MemberType NoteProperty -Name 'SupportPersistentReservations' -Value $driveMap.($disk.Path).SupportPersistentReservations
    }

    return write_cache $CACHE_NAME_DISK_LIST $result
}

function get_Directory_list() {
    param ([string]$action, [string]$uuid)
    DBGLOG("Run GetDirectoryList")

    $ParamJson = read_parm_file $uuid
    [string]$filePath = $ParamJson.FilePath
    [int]$page = $ParamJson.Page
    [int]$size = $ParamJson.Size
    DBGLOG("FilePath: $FilePath")

    $files = (
        Get-ChildItem -Path $filePath |
        Select-Object -Property Mode, LastWriteTime, Length, Name |
        ConvertTo-Csv | ConvertFrom-Csv
    )

    $result = [System.Collections.ArrayList]::new()
    for ($i = ($page - 1) * $size; $i -lt $page * $size -and $i -lt $files.Count; $i++) {
        $t = $result.Add($files[$i]);
    }

    $arrayResult = ConvertTo-Json -depth 1 @($result)
    $result = '{"result":' + $arrayResult + ', "total": ' + $files.Count + '}'
    $res = write_result_file $uuid $result
    return $res
}

function get_vm_info {
    param ([string]$action, [string]$uuid)
    DBGLOG("Run GetVmInfo.")

    $paramJson = read_parm_file $uuid
    $VMId = $paramJson.VMId

    DBGLOG("VMId: $VMId")
    $result = (
        Get-VM -Id $VMId | Select-Object Id, ConfigurationLocation, CheckpointFileLocation |
        ConvertTo-Json -depth 1
    )
    $res = write_result_file $uuid $result
    return $res
}

function get_vm_harddisk_drive {
    param ([string]$action, [string]$uuid)
    DBGLOG("Run GetVmHardDiskDrive.")

    $paramJson = read_parm_file $uuid
    $VMId = $paramJson.VMId
    DBGLOG("VMId: $VMId")
    $result = (
        Get-VM -Id $VMId | Get-VMHardDiskDrive |
        select-object Path, Id | ConvertTo-Csv | ConvertFrom-Csv
    )
    DBGLOG("result: $result")
    $result = format_result $result
    $res = write_result_file $uuid $result
    return $res
}

function create_vhd {
    param ([string]$action, [string]$uuid)
    DBGLOG("Run CreateVHD.")
    $paramJson = read_parm_file $uuid
    $path = $paramJson.path
    $size = $paramJson.size
    $type = $paramJson.type
    DBGLOG("Param: $path, $size, $type.")
    if ($type -eq "Fixed") {
        $result = (
            New-VHD -Path $path -SizeBytes $size -Fixed | ConvertTo-Json -depth 1
        )
    }
    else {
        $result = (
            New-VHD -Path $path -SizeBytes $size | ConvertTo-Json -depth 1
        )
    }
    DBGLOG("Result: $result")
    $res = write_result_file $uuid $result
    return $res
}

function get_vm_iplist_fromcache() {
    param ([string]$action, [string]$uuid)
    $cached = target_cached $CACHE_NAME_VM_IP_LIST
    if (!$cached) {
        # write cache
        $res = get_vm_iplist
        if (!$res) {
            ERRLOG("Write cache failed.")
            return $FAILED
        }
    }
    # read cache
    $result = read_cache $CACHE_NAME_VM_IP_LIST
    return write_result $uuid $result
}

function get_vm_iplist() {
    DBGLOG("Run get_host_iplist")

    $ParamJson = read_parm_file $uuid
    [string]$VMId = $ParamJson.VMId
    DBGLOG("VMId: $VMId")

    $ips = (
        Get-VM -Id $VMId | Get-VMNetworkAdapter |
        Select-Object -Property IPAddresses
    )
    $result = $ips.IPAddresses
    return write_cache $CACHE_NAME_VM_IP_LIST $result
} 