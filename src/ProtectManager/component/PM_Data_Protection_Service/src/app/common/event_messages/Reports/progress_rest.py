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
from enum import Enum
from typing import List

from pydantic import BaseModel, Field

from app.common.event_messages.Common.enumerations import EnvType, ProtectedObjectType


class JobType(str, Enum):
    """
    Enum added to fit GUI mock. Job's (workflow) type.
    """

    SYSTEM = "SYSTEM"
    BACKUP = "BACKUP"
    RESTORE = "RESTORE"
    LIVE_RESTORE = "LIVE_RESTORE"
    LIVE_MOUNT = "live_mount"
    REPLICATION = "copy_replication"
    ARCHIVE = "archive"
    DELETE_COPY = "DELETE_COPY"
    DB_IDENTIFICATION = "DB_IDENTIFICATION"
    DB_DESESITIZATION = "DB_DESESITIZATION"


class JobsStatusType(str, Enum):
    running = "running"
    pending = "pending"
    succeeded = "succeeded"
    failed = "failed"
    cleaning = "cleaning"
    aborted = "aborted"
    aborting = "aborting"


class OperationType(str, Enum):
    end = "end"
    initialize_backup = "initialize_backup"
    get_chain_info = "get_chain_info"
    lock_backup_resources = "lock_backup_resources"
    do_backup = "do_backup"
    add_snap = "add_snap"
    snap_done = "snap_done"
    finalize_backup = "finalize_backup"
    start_scan = "start_scan"
    lock_scan_resources = "lock_scan_resources"
    scan_backup = "scan_backup"
    index_scan = "index_scan"
    finalize_scan = "finalize_scan"
    verify_user = "verify_user"
    verify_license = "verify_license"
    get_snap_and_dest_info = "get_snap_and_dest_info"
    lock_resources = "lock_resources"
    start_restore = "start_restore"
    lock_restore_resources = "lock_restore_resources"
    do_restore = "do_restore"
    finalize_restore = "finalize_restore"
    delete_flow_step = "delete_flow_step"
    delete_flow_progress = "delete_flow_progress"
    verify_user_and_license = "verify_user_and_license"
    get_catalog = "get_catalog"
    unlock_resources = "unlock_resources"
    delete_catalog = "delete_catalog"
    delete_index = "delete_index"
    delete_data = "delete_data"
    clean_context = "clean_context"
    delete_finish = "delete_finish"
    lock_request = "lock_request"
    unlock_request = "unlock_request"
    file_system_scan = "file_system_scan"
    file_system_index = "file_system_index"
    index_delete = "index_delete"
    catalog_get_chain_info = "catalog_get_chain_info"
    catalog_add_backup_snap = "catalog_add_backup_snap"
    catalog_delete_backup_snap_by_id = "catalog_delete_backup_snap_by_id"
    catalog_get_snap_info = "catalog_get_snap_info"
    catalog_get_backup_snap_meta_list = "catalog_get_backup_snap_meta_list"
    catalog_update_es = "catalog_update_elasticsearch"
    livemount_vm_start = "livemount_vm_start"
    livemount_vm_locked = "livemount_vm_locked"
    livemount_vm_eam_done = "livemount_vm_eam_done"
    remove_livemount_vm_start = "remove_livemount_vm_start"
    remove_livemount_vm_locked = "remove_livemount_vm_locked"
    remove_livemount_vm_eam_done = "remove_livemount_vm_eam_done"


class ProgressReporterFilterParams(BaseModel):
    """
    Model of all filter options that can be used when getting list of jobs/tasks
    """

    protected_obj_name: str = Field(
        default=None,
        description="Filter by protected object name, supports wildcard (*)",
    )
    protected_obj_type: ProtectedObjectType = Field(
        default=None, description="Filter by protected object type"
    )
    job_type: JobType = Field(default=None, description="Filter by job type")
    job_status: List[JobsStatusType] = Field(
        default=None, description="Filter by job status/es"
    )
    start_time: float = Field(
        default=None,
        description="Filter by minimal start time: UTC time, seconds from the epoch",
    )
    end_time: float = Field(
        default=None,
        description="Filter by maximum end time: UTC time, seconds from the epoch",
    )
    snapshot_start_time: float = Field(
        default=None,
        description="Filter by Snapshot's minimal start time: UTC time, seconds from the epoch",
    )
    snapshot_end_time: float = Field(
        default=None,
        description="Filter by Snapshot's maximum end time: UTC time, seconds from the epoch",
    )


class ProgressReporterRequest(BaseModel):
    """
    Model of the request sent when querying for list of jobs/tasks
    """

    startIndex: int = Field(
        ...,
        ge=0,
        description="Return entries from the entire list starting at this index",
    )
    pageSize: int = Field(
        ..., ge=1, description="Return this many items or less in the result",
    )
    filterParams: ProgressReporterFilterParams = Field(...)


class GetJobsResponse(BaseModel):
    """
    Model of the response to get_tasks(jobs) query
    """

    jobs: List[str] = Field(..., description="A list of job (request) ID's")
    startIndex: int = Field(
        default=None, ge=0, description="Index from which list's values were included",
    )
    currentCount: int = Field(
        default=None,
        ge=0,
        description="Number of jobs returned in parameter 'jobs' (list's length)",
    )
    total: int = Field(
        default=None,
        ge=0,
        description="Total expected number of results (might be larger than pageSize requested)",
    )


class ProtectedObjectInfo(BaseModel):
    """
    Details about the protected object.
    Values should be sent by the initiator of the Workflow (workflow manager)
    """

    name: str = Field(
        ..., description="Protected-object's name, for example: name of the vm"
    )
    location: str = Field(
        ..., description="An ip string, protected object's/environment's ip"
    )
    po_type: ProtectedObjectType = Field(...)
    env_type: EnvType = Field(
        default=EnvType.empty,
        description="Optional, information about the protected environment's type",
    )


class ProgressJobInfo(BaseModel):
    """
    Holds job's (task's/workflow's) progress information.
    Model is used when answering a request to get information of a specific job
    """

    job_type: JobType = Field(...)
    job_status: JobsStatusType = Field(..., description="Entire job (workflow) status")
    progress: int = Field(
        ..., ge=0, le=100, description="Represents the entire job progress by percents"
    )
    speed: float = Field(
        default=None,
        ge=0,
        description="Optional value, represents data transfer speed, in MBps. None if value can't be provided.",
    )
    start_time: float = Field(
        ..., description="Job's starting time - UTC, seconds from the epoch"
    )
    end_time: float = Field(
        default=None,
        description="Job's end time - UTC, seconds from the epoch. None if still executing.",
    )


class ProgressSnapInfo(BaseModel):
    """
    Holds information about the snapshot
    """

    snap_timestamp: float = Field(
        ..., description="Snapshot's time stamp - UTC, seconds from the epoch"
    )


class ProgressTargetInfo(BaseModel):
    """
    Information about the target environment/object -
    for example, name & ip of the environment a restore operation was made to.
    """

    target_name: str = Field(...)
    target_location: str = Field(...)


class ProgressJobInfoEvent(BaseModel):
    """
    Details of an event/step in the workflow
    """

    event_time: float = Field(..., description="UTC time, seconds from the epoch")
    event_message: str = Field(
        ..., max_length=256, description="Description of the current step (event)"
    )


class GetSingleJobInfoOut(BaseModel):
    """
    Model of the response to query 'get_task_details'
    """

    protectedObjectInfo: ProtectedObjectInfo = Field(...)
    jobInfo: ProgressJobInfo = Field(...)
    snapshotInfo: ProgressSnapInfo = Field(
        default=None,
        description="Backup Workflows: holds the created-snapshot's info. "
                    "Restore/delete Workflows: optional param - the deleted/restored snapshot's details",
    )
    targetInfo: ProgressTargetInfo = Field(default=None)
    eventLogInfo: List[ProgressJobInfoEvent] = Field(...)


class WorkflowStep(BaseModel):
    """
    Represents a step in the entire workflow execution
    """

    order: int = Field(
        ...,
        ge=0,
        description="Step order compared to other steps in the entire workflow execution",
    )
    operation: OperationType = Field(
        ...,
        description="Operation type, enum values (example: OperationType.scan_backup)",
    )
    weight: float = Field(
        ...,
        ge=0,
        description="""Weight of this step compared to the other steps in entire workflow execution.
                        Example usage:
                                WorkflowDetails that has only two WorkflowSteps:
                                a - weight=1, b - weight=3
                                the progress (in percents) reported (by Kafka events) of each is:
                                a - progress=50, b - progress=100
                                entire workflow progress will be defined as followed:
                                workflow_progress =
                                (progress(a)*a.weight + progress(b)*b.weight) / (a.weight+b.weight)
                                example value: (50*1 + 100*3) / (1+3) = 87.5%
        """,
    )


class ProgressInitInfoIn(BaseModel):
    """
    Model of a REST request sent by the Scheduler or Workflow Manager
    when workflow (job/task) is initiated.
    Contains details that will update Progress Reporter with:
    1. A 'mapping' of request_id to workflow and user-id
    2. Steps Workflow runs during its execution, including the weight of each step
    3. Protected object's details
    4. Workflow's start time
    """

    request_id: str = Field(..., description="Flow (context) ID")
    user_id: str = Field(
        ..., description="EmeiStor user's ID (of user that initiated current workflow)"
    )
    workflow_steps: List[WorkflowStep] = Field(
        ..., description="List of all steps in workflow's execution"
    )
    protected_object_info: ProtectedObjectInfo = Field(...)
    job_type: JobType = Field(...)
    start_time: float = Field(
        ..., description="Workflow's (job's) start time: UTC, seconds from the epoch"
    )
    workflow_details: str = Field(
        default=None,
        description="Description of workflow (can hold a description of this current initiation step)",
    )
    snapshot_info: ProgressSnapInfo = Field(
        default=None,
        description="Optional, relevant on restore/delete Workflows: relevant snapshot info",
    )
    target_info: ProgressTargetInfo = Field(
        default=None,
        description="Optional, relevant on restore Workflow: information about the target to restore to",
    )
