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

filter ProcessWMIJob
{
    param
    (
        [WMI]$WmiClass = $null,
        [string]$MethodName = $null
    )
    $errorCode = 0
    $returnObject = $_
 
    if ($_.ReturnValue -eq 4096)
    {
        $Job = [WMI]$_.Job
        $returnObject = $Job
 
        while ($Job.JobState -eq 4)
        {
            Write-Progress -Activity $Job.Caption -Status ($Job.JobStatus + " - " + $Job.PercentComplete + "%") -PercentComplete $Job.PercentComplete
            Start-Sleep -seconds 1
            $Job.PSBase.Get()
        }
        if ($Job.JobState -ne 7)
        {
            if ($Job.ErrorDescription -ne "")
            {
                Write-Error $Job.ErrorDescription
                Throw $Job.ErrorDescription
            }
            else
            {
                $errorCode = $Job.ErrorCode
            }
        }
        Write-Progress -Activity $Job.Caption -Status $Job.JobStatus -PercentComplete 100 -Completed:$true
    }
    elseif($_.ReturnValue -ne 0)
    {
        $errorCode = $_.ReturnValue
    }
 
    if ($errorCode -ne 0)
    {
        Write-Error "Hyper-V WMI Job Failed!"
        if ($WmiClass -and $MethodName)
        {
            $psWmiClass = [WmiClass]("\\" + $WmiClass.__SERVER + "\" + $WmiClass.__NAMESPACE + ":" + $WmiClass.__CLASS)
            $psWmiClass.PSBase.Options.UseAmendedQualifiers = $TRUE
            $MethodQualifierValues = ($psWmiClass.PSBase.Methods[$MethodName].Qualifiers)["Values"]
            $indexOfError = [System.Array]::IndexOf(($psWmiClass.PSBase.Methods[$MethodName].Qualifiers)["ValueMap"].Value, [string]$errorCode)
            if (($indexOfError -ne "-1") -and $MethodQualifierValues)
            {
                Throw "ReturnCode: ", $errorCode, " ErrorMessage: '", $MethodQualifierValues.Value[$indexOfError], "' - when calling $MethodName"
            }
            else
            {
                Throw "ReturnCode: ", $errorCode, " ErrorMessage: 'MessageNotFound' - when calling $MethodName"
            }
        }
        else
        {
            Throw "ReturnCode: ", $errorCode, "When calling $MethodName - for rich error messages provide classpath and method name."
        }
    }
    return $returnObject
}
 
function New_VmBackupCheckpoint($vmId,$ConsistencyLevel)
{
    INFOLOG($vmId)
    INFOLOG($ConsistencyLevel)
    # Retrieve an instance of the virtual machine management service
    $Msvm_VirtualSystemManagementService = Get-WmiObject -Namespace root\virtualization\v2 -Class Msvm_VirtualSystemManagementService
    # Retrieve an instance of the virtual machine snapshot service
    $Msvm_VirtualSystemSnapshotService = Get-WmiObject -Namespace root\virtualization\v2 -Class Msvm_VirtualSystemSnapshotService
    $filter = "Name='$vmId'"
    # Retrieve an instance of the virtual machine computer system that will be snapshotted
    $Msvm_ComputerSystem = Get-WmiObject -Namespace root\virtualization\v2 -Class Msvm_ComputerSystem -Filter $filter
    # Create an instance of the Msvm_VirtualSystemSnapshotSettingData, this class provides options on how the checkpoint will be created.
    $Msvm_VirtualSystemSnapshotSettingData = ([WMIClass]"\\.\root\virtualization\v2:Msvm_VirtualSystemSnapshotSettingData").CreateInstance()
    # Identify the consistency level for the snapshot.
    # 1: Application Consistent
    # 2: Crash Consistent
    switch ($ConsistencyLevel)
    {
        "ApplicationConsistent" {
        $Msvm_VirtualSystemSnapshotSettingData.ConsistencyLevel = 1
        }
        "CrashConsistent" {
        $Msvm_VirtualSystemSnapshotSettingData.ConsistencyLevel = 2
        }
        default {
        throw "Unexpected Consistancy Level Specified"
        }
    }
    # Specify the behavior for disks that cannot be snapshotted (i.e. pass-through, virtual fibre channel)
    $Msvm_VirtualSystemSnapshotSettingData.IgnoreNonSnapshottableDisks = $true
    # Create the virtual machine snapshot, this method returns a job object.
    $job = $Msvm_VirtualSystemSnapshotService.CreateSnapshot(
        $Msvm_ComputerSystem,
        $Msvm_VirtualSystemSnapshotSettingData.GetText(2),
        32768)
    # Waits for the job to complete and processes any errors.
    ($job | ProcessWMIJob -WmiClass $Msvm_VirtualSystemSnapshotService -MethodName "CreateSnapshot") | Out-Null
    # Retrieves the snapshot object resulting from the snapshot.
    $snapshot = (([WMI]$job.Job).GetRelated("Msvm_VirtualSystemSettingData") | % {$_})
    # Returns the snapshot instance

    $snapname = "databackup_$vmId"
    $snapshot.ElementName = $snapname
    [System.Management.ManagementBaseObject]$result = $Msvm_VirtualSystemManagementService.ModifySystemSettings($snapshot.GetText(2))
    return $snapshot
}

function create_vm_checkpoint() {
    param ([string]$action, [string]$uuid)
    DBGLOG("Run CreateVMCheckPoint")

    $ParamJson = read_parm_file $uuid
    [string]$vmid = $ParamJson.vmid
    [string]$consistencyLevel = $ParamJson.consistencyLevel

    $result = (
        New_VmBackupCheckpoint $vmid $consistencyLevel |
        Select-Object -Property ElementName, ConfigurationDataRoot, GuestStateFile, InstanceID, ConfigurationID |
        ConvertTo-Json -depth 1
    )
    $res = write_result_file $uuid $result
    return $res
}

function Get_VmBackupCheckpoints
{
    [CmdletBinding(DefaultParametersetname="vmname")]
    Param(
      [Parameter(Mandatory=$True, ParameterSetName="vmid")]
      [string]$VmId = [String]::Empty,
      [Parameter(Mandatory=$True, ParameterSetName="vmname")]
      [string]$VmName = [String]::Empty
    )

    if ($PsCmdlet.ParameterSetName -eq "vmname"){
        $filter = "ElementName='$vmName'"
    } else {
        $filter = "Name='$VmId'"
    }

    # Retrieve an instance of the virtual machine computer system that contains recovery checkpoints
    $Msvm_ComputerSystem = Get-WmiObject -Namespace root\virtualization\v2 -Class Msvm_ComputerSystem -Filter $filter

    # Retrieve all snapshot associations for the virtual machine
    $allSnapshotAssociations = $Msvm_ComputerSystem.GetRelationships("Msvm_SnapshotOfVirtualSystem")

    # Enumerate across all of the instances and add all recovery snapshots to an array
    $virtualSystemSnapshots = @()
    $enum = $allSnapshotAssociations.GetEnumerator()
    $enum.Reset()
    while($enum.MoveNext())
    {
        if (([WMI] $enum.Current.Dependent).VirtualSystemType -eq "Microsoft:Hyper-V:Snapshot:Recovery")
        {
            $virtualSystemSnapshots += ([WMI] $enum.Current.Dependent)
        }
    }

    # Return the array of recovery snapshots
    $virtualSystemSnapshots
}

function convert_vmbackupcheckpoint
{
    Param(
      [Parameter(Mandatory=$True)]
      [System.Management.ManagementObject]$BackupCheckpoint = $null
    )

    # Retrieve an instance of the snapshot management service
    $Msvm_VirtualSystemSnapshotService = Get-WmiObject -Namespace root\virtualization\v2 -Class Msvm_VirtualSystemSnapshotService

    # Convert the snapshot to a reference point, this function returns a job object.
    $job = $Msvm_VirtualSystemSnapshotService.ConvertToReferencePoint($BackupCheckpoint)

    # Wait for the job to complete.
    ($job | ProcessWMIJob -WmiClass $Msvm_VirtualSystemSnapshotService -MethodName "ConvertToReferencePoint") | Out-Null

    # The new reference point object is related to the job, GetReleated
    # always returns an array in this case there is only one member
    $refPoint = (([WMI]$job.Job).GetRelated("Msvm_VirtualSystemReferencePoint") | % {$_})

    # Return the reference point object
    return $refPoint
}

function get_vmreferencepoints
{
    [CmdletBinding(DefaultParametersetname="vmname")]
    Param(
      [Parameter(Mandatory=$True, ParameterSetName="vmid")]
      [string]$VmId = [String]::Empty,
      [Parameter(Mandatory=$True, ParameterSetName="vmname")]
      [string]$VmName = [String]::Empty
    )
 
    if ($PsCmdlet.ParameterSetName -eq "vmname"){
        $filter = "ElementName='$vmName'"
    } else {
        $filter = "Name='$VmId'"
    }
    # Retrieve an instance of the virtual machine computer system that contains reference points
    $Msvm_ComputerSystem = Get-WmiObject -Namespace root\virtualization\v2 -Class Msvm_ComputerSystem -Filter $filter
 
    # Retrieve all refrence associations of the virtual machine
    $allrefPoints = $Msvm_ComputerSystem.GetRelationships("Msvm_ReferencePointOfVirtualSystem")
 
    # Enumerate across all of the instances and add all recovery points to an array
    $virtualSystemRefPoint = @()
    $enum = $allrefPoints.GetEnumerator()
    $enum.Reset()
    while($enum.MoveNext())
    {
        $virtualSystemRefPoint += ([WMI] $enum.Current.Dependent)
    }
 
    # Return the array of recovery points
    $virtualSystemRefPoint
}
 
function remove_vmreferencepoint
{
    Param(
      [Parameter(Mandatory=$True)]
      [System.Management.ManagementObject]$ReferencePoint = $null
    )
 
 
    # Retrieve an instance of the virtual machine refrence point service
    $Msvm_VirtualSystemReferencePointService = Get-WmiObject -Namespace root\virtualization\v2 -Class Msvm_VirtualSystemReferencePointService
 
    # Removes the virtual machine reference, this method returns a job object.
    $job = $Msvm_VirtualSystemReferencePointService.DestroyReferencePoint($ReferencePoint)
 
    # Waits for the job to complete and processes any errors.
    ($job | ProcessWMIJob -WmiClass $Msvm_VirtualSystemReferencePointService -MethodName "DestroyReferencePoint") | Out-Null
}
 
# define func
function convert_vm_checkpoint() {
    param ([string]$action, [string]$uuid)
    DBGLOG("Run ConvertVMCheckpoint")

    $param_json = read_parm_file $uuid
    [string]$vm_id = $param_json.vmid
    [string]$check_point_name = $param_json.checkPointName
    DBGLOG("VMId: $vm_id")
    DBGLOG("check_point_name: $check_point_name")

    $check_points = (
        Get_VmBackupCheckpoints -vmid $vm_id
    )
    INFOLOG("Get_VmBackupCheckpoints success")
    $target_point = ""
    foreach ($check_point in $check_points) {
        if ($check_point["ElementName"] -eq "$check_point_name")
        {
            $target_point = $check_point
            break;
        }
    }
    INFOLOG("Target check point $target_point.")

    $result = (
        convert_vmbackupcheckpoint $target_point |
        Select-Object -Property ElementName, ConsistencyLevel, InstanceID, ReferencePointType, ResilientChangeTrackingIdentifiers, VirtualDiskIdentifiers |
        ConvertTo-Json -depth 1
    )

    $res = write_result_file $uuid $result
    return $res
}

function get_vm_referrence_points() {
    param (
        [string]$uuid,
        [string]$vmname
    )
    DBGLOG("Run GetVMReferencePoints")

    $result = (
        get_vmreferencepoints -VMName $vmname |
        Select-Object -Property ElementName, ConsistencyLevel, InstanceID, ReferencePointType, ResilientChangeTrackingIdentifiers, VirtualDiskIdentifiers |
        ConvertTo-Json -depth 1
    )
    $res = write_result_file $uuid $result
    return $res
}

function delete_vm_all_referrence_points() {
    param ([string]$action, [string]$uuid)
    DBGLOG("Run DeleteVMAllReferencePoints")

    $param_json = read_parm_file $uuid
    [string]$vm_id = $param_json.vmid
    DBGLOG("VMId: $vm_id")
    
    $obj = (
        get_vmreferencepoints -VMId $vm_id
    )

    $result = ""
    foreach ($item in $obj) {
        $ret = (
            remove_vmreferencepoint $item |
            ConvertTo-Json -depth 1
        )
    }
    $result = '{"code":0,"bodyErr":0}'

    $res = write_result_file $uuid $result
    return $res
}

function delete_vm_referrence_point() {
    param ([string]$action, [string]$uuid)
    DBGLOG("Run DeleteVMReferrencePoint")

    $param_json = read_parm_file $uuid
    [string]$vm_id = $param_json.vmid
    [string]$instance_id = $param_json.instanceid
    DBGLOG("VMId: $vm_id")
    DBGLOG("InstanceID: $instance_id")
    $ids = $instance_id.split(";");
    
    $obj = ""
    foreach ($id in $ids) {
        $obj = (
            get_vmreferencepoints -VMId $vm_id |
            where-object {$_.InstanceID -eq $id}
        )
        $ret = (
            remove_vmreferencepoint $obj |
            ConvertTo-Json -depth 1
        )
    }
    $result = '{"code":0,"bodyErr":0}'
 
    $res = write_result_file $uuid $result
    return $res
}

function delete_vm_referrence_point_except() {
    param ([string]$action, [string]$uuid)
    DBGLOG("Run DeleteVMReferrencePointExcept")
 
    $param_json = read_parm_file $uuid
    [string]$vm_id = $param_json.vmid
    [string]$instance_id = $param_json.instanceid
    DBGLOG("VMId: $vm_id")
    DBGLOG("InstanceID: $instance_id")
 
    $obj = (
        get_vmreferencepoints -VMId $vm_id
    )
 
    foreach ($item in $obj) {
        DBGLOG("item: $item")
        if ($item["InstanceID"] -eq "$instance_id")
        {
            continue;
        }
        $ret = (
            remove_vmreferencepoint $item |
            ConvertTo-Json -depth 1
        )
    }
    $result = '{"code":0,"bodyErr":0}'
 
    $res = write_result_file $uuid $result
    return $res
}

function get_vm_checkpoints() {
    param (
        [string]$uuid,
        [string]$vmname
    )
    DBGLOG("Run GetVMCheckPoints")

    $result = (
        Get-VmBackupCheckpoints -VMName $vmname |
        Select-Object -Property ElementName, ConfigurationDataRoot, GuestStateFile, InstanceID |
        ConvertTo-Json -depth 1
    )
    $res = write_result_file $uuid $result
    return $res
}

function delete_vm_all_checkpoints() {
    param (
        [string]$uuid,
        [string]$vmname
    )
    DBGLOG("Run DeleteVMAllCheckPoints")

    $result = {
        Remove-VMCheckpoint -VMName $vmname |
        ConvertTo-Json -depth 1
    }

    $res = write_result_file $uuid $result
    return $res
}

function delete_vm_checkpoint() {
    param ([string]$action, [string]$uuid)
    INFOLOG("Run DeleteVMCheckPoint")
    $param_json = read_parm_file $uuid

    [string]$vm_id = $param_json.vmid
    [string]$snapShotName = $param_json.snapShotName
    
    $result = (
        Get-VM -Id $vm_id | Remove-VMSnapshot -Name "$snapShotName" |
        ConvertTo-Json -depth 1
    )
    $res = write_result_file $uuid $result
    return $res
}