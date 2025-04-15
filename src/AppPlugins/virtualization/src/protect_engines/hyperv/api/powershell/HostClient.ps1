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
        $UniqIdSession = Get-PSSession | Where-Object { $_.Name -eq $uuid }

        if ($UniqIdSession -eq $null) {
            $res = get_VM_list
        } else {
            $res = get_VM_list_session
        }
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
    $result = (
        Get-VM |
        Select-Object -Property Name,ID,State,Version,Generation,ConfigurationLocation,CheckpointFileLocation |
        ConvertTo-Csv | ConvertFrom-Csv
    )
    return write_cache $CACHE_NAME_VM_LIST $result
}

function get_VM_list_session() {
    DBGLOG("Run GetVMList")
    $UniqIdSession = Get-PSSession | Where-Object { $_.Name -eq $uuid }
    $result = (
        Invoke-Command -Session $UniqIdSession -ScriptBlock {Get-VM} |
        Select-Object -Property Name,ID,State,Version,Generation,ConfigurationLocation,CheckpointFileLocation |
        ConvertTo-Csv | ConvertFrom-Csv
    )
    return write_cache $CACHE_NAME_VM_LIST $result
}

function get_disklist_fromcache() {
    param ([string]$action, [string]$uuid)
    $ParamJson = read_parm_file $uuid
    [string]$VMId = $ParamJson.VMId
    if ([String]::IsNullOrEmpty($VMId)) {
        $cached = target_cached $CACHE_NAME_DISK_LIST
    }
    if (!$cached) {
        # write cache
        $UniqIdSession = Get-PSSession | Where-Object { $_.Name -eq $uuid }

        if ($UniqIdSession -eq $null) {
            $res = get_Disk_list
        } else {
            $res = get_Disk_list_session
        }
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

function get_Disk_list_session() {
    INFOLOG("Run GetDiskList session")

    $ParamJson = read_parm_file $uuid
    [string]$VMId = $ParamJson.VMId
    DBGLOG("VMId: $VMId")

    $UniqIdSession = Get-PSSession | Where-Object { $_.Name -eq $uuid }

    if (![String]::IsNullOrEmpty($VMId)) {
        $vm = Invoke-Command -Session $UniqIdSession -ScriptBlock {Get-VM -Id $Using:VMId}
    } else {
        $vm = Invoke-Command -Session $UniqIdSession -ScriptBlock {Get-VM}
    }
    $result = @()
    $driveMap = @{}
    foreach ($vmInfo in $vm) {
        $singleVmVhd = (
            Invoke-Command -Session $UniqIdSession -ScriptBlock {Get-VHD -VMId $Using:vmInfo.VMId} |
            Select-Object -Property DiskIdentifier,FileSize,Size,VhdFormat,VhdType,Path,ParentPath |
            ConvertTo-Csv | ConvertFrom-Csv
        )
        INFOLOG("singleVmVhd $singleVmVhd")
        $result += $singleVmVhd
        $vhdPathList = @()
        foreach ($vhd in $singleVmVhd) {
            $vhdpath = $vhd.Path
            $vhdPathList += $vhdpath
        }
        $singleVmHdDrives = (
            Invoke-Command -Session $UniqIdSession -ScriptBlock {Get-VMHardDiskDrive -VMName $Using:vmInfo.Name} |
            Where-Object {$_.VMId -eq $vmInfo.VMId}
        )
        foreach ($drive in $singleVmHdDrives) {
            $drivepath = $drive.Path
            if ($vhdPathList -notcontains $drivepath) {
                $drive | Add-Member -MemberType NoteProperty -Name 'IsPhysicalHardDisk' -Value $true -Force
                $phyDisk = New-Object PSObject -Property @{
                               IsPhysicalHardDisk = $true
                               VhdType = "Physical drive"
                               DiskIdentifier = $drive.Id
                               Path = $drivepath
                           }
                if ($result.GetType().IsArray) {
                    $result += $phyDisk
                } else {
                    $array = @($result)
                    $array += $phyDisk
                    $result = $array
                }
            } else {
                $drive | Add-Member -MemberType NoteProperty -Name 'IsPhysicalHardDisk' -Value $false -Force
            }
            try {
                $driveMap.Add($drivepath, $drive)
                $drive | Add-Member -MemberType NoteProperty -Name 'NeedAttension' -Value "false"
            } catch {
                ERRLOG("Hyper-V path has been added $_")
                $drive | Add-Member -MemberType NoteProperty -Name 'NeedAttension' -Value "true"
                $driveMap[$drivepath] = $drive
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
        $disk | Add-Member -MemberType NoteProperty -Name 'IsPhysicalHardDisk' -Value $driveMap.($disk.Path).IsPhysicalHardDisk
    }

    return write_cache $CACHE_NAME_DISK_LIST $result
}

function get_Disk_list() {
    INFOLOG("Run GetDiskList")

    $ParamJson = read_parm_file $uuid
    [string]$VMId = $ParamJson.VMId
    DBGLOG("VMId: $VMId")

    if (![String]::IsNullOrEmpty($VMId)) {
        $vm = Get-VM -Id $VMId
    } else {
        $vm = Get-VM
    }
    $result = @()
    $driveMap = @{}
    foreach ($vmInfo in $vm) {
        $singleVmVhd = (
            Get-VHD -VMId $vmInfo.VMId |
            Select-Object -Property DiskIdentifier,FileSize,Size,VhdFormat,VhdType,Path,ParentPath |
            ConvertTo-Csv | ConvertFrom-Csv
        )
        $result += $singleVmVhd
        $vhdPathList = @()
        foreach ($vhd in $singleVmVhd) {
            $vhdpath = $vhd.Path
            $vhdPathList += $vhdpath
        }
        $singleVmHdDrives = (
            Get-VMHardDiskDrive -VMName $vmInfo.Name |
            Where-Object {$_.VMId -eq $vmInfo.VMId}
        )
        foreach ($drive in $singleVmHdDrives) {
            $drivepath = $drive.Path
            if ($vhdPathList -notcontains $drivepath) {
                $drive | Add-Member -MemberType NoteProperty -Name 'IsPhysicalHardDisk' -Value $true -Force
                $phyDisk = New-Object PSObject -Property @{
                               IsPhysicalHardDisk = $true
                               VhdType = "Physical drive"
                               DiskIdentifier = $drive.Id
                               Path = $drivepath
                           }
                if ($result.GetType().IsArray) {
                    $result += $phyDisk
                } else {
                    $array = @($result)
                    $array += $phyDisk
                    $result = $array
                }
            } else {
                $drive | Add-Member -MemberType NoteProperty -Name 'IsPhysicalHardDisk' -Value $false -Force
            }
            try {
                $driveMap.Add($drivepath, $drive)
                $drive | Add-Member -MemberType NoteProperty -Name 'NeedAttension' -Value "false"
            } catch {
                ERRLOG("Hyper-V path has been added $_")
                $drive | Add-Member -MemberType NoteProperty -Name 'NeedAttension' -Value "true"
                $driveMap[$drivepath] = $drive
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
        $disk | Add-Member -MemberType NoteProperty -Name 'IsPhysicalHardDisk' -Value $driveMap.($disk.Path).IsPhysicalHardDisk
    }

    return write_cache $CACHE_NAME_DISK_LIST $result
}

function get_Directory_list() {
    param ([string]$action, [string]$uuid)
    DBGLOG("Run GetDirectoryList")
    $UniqIdSession = Get-PSSession | Where-Object { $_.Name -eq $uuid }
    $ParamJson = read_parm_file $uuid
    [string]$filePath = $ParamJson.FilePath
    [int]$page = $ParamJson.Page
    [int]$size = $ParamJson.Size
    DBGLOG("FilePath: $FilePath")

    if ($UniqIdSession -eq $null) {
        $files = (
            Get-ChildItem -Path $filePath |
            Select-Object -Property Mode, LastWriteTime, Length, Name |
            ConvertTo-Csv | ConvertFrom-Csv
        )
    } else {
        $files = (
            Invoke-Command -Session $UniqIdSession -ScriptBlock {Get-ChildItem -Path $Using:filePath} |
            Select-Object -Property Mode, LastWriteTime, Length, Name |
            ConvertTo-Csv | ConvertFrom-Csv
        )
    }
    

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
        Get-VM -Id $VMId | Select-Object Id, Generation, ConfigurationLocation, CheckpointFileLocation |
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
    DBGLOG("Run get_host_iplist")

    $ParamJson = read_parm_file $uuid
    [string]$VMId = $ParamJson.VMId
    INFOLOG("VMId: $VMId")

    $vm = Get-VM -Id $VMId

    $ips = Get-VMNetworkAdapter -VMName $vm.Name | Where-Object { $_.VMId -eq $VMId } | Select-Object -Property IPAddresses 

    $result = $ips.IPAddresses
    $result = format_result $result
    $res = write_result_file $uuid $result
    DBGLOG("VMId: $result,  $uuid")
    return $res
}