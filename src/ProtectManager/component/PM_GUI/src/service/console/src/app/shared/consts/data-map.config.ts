/*
 * This file is a part of the open-eBackup project.
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
 * If a copy of the MPL was not distributed with this file, You can obtain one at
 * http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) [2024] Huawei Technologies Co.,Ltd.
 *
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 */
import {
  AlarmColorConsts,
  ColorConsts,
  JobColorConsts,
  RunningStatusColorConsts,
  WormStatusEnum
} from './common.const';
import { CopyDataSelectionPolicy, MountTargetLocation } from './explore.const';
import {
  ApplicationType,
  PolicyAction,
  SlaType,
  VmFileReplaceStrategy
} from './protection.const';
import { FilterType, NodeType, SearchRange } from './search.const';

// 映射配置
// 后端数据值映射为视图的配置
/* 配置对象示例:
{
  // 模型_属性
  SLA_backupMode: {
    // 和枚举key类似
    FullIncremental: {
      // 后台值
      value: '1',
      // 前台国际化keyid, 文本映射
      label: 'sla_term_FullIncremental'
      // 颜色, Status组件使用此配置
      color: ColorConsts.NORMAL
    }
  }
}
*/
export const DataMap = {
  logBackupStatus: {
    enable: {
      value: '1',
      label: 'common_enable_label'
    },
    disable: {
      value: '0',
      label: 'common_disable_label'
    }
  },
  Switch_Status: {
    enable: {
      value: true,
      label: 'common_enable_label'
    },
    disable: {
      value: false,
      label: 'common_disable_label'
    }
  },
  airGapSwitchStatus: {
    enable: {
      value: true,
      label: 'common_open_label'
    },
    disable: {
      value: false,
      label: 'common_close_label'
    }
  },
  airgapStatus: {
    enable: {
      value: 'enable',
      label: 'common_enable_label',
      color: ColorConsts.NORMAL
    },
    disable: {
      value: 'disable',
      label: 'common_disable_label',
      color: ColorConsts.OFFLINE
    }
  },
  SLA_backupMode: {
    FullIncremental: {
      value: 1,
      label: 'Full+Incremental'
    },
    IncrementalForever: {
      value: 2,
      label: 'Incremental Forever'
    },
    CDP: {
      value: 3,
      label: 'CDP'
    }
  },
  resource_LinkStatus: {
    normal: {
      value: 1,
      label: 'common_online_label',
      color: ColorConsts.NORMAL
    },
    offline: {
      value: 0,
      label: 'common_off_label',
      color: ColorConsts.OFFLINE
    },
    migrating: {
      value: 2,
      label: 'common_migrating_label',
      color: ColorConsts.RUNNING
    }
  },
  gaussDBT_Resource_LinkStatus: {
    normal: {
      value: 'Normal',
      label: 'common_online_label',
      color: ColorConsts.NORMAL
    },
    offline: {
      value: 'Offline',
      label: 'common_off_label',
      color: ColorConsts.OFFLINE
    },
    abnormal: {
      value: 'Abnormal',
      label: 'common_status_exception_label',
      color: ColorConsts.ABNORMAL
    },
    degraded: {
      value: 'Degraded',
      label: 'protection_degraded_label',
      color: ColorConsts.WARN
    },
    unavailable: {
      value: 'Unavailable',
      label: 'protection_unvaliable_label',
      color: ColorConsts.OFFLINE
    }
  },
  gaussDBDWS_Resource_LinkStatus: {
    online: {
      value: '1',
      label: 'common_status_normal_label',
      color: ColorConsts.NORMAL
    },
    offline: {
      value: '0',
      label: 'common_off_label',
      color: ColorConsts.OFFLINE
    },
    unavailable: {
      value: '5',
      label: 'protection_unvaliable_label',
      color: ColorConsts.ABNORMAL
    },
    degraded: {
      value: '6',
      label: 'protection_degraded_label',
      color: ColorConsts.WARN
    },
    unstarted: {
      value: '7',
      label: 'common_status_unstarted_label',
      color: ColorConsts.OFFLINE
    }
  },
  opengauss_Clusterstate: {
    normal: {
      value: 'Normal',
      label: 'common_status_normal_label',
      color: ColorConsts.NORMAL
    },
    unavailable: {
      value: 'Unavailable',
      label: 'protection_unvaliable_label',
      color: ColorConsts.OFFLINE
    },
    degraded: {
      value: 'Degraded',
      label: 'protection_degraded_label',
      color: ColorConsts.WARN
    }
  },
  gaussDBInstance: {
    online: {
      value: '1',
      label: 'protection_backup_label',
      color: ColorConsts.NORMAL
    },
    offline: {
      value: '0',
      label: 'common_off_label',
      color: ColorConsts.OFFLINE
    },
    stopped: {
      value: '9',
      label: 'protection_unbackup_label',
      color: ColorConsts.ABNORMAL
    }
  },
  gaussDBT_Node_Status: {
    online: {
      value: 'ONLINE',
      label: 'common_online_label',
      color: ColorConsts.NORMAL
    },
    offline: {
      value: 'OFFLINE',
      label: 'common_off_label',
      color: ColorConsts.OFFLINE
    },
    stopped: {
      value: 'STOPPED',
      label: 'common_job_abort_label',
      color: ColorConsts.ABNORMAL
    }
  },
  RedisNodeStatus: {
    online: {
      value: '1',
      label: 'common_online_label',
      color: ColorConsts.NORMAL
    },
    offline: {
      value: '0',
      label: 'common_off_label',
      color: ColorConsts.OFFLINE
    },
    abnormal: {
      value: '8',
      label: 'common_status_exception_label',
      color: ColorConsts.ABNORMAL
    }
  },
  gaussDBTClusterType: {
    single: {
      value: 'GaussDBT-single',
      label: 'protection_deployment_single_label'
    },
    cluster: {
      value: 'GaussDBT',
      label: 'operation_target_cluster_label'
    }
  },
  gaussDBT_Node_Type: {
    primary: {
      value: '1',
      label: 'protection_primary_mode_label'
    },
    standby: {
      value: '2',
      label: 'protection_standby_mode_label'
    }
  },
  Dameng_Node_Type: {
    primary: {
      value: '1',
      label: 'protection_primary_mode_label'
    },
    standby: {
      value: '2',
      label: 'protection_standby_mode_label'
    }
  },
  postgre_Auth_Method_Type: {
    os: {
      value: 0,
      label: 'protection_os_id_auth_label',
      color: ColorConsts.NORMAL
    },
    database: {
      value: 2,
      label: 'protection_database_auth_label',
      color: ColorConsts.NORMAL
    }
  },
  Redis_Node_Type: {
    primary: {
      value: '1',
      label: 'protection_primary_mode_label'
    },
    standby: {
      value: '2',
      label: 'protection_standby_mode_label'
    }
  },
  redis_Auth_Method_Type: {
    os: {
      value: 0,
      label: 'protection_os_id_auth_label',
      color: ColorConsts.NORMAL
    },
    kerber: {
      value: 5,
      label: 'protection_kerberos_auth_label',
      color: ColorConsts.NORMAL
    }
  },
  clickHouse_Auth_Method_Type: {
    database: {
      value: 2,
      label: 'protection_database_auth_label',
      color: ColorConsts.NORMAL
    },
    kerber: {
      value: 5,
      label: 'protection_kerberos_auth_label',
      color: ColorConsts.NORMAL
    }
  },
  cnwareLinkStatus: {
    normal: {
      value: '1',
      label: 'common_online_label',
      color: ColorConsts.NORMAL
    },
    offline: {
      value: '0',
      label: 'common_status_exception_label',
      color: ColorConsts.ABNORMAL
    },
    shutoff: {
      value: '2',
      label: 'common_off_label',
      color: ColorConsts.OFFLINE
    },
    pause: {
      value: '3',
      label: 'common_status_pause_label',
      color: ColorConsts.OFFLINE
    }
  },
  HCS_Host_LinkStatus: {
    normal: {
      value: 'active',
      label: 'common_online_label',
      color: ColorConsts.NORMAL
    },
    offline: {
      value: 'stopped',
      label: 'common_off_label',
      color: ColorConsts.OFFLINE
    },
    shutoff: {
      value: 'shutoff',
      label: 'common_off_label',
      color: ColorConsts.OFFLINE
    },
    hardReboot: {
      value: 'hard_reboot',
      label: 'common_status_exception_label',
      color: ColorConsts.ABNORMAL
    },
    suspended: {
      value: 'suspended',
      label: 'common_host_suspend_status_label',
      color: ColorConsts.RUNNING
    },
    build: {
      value: 'build',
      label: 'common_status_exception_label',
      color: ColorConsts.ABNORMAL
    },
    deleted: {
      value: 'deleted',
      label: 'common_status_exception_label',
      color: ColorConsts.ABNORMAL
    },
    error: {
      value: 'error',
      label: 'common_status_exception_label',
      color: ColorConsts.ABNORMAL
    },
    migrating: {
      value: 'migrating',
      label: 'common_status_exception_label',
      color: ColorConsts.ABNORMAL
    },
    reboot: {
      value: 'reboot',
      label: 'common_status_exception_label',
      color: ColorConsts.ABNORMAL
    },
    resize: {
      value: 'resize',
      label: 'common_status_exception_label',
      color: ColorConsts.ABNORMAL
    },
    revertResize: {
      value: 'revert_resize',
      label: 'common_status_exception_label',
      color: ColorConsts.ABNORMAL
    },
    shelved: {
      value: 'shelved',
      label: 'common_status_exception_label',
      color: ColorConsts.ABNORMAL
    },
    shelvedOffloaded: {
      value: 'shelved_offloaded',
      label: 'common_status_exception_label',
      color: ColorConsts.ABNORMAL
    },
    unknown: {
      value: 'unknown',
      label: 'common_status_exception_label',
      color: ColorConsts.ABNORMAL
    },
    verifyResize: {
      value: 'verify_size',
      label: 'common_status_exception_label',
      color: ColorConsts.ABNORMAL
    },
    abnormal2: {
      value: 'live_volume_migrate_fail',
      label: 'common_status_exception_label',
      color: ColorConsts.ABNORMAL
    },
    abnormal3: {
      value: 'volume_migrate_fail',
      label: 'common_status_exception_label',
      color: ColorConsts.ABNORMAL
    },
    abnormal4: {
      value: 'post_live_migrate_fail',
      label: 'common_status_exception_label',
      color: ColorConsts.ABNORMAL
    },
    abnormal5: {
      value: 'finish_resize_server_failed',
      label: 'common_status_exception_label',
      color: ColorConsts.ABNORMAL
    },
    reboot1: {
      value: 'reboot',
      label: 'common_status_exception_label',
      color: ColorConsts.ABNORMAL
    },
    hardReboot2: {
      value: 'hard_reboot',
      label: 'common_status_exception_label',
      color: ColorConsts.ABNORMAL
    },
    rebuild: {
      value: 'rebuild',
      label: 'common_status_exception_label',
      color: ColorConsts.ABNORMAL
    },
    softDelete: {
      value: 'soft_delete',
      label: 'common_soft_delete_label',
      color: ColorConsts.WARN
    },
    paused: {
      value: 'paused',
      label: 'common_status_pause_label',
      color: ColorConsts.OFFLINE
    }
  },
  resource_Host_LinkStatus: {
    normal: {
      value: 1,
      label: 'common_online_label',
      color: ColorConsts.NORMAL
    },
    offline: {
      value: 0,
      label: 'common_off_label',
      color: ColorConsts.OFFLINE
    },
    upgradingQueuing: {
      value: 3,
      label: 'common_upgrading_queuing_label',
      color: ColorConsts.RUNNING
    },
    upgrading: {
      value: 4,
      label: 'common_upgrading_label',
      color: ColorConsts.RUNNING
    },
    modifyApplications: {
      value: 5,
      label: 'protection_modify_host_applications_running_label',
      color: ColorConsts.RUNNING
    }
  },
  resource_LinkStatus_Special: {
    normal: {
      value: '1',
      label: 'common_online_label',
      color: ColorConsts.NORMAL
    },
    offline: {
      value: '0',
      label: 'common_off_label',
      color: ColorConsts.OFFLINE
    }
  },
  resourceLinkStatusTenantSet: {
    online: {
      value: '1',
      label: 'common_online_label',
      color: ColorConsts.NORMAL
    },
    offline: {
      value: '0',
      label: 'common_off_label',
      color: ColorConsts.OFFLINE
    },
    partOnline: {
      value: '8',
      label: 'common_partonline_label',
      color: ColorConsts.WARN
    }
  },
  clickHouse_node_status: {
    normal: {
      value: '27',
      label: 'common_online_label',
      color: ColorConsts.NORMAL
    },
    offline: {
      value: '28',
      label: 'common_off_label',
      color: ColorConsts.OFFLINE
    }
  },
  clickHouse_cluster_status: {
    normal: {
      value: '1',
      label: 'common_online_label',
      color: ColorConsts.NORMAL
    },
    offline: {
      value: '0',
      label: 'common_off_label',
      color: ColorConsts.OFFLINE
    },
    unnormal: {
      value: '8',
      label: 'common_status_exception_label',
      color: ColorConsts.ABNORMAL
    }
  },
  openGauss_InstanceStatus: {
    normal: {
      value: 'Normal',
      label: 'common_online_label',
      color: ColorConsts.NORMAL
    },
    offline: {
      value: 'Offline',
      label: 'common_off_label',
      color: ColorConsts.OFFLINE
    }
  },
  opengauss_Role: {
    primary: {
      value: '1',
      label: 'protection_primary_mode_label'
    },
    standby: {
      value: '2',
      label: 'protection_standby_mode_label'
    }
  },

  vm_LinkStatus: {
    normal: {
      value: 1,
      label: 'protection_vm_status_on_label',
      color: ColorConsts.NORMAL
    },
    offline: {
      value: 0,
      label: 'protection_vm_status_off_label',
      color: ColorConsts.OFFLINE
    }
  },

  fcVMLinkStatus: {
    running: {
      value: 'running',
      label: 'common_running_label',
      color: ColorConsts.NORMAL
    },
    stopped: {
      value: 'stopped',
      label: 'common_job_stopped_label',
      color: ColorConsts.OFFLINE
    },
    unknown: {
      value: 'unknown',
      label: 'common_status_exception_label',
      color: ColorConsts.ABNORMAL
    },
    hibernated: {
      value: 'hibernated',
      label: 'common_status_exception_label',
      color: ColorConsts.ABNORMAL
    },
    creating: {
      value: 'creating',
      label: 'common_status_exception_label',
      color: ColorConsts.ABNORMAL
    },
    shuttingDown: {
      value: 'shutting-down',
      label: 'common_status_exception_label',
      color: ColorConsts.ABNORMAL
    },
    migrating: {
      value: 'migrating',
      label: 'common_status_exception_label',
      color: ColorConsts.ABNORMAL
    },
    faultResuming: {
      value: 'fault-resuming',
      label: 'common_status_exception_label',
      color: ColorConsts.ABNORMAL
    },
    starting: {
      value: 'starting',
      label: 'common_status_exception_label',
      color: ColorConsts.ABNORMAL
    },
    stopping: {
      value: 'stopping',
      label: 'common_status_exception_label',
      color: ColorConsts.ABNORMAL
    },
    hibernating: {
      value: 'hibernating',
      label: 'common_status_exception_label',
      color: ColorConsts.ABNORMAL
    },
    recycling: {
      value: 'recycling',
      label: 'common_status_exception_label',
      color: ColorConsts.ABNORMAL
    }
  },

  ApsaraStackStatus: {
    running: {
      value: 'Running',
      label: 'common_running_label',
      color: ColorConsts.NORMAL
    },
    starting: {
      value: 'Starting',
      label: 'common_starting_label',
      color: ColorConsts.ABNORMAL
    },
    stopping: {
      value: 'Stopping',
      label: 'common_job_aborting_label',
      color: ColorConsts.ABNORMAL
    },
    stopped: {
      value: 'Stopped',
      label: 'common_job_stopped_label',
      color: ColorConsts.OFFLINE
    }
  },

  Database_Resource_LinkStatus: {
    normal: {
      value: '0',
      label: 'common_online_label',
      color: ColorConsts.NORMAL
    },
    offline: {
      value: '1',
      label: 'common_off_label',
      color: ColorConsts.OFFLINE
    }
  },
  HCSCopyDataVerifyStatus: {
    // 未校验
    noVerify: {
      value: '0',
      label: '--'
    },
    // 校验失败
    Invalid: {
      value: '1',
      label: 'common_status_invalid_label'
    },
    // 校验成功
    available: {
      value: '2',
      label: 'common_status_available_label'
    },
    // 校验文件不存在
    noGenerate: {
      value: '3',
      label: '--'
    }
  },

  HCSCopyDataStatus: {
    Invalid: {
      value: 'Invalid',
      label: 'Invalid'
    },
    noGenerate: {
      value: 'noGenerate',
      label: 'noGenerate'
    }
  },

  copydata_validStatus: {
    normal: {
      value: 'Normal',
      label: 'common_status_normal_label',
      color: ColorConsts.NORMAL
    },
    invalid: {
      value: 'Invalid',
      label: 'common_status_invalid_label',
      color: ColorConsts.OFFLINE
    },
    deleting: {
      value: 'Deleting',
      label: 'common_status_deleting_label',
      color: ColorConsts.RUNNING
    },
    restoring: {
      value: 'Restoring',
      label: 'common_status_restoring_label',
      color: ColorConsts.RUNNING
    },
    mounting: {
      value: 'Mounting',
      label: 'common_status_mounting_label',
      color: ColorConsts.RUNNING
    },
    verifying: {
      value: 'Verifying',
      label: 'common_status_verifying_label',
      color: ColorConsts.RUNNING
    },
    unmounting: {
      value: 'Unmounting',
      label: 'common_unmounting_label',
      color: ColorConsts.RUNNING
    },
    mounted: {
      value: 'Mounted',
      label: 'common_mounted_label',
      color: ColorConsts.NORMAL
    },
    deleteFailed: {
      value: 'DeleteFailed',
      label: 'common_delete_failed_label',
      color: ColorConsts.ABNORMAL
    },
    sharing: {
      value: 'Sharing',
      label: 'common_status_sharing_label',
      color: ColorConsts.RUNNING
    },
    downloading: {
      value: 'Downloading',
      label: 'common_status_downloading_label',
      color: ColorConsts.RUNNING
    }
  },
  Verify_Status: {
    true: {
      value: true,
      label: 'common_authed_label',
      color: ColorConsts.NORMAL
    },
    false: {
      value: false,
      label: 'common_unauth_label',
      color: ColorConsts.OFFLINE
    }
  },
  Asm_Status: {
    notNeedAuth: {
      value: 0,
      label: 'protection_not_need_auth_label',
      color: ColorConsts.OFFLINE
    },
    auth: {
      value: 2,
      label: 'common_authed_label',
      color: ColorConsts.NORMAL
    },
    unauth: {
      value: 1,
      label: 'common_unauth_label',
      color: ColorConsts.OFFLINE
    }
  },
  Protection_VM_copy_structure: {
    vCenter: {
      value: 0,
      label: 'protection_vcenter_label'
    },
    host: {
      value: 1,
      label: 'common_host_label'
    },
    cluster: {
      value: 2,
      label: 'protection_cluster_label'
    },
    vm: {
      value: 3,
      label: 'common_vm_label'
    },
    files: {
      value: 4,
      label: 'common_files_label'
    }
  },
  copyDataWormStatus: {
    [WormStatusEnum.UNSET]: {
      value: WormStatusEnum.UNSET,
      label: 'explore_copy_worm_unset_label'
    },
    [WormStatusEnum.SETTING]: {
      value: WormStatusEnum.SETTING,
      label: 'explore_copy_worm_setting_label'
    },
    [WormStatusEnum.SET_SUCCESS]: {
      value: WormStatusEnum.SET_SUCCESS,
      label: 'explore_copy_worm_setted_label'
    },
    [WormStatusEnum.SET_FAILED]: {
      value: WormStatusEnum.SET_FAILED,
      label: 'explore_copy_worm_set_fail_label'
    }
  },
  CopyData_fileIndex: {
    unIndexed: {
      value: 'Unindexed',
      label: 'protection_file_no_indexed_label',
      icon: 'aui-file-unIndexed'
    },
    indexed: {
      value: 'Indexed',
      label: 'common_indexed_label',
      icon: 'aui-file-indexed'
    },
    indexing: {
      value: 'Indexing',
      label: 'common_indexing_label',
      icon: 'aui-icon-loading'
    },
    deleting: {
      value: 'Index_deleting',
      label: 'protection_index_deleting_label',
      icon: 'aui-icon-deleting'
    },
    deletedFailed: {
      value: 'Index_fail',
      label: 'protection_index_failed_label',
      icon: 'aui-icon-deleted_failed'
    }
  },
  CopyData_generatedType: {
    import: {
      value: 'CommonInterfaceBackup',
      label: 'common_common_interface_backup_label'
    },
    replicate: {
      value: 'Replicated',
      label: 'common_replication_label'
    },
    backup: {
      value: 'Backup',
      label: 'common_backup_label'
    },
    cloudArchival: {
      value: 'CloudArchive',
      label: 'common_cloud_archive_label'
    },
    tapeArchival: {
      value: 'TapeArchive',
      label: 'common_tape_archive_label'
    },
    liveMount: {
      value: 'live_mount',
      label: 'common_live_mount_label'
    },
    download: {
      value: 'Download',
      label: 'common_download_label'
    },
    snapshot: {
      value: 'Snapshot',
      label: 'common_snapshot_label'
    },
    Imported: {
      value: 'Imported',
      label: 'common_import_label'
    },
    cascadedReplication: {
      value: 'CascadedReplication',
      label: 'common_cascaded_replication_label'
    },
    reverseReplication: {
      value: 'ReverseReplication',
      label: 'common_reverse_replication_label'
    }
  },
  CopyData_copyLocation: {
    local: {
      value: 'Local',
      label: 'common_local_label'
    },
    remote: {
      value: 'Remote',
      label: 'protection_copy_remote_label'
    },
    cloud: {
      value: 'Cloud',
      label: 'common_copy_cloud_label'
    },
    Storage: {
      value: 'Storage',
      label: 'common_storage_label'
    }
  },
  copyDataSanclient: {
    yes: {
      value: 'true',
      label: 'common_yes_label'
    },
    no: {
      value: 'false',
      label: 'common_no_label'
    }
  },
  copyDataVolume: {
    enable: {
      value: true,
      label: 'common_enabled_label'
    },
    disabled: {
      value: false,
      label: 'common_disabled_label'
    }
  },
  Job_status: {
    initialization: {
      value: 'READY',
      label: 'common_ready_label',
      color: JobColorConsts.INIT
    },
    pending: {
      value: 'PENDING',
      label: 'common_pending_label',
      color: JobColorConsts.PENDING
    },
    dispatching: {
      value: 'DISPATCHING',
      label: 'common_dispatch_label',
      color: JobColorConsts.PENDING
    },
    redispatch: {
      value: 'REDISPATCH',
      label: 'common_dispatch_label',
      color: JobColorConsts.PENDING
    },
    running: {
      value: 'RUNNING',
      label: 'common_running_label',
      color: JobColorConsts.RUNNING
    },
    success: {
      value: 'SUCCESS',
      label: 'common_success_label',
      color: JobColorConsts.SUCCESSFUL
    },
    partial_success: {
      value: 'PARTIAL_SUCCESS',
      label: 'common_partial_success_label',
      color: ColorConsts.WARN
    },
    aborted: {
      value: 'ABORTED',
      label: 'common_job_stopped_label',
      color: JobColorConsts.ABORTED
    },
    aborting: {
      value: 'ABORTING',
      label: 'common_running_label',
      color: JobColorConsts.RUNNING
    },
    failed: {
      value: 'FAIL',
      label: 'common_fail_label',
      color: JobColorConsts.FAILED
    },
    abnormal: {
      value: 'ABNORMAL',
      label: 'common_fail_label',
      color: JobColorConsts.FAILED
    },
    cancelled: {
      value: 'CANCELLED',
      label: 'common_job_stopped_label',
      color: JobColorConsts.ABORTED
    },
    abort_failed: {
      value: 'ABORT_FAILED',
      label: 'common_fail_label',
      color: JobColorConsts.FAILED
    },
    dispatch_failed: {
      value: 'DISPATCH_FAILED',
      label: 'common_fail_label',
      color: JobColorConsts.FAILED
    }
  },
  markStatus: {
    notSupport: {
      value: '0',
      label: 'common_not_support_label',
      color: ColorConsts.OFFLINE
    },
    notHandled: {
      value: '1',
      label: 'common_not_handled_label',
      color: ColorConsts.ABNORMAL
    },
    handled: {
      value: '2',
      label: 'common_handled_label',
      color: ColorConsts.NORMAL
    },
    retried: {
      value: '3',
      label: 'common_retried_label',
      color: ColorConsts.NORMAL
    }
  },
  Job_type: {
    deleteUser: {
      value: 'delete_user',
      label: 'insight_delete_user_label'
    },
    host_register: {
      value: 'host_register',
      label: 'protection_client_registration_label'
    },
    protect_agent_update: {
      value: 'protect_agent_update',
      label: 'common_agent_update_label'
    },
    resource_scan: {
      value: 'resource_scan',
      label: 'common_register_label'
    },
    job_type_manual_scan_environment: {
      value: 'job_type_manual_scan_resource',
      label: 'common_rescan_label'
    },
    multi_cluster_sync: {
      value: 'multi_cluster_sync',
      label: 'common_cluster_sync_info_label'
    },
    migrate: {
      value: 'migrate',
      label: 'common_host_migrate_label'
    },
    backup_job: {
      value: 'BACKUP',
      label: 'common_backup_label'
    },
    restore_job: {
      value: 'RESTORE',
      label: 'common_recovery_label'
    },
    live_restore_job: {
      value: 'INSTANT_RESTORE',
      label: 'common_live_restore_job_label'
    },
    live_mount_job: {
      value: 'live_mount',
      label: 'common_live_mount_label'
    },
    unmout: {
      value: 'unmount',
      label: 'common_unmount_label'
    },
    copy_data_job: {
      value: 'copy_replication',
      label: 'common_replicate_label'
    },
    archive_job: {
      value: 'archive',
      label: 'common_archive_label'
    },
    archive_import_job: {
      value: 'archive_import',
      label: 'common_archive_import_label'
    },
    copies_verify_job: {
      value: 'copy_verify',
      label: 'common_copies_verification_label'
    },
    delete_copy_job: {
      value: 'COPY_DELETE',
      label: 'common_delete_copy_label'
    },
    db_identification: {
      value: 'DB_IDENTIFICATION',
      label: 'common_sens_data_id_label'
    },
    db_desesitization: {
      value: 'DB_DESESITIZATION',
      label: 'common_data_desensitization_label'
    },
    resource_protection: {
      value: 'resource_protection',
      label: 'common_resource_protection_label'
    },
    resource_protection_modify: {
      value: 'resource_protection_modify',
      label: 'common_resource_protection_modify_label'
    },
    antiRansomware: {
      value: 'anti_ransomware',
      label: 'explore_intelligent_detection_label'
    },
    copyExpired: {
      value: 'COPY_EXPIRE',
      label: 'common_expire_copy_label'
    },
    modifyApplications: {
      value: 'protect_agent_update_plugin_type',
      label: 'protection_modify_host_applications_label'
    },
    add_backup_node: {
      value: 'add_backup_member_cluster',
      label: 'system_add_backup_node_label'
    },
    delete_backup_node: {
      value: 'delete_backup_member_cluster',
      label: 'system_delete_backup_node_label'
    },
    add_ha_member: {
      value: 'add_cluster_ha',
      label: 'system_add_ha_label'
    },
    delete_ha_member: {
      value: 'delete_cluster_ha',
      label: 'system_delete_ha_label'
    },
    edit_ha_params: {
      value: 'update_cluster_ha',
      label: 'system_edit_ha_label'
    },
    upgrateBackupStorageUnit: {
      value: 'upgrade_backup_storage_unit',
      label: 'system_upgrate_backup_storage_unit_label'
    },
    exercise: {
      value: 'exercise',
      label: 'explore_job_exercise_type_label'
    },
    pushUpdate: {
      value: 'CertPushUpdate',
      label: 'system_import_internal_certificate_label'
    },
    groupBackup: {
      value: 'GROUP_BACKUP',
      label: 'explore_group_backup_label'
    }
  },
  USRE_LOCK: {
    lock: {
      value: true,
      label: 'common_locked_label'
    },
    unlock: {
      value: false,
      label: 'common_unlocked_label'
    }
  },
  ssoStatus: {
    activated: {
      value: '1',
      label: 'common_activation_label',
      color: ColorConsts.NORMAL
    },
    deactivated: {
      value: '0',
      label: 'common_deactive_label',
      color: ColorConsts.OFFLINE
    }
  },
  Resource_BaseType: {
    // 资源类型，更底层
    windows: {
      value: 'windows',
      label: 'protection_windows_label'
    },
    linux: {
      value: 'linux',
      label: 'protection_linux_label'
    },
    oracle: {
      value: 'Oracle',
      label: 'common_oracle_label'
    },
    sqlServer: {
      value: 'sql_server',
      label: 'protection_sql_server_label'
    },
    vmware: {
      value: 'vmware_vm',
      label: 'common_vmware_label'
    },
    vcenter: {
      value: 'vmware_vcenter',
      label: 'protection_vcenter_label'
    },
    fusionspereCenter: {
      value: 'fusionsphere_center',
      label: 'protection_fusionspere_center_label'
    },
    fusionspereVM: {
      value: 'fusionsphere_vm',
      label: 'protection_fusionspere_vm_label'
    },
    others: {
      value: 'other',
      label: 'common_others_label'
    }
  },
  Alarm_Severity: {
    // tslint:disable-next-line: object-literal-key-quotes
    '1': {
      value: 'warning',
      label: 'common_alarms_warning_label',
      color: AlarmColorConsts.WARNING,
      iconStr: 'warning'
    },
    // tslint:disable-next-line: object-literal-key-quotes
    '3': {
      value: 'major',
      label: 'common_alarms_major_label',
      color: AlarmColorConsts.MAJOR,
      iconStr: 'major'
    },
    // tslint:disable-next-line: object-literal-key-quotes
    '4': {
      value: 'critical',
      label: 'common_alarms_critical_label',
      color: AlarmColorConsts.CRITICAL,
      iconStr: 'critical'
    },
    // tslint:disable-next-line: object-literal-key-quotes
    '0': {
      value: 'info',
      label: 'common_alarms_info_label',
      color: AlarmColorConsts.INFO,
      iconStr: 'info'
    }
  },
  alarmNotifySeverity: {
    warning: {
      value: 'WARNING',
      label: 'common_alarms_warning_label',
      color: AlarmColorConsts.WARNING,
      iconStr: 'warning'
    },
    major: {
      value: 'MAJOR',
      label: 'common_alarms_major_label',
      color: AlarmColorConsts.MAJOR,
      iconStr: 'major'
    },
    critical: {
      value: 'CRITICAL',
      label: 'common_alarms_critical_label',
      color: AlarmColorConsts.CRITICAL,
      iconStr: 'critical'
    }
  },
  alarmNotifyLanguage: {
    chinese: {
      value: 1,
      label: 'common_chinese_label',
      isLeaf: true
    },
    english: {
      value: 2,
      label: 'common_english_label',
      isLeaf: true
    }
  },
  Report_Status: {
    running: {
      value: 'GENERATING',
      label: 'insight_report_running_label',
      color: ColorConsts.RUNNING
    },
    fail: {
      value: 'GENERATE_FAILED',
      label: 'insight_report_fail_label',
      color: ColorConsts.ABNORMAL
    },
    success: {
      value: 'GENERATE_SUCCESS',
      label: 'common_generated_label',
      color: ColorConsts.NORMAL
    }
  },
  Report_Generate_Type: {
    auto: {
      value: '0',
      label: 'common_auto_label'
    },
    manual: {
      value: '1',
      label: 'common_manual_label'
    }
  },
  Generated_Time_Unit: {
    month: {
      value: '0',
      label: 'common_months_label'
    },
    week: {
      value: '1',
      label: 'common_weeks_label'
    },
    day: {
      value: '2',
      label: 'common_days_label'
    }
  },
  Report_Generated_Period: {
    week: {
      value: 'LAST_WEEK',
      label: 'common_last_week_label'
    },
    month: {
      value: 'LAST_MONTH',
      label: 'common_last_month_label'
    },
    threeMonth: {
      value: 'LAST_THREE_MONTH',
      label: 'common_last_three_month_label'
    },
    custom: {
      value: 'CUSTOMIZATION',
      label: 'common_customize_label'
    }
  },
  Report_Format: {
    xls: {
      value: 'XLS',
      label: 'xls',
      isLeaf: true
    },
    pdf: {
      value: 'PDF',
      label: 'pdf',
      isLeaf: true
    }
  },
  reportFrequency: {
    oneDay: {
      label: '1',
      value: 'ONE_DAY'
    },
    fiveDays: {
      label: '5',
      value: 'FIVE_DAYS'
    },
    tenDays: {
      label: '10',
      value: 'TEN_DAYS'
    },
    thirtyDays: {
      label: '30',
      value: 'THIRTY_DAYS'
    }
  },
  reportGeneratedIntervalUnit: {
    day: {
      label: 'common_by_day_label',
      value: 'DAILY',
      isLeaf: true
    },
    week: {
      label: 'common_by_week_label',
      value: 'WEEKLY',
      isLeaf: true
    },
    month: {
      label: 'common_by_month_label',
      value: 'MONTHLY',
      isLeaf: true
    }
  },
  Generated_Action: {
    generate_only: {
      value: '0',
      label: 'insight_report_generated_only_label'
    },
    generate_send: {
      value: '1',
      label: 'insight_report_generated_send_label'
    }
  },
  User_Login_Status: {
    online: {
      value: 'true',
      label: 'common_online_label',
      color: ColorConsts.NORMAL
    },
    offline: {
      value: 'false',
      label: 'common_off_label',
      color: ColorConsts.OFFLINE
    }
  },
  UserLoginStatusRBAC: {
    online: {
      value: true,
      label: 'common_online_label',
      color: ColorConsts.NORMAL
    },
    offline: {
      value: false,
      label: 'common_off_label',
      color: ColorConsts.OFFLINE
    }
  },
  Archive_Storage_Type: {
    local: {
      value: 1,
      label: 'Local'
    },
    bluRay: {
      value: 4,
      label: 'Blu-ray'
    },
    s3: {
      value: 2,
      label: 'common_object_storage_label'
    }
  },
  Media_Pool_Type: {
    rw: {
      value: 'RW',
      label: 'RW'
    },
    worm: {
      value: 'WORM',
      label: 'WORM'
    },
    unknown: {
      value: 'UNKNOWN',
      label: 'common_unknown_label'
    }
  },
  Media_Pool_Status: {
    online: {
      value: 'ONLINE',
      label: 'common_online_label',
      color: ColorConsts.NORMAL
    },
    offline: {
      value: 'OFFLINE',
      label: 'common_off_label',
      color: ColorConsts.OFFLINE
    },
    partOnline: {
      value: 'PART_ONLINE',
      label: 'common_partonline_label',
      color: ColorConsts.WARN
    }
  },
  Media_Tape_Status: {
    online: {
      value: 'ONLINE',
      label: 'common_online_label',
      color: ColorConsts.NORMAL
    },
    offline: {
      value: 'OFFLINE',
      label: 'common_off_label',
      color: ColorConsts.OFFLINE
    }
  },
  Archive_Resource_Type: {
    vmware: {
      value: 1,
      label: 'VMware'
    },
    oracle: {
      value: 2,
      label: 'Oracle'
    },
    dorado: {
      value: 2,
      label: 'common_nas_file_system_label'
    },
    nas: {
      value: 3,
      label: 'NAS Share'
    }
  },
  Archive_Storage_Status: {
    online: {
      value: 0,
      label: 'common_online_label',
      color: ColorConsts.NORMAL
    },
    partOnline: {
      value: 1,
      label: 'common_partonline_label',
      color: ColorConsts.WARN
    },
    offline: {
      value: -1,
      label: 'common_off_label',
      color: ColorConsts.OFFLINE
    }
  },
  Archive_Tape_Status: {
    enable: {
      value: 'ENABLE',
      label: 'system_tape_enabled_label',
      color: ColorConsts.NORMAL
    },
    disable: {
      value: 'DISABLE',
      label: 'system_tape_disabled_label',
      color: ColorConsts.OFFLINE
    }
  },
  Tape_Slot_Type: {
    invalid: {
      value: 'INVALID',
      label: 'common_status_invalid_label'
    },
    import: {
      value: 'IMPORT_EXPORT',
      label: 'system_tape_slot_import_label'
    },
    other: {
      value: 'OTHER',
      label: 'common_others_label'
    }
  },
  Library_Tape_Status: {
    ready: {
      value: 'READY',
      label: 'system_tape_in_ready_label',
      color: ColorConsts.NORMAL
    },
    inLibrary: {
      value: 'IN_LIBRARY',
      label: 'system_tape_in_library_label',
      color: ColorConsts.NORMAL
    },
    notInLibrary: {
      value: 'NOT_IN_LIBRARY',
      label: 'system_tape_not_in_library_label',
      color: ColorConsts.OFFLINE
    },
    identifying: {
      value: 'IDENTIFYING',
      label: 'system_tape_identifying_label',
      color: ColorConsts.RUNNING
    },
    deleting: {
      value: 'DELETING',
      label: 'common_status_deleting_label',
      color: ColorConsts.WARN
    },
    importing: {
      value: 'IMPORTING',
      label: 'system_tape_importing_label',
      color: ColorConsts.RUNNING
    },
    exporting: {
      value: 'EXPORTING',
      label: 'system_tape_exporting_label',
      color: ColorConsts.RUNNING
    },
    erasing: {
      value: 'ERASING',
      label: 'system_tape_erasing_label',
      color: ColorConsts.RUNNING
    }
  },
  Tape_Write_Status: {
    unknown: {
      value: 'UNKNOWN',
      label: 'common_unknown_label',
      color: ColorConsts.OFFLINE
    },
    empty: {
      value: 'EMPTY',
      label: 'common_unuse_label',
      color: ColorConsts.NORMAL
    },
    written: {
      value: 'WRITTEN',
      label: 'system_tape_writen_label',
      color: ColorConsts.RUNNING
    },
    full: {
      value: 'FULL',
      label: 'system_tape_full_label',
      color: ColorConsts.WARN
    },
    error: {
      value: 'ERROR',
      label: 'common_error_label',
      color: ColorConsts.ABNORMAL
    }
  },
  Tape_Retention_Type: {
    immediate: {
      value: 'IMMEDIATE',
      label: 'system_tape_immediate_label'
    },
    permanent: {
      value: 'PERMANENT',
      label: 'system_tape_permanent_label'
    },
    temporary: {
      value: 'TEMPORARY',
      label: 'system_tape_temporary_label'
    }
  },
  Tape_Retention_Unit: {
    day: {
      value: 'DAY',
      label: 'common_days_label'
    },
    month: {
      value: 'MONTH',
      label: 'common_months_label'
    },
    year: {
      value: 'YEAR',
      label: 'common_years_label'
    }
  },
  Tape_Block_Size: {
    small: {
      value: 64,
      label: '64 KB'
    },
    normal: {
      value: 128,
      label: '128 KB'
    },
    large: {
      value: 256,
      label: '256 KB'
    }
  },
  Small_File_Size: {
    small: {
      value: 128,
      label: '128 KB'
    },
    normal: {
      value: 256,
      label: '256 KB'
    },
    large: {
      value: 1024,
      label: '1024 KB'
    },
    xlarge: {
      value: 4096,
      label: '4096 KB'
    }
  },
  Archive_Compression_Status: {
    enable: {
      value: 'ENABLE',
      label: 'common_enable_label',
      color: ColorConsts.NORMAL
    },
    disable: {
      value: 'DISABLE',
      label: 'common_disable_label',
      color: ColorConsts.OFFLINE
    }
  },
  Cluster_Status: {
    unknown: {
      value: 0,
      label: 'common_unknown_label',
      color: ColorConsts.OFFLINE
    },
    online: {
      value: 27,
      label: 'common_online_label',
      color: ColorConsts.NORMAL
    },
    offline: {
      value: 28,
      label: 'common_off_label',
      color: ColorConsts.OFFLINE
    },
    partOffline: {
      value: 30,
      label: 'common_part_abnormal_label',
      color: ColorConsts.WARN
    }
  },
  authType: {
    Unauthorized: {
      value: 0,
      label: 'common_storage_pool_unauthorized_label'
    },
    unitGroupAuth: {
      value: 1,
      label: 'common_storage_unit_auth_label'
    },
    unitAuth: {
      value: 2,
      label: 'common_storage_unit_group_auth_label'
    }
  },

  HealthStatus: {
    degraded: {
      value: 5,
      label: 'protection_degraded_label',
      color: ColorConsts.OFFLINE
    },
    online: {
      value: 1,
      label: 'common_status_normal_label',
      color: ColorConsts.NORMAL
    },
    abnormal: {
      value: 2,
      label: 'common_status_abnormal_label',
      color: ColorConsts.OFFLINE
    }
  },

  StoragePoolRunningStatus: {
    preCopy: {
      value: 14,
      label: 'common_pre_copy_label',
      color: RunningStatusColorConsts.otherStatus
    },
    rebuilt: {
      value: 16,
      label: 'common_rebuilt_label',
      color: RunningStatusColorConsts.otherStatus
    },
    online: {
      value: 27,
      label: 'common_online_label',
      color: ColorConsts.NORMAL
    },
    offline: {
      value: 28,
      label: 'common_off_label',
      color: ColorConsts.OFFLINE
    },
    Balancing: {
      value: 32,
      label: 'common_balancing_label',
      color: RunningStatusColorConsts.otherStatus
    },
    Initializing: {
      value: 53,
      label: 'common_initializing_label',
      color: RunningStatusColorConsts.otherStatus
    },
    Deleting: {
      value: 106,
      label: 'common_status_deleting_label',
      color: RunningStatusColorConsts.otherStatus
    }
  },

  DistributedStoragePoolRunningStatus: {
    online: {
      value: 27,
      label: 'common_online_label',
      color: ColorConsts.NORMAL
    },
    offline: {
      value: 28,
      label: 'common_off_label',
      color: ColorConsts.OFFLINE
    },
    faulty: {
      value: 1,
      label: 'common_status_abnormal_label',
      color: ColorConsts.ABNORMAL
    },
    writeProtected: {
      value: 2,
      label: 'common_write_protected_label',
      color: ColorConsts.ABNORMAL
    },
    stopped: {
      value: 3,
      label: 'common_stopped_label',
      color: ColorConsts.OFFLINE
    },
    faultyAndWriteProtected: {
      value: 4,
      label: 'common_faulty_and_write_protected_label',
      color: ColorConsts.ABNORMAL
    },
    migratingData: {
      value: 5,
      label: 'common_migrating_data_label',
      color: RunningStatusColorConsts.otherStatus
    },
    degraded: {
      value: 7,
      label: 'protection_degraded_label',
      color: RunningStatusColorConsts.otherStatus
    },
    rebuildingData: {
      value: 8,
      label: 'common_rebuilding_data_label',
      color: RunningStatusColorConsts.otherStatus
    },
    deleting: {
      value: 9,
      label: 'common_status_deleting_label',
      color: RunningStatusColorConsts.otherStatus
    },
    deleteFailed: {
      value: 10,
      label: 'common_delete_failed_label',
      color: ColorConsts.ABNORMAL
    }
  },

  DistributedClusterStatus: {
    healthy: {
      value: 0,
      label: 'common_status_healthy_label',
      color: ColorConsts.NORMAL
    },
    abnormal: {
      value: 1,
      label: 'common_status_abnormal_label',
      color: ColorConsts.OFFLINE
    }
  },

  Node_Status: {
    upgrating: {
      value: 25,
      label: 'system_upgrating_label'
    },
    online: {
      value: 27,
      label: 'common_online_label'
    },
    setting: {
      value: 26,
      label: 'system_net_plane_setting_label'
    },
    offline: {
      value: 28,
      label: 'common_off_label'
    },
    deleting: {
      value: 29,
      label: 'common_status_deleting_label'
    }
  },

  Redis_Status: {
    online: {
      value: '27',
      label: 'common_online_label',
      color: ColorConsts.NORMAL
    },
    offline: {
      value: '28',
      label: 'common_off_label',
      color: ColorConsts.OFFLINE
    }
  },
  Certificate_Status: {
    effective: {
      value: 1,
      label: 'common_valid_label',
      color: ColorConsts.NORMAL
    },
    expired: {
      value: 0,
      label: 'common_status_invalid_label',
      color: ColorConsts.OFFLINE
    },
    replacing: {
      value: 2,
      label: 'common_status_replacing_label',
      color: ColorConsts.RUNNING
    },
    validity: {
      value: true,
      label: 'common_valid_label',
      color: ColorConsts.NORMAL
    },
    invalidity: {
      value: false,
      label: 'common_status_invalid_label',
      color: ColorConsts.OFFLINE
    }
  },
  Cluster_Type: {
    local: {
      value: 1,
      label: 'system_local_cluster_label'
    },
    target: {
      value: 2,
      label: 'common_target_cluster_label'
    }
  },
  Component_Type: {
    internal: {
      value: '0',
      label: 'system_client_certificate_label'
    },
    vmware: {
      value: '1',
      label: 'VMware'
    },
    email: {
      value: '2',
      label: 'Email'
    },
    s3: {
      value: '3',
      label: 'common_object_storage_label'
    },
    a8000: {
      value: '4',
      label: 'common_cluster_type_label'
    },
    other: {
      value: '5',
      label: 'common_others_label'
    },
    externalStorage: {
      value: '6',
      label: 'system_external_storage_label'
    },
    communicationComponent: {
      value: '7',
      label: 'system_internal_communicate_component_label'
    },
    redisComponent: {
      value: '8',
      label: 'system_internal_database_component_label'
    },
    protectAgent: {
      value: '9',
      label: 'protection_protect_agent_ert_label'
    },
    ldap: {
      value: '10',
      label: 'LDAP'
    },
    hcsIam: {
      value: '11',
      label: 'HCS IAM'
    },
    ha: {
      value: '12',
      label: 'HA'
    },
    externalSystems: {
      value: '13',
      label: 'protection_external_system_label'
    },
    adfs: {
      value: '14',
      label: 'Windows ADFS'
    },
    syslog: {
      value: '15',
      label: 'Syslog'
    }
  },
  Certificate_Type: {
    unknown: {
      value: 0,
      label: 'UNKNOWN'
    },
    client: {
      value: 1,
      label: 'CLIENT'
    },
    ca: {
      value: 2,
      label: 'CA'
    },
    issued: {
      value: 3,
      label: 'ISSUED'
    }
  },
  sftpProtectionTimeUnit: {
    minute: {
      value: '44',
      label: 'common_minutes_label'
    },
    hour: {
      value: '45',
      label: 'common_hours_label'
    },
    day: {
      value: '46',
      label: 'common_days_label'
    },
    month: {
      value: '47',
      label: 'common_months_label'
    },
    year: {
      value: '48',
      label: 'common_years_label'
    }
  },
  Interval_Unit: {
    minute: {
      value: 'm',
      label: 'common_minutes_label'
    },
    hour: {
      value: 'h',
      label: 'common_hours_label'
    },
    day: {
      value: 'd',
      label: 'common_days_label'
    },
    week: {
      value: 'w',
      label: 'common_weeks_label'
    },
    month: {
      value: 'MO',
      label: 'common_months_label'
    },
    year: {
      value: 'y',
      label: 'common_years_label'
    },
    persistent: {
      value: 'p',
      label: 'common_persistent_label'
    }
  },
  recoveryDrillUnit: {
    hour: {
      value: 'h',
      label: 'common_hours_label'
    },
    day: {
      value: 'd',
      label: 'common_days_label'
    },
    month: {
      value: 'M',
      label: 'common_months_label'
    },
    year: {
      value: 'Y',
      label: 'common_years_label'
    }
  },
  Capacity_Unit: {
    kb: {
      value: 'KB',
      label: 'KB',
      convertByte: 1024
    },
    mb: {
      value: 'MB',
      label: 'MB',
      convertByte: 1024 * 1024
    },
    gb: {
      value: 'GB',
      label: 'GB',
      convertByte: 1024 * 1024 * 1024
    },
    tb: {
      value: 'TB',
      label: 'TB',
      convertByte: 1024 * 1024 * 1024 * 1024
    },
    pb: {
      value: 'PB',
      label: 'PB',
      convertByte: 1024 * 1024 * 1024 * 1024 * 1024
    }
  },
  Volume_Type: {
    standard: {
      value: 1,
      label: 'system_standard_volume_label',
      tip: 'system_standard_volume_tip_label'
    },
    meta: {
      value: 3,
      label: 'system_meta_volume_label',
      tip: 'system_meta_volume_tip_label'
    },
    self: {
      value: 5,
      label: 'system_self_volume_label',
      tip: 'system_self_volume_tip_label'
    }
  },
  Volume_Status: {
    error: {
      value: 1,
      label: 'common_status_exception_label',
      color: ColorConsts.ABNORMAL
    },
    normal: {
      value: 2,
      label: 'common_status_normal_label',
      color: ColorConsts.NORMAL
    },
    creating: {
      value: 3,
      label: 'common_status_creating_label',
      color: ColorConsts.RUNNING
    },
    deleting: {
      value: 4,
      label: 'common_status_deleting_label',
      color: ColorConsts.RUNNING
    },
    modifing: {
      value: 5,
      label: 'common_status_modifing_label',
      color: ColorConsts.RUNNING
    },
    migrating: {
      value: 6,
      label: 'common_migrating_label',
      color: ColorConsts.RUNNING
    },
    creationFailed: {
      value: 7,
      label: 'common_create_failed_label',
      color: ColorConsts.ABNORMAL
    },
    deletionFailed: {
      value: 8,
      label: 'common_delete_failed_label',
      color: ColorConsts.ABNORMAL
    }
  },
  Volume_Create_Type: {
    auto: {
      value: false,
      label: 'common_auto_label'
    },
    manual: {
      value: true,
      label: 'common_manual_label'
    }
  },
  Job_Target_Type: {
    local: {
      value: 'COMMON',
      label: 'system_local_user_label'
    },
    ldap: {
      value: 'LDAP',
      label: 'system_ldap_service_user_label'
    },
    ldapGroup: {
      value: 'LDAPGROUP',
      label: 'system_ldap_service_user_group_label'
    },
    saml: {
      value: 'SAML',
      label: 'system_saml_service_user_label'
    },
    ProtectAgent: {
      value: 'ProtectAgent',
      label: 'protection_backup_proxy_label'
    },
    DBBackupAgent: {
      value: 'DBBackupAgent',
      label: 'protection_dbbackupagent_native_label'
    },
    VMBackupAgent: {
      value: 'VMBackupAgent',
      label: 'protection_vmbackupagent_native_label'
    },
    DWSBackupAgent: {
      value: 'DWSBackupAgent',
      label: 'protection_dwsbackupagent_native_label'
    },
    UBackupAgent: {
      value: 'UBackupAgent',
      label: 'protection_ubackupagent_native_label'
    },
    SBackupAgent: {
      value: 'SBackupAgent',
      label: 'protection_sbackupagent_native_label'
    },
    drillDatabase: {
      value: 'ExerciseDefaultDatabase',
      label: 'common_database_single_label'
    },
    Fileset: {
      value: 'Fileset',
      label: 'common_fileset_label'
    },
    ActiveDirectory: {
      value: 'ADDS',
      label: 'Active Directory'
    },
    volume: {
      value: 'Volume',
      label: 'protection_volume_label'
    },
    oracle: {
      value: 'Oracle',
      label: 'common_oracle_database_label'
    },
    oracleCluster: {
      value: 'Oracle-cluster',
      label: 'common_oracle_cluster_database_label'
    },
    dbTwoCluster: {
      value: 'DB2-cluster',
      label: 'explore_db2_cluster_label'
    },
    dbTwoInstance: {
      value: 'DB2-instance',
      label: 'explore_db2_instance_label'
    },
    dbTwoClusterInstance: {
      value: 'DB2-clusterInstance',
      label: 'explore_db2_cluster_instance_label'
    },
    dbTwoDatabase: {
      value: 'DB2-database',
      label: 'explore_db2_database_label'
    },
    dbTwoTableSet: {
      value: 'DB2-tablespace',
      label: 'explore_db2_tablespace_label'
    },
    SQLServerCluster: {
      value: 'SQLServer-cluster',
      label: 'explore_sqlserver_cluster_label'
    },
    SQLServerInstance: {
      value: 'SQLServer-instance',
      label: 'explore_sqlserver_instance_label'
    },
    sqlServerClusterInstance: {
      value: 'SQLServer-clusterInstance',
      label: 'explore_sqlserver_cluster_instance_label'
    },
    SQLServerGroup: {
      value: 'SQLServer-alwaysOn',
      label: 'explore_sqlserver_group_label'
    },
    SQLServerDatabase: {
      value: 'SQLServer-database',
      label: 'explore_sqlserver_database_label'
    },
    MySQLCluster: {
      value: 'MySQL-cluster',
      label: 'explore_mysql_cluster_label'
    },
    MySQLInstance: {
      value: 'MySQL-instance',
      label: 'explore_mysql_instance_label'
    },
    MySQLClusterInstance: {
      value: 'MySQL-clusterInstance',
      label: 'explore_mysql_clusterinstance_label'
    },
    MySQLDatabase: {
      value: 'MySQL-database',
      label: 'explore_mysql_database_label'
    },
    PostgreInstance: {
      value: 'PostgreInstance',
      label: 'common_postgre_instance_label'
    },
    PostgreClusterInstance: {
      value: 'PostgreClusterInstance',
      label: 'common_postgre_cluster_instance_label'
    },
    OpenGauss: {
      value: 'OpenGauss',
      label: 'common_opengauss_cluster_label'
    },
    openGaussInstance: {
      value: 'OpenGauss-instance',
      label: 'common_opengauss_instance_label'
    },
    openGaussDatabase: {
      value: 'OpenGauss-database',
      label: 'common_opengauss_database_label'
    },
    GaussDB_T: {
      value: 'GaussDBT',
      label: 'GaussDB T'
    },
    gaussdbTSingle: {
      value: 'GaussDBT-single',
      label: 'common_gaussdbt_single_label'
    },
    dwsCluster: {
      value: 'DWS-cluster',
      label: 'explore_dws_cluster_label'
    },
    dwsSchema: {
      value: 'DWS-schema',
      label: 'explore_dws_schema_label'
    },
    dwsTable: {
      value: 'DWS-table',
      label: 'explore_dws_table_label'
    },
    Redis: {
      value: 'Redis',
      label: 'Redis'
    },
    damengCluster: {
      value: 'Dameng-cluster',
      label: 'common_dameng_cluster_label'
    },
    damengSingleNode: {
      value: 'Dameng-singleNode',
      label: 'common_dameng_single_label'
    },
    KingBaseInstance: {
      value: 'KingBaseInstance',
      label: 'common_kingbase_single_instance_label'
    },
    KingBaseClusterInstance: {
      value: 'KingBaseClusterInstance',
      label: 'common_kingbase_cluster_instance_label'
    },
    oceanBaseCluster: {
      value: 'OceanBase-cluster',
      label: 'protection_oceanbase_cluster_label'
    },
    oceanBaseTenant: {
      value: 'OceanBase-tenant',
      label: 'protection_oceanbase_tenant_label'
    },
    ClickHouse: {
      value: 'ClickHouse',
      label: 'ClickHouse'
    },
    mongodbSingleInstance: {
      value: 'MongoDB-single',
      label: 'protection_mongodb_single_instance_label'
    },
    mongodbClusterInstance: {
      value: 'MongoDB-cluster',
      label: 'protection_mongodb_cluster_instance_label'
    },
    goldendbCluter: {
      value: 'GoldenDB-cluster',
      label: 'protection_goldendb_cluster_label'
    },
    goldendbInstance: {
      value: 'GoldenDB-clusterInstance',
      label: 'protection_goldendb_instance_label'
    },
    informixService: {
      value: 'Informix-service',
      label: 'protection_informix_service_label'
    },
    informixInstance: {
      value: 'Informix-singleInstance',
      label: 'protection_informix_instance_label'
    },
    informixClusterInstance: {
      value: 'Informix-clusterInstance',
      label: 'protection_informix_cluster_instance_label'
    },
    tdsqlCluster: {
      value: 'TDSQL-cluster',
      label: 'protection_tdsql_cluster_label'
    },
    tdsqlInstance: {
      value: 'TDSQL-clusterInstance',
      label: 'protection_tdsql_non_distributed_instance_label'
    },
    tdsqlDistributedInstance: {
      value: 'TDSQL-clusterGroup',
      label: 'protection_tdsql_distributed_instance_label'
    },
    tidbCluster: {
      value: 'TiDB-cluster',
      label: 'protection_tidb_cluster_label'
    },
    tidbDatabase: {
      value: 'TiDB-database',
      label: 'protection_tidb_database_label'
    },
    tidbTable: {
      value: 'TiDB-table',
      label: 'protection_tidb_table_label'
    },
    GeneralDatabase: {
      value: 'GeneralDb',
      label: 'protection_general_database_label'
    },
    VMware: {
      value: 'VMware',
      label: 'common_vm_virtual_platform_label'
    },
    VMwarevCenterServer: {
      value: 'VMware vCenter Server',
      label: 'common_vm_virtual_platform_label'
    },
    clusterComputeResource: {
      value: 'vim.ClusterComputeResource',
      label: 'common_vm_virtual_cluster_label'
    },
    vmwareHostSystem: {
      value: 'vim.HostSystem',
      label: 'common_vm_host_system_label'
    },
    vmwareEsx: {
      value: 'VMware ESX',
      label: 'VMware ESX'
    },
    vmwareEsxi: {
      value: 'VMware ESXi',
      label: 'VMware ESXi'
    },
    vmware: {
      value: 'vim.VirtualMachine',
      label: 'common_vm_virtual_machine_label'
    },
    cNware: {
      value: 'CNware',
      label: 'common_cnware_env_label'
    },
    cNwareHostPool: {
      value: 'CNwareHostPool',
      label: 'common_cnware_host_pool_label'
    },
    cNwareCluster: {
      value: 'CNwareCluster',
      label: 'common_cnware_cluster_label'
    },
    cNwareHost: {
      value: 'CNwareHost',
      label: 'common_cnware_host_label'
    },
    cNwareVm: {
      value: 'CNwareVm',
      label: 'common_cnware_vm_label'
    },
    FusionComputePlatform: {
      value: 'Platform__and__FusionCompute',
      label: 'common_fc_platform_label'
    },
    FusionComputeCluster: {
      value: 'Cluster__and__FusionCompute',
      label: 'common_fc_cluster_label'
    },
    FusionComputeHost: {
      value: 'Host__and__FusionCompute',
      label: 'common_fc_host_label'
    },
    FusionCompute: {
      value: 'FusionCompute',
      label: 'common_fc_vm_label'
    },
    FusionOneComputePlatform: {
      value: 'Platform__and__FusionOneCompute',
      label: 'common_fo_platform_label'
    },
    FusionOneComputeCluster: {
      value: 'Cluster__and__FusionOneCompute',
      label: 'common_fo_cluster_label'
    },
    FusionOneComputeHost: {
      value: 'Host__and__FusionOneCompute',
      label: 'common_fo_host_label'
    },
    FusionOneCompute: {
      value: 'FusionOneCompute',
      label: 'common_fo_vm_label'
    },
    kubernetes: {
      value: 'Kubernetes',
      label: 'protection_kubernetes_cluster_label'
    },
    kubernetesNamespace: {
      value: 'KubernetesNamespace',
      label: 'common_kubernetes_namespace_label'
    },
    kubernetesStatefulSet: {
      value: 'KubernetesStatefulSet',
      label: 'Kubernetes FlexVolume StatefulSet'
    },
    kubernetesClusterCommon: {
      value: 'KubernetesClusterCommon',
      label: 'protection_kubernetes_container_cluster_label'
    },
    kubernetesNamespaceCommon: {
      value: 'KubernetesNamespaceCommon',
      label: 'protection_kubernetes_container_namespace_label'
    },
    kubernetesDatasetCommon: {
      value: 'KubernetesDatasetCommon',
      label: 'protection_kubernetes_container_dataset_label'
    },
    HDFSFileset: {
      value: 'HDFSFileset',
      label: 'resource_sub_type_hdfsfileset_label'
    },
    HBaseBackupSet: {
      value: 'HBaseBackupSet',
      label: 'resource_sub_type_hbase_backup_set_label'
    },
    HiveBackupSet: {
      value: 'HiveBackupSet',
      label: 'protection_hive_backup_set_label'
    },
    ElasticSearch: {
      value: 'ElasticSearchBackupSet',
      label: 'protection_elasticsearch_backupset_label'
    },
    OceanStorDorado_6_1_3: {
      value: 'DoradoV6',
      label: 'OceanStor Dorado 6.x'
    },
    OceanStor_6_1_3: {
      value: 'OceanStorV6',
      label: 'OceanStor 6.x'
    },
    OceanStor_v5: {
      value: 'OceanStorV5',
      label: 'OceanStor V5'
    },
    OceanStorPacific: {
      value: 'OceanStorPacific',
      label: 'OceanStor Pacific'
    },
    NetApp: {
      value: 'NetApp',
      label: 'NetApp ONTAP'
    },
    s3Storage: {
      value: 'S3.storage',
      label: 'common_object_storage_label'
    },
    NASFileSystem: {
      value: 'NasFileSystem',
      label: 'common_nas_file_system_label'
    },
    ndmp: {
      value: 'NDMP-BackupSet',
      label: 'protection_ndmp_protocol_label'
    },
    ndmpServer: {
      value: 'NDMP-server',
      label: 'protection_ndmp_server_label'
    },
    NASShare: {
      value: 'NasShare',
      label: 'common_nas_shared_label'
    },
    ObjectSet: {
      value: 'ObjectSet',
      label: 'protection_object_set_label'
    },
    Openstack: {
      value: 'OpenStackContainer',
      label: 'common_open_stack_label'
    },
    OpenStackProject: {
      value: 'OpenStackProject',
      label: 'protection_openstack_project_label'
    },
    OpenStackCloudServer: {
      value: 'OpenStackCloudServer',
      label: 'protection_openstack_clouhost_label'
    },
    HCSContainer: {
      value: 'HCSContainer',
      label: 'explore_hcs_cloud_container_label'
    },
    hcsEnvOp: {
      value: 'HcsEnvOp',
      label: 'protection_hcs_envop_label'
    },
    HCSTenant: {
      value: 'HCSTenant',
      label: 'explore_hcs_cloud_tenant_label'
    },
    HCSProject: {
      value: 'HCSProject',
      label: 'explore_hcs_cloud_project_label'
    },
    HCSCloudHost: {
      value: 'HCSCloudHost',
      label: 'explore_hcs_cloud_host_label'
    },
    gaussdbForOpengaussProject: {
      value: 'HCSGaussDBProject',
      label: 'protection_gaussdb_project_label'
    },
    gaussdbForOpengaussInstance: {
      value: 'HCSGaussDBInstance',
      label: 'protection_gaussdb_instance_label'
    },
    lightCloudGaussdbProject: {
      value: 'TPOPSGaussDBProject',
      label: 'protection_light_cloud_gaussdb_project_label'
    },
    lightCloudGaussdbInstance: {
      value: 'TPOPSGaussDBInstance',
      label: 'protection_light_cloud_gaussdb_instance_label'
    },
    ImportCopy: {
      value: 'ImportCopy',
      label: 'common_import_copy_label'
    },
    LocalFileSystem: {
      value: 'CloudBackupFileSystem',
      label: 'common_local_file_system_label'
    },
    LocalLun: {
      value: 'LUN',
      label: 'protection_local_lun_label'
    },
    cyberOceanStorPacific: {
      value: 'CyberEnginePacific',
      label: 'OceanStor Pacific'
    },
    OceanStorDorado: {
      value: 'CyberEngineDoradoV6',
      label: 'OceanStor Dorado'
    },
    OceanProtect: {
      value: 'CyberEngineOceanProtect',
      label: 'OceanProtect'
    },
    fileSystem: {
      value: 'FileSystem',
      label: 'common_file_system_label'
    },
    BackupMemberCluster: {
      value: 'BackMemberCluster',
      label: 'common_backup_cluster_label'
    },
    certificate: {
      value: 'Certificate',
      label: 'system_job_certificate_import_label'
    },
    commonShare: {
      value: 'CommonShare',
      label: 'protection_commonshare_label'
    },
    ExchangeSingle: {
      value: 'Exchange-single-node',
      label: 'protection_exchange_single_node_label'
    },
    ExchangeGroup: {
      value: 'Exchange-group',
      label: 'protection_exchange_avail_group_label'
    },
    ExchangeDataBase: {
      value: 'Exchange-database',
      label: 'protection_exchange_database_label'
    },
    ExchangeEmail: {
      value: 'Exchange-mailbox',
      label: 'protection_exchange_email_label'
    },
    ApsaraStack: {
      value: 'ApsaraStack',
      label: 'protection_ali_cloud_label'
    },
    APSResourceSet: {
      value: 'APS-resourceSet',
      label: 'protection_ali_resourceset_label'
    },
    APSCloudServer: {
      value: 'APS-instance',
      label: 'protection_ali_server_label'
    },
    APSZone: {
      value: 'APS-zone',
      label: 'protection_ali_zone_label'
    },
    hypervScvmm: {
      value: 'HyperV.SCVMM',
      label: 'protection_hyperv_scvmm_label'
    },
    hypervCluster: {
      value: 'HyperV.Cluster',
      label: 'protection_hyperv_cluster_label'
    },
    hypervHost: {
      value: 'HyperV.Host',
      label: 'protection_hyperv_host_label'
    },
    hypervVM: {
      value: 'HyperV.VM',
      label: 'common_hyper_vm_machine_label'
    },
    saphanaInstance: {
      value: 'SAPHANA-instance',
      label: 'protection_saphana_instance_label'
    },
    saphanaDatabase: {
      value: 'SAPHANA-database',
      label: 'protection_saphana_database_label'
    }
  },
  globalResourceType: {
    ExchangeSingle: {
      value: 'Exchange-single-node',
      label: 'protection_exchange_single_node_label'
    },
    ExchangeGroup: {
      value: 'Exchange-group',
      label: 'protection_exchange_avail_group_label'
    },
    ExchangeDataBase: {
      value: 'Exchange-database',
      label: 'protection_exchange_database_label'
    },
    ExchangeEmail: {
      value: 'Exchange-mailbox',
      label: 'protection_exchange_email_label'
    },
    APSRegion: {
      value: 'APS-region',
      label: 'protection_region_label'
    },
    APSZone: {
      value: 'APS-zone',
      label: 'protection_available_zone_label'
    },
    ApsaraStack: {
      value: 'ApsaraStack',
      label: 'protection_ali_cloud_label'
    },
    APSResourceSet: {
      value: 'APS-resourceSet',
      label: 'common_resource_set_label'
    },
    APSCloudServer: {
      value: 'APS-instance',
      label: 'common_cloud_server_label'
    },
    tdsqlNonDistributedInstance: {
      value: 'TDSQL-clusterInstance',
      label: 'protection_tdsql_non_distributed_instance_label'
    },
    tdsqlDistributedInstance: {
      value: 'TDSQL-clusterGroup',
      label: 'protection_tdsql_distributed_instance_label'
    },
    tdsqlCluster: {
      value: 'TDSQL-cluster',
      label: 'protection_tdsql_cluster_label'
    },
    ObjectSet: {
      value: 'ObjectSet',
      label: 'protection_object_set_label'
    },
    ObjectStorage: {
      value: 'ObjectStorage',
      label: 'common_object_storage_label'
    },
    tidbCluster: {
      value: 'TiDB-cluster',
      label: 'protection_tidb_cluster_label'
    },
    tidbDatabase: {
      value: 'TiDB-database',
      label: 'protection_tidb_database_label'
    },
    tidbTable: {
      value: 'TiDB-table',
      label: 'protection_tidb_table_label'
    },
    oceanBaseCluster: {
      value: 'OceanBase-cluster',
      label: 'protection_oceanbase_cluster_label'
    },
    oceanBaseTenant: {
      value: 'OceanBase-tenant',
      label: 'protection_oceanbase_tenant_label'
    },
    informixService: {
      value: 'Informix-service',
      label: 'protection_informix_service_label'
    },
    informixInstance: {
      value: 'Informix-singleInstance',
      label: 'protection_informix_instance_label'
    },
    informixClusterInstance: {
      value: 'Informix-clusterInstance',
      label: 'protection_informix_cluster_instance_label'
    },
    generalDatabase: {
      value: 'GeneralDB',
      label: 'protection_general_database_label'
    },
    dbTwoCluster: {
      value: 'DB2-cluster',
      label: 'explore_db2_cluster_label'
    },
    dbTwoInstance: {
      value: 'DB2-instance',
      label: 'explore_db2_instance_label'
    },
    dbTwoClusterInstance: {
      value: 'DB2-clusterInstance',
      label: 'explore_db2_cluster_instance_label'
    },
    dbTwoDatabase: {
      value: 'DB2-database',
      label: 'explore_db2_database_label'
    },
    dbTwoTableSet: {
      value: 'DB2-tablespace',
      label: 'explore_db2_tablespace_label'
    },
    gaussdbForOpengaussProject: {
      value: 'HCSGaussDBProject',
      label: 'protection_gaussdb_project_label'
    },
    gaussdbForOpengaussInstance: {
      value: 'HCSGaussDBInstance',
      label: 'protection_gaussdb_instance_label'
    },
    lightCloudGaussdbProject: {
      value: 'TPOPSGaussDBProject',
      label: 'protection_light_cloud_gaussdb_project_label'
    },
    lightCloudGaussdbInstance: {
      value: 'TPOPSGaussDBInstance',
      label: 'protection_light_cloud_gaussdb_instance_label'
    },
    clickhouseTableSet: {
      value: 'clickhouse-tableset',
      label: 'protection_clickhouse_tableset_label'
    },
    clickhouseCluster: {
      value: 'clickhouse-cluster',
      label: 'protection_clickhouse_cluster_label'
    },
    clickhouseDatabase: {
      value: 'clickhouse-database',
      label: 'protection_clickhouse_database_label'
    },
    clickhouseTable: {
      value: 'clickhouse-table',
      label: 'protection_clickhouse_table_label'
    },
    clickhouseNode: {
      value: 'clickhouse-node',
      label: 'protection_clickhouse_node_label'
    },
    ProtectAgent: {
      value: 'ProtectAgent',
      label: 'protection_backup_proxy_label'
    },
    DBBackupAgent: {
      value: 'DBBackupAgent',
      label: 'protection_dbbackupagent_native_label'
    },
    VMBackupAgent: {
      value: 'VMBackupAgent',
      label: 'protection_vmbackupagent_native_label'
    },
    DWSBackupAgent: {
      value: 'DWSBackupAgent',
      label: 'protection_dwsbackupagent_native_label'
    },
    UBackupAgent: {
      value: 'UBackupAgent',
      label: 'protection_ubackupagent_native_label'
    },
    Fileset: {
      value: 'Fileset',
      label: 'common_fileset_label'
    },
    volume: {
      value: 'Volume',
      label: 'protection_volume_label'
    },
    oracle: {
      value: 'Oracle',
      label: 'common_oracle_database_label'
    },
    SQLServerCluster: {
      value: 'SQLServer-cluster',
      label: 'explore_sqlserver_cluster_label'
    },
    SQLServerInstance: {
      value: 'SQLServer-instance',
      label: 'explore_sqlserver_instance_label'
    },
    sqlServerClusterInstance: {
      value: 'SQLServer-clusterInstance',
      label: 'explore_sqlserver_cluster_instance_label'
    },
    SQLServerGroup: {
      value: 'SQLServer-alwaysOn',
      label: 'explore_sqlserver_group_label'
    },
    SQLServerDatabase: {
      value: 'SQLServer-database',
      label: 'explore_sqlserver_database_label'
    },
    MySQLCluster: {
      value: 'MySQL-cluster',
      label: 'explore_mysql_cluster_label'
    },
    MySQLInstance: {
      value: 'MySQL-instance',
      label: 'explore_mysql_instance_label'
    },
    MySQLClusterInstance: {
      value: 'MySQL-clusterInstance',
      label: 'explore_mysql_clusterinstance_label'
    },
    MySQLDatabase: {
      value: 'MySQL-database',
      label: 'explore_mysql_database_label'
    },
    PostgreInstance: {
      value: 'PostgreInstance',
      label: 'common_postgre_instance_label'
    },
    PostgreClusterInstance: {
      value: 'PostgreClusterInstance',
      label: 'common_postgre_cluster_instance_label'
    },
    OpenGauss: {
      value: 'OpenGauss',
      label: 'common_opengauss_cluster_label'
    },
    openGaussInstance: {
      value: 'OpenGauss-instance',
      label: 'common_opengauss_instance_label'
    },
    openGaussDatabase: {
      value: 'OpenGauss-database',
      label: 'common_opengauss_database_label'
    },
    GaussDBT: {
      value: 'GaussDBT',
      label: 'GaussDB T'
    },
    gaussdbTSingle: {
      value: 'GaussDBT-single',
      label: 'GaussDB T'
    },
    dwsCluster: {
      value: 'DWS-cluster',
      label: 'explore_dws_cluster_label'
    },
    dwsSchema: {
      value: 'DWS-schema',
      label: 'explore_dws_schema_label'
    },
    dwsTable: {
      value: 'DWS-table',
      label: 'explore_dws_table_label'
    },
    Redis: {
      value: 'Redis',
      label: 'Redis'
    },
    damengCluster: {
      value: 'Dameng-cluster',
      label: 'common_dameng_cluster_label'
    },
    damengSingleNode: {
      value: 'Dameng-singleNode',
      label: 'common_dameng_single_label'
    },
    KingBaseInstance: {
      value: 'KingBaseInstance',
      label: 'common_kingbase_single_instance_label'
    },
    KingBaseClusterInstance: {
      value: 'KingBaseClusterInstance',
      label: 'common_kingbase_cluster_instance_label'
    },
    ClickHouse: {
      value: 'ClickHouse',
      label: 'ClickHouse'
    },
    VMware: {
      value: 'VMware',
      label: 'common_vm_virtual_platform_label'
    },
    VMwarevCenterServer: {
      value: 'VMware vCenter Server',
      label: 'common_vm_virtual_platform_label'
    },
    vmwareEsx: {
      value: 'VMware ESX',
      label: 'VMware ESX'
    },
    vmwareEsxi: {
      value: 'VMware ESXi',
      label: 'VMware ESXi'
    },
    clusterComputeResource: {
      value: 'vim.ClusterComputeResource',
      label: 'common_vm_virtual_cluster_label'
    },
    vmwareHostSystem: {
      value: 'vim.HostSystem',
      label: 'common_vm_host_system_label'
    },
    vmware: {
      value: 'vim.VirtualMachine',
      label: 'common_vm_virtual_machine_label'
    },
    cNware: {
      value: 'CNware',
      label: 'common_cnware_env_label'
    },
    cNwareHostPool: {
      value: 'CNwareHostPool',
      label: 'common_cnware_host_pool_label'
    },
    cNwareCluster: {
      value: 'CNwareCluster',
      label: 'common_cnware_cluster_label'
    },
    cNwareHost: {
      value: 'CNwareHost',
      label: 'common_cnware_host_label'
    },
    cNwareVm: {
      value: 'CNwareVm',
      label: 'common_cnware_vm_label'
    },
    FusionComputePlatform: {
      value: 'Platform__and__FusionCompute',
      label: 'common_fc_platform_label'
    },
    FusionComputeCluster: {
      value: 'Cluster__and__FusionCompute',
      label: 'common_fc_cluster_label'
    },
    FusionComputeHost: {
      value: 'Host__and__FusionCompute',
      label: 'common_fc_host_label'
    },
    FusionCompute: {
      value: 'FusionCompute',
      label: 'common_fc_vm_label'
    },
    kubernetes: {
      value: 'Kubernetes',
      label: 'protection_kubernetes_cluster_label'
    },
    kubernetesNamespace: {
      value: 'KubernetesNamespace',
      label: 'common_kubernetes_namespace_label'
    },
    kubernetesStatefulSet: {
      value: 'KubernetesStatefulSet',
      label: 'Kubernetes StatefulSet'
    },
    kubernetesClusterCommon: {
      value: 'KubernetesClusterCommon',
      label: 'protection_kubernetes_container_cluster_label'
    },
    kubernetesNamespaceCommon: {
      value: 'KubernetesNamespaceCommon',
      label: 'protection_kubernetes_container_namespace_label'
    },
    kubernetesDatasetCommon: {
      value: 'KubernetesDatasetCommon',
      label: 'protection_kubernetes_container_dataset_label'
    },
    HDFSFileset: {
      value: 'HDFSFileset',
      label: 'resource_sub_type_hdfsfileset_label'
    },
    HBaseBackupSet: {
      value: 'HBaseBackupSet',
      label: 'resource_sub_type_hbase_backup_set_label'
    },
    HiveBackupSet: {
      value: 'HiveBackupSet',
      label: 'protection_hive_backup_set_label'
    },
    ElasticSearch: {
      value: 'ElasticSearchBackupSet',
      label: 'protection_elasticsearch_backupset_label'
    },
    OceanStorDorado613: {
      value: 'DoradoV6',
      label: 'OceanStor Dorado 6.x'
    },
    OceanStor613: {
      value: 'OceanStorV6',
      label: 'OceanStor 6.x'
    },
    OceanStorV5: {
      value: 'OceanStorV5',
      label: 'OceanStor V5'
    },
    OceanStorPacific: {
      value: 'OceanStorPacific',
      label: 'OceanStor Pacific'
    },
    NetApp: {
      value: 'NetApp',
      label: 'NetApp ONTAP'
    },
    s3Storage: {
      value: 'S3.storage',
      label: 'common_object_storage_label'
    },
    NASFileSystem: {
      value: 'NasFileSystem',
      label: 'common_nas_file_system_label'
    },
    NASShare: {
      value: 'NasShare',
      label: 'common_nas_shared_label'
    },
    HCSContainer: {
      value: 'HCSContainer',
      label: 'explore_hcs_cloud_container_label'
    },
    HCSTenant: {
      value: 'HCSTenant',
      label: 'explore_hcs_cloud_tenant_label'
    },
    HCSProject: {
      value: 'HCSProject',
      label: 'explore_hcs_cloud_project_label'
    },
    HCSCloudHost: {
      value: 'HCSCloudHost',
      label: 'explore_hcs_cloud_host_label'
    },
    ImportCopy: {
      value: 'ImportCopy',
      label: 'common_import_copy_label'
    },
    LocalFileSystem: {
      value: 'CloudBackupFileSystem',
      label: 'common_local_file_system_label'
    },
    openStackContainer: {
      value: 'OpenStackContainer',
      label: 'common_open_stack_label'
    },
    openStackProject: {
      value: 'OpenStackProject',
      label: 'protection_openstack_project_label'
    },
    openStackCloudServer: {
      value: 'OpenStackCloudServer',
      label: 'protection_openstack_clouhost_label'
    },
    mongodbSingleInstance: {
      value: 'MongoDB-single',
      label: 'protection_mongodb_single_instance_label'
    },
    mongodbClusterInstance: {
      value: 'MongoDB-cluster',
      label: 'protection_mongodb_cluster_instance_label'
    },
    saphanaInstance: {
      value: 'SAPHANA-instance',
      label: 'protection_saphana_instance_label'
    },
    saphanaDatabase: {
      value: 'SAPHANA-database',
      label: 'protection_saphana_database_label'
    },
    hyperVScvmm: {
      value: 'HyperV.SCVMM',
      label: 'protection_hyperv_scvmm_label'
    },
    hyperVCluster: {
      value: 'HyperV.Cluster',
      label: 'protection_hyperv_cluster_label'
    },
    hyperVHost: {
      value: 'HyperV.Host',
      label: 'protection_hyperv_host_label'
    },
    hyperVVm: {
      value: 'HyperV.VM',
      label: 'common_hyper_vm_machine_label'
    }
  },
  Microsoft_Windows_Server_Version: {
    2012: {
      value: 'Microsoft Windows Server 2012 R2',
      label: 'Microsoft Windows Server 2012 R2'
    },
    2016: {
      value: 'Microsoft Windows Server 2016',
      label: 'Microsoft Windows Server 2016'
    },
    2019: {
      value: 'Microsoft Windows Server 2019',
      label: 'Microsoft Windows Server 2019'
    }
  },
  Os_Type: {
    windows: {
      value: 'windows',
      label: 'Windows'
    },
    linux: {
      value: 'linux',
      label: 'Linux'
    },
    unix: {
      value: 'unix',
      label: 'Unix'
    },
    redhat: {
      value: 'RedHat',
      label: 'RedHat'
    },
    oel: {
      value: 'OEL',
      label: 'OEL'
    },
    aix: {
      value: 'aix',
      label: 'AIX'
    },
    hpux: {
      value: 'hp_ux',
      label: 'HP-UX'
    },
    sunos: {
      value: 'sunos',
      label: 'SunOS'
    },
    solaris: {
      value: 'solaris',
      label: 'Solaris'
    },
    openvm: {
      value: 'open_vms',
      label: 'OpenVMS'
    }
  },
  vmwareOsType: {
    windows: {
      value: 'Windows',
      label: 'Windows'
    },
    linux: {
      value: 'Linux',
      label: 'Linux'
    },
    other: {
      value: 'Other',
      label: 'common_others_label'
    }
  },
  Disk_Status: {
    SCSI: {
      value: 'SCSI',
      label: 'SCSI'
    },
    VDB: {
      value: 'VDB',
      label: 'VBD'
    }
  },
  Disk_Mode: {
    true: {
      value: 'true',
      label: 'common_system_disk_label'
    },
    false: {
      value: 'false',
      label: 'common_data_disk_label'
    }
  },
  slaAssociateStatus: {
    associate: {
      value: true,
      label: 'common_sla_associate_label',
      color: ColorConsts.NORMAL
    },
    noAssociate: {
      value: false,
      label: 'common_sla_not_associate_label',
      color: ColorConsts.OFFLINE
    }
  },
  hostTrustworthinessStatus: {
    activated: {
      value: true,
      label: 'common_host_has_trustworthiness_label',
      color: ColorConsts.NORMAL
    },
    deactivated: {
      value: false,
      label: 'common_host_has_not_trustworthiness_label',
      color: ColorConsts.OFFLINE
    }
  },
  Sla_Status: {
    activated: {
      value: true,
      label: 'common_activation_label',
      color: ColorConsts.NORMAL
    },
    deactivated: {
      value: false,
      label: 'common_deactive_label',
      color: ColorConsts.OFFLINE
    }
  },
  Sla_Compliance: {
    true: {
      value: true,
      label: 'common_sla_compliance_y_label',
      color: ColorConsts.NORMAL
    },
    false: {
      value: false,
      label: 'common_sla_compliance_n_label',
      color: ColorConsts.OFFLINE
    }
  },
  Sla_Type: {
    gold: {
      value: 'Gold',
      label: 'Gold',
      color: '#F7E08C'
    },
    silver: {
      value: 'Silver',
      label: 'Silver',
      color: '#D4D9E6'
    },
    bronze: {
      value: 'Bronze',
      label: 'Bronze',
      color: '#FA8E5A'
    }
  },
  samlSsoProtocol: {
    version: {
      value: 'SAML2.0',
      label: 'SAML2.0'
    }
  },
  Resource_Status: {
    exist: {
      value: 'EXIST',
      label: 'common_exist_label',
      color: ColorConsts.NORMAL
    },
    notExist: {
      value: 'NOT_EXIST',
      label: 'common_not_exist_label',
      color: ColorConsts.OFFLINE
    }
  },
  Resource_Type: {
    APSRegion: {
      value: 'APS-region',
      label: 'protection_region_label'
    },
    APSZone: {
      value: 'APS-zone',
      label: 'protection_available_zone_label'
    },
    ApsaraStack: {
      value: 'ApsaraStack',
      label: 'protecion_ali_cloud_label'
    },
    APSResourceSet: {
      value: 'APS-resourceSet',
      label: 'common_resource_set_label'
    },
    APSCloudServer: {
      value: 'APS-instance',
      label: 'common_cloud_server_label'
    },
    tdsql: {
      value: 'TDSQL',
      label: 'TDSQL'
    },
    tdsqlDistributedInstance: {
      value: 'TDSQL-clusterGroup',
      label: 'TDSQL'
    },
    commonShare: {
      value: 'CommonShare',
      label: 'CommonShare'
    },
    ActiveDirectory: {
      value: 'ADDS',
      label: 'Active Directory'
    },
    ObjectStorage: {
      value: 'ObjectStorage',
      label: 'common_object_storage_label'
    },
    ObjectSet: {
      value: 'ObjectSet',
      label: 'protection_object_set_label'
    },
    tdsqlCluster: {
      value: 'TDSQL-cluster',
      label: 'TDSQL'
    },
    tdsqlInstance: {
      value: 'TDSQL-clusterInstance',
      label: 'TDSQL'
    },
    informixService: {
      value: 'Informix-service',
      label: 'protection_informix_service_label'
    },
    informixInstance: {
      value: 'Informix-singleInstance',
      label: 'protection_informix_instance_label'
    },
    informixClusterInstance: {
      value: 'Informix-clusterInstance',
      label: 'protection_informix_cluster_instance_label'
    },
    tidb: {
      value: 'TiDB',
      label: 'TiDB'
    },
    tidbCluster: {
      value: 'TiDB-cluster',
      label: 'protection_tidb_cluster_label'
    },
    tidbDatabase: {
      value: 'TiDB-database',
      label: 'protection_tidb_database_label'
    },
    tidbTable: {
      value: 'TiDB-table',
      label: 'protection_tidb_table_label'
    },
    gaussdbForOpengauss: {
      value: 'HCSGaussDBInstance',
      label: 'protection_gaussdb_for_opengauss_label'
    },
    gaussdbForOpengaussProject: {
      value: 'HCSGaussDBProject',
      label: 'protection_gaussdb_for_opengauss_label'
    },
    gaussdbForOpengaussInstance: {
      value: 'HCSGaussDBInstance',
      label: 'protection_gaussdb_for_opengauss_label'
    },
    lightCloudGaussdbProject: {
      value: 'TPOPSGaussDBProject',
      label: 'protection_gaussdb_for_opengauss_label'
    },
    lightCloudGaussdbInstance: {
      value: 'TPOPSGaussDBInstance',
      label: 'protection_gaussdb_for_opengauss_label'
    },
    generalDatabase: {
      value: 'GeneralDb',
      label: 'protection_general_database_label'
    },
    generalInstance: {
      value: 'GeneralInstance',
      label: 'protection_general_database_label'
    },
    DistributedNas: {
      value: 'DistributedNas',
      label: 'DistributedNas'
    },
    OpenGauss: {
      value: 'OpenGauss',
      label: 'openGauss'
    },
    OpenGauss_instance: {
      value: 'OpenGauss-instance',
      label: 'protection_database_instance_label'
    },
    OpenGauss_database: {
      value: 'OpenGauss-database',
      label: 'common_database_label'
    },
    FusionCompute: {
      value: 'FusionCompute',
      label: 'common_fusion_compute_label'
    },
    DBBackupAgent: {
      value: 'DBBackupAgent',
      label: 'common_host_label'
    },
    VMBackupAgent: {
      value: 'VMBackupAgent',
      label: 'common_host_label'
    },
    ABBackupClient: {
      value: 'ABBackupClient',
      label: 'common_host_label'
    },
    DWSBackupAgent: {
      value: 'DWSBackupAgent',
      label: 'common_host_label'
    },
    UBackupAgent: {
      value: 'UBackupAgent',
      label: 'protection_ubackupagent_native_label'
    },
    SBackupAgent: {
      value: 'SBackupAgent',
      label: 'protection_sbackupagent_native_label'
    },
    goldendb: {
      value: 'GoldenDB',
      label: 'protection_goldendb_label'
    },
    goldendbCluter: {
      value: 'GoldenDB-cluster',
      label: 'protection_goldendb_label'
    },
    goldendbInstance: {
      value: 'GoldenDB-clusterInstance',
      label: 'protection_goldendb_label'
    },
    fileset: {
      value: 'Fileset',
      label: 'common_fileset_label'
    },
    volume: {
      value: 'Volume',
      label: 'protection_volume_label'
    },
    oracle: {
      value: 'Oracle',
      label: 'Oracle'
    },
    oracleCluster: {
      value: 'Oracle-cluster',
      label: 'Oracle'
    },
    vmware: {
      value: 'VMware',
      label: 'VMware'
    },
    vmwareVcenterServer: {
      value: 'VMware vCenter Server',
      label: 'vCenter Server',
      icon: 'aui-icon-vCenter'
    },
    fusionComputePlatform: {
      value: 'Platform',
      label: 'common_fc_platform_label',
      icon: 'aui-icon-vCenter'
    },
    fusionComputeCNA: {
      value: 'Host',
      label: 'common_fc_host_label',
      icon: 'aui-icon-host'
    },
    fusionComputeCluster: {
      value: 'Cluster',
      label: 'common_fc_cluster_label',
      icon: 'aui-icon-cluster'
    },

    fusionComputeVirtualMachine: {
      value: 'VM',
      label: 'common_fc_vm_label',
      icon: 'aui-sla-vm'
    },
    dataCenter: {
      value: 'vim.Datacenter',
      label: 'protection_vm_datacenter_label',
      icon: 'aui-icon-dataCenter'
    },
    virtualApp: {
      value: 'vim.VirtualApp',
      label: 'vApp',
      icon: 'aui-icon-vApp'
    },
    vmwareEsx: {
      value: 'VMware ESX',
      label: 'ESX',
      icon: 'aui-icon-host'
    },
    vmwareEsxi: {
      value: 'VMware ESXi',
      label: 'ESXi',
      icon: 'aui-icon-host'
    },
    virtualMachine: {
      value: 'vim.VirtualMachine',
      label: 'common_virtual_machine_label',
      icon: 'aui-sla-vm'
    },
    cnwareVirtualMachine: {
      value: 'cnm.VirtualMachine',
      label: 'common_virtual_machine_label'
    },
    HCS: {
      value: 'HCS',
      label: 'Container',
      icon: 'aui-icon-hcs-platform'
    },
    Region: {
      value: 'Region',
      label: 'Region',
      icon: 'aui-icon-hcs-region'
    },
    Project: {
      value: 'Project',
      label: 'Project',
      icon: 'aui-icon-hcs-project'
    },
    Tenant: {
      value: 'Tenant',
      label: 'common_virtual_machine_label',
      icon: 'aui-icon-hcs-tenant'
    },
    CloudHost: {
      value: 'CloudHost',
      label: 'CloudHost',
      icon: 'aui-icon-host'
    },
    msVirtualMachine: {
      value: 'ms.VirtualMachine',
      label: 'common_virtual_machine_label',
      icon: 'aui-hyperv'
    },
    fusionSphere: {
      value: 'FusionSphere',
      label: 'FusionSphere'
    },
    hyperV: {
      value: 'Hyper-V',
      label: 'Hyper-V',
      icon: 'aui-hyperv'
    },
    hyperVScvmm: {
      value: 'HyperV.SCVMM',
      label: 'SCVMM'
    },
    hyperVCluster: {
      value: 'HyperV.Cluster',
      label: 'protection_cluster_label'
    },
    hyperVHost: {
      value: 'HyperV.Host',
      label: 'common_host_label'
    },
    hyperVVm: {
      value: 'HyperV.VM',
      label: 'common_virtual_machine_label'
    },
    clusterComputeResource: {
      value: 'vim.ClusterComputeResource',
      label: 'protection_cluster_label',
      icon: 'aui-icon-cluster'
    },
    hostSystem: {
      value: 'vim.HostSystem',
      label: 'common_vm_host_system_label',
      icon: 'aui-icon-host'
    },
    msHostSystem: {
      value: 'ms.HostSystem',
      label: 'common_hyperv_host_system_label',
      icon: 'aui-icon-host'
    },
    folder: {
      value: 'vim.Folder',
      label: 'common_directory_label',
      icon: 'aui-icon-directory'
    },
    resourcePool: {
      value: 'vim.ResourcePool',
      label: 'protection_vm_resource_pool_label',
      icon: 'aui-icon-resourcePool'
    },
    applicationGroup: {
      value: 'application',
      label: 'common_application_group_label'
    },
    openStack: {
      value: 'OpenStack',
      label: 'OpenStack'
    },
    HCSCloudHost: {
      value: 'HCSCloudHost',
      label: 'common_cloud_server_label'
    },
    HCSProject: {
      value: 'HCSProject',
      label: 'common_project_resource_label'
    },
    HCSTenant: {
      value: 'HCSTenant',
      label: 'common_tenant_label'
    },
    HCSContainer: {
      value: 'HCSContainer',
      label: 'explore_hcs_cloud_container_label'
    },
    HCSRegion: {
      value: 'HCSRegion',
      label: 'system_area_label'
    },
    MySQL: {
      value: 'MySQL',
      label: 'MySQL'
    },
    PostgreSQL: {
      value: 'PostgreSQL',
      label: 'PostgreSQL'
    },
    KingBase: {
      value: 'KingBase',
      label: 'Kingbase'
    },
    MySQLCluster: {
      value: 'MySQL-cluster',
      label: 'MySQL-cluster'
    },
    MySQLInstance: {
      value: 'MySQL-instance',
      label: 'MySQL-instance'
    },
    MySQLClusterInstance: {
      value: 'MySQL-clusterInstance',
      label: 'MySQL-clusterInstance'
    },
    MySQLClusterInstanceNode: {
      value: 'MySQL-clusterInstanceNode',
      label: 'MySQL-clusterInstanceNode'
    },
    MySQLDatabase: {
      value: 'MySQL-database',
      label: 'MySQL-database'
    },
    PostgreSQLInstance: {
      value: 'PostgreInstance',
      label: 'PostgreInstance'
    },
    PostgreSQLClusterInstance: {
      value: 'PostgreClusterInstance',
      label: 'PostgreClusterInstance'
    },
    PostgreSQLCluster: {
      value: 'PostgreCluster',
      label: 'PostgreCluster'
    },
    FusionComputePlatform: {
      value: 'Platform',
      label: 'Platform'
    },
    FusionComputeCNA: {
      value: 'Host',
      label: 'Host'
    },
    FusionComputeCluster: {
      value: 'Cluster',
      label: 'Cluster'
    },
    FusionComputeVM: {
      value: 'VM',
      label: 'VM'
    },
    Redis: {
      value: 'Redis',
      label: 'Redis'
    },
    KingBaseCluster: {
      value: 'KingBaseCluster',
      label: 'KingBaseCluster'
    },
    KingBaseInstance: {
      value: 'KingBaseInstance',
      label: 'KingBaseInstance'
    },
    KingBaseClusterInstance: {
      value: 'KingBaseClusterInstance',
      label: 'KingBaseClusterInstance'
    },
    DB2: {
      value: 'DB2',
      label: 'DB2'
    },
    dbTwoCluster: {
      value: 'DB2-cluster',
      label: 'protection_cluster_label'
    },
    dbTwoInstance: {
      value: 'DB2-instance',
      label: 'protection_database_instance_label'
    },
    dbTwoClusterInstance: {
      value: 'DB2-clusterInstance',
      label: 'protection_cluster_instance_label'
    },
    dbTwoDatabase: {
      value: 'DB2-database',
      label: 'common_database_label'
    },
    dbTwoTableSet: {
      value: 'DB2-tablespace',
      label: 'common_file_table_level_label'
    },
    SQLServer: {
      value: 'SQLServer',
      label: 'SQL Server'
    },
    SQLServerCluster: {
      value: 'SQLServer-cluster',
      label: 'SQLServer-cluster'
    },
    SQLServerClusterInstance: {
      value: 'SQLServer-clusterInstance',
      label: 'SQLServer-clusterinstance'
    },
    SQLServerInstance: {
      value: 'SQLServer-instance',
      label: 'SQLServer-instance'
    },
    SQLServerGroup: {
      value: 'SQLServer-alwaysOn',
      label: 'SQLServer-alwaysOn'
    },
    SQLServerDatabase: {
      value: 'SQLServer-database',
      label: 'SQLServer-database'
    },
    GaussDB: {
      value: 'GaussDB',
      label: 'GaussDB'
    },
    GaussDB_T: {
      value: 'GaussDBT',
      label: 'GaussDB T'
    },
    gaussdbTSingle: {
      value: 'GaussDBT-single',
      label: 'GaussDB T'
    },
    GaussDB_DWS: {
      value: 'DWS-cluster',
      label: 'common_dws_label'
    },
    DWS_Cluster: {
      value: 'DWS-cluster',
      label: 'DWS Cluster'
    },
    DWS_Database: {
      value: 'DWS-database',
      label: 'GaussDB DWS Database'
    },
    DWS_Schema: {
      value: 'DWS-schema',
      label: 'GaussDB DWS Schema'
    },
    DWS_Table: {
      value: 'DWS-table',
      label: 'GaussDB DWS Schema'
    },
    Dameng: {
      value: 'Dameng',
      label: 'Dameng'
    },
    Dameng_singleNode: {
      value: 'Dameng-singleNode',
      label: 'Dameng_singleNode'
    },
    Dameng_cluster: {
      value: 'Dameng-cluster',
      label: 'Dameng_cluster'
    },
    GBase: {
      value: 'GBase',
      label: 'GBase'
    },
    gbaseCluster: {
      value: 'Gbase-service',
      label: 'common_gbase_cluster_label'
    },
    gbaseInstance: {
      value: 'Gbase-singleInstance',
      label: 'common_gbase_cluster_instance_label'
    },
    gbaseClusterInstance: {
      value: 'Gbase-clusterInstance',
      label: 'common_gbase_single_instance_label'
    },
    Exchange: {
      value: 'Exchange',
      label: 'Exchange'
    },
    ExchangeDataBase: {
      value: 'Exchange-database',
      label: 'Exchange-database'
    },
    ExchangeEmail: {
      value: 'Exchange-mailbox',
      label: 'Exchange-mailbox'
    },
    ExchangeSingle: {
      value: 'Exchange-single-node',
      label: 'protection_deployment_single_label'
    },
    ExchangeGroup: {
      value: 'Exchange-group',
      label: 'protection_availability_group_label'
    },
    OracleApp: {
      value: 'OracleApp',
      label: 'OracleApp'
    },
    MongoDB: {
      value: 'MongoDB',
      label: 'MongoDB'
    },
    MongodbSingleInstance: {
      value: 'MongoDB-single',
      label: 'protection_single_instance_label'
    },
    MongodbClusterInstance: {
      value: 'MongoDB-cluster',
      label: 'protection_cluster_instance_label'
    },
    FusionInsight: {
      value: 'FusionInsight',
      label: 'FusionInsight'
    },
    Replica: {
      value: 'Replica',
      label: 'Replica'
    },
    SapHana: {
      value: 'SapHana',
      label: 'SAP HANA'
    },
    H3cCas: {
      value: 'H3cCas',
      label: 'H3C Cas'
    },
    NASShare: {
      value: 'NasShare',
      label: 'common_nas_shared_label'
    },
    NASFileSystem: {
      value: 'NasFileSystem',
      label: 'common_nas_file_system_label'
    },
    NasEquipment: {
      value: 'NasEquipment',
      label: 'protection_storage_device_label'
    },
    KubernetesCommon: {
      value: 'K8S-Common-dataset',
      label: 'Kubernetes-Common'
    },
    KubernetesMySQL: {
      value: 'K8S-MySQL-dataset',
      label: 'Kubernetes-MySQL'
    },
    RuleManagement: {
      value: 'RuleManagement',
      label: 'common_rule_management_label'
    },
    ImportCopy: {
      value: 'ImportCopy',
      label: 'common_import_copy_label'
    },
    HDFS: {
      value: 'HDFS',
      label: 'HDFS'
    },
    HDFSFileset: {
      value: 'HDFSFileset',
      label: 'resource_sub_type_hdfsfileset_label'
    },
    HBase: {
      value: 'HBase',
      label: 'HBase'
    },
    HBaseBackupSet: {
      value: 'HBaseBackupSet',
      label: 'protection_backup_set_label'
    },
    HBaseNameSpace: {
      value: 'HBaseNamespace',
      label: 'HBaseNameSpace'
    },
    HBaseTable: {
      value: 'HBaseTable',
      label: 'HBaseTable'
    },
    Hive: {
      value: 'Hive',
      label: 'Hive'
    },
    HiveBackupSet: {
      value: 'HiveBackupSet',
      label: 'protection_hive_backup_set_label'
    },
    LocalFileSystem: {
      value: 'CloudBackupFileSystem',
      label: 'common_local_file_system_label'
    },
    LocalLun: {
      value: 'LUN',
      label: 'protection_local_lun_label'
    },
    KubernetesNamespace: {
      value: 'KubernetesNamespace',
      label: 'Kubernetes-Namespace'
    },
    KubernetesStatefulset: {
      value: 'KubernetesStatefulSet',
      label: 'Kubernetes-Statefulset'
    },
    Kubernetes: {
      value: 'Kubernetes',
      label: 'protection_kubernetes_flexvolume_label'
    },
    kubernetesClusterCommon: {
      value: 'KubernetesClusterCommon',
      label: 'KubernetesClusterCommon'
    },
    kubernetesNamespaceCommon: {
      value: 'KubernetesNamespaceCommon',
      label: 'KubernetesNamespaceCommon'
    },
    kubernetesDatasetCommon: {
      value: 'KubernetesDatasetCommon',
      label: 'KubernetesDatasetCommon'
    },
    Elasticsearch: {
      value: 'ElasticSearch',
      label: 'ElasticSearch'
    },
    ElasticsearchBackupSet: {
      value: 'ElasticSearchBackupSet',
      label: 'ElasticSearchBackupSet'
    },
    ClickHouse: {
      value: 'ClickHouse',
      label: 'ClickHouse'
    },
    ClickHouseCluster: {
      value: 'ClickHouseCluster',
      label: 'ClickHouseCluster'
    },
    ClickHouseDatabase: {
      value: 'ClickHouseDatabase',
      label: 'ClickHouseDatabase'
    },
    ClickHouseTableset: {
      value: 'ClickHouseTableset',
      label: 'ClickHouseTableset'
    },
    stackProject: {
      value: 'StackProject',
      label: 'common_project_label'
    },
    openStackProject: {
      value: 'OpenStackProject',
      label: 'common_project_label'
    },
    openStackCloudServer: {
      value: 'OpenStackCloudServer',
      label: 'common_cloud_server_label'
    },
    OceanBase: {
      value: 'OceanBase',
      label: 'protection_oceanbase_label'
    },
    OceanBaseCluster: {
      value: 'OceanBase-cluster',
      label: 'protection_oceanbase_cluster_label'
    },
    OceanBaseTenant: {
      value: 'OceanBase-tenant',
      label: 'protection_oceanbase_tenant_label'
    },
    cNware: {
      value: 'CNware',
      label: 'common_cnware_env_label'
    },
    cNwareHostPool: {
      value: 'CNwareHostPool',
      label: 'common_cnware_host_pool_label'
    },
    cNwareCluster: {
      value: 'CNwareCluster',
      label: 'common_cnware_cluster_label'
    },
    cNwareHost: {
      value: 'CNwareHost',
      label: 'common_cnware_host_label'
    },
    cNwareVm: {
      value: 'CNwareVm',
      label: 'common_cnware_vm_label'
    },
    cNwareDisk: {
      value: 'CNwareDisk',
      label: 'common_disk_label'
    },
    cloudGroup: {
      value: 'CloudVmGroup',
      label: 'common_cnware_vm_label'
    },
    vmGroup: {
      value: 'VmGroup',
      label: 'protection_vm_group_label'
    },
    ndmp: {
      value: 'NDMP-BackupSet'
    },
    saphanaInstance: {
      value: 'SAPHANA-instance',
      label: 'protection_saphana_instance_label'
    },
    saphanaDatabase: {
      value: 'SAPHANA-database',
      label: 'protection_saphana_database_label'
    },
    fusionOne: {
      value: 'FusionOneCompute',
      label: 'protection_fusionone_label'
    }
  },
  clickHouseResourceType: {
    TableSet: {
      value: 'TableSet',
      label: 'protection_table_set_label'
    },
    Database: {
      value: 'Database',
      label: 'common_database_label'
    }
  },
  tidbResourceType: {
    cluster: {
      value: 'TiDB-cluster',
      label: 'protection_tidb_cluster_label'
    },
    database: {
      value: 'TiDB-database',
      label: 'protection_tidb_database_label'
    },
    table: {
      value: 'TiDB-table',
      label: 'protection_tidb_table_label'
    }
  },
  specialResourceType: {
    VM: {
      value: 'VM',
      label: 'common_fc_vm_label'
    },
    Host: {
      value: 'Host',
      label: 'common_fc_host_label'
    },
    Cluster: {
      value: 'Cluster',
      label: 'common_fc_cluster_label'
    },
    Platform: {
      value: 'Platform',
      label: 'common_fc_platform_label'
    }
  },
  Files_Filter_Type: {
    file: {
      value: 1,
      label: 'common_files_label'
    },
    content: {
      value: 2,
      label: 'common_directory_label'
    },
    format: {
      value: 3,
      label: 'common_format_label'
    },
    date: {
      value: 4,
      label: 'protection_date_label'
    }
  },
  Filter_Format_Type: {
    office: {
      value: 1,
      label: 'protection_format_office_label'
    },
    media: {
      value: 2,
      label: 'protection_format_media_label'
    },
    picture: {
      value: 3,
      label: 'protection_format_picture_label'
    },
    pdf: {
      value: 4,
      label: 'protection_format_pdf_label'
    },
    web: {
      value: 5,
      label: 'protection_format_web_label'
    },
    zip: {
      value: 6,
      label: 'protection_format_zip_label'
    }
  },
  Filter_Date_Type: {
    start: {
      value: 1,
      label: 'common_create_time_label'
    },
    modify: {
      value: 2,
      label: 'protection_last_modifyed_label'
    },
    interview: {
      value: 3,
      label: 'protection_interview_time_label'
    }
  },
  Anony_Rule: {
    shuffing: {
      value: 'shuffing',
      label: 'protection_shuffing_label'
    }
  },
  Application_Type: {
    LocalLun: {
      value: ApplicationType.LocalLun,
      label: 'protection_local_lun_label'
    },
    General: {
      value: ApplicationType.Common,
      label: 'common_general_label'
    },
    Fileset: {
      value: ApplicationType.Fileset,
      label: 'common_fileset_label'
    },
    volume: {
      value: ApplicationType.Volume,
      label: 'protection_volume_label'
    },
    Oracle: {
      value: ApplicationType.Oracle,
      label: 'Oracle'
    },
    db2: {
      value: ApplicationType.DB2,
      label: 'protection_db_two_label'
    },
    [ApplicationType.Vmware]: {
      value: ApplicationType.Vmware,
      label: 'VMware'
    },
    cnware: {
      value: ApplicationType.CNware,
      label: 'common_cnware_label'
    },
    ABBackupClient: {
      value: ApplicationType.Volume,
      label: 'common_host_label'
    },
    Replica: {
      value: ApplicationType.Replica,
      label: 'common_copy_a_copy_label'
    },
    HDFSFileset: {
      value: ApplicationType.HDFS,
      label: 'HDFS'
    },
    HBaseBackupSet: {
      value: ApplicationType.HBase,
      label: 'HBase'
    },
    HiveBackupSet: {
      value: ApplicationType.Hive,
      label: 'Hive'
    },
    NASShare: {
      value: ApplicationType.NASShare,
      label: 'common_nas_shared_label'
    },
    NASFileSystem: {
      value: ApplicationType.NASFileSystem,
      label: 'common_nas_file_system_label'
    },
    Ndmp: {
      value: ApplicationType.Ndmp,
      label: 'protection_ndmp_protocol_label'
    },
    ImportCopy: {
      value: ApplicationType.ImportCopy,
      label: 'common_import_copy_label'
    },
    GaussDBT: {
      value: ApplicationType.GaussDBT,
      label: 'GaussDB T'
    },
    GaussDBDWS: {
      value: ApplicationType.GaussDBDWS,
      label: 'common_dws_label'
    },
    MySQL: {
      value: ApplicationType.MySQL,
      label: 'protection_mysql_label'
    },
    Redis: {
      value: ApplicationType.Redis,
      label: 'Redis'
    },
    KingBase: {
      value: ApplicationType.KingBase,
      label: 'Kingbase'
    },
    ClickHouse: {
      value: ApplicationType.ClickHouse,
      label: 'ClickHouse'
    },
    mongodb: {
      value: ApplicationType.MongoDB,
      label: 'MongoDB'
    },
    goldendb: {
      value: ApplicationType.GoldenDB,
      label: 'protection_goldendb_label'
    },
    PostgreSQL: {
      value: ApplicationType.PostgreSQL,
      label: 'PostgreSQL'
    },
    Openstack: {
      value: ApplicationType.OpenStack,
      label: 'common_open_stack_label'
    },
    HCSCloudHost: {
      value: ApplicationType.HCSCloudHost,
      label: 'common_cloud_label'
    },
    LocalFileSystem: {
      value: ApplicationType.LocalFileSystem,
      label: 'common_local_file_system_label'
    },
    Dameng: {
      value: ApplicationType.Dameng,
      label: 'Dameng'
    },
    OpenGauss: {
      value: ApplicationType.OpenGauss,
      label: 'OpenGauss'
    },
    ElasticSearch: {
      value: ApplicationType.Elasticsearch,
      label: 'ElasticSearch'
    },
    Kubernetes: {
      value: ApplicationType.KubernetesStatefulSet,
      label: 'protection_kubernetes_flexvolume_label'
    },
    kubernetesContainer: {
      value: ApplicationType.KubernetesDatasetCommon,
      label: 'protection_kubernetes_container_label'
    },
    FusionCompute: {
      value: ApplicationType.FusionCompute,
      label: 'FusionCompute'
    },
    FusionOne: {
      value: ApplicationType.FusionOne,
      label: 'protection_fusionone_label'
    },
    SQLServer: {
      value: ApplicationType.SQLServer,
      label: 'SQL Server'
    },
    gaussdbForOpengauss: {
      value: ApplicationType.GaussDBForOpenGauss,
      label: 'protection_gaussdb_for_opengauss_label'
    },
    informix: {
      value: ApplicationType.Informix,
      label: 'Informix/GBase 8s'
    },
    lightCloudgaussdb: {
      value: ApplicationType.LightCloudGaussDB,
      label: 'protection_light_cloud_gaussdb_label'
    },
    ApsaraStack: {
      value: ApplicationType.ApsaraStack,
      label: 'protection_ali_cloud_label'
    },
    tdsql: {
      value: ApplicationType.TDSQL,
      label: 'TDSQL'
    },
    generalDatabase: {
      value: ApplicationType.GeneralDatabase,
      label: 'protection_general_database_label'
    },
    OceanBase: {
      value: ApplicationType.OceanBase,
      label: 'OceanBase'
    },
    tidb: {
      value: ApplicationType.TiDB,
      label: 'TiDB'
    },
    commonShare: {
      value: ApplicationType.CommonShare,
      label: 'protection_commonshare_label'
    },
    activeDirectory: {
      value: ApplicationType.ActiveDirectory,
      label: 'Active Directory'
    },
    objectStorage: {
      value: ApplicationType.ObjectStorage,
      label: 'common_object_storage_label'
    },
    Exchange: {
      value: ApplicationType.Exchange,
      label: 'Exchange'
    },
    hyperV: {
      value: ApplicationType.HyperV,
      label: 'common_hyperv_label'
    },
    saphana: {
      value: ApplicationType.SapHana,
      label: 'SAP HANA'
    }
  },
  Sla_Category: {
    Backup: {
      value: SlaType.Backup,
      label: 'common_backup_label'
    },
    DisasterRecovery: {
      value: SlaType.DisasterRecovery,
      label: 'protection_disater_recovery_label'
    }
  },
  Encryption_Algorithm: {
    AES56: {
      value: 'AES256',
      label: 'AES256'
    }
  },
  Incremental_Mode: {
    cumulativeIncrement: {
      value: PolicyAction.DIFFERENCE,
      label: 'common_diff_backup_label'
    },
    differenceIncrement: {
      value: PolicyAction.INCREMENT,
      label: 'common_incremental_backup_label'
    }
  },
  Global_Search_Resource_Type: {
    Fileset: {
      value: FilterType.Fileset,
      label: 'common_fileset_label'
    },
    CnwareVm: {
      value: FilterType.CnwareVm,
      label: 'common_cnware_vm_label'
    },
    HypervVm: {
      value: FilterType.HyperV,
      label: 'common_hyper_vm_machine_label'
    },
    Volume: {
      value: FilterType.Volume,
      label: 'protection_volume_label'
    },
    DfsFileset: {
      value: FilterType.DfsFileset,
      label: 'common_fileset_label'
    },
    DbbackupAgent: {
      value: FilterType.DbbackupAgent,
      label: 'common_host_label'
    },
    AbbackupClient: {
      value: FilterType.AbbackupClient,
      label: 'common_host_label'
    },
    VmbackupAgent: {
      value: FilterType.VmbackupAgent,
      label: 'common_host_label'
    },
    Oracle: {
      value: FilterType.Oracle,
      label: 'common_oracle_label'
    },
    Vcenter: {
      value: FilterType.Vcenter,
      label: 'common_vm_vcenter_server_label'
    },
    Esx: {
      value: FilterType.Esx,
      label: 'common_vm_esx_label'
    },
    Esxi: {
      value: FilterType.Esxi,
      label: 'common_vm_esxi_label'
    },
    ClusterComputeResource: {
      value: FilterType.ClusterComputeResource,
      label: 'common_vm_cluster_compute_resource_label'
    },
    HostSystem: {
      value: FilterType.HostSystem,
      label: 'common_vm_host_system_label'
    },
    Folder: {
      value: FilterType.Folder,
      label: 'common_vm_folder_label'
    },
    ResourcePool: {
      value: FilterType.ResourcePool,
      label: 'common_vm_resource_pool_label'
    },
    VirtualMachine: {
      value: FilterType.VimVirtualMachine,
      label: 'common_vm_virtual_machine_label'
    },
    VimDataCenter: {
      value: FilterType.VimDataCenter,
      label: 'common_vm_data_center_label'
    },
    VimVirtualApp: {
      value: FilterType.VimVirtualApp,
      label: 'common_vm_virtual_app_label'
    },
    LocalFileSystem: {
      value: FilterType.LocalFileSystem,
      label: 'common_local_file_system_label'
    },
    NASFileSystem: {
      value: FilterType.NasFileSystem,
      label: 'common_nas_file_system_label'
    },
    NASShare: {
      value: FilterType.NasShare,
      label: 'common_nas_shared_label'
    },
    Ndmp: {
      value: FilterType.Ndmp,
      label: 'protection_ndmp_protocol_label'
    },
    HDFSFileset: {
      value: FilterType.HDFSFileset,
      label: 'resource_sub_type_hdfsfileset_label'
    },
    fusionCompute: {
      value: FilterType.FusionCompute,
      label: 'common_fc_vm_label'
    },
    fusionOneCompute: {
      value: FilterType.FusionOneCompute,
      label: 'common_fo_vm_label'
    },
    hcsCloudhost: {
      value: FilterType.HCSCloudHost,
      label: 'explore_hcs_cloud_host_label'
    },
    ObjectStorage: {
      value: FilterType.ObjectStorage,
      label: 'common_object_storage_label'
    },
    OpenStack: {
      value: FilterType.OpenstackCloudServer,
      label: 'common_open_stack_label'
    },
    ApsaraStack: {
      value: FilterType.APSCloudServer,
      label: 'protection_ali_cloud_label'
    }
  },
  Global_Copy_Data_Type: {
    Fileset: {
      value: FilterType.Fileset,
      label: 'common_fileset_label'
    },
    VimVirtualMachine: {
      value: FilterType.VimVirtualMachine,
      label: 'common_vmware_label'
    },
    NASFileSystem: {
      value: FilterType.NasFileSystem,
      label: 'common_nas_file_system_label'
    },
    NASShare: {
      value: FilterType.NasShare,
      label: 'common_nas_shared_label'
    }
  },
  CopyData_Selection_Policy: {
    lastHour: {
      value: CopyDataSelectionPolicy.LastHour,
      label: 'common_last_one_hour_label'
    },
    lastDay: {
      value: CopyDataSelectionPolicy.LastDay,
      label: 'common_last_day_label'
    },
    lastWeek: {
      value: CopyDataSelectionPolicy.LastWeek,
      label: 'common_last_week_label'
    },
    lastMonth: {
      value: CopyDataSelectionPolicy.LastMonth,
      label: 'common_last_month_label'
    },
    latest: {
      value: CopyDataSelectionPolicy.Latest,
      label: 'explore_always_latest_label'
    }
  },
  Target_Location: {
    original: {
      value: MountTargetLocation.Original,
      label: 'common_restore_to_origin_location_label'
    },
    others: {
      value: MountTargetLocation.Others,
      label: 'common_restore_to_new_location_label'
    }
  },
  CopyData_Generation: {
    one: {
      value: 1,
      label: '1'
    },
    two: {
      value: 2,
      label: '2'
    }
  },
  LiveMount_Status: {
    ready: {
      value: 'ready',
      label: 'common_ready_label',
      color: ColorConsts.RUNNING
    },
    mounting: {
      value: 'mounting',
      label: 'common_status_mounting_label',
      color: ColorConsts.RUNNING
    },
    mountFailed: {
      value: 'mount_failed',
      label: 'common_status_mount_failed_label',
      color: ColorConsts.ABNORMAL
    },
    unmounting: {
      value: 'unmounting',
      label: 'common_unmounting_label',
      color: ColorConsts.RUNNING
    },
    migrating: {
      value: 'migrating',
      label: 'common_migrating_label',
      color: ColorConsts.RUNNING
    },
    available: {
      value: 'available',
      label: 'common_status_available_label',
      color: ColorConsts.NORMAL
    },
    invalid: {
      value: 'invalid',
      label: 'common_status_invalid_label',
      color: ColorConsts.ABNORMAL
    }
  },
  Instance_Type: {
    single: {
      value: 0,
      label: 'protection_single_instance_label'
    },
    cluster: {
      value: 1,
      label: 'protection_cluster_instance_label'
    }
  },
  Registration_Status: {
    registered: {
      value: true,
      label: 'common_registered_label',
      color: ColorConsts.NORMAL
    },
    unRegistered: {
      value: false,
      label: 'common_unregistered_label',
      color: ColorConsts.OFFLINE
    }
  },
  Database_Auth_Method: {
    os: {
      value: 1,
      label: 'protection_os_auth_label'
    },
    db: {
      value: 2,
      label: 'protection_database_auth_label'
    }
  },
  asmAuthMethod: {
    os: {
      value: 1,
      label: 'protection_os_auth_label'
    },
    asm: {
      value: 2,
      label: 'protection_asm_auth_label'
    }
  },
  oracleClusterType: {
    rac: {
      value: 'RAC',
      label: 'RAC'
    }
  },
  Detection_Whitelist_Type: {
    dir: {
      value: 'DIR',
      label: 'common_directory_label'
    },
    file: {
      value: 'FILE',
      label: 'explore_file_extension_label'
    }
  },
  whitelistType: {
    dir: {
      value: 'DIR',
      label: 'explore_directory_name_label'
    },
    file: {
      value: 'FILE',
      label: 'explore_file_name_extension_label'
    }
  },
  Dameng_Auth_Method: {
    os: {
      value: '1',
      label: 'protection_os_auth_label'
    },
    db: {
      value: '2',
      label: 'protection_database_auth_label'
    }
  },
  Postgre_Auth_Method: {
    os: {
      value: 0,
      label: 'protection_os_auth_label'
    },
    db: {
      value: 2,
      label: 'protection_database_auth_label'
    }
  },
  GBase_Version: {
    GBase8S: {
      value: '8S',
      label: 'GBase 8S'
    },
    GBase8S83: {
      value: '8.3',
      label: 'GBase 8S-8.3'
    },
    GBase8S87: {
      value: '8.7',
      label: 'GBase 8S-8.7'
    }
  },
  LiveMount_Latency: {
    zeroDotsFive: {
      value: 500,
      label: '0.5 ms'
    },
    oneDotsFive: {
      value: 1500,
      label: '1.5 ms'
    }
  },
  Storage_Status: {
    normal: {
      value: 'NORMAL',
      label: 'common_status_normal_label',
      color: ColorConsts.NORMAL
    },
    abnormal: {
      value: 'ABNORMAL',
      label: 'common_status_abnormal_label',
      color: ColorConsts.ABNORMAL
    },
    unauthenticated: {
      value: 'UNAUTHENTICATED',
      label: 'common_unauth_label',
      color: ColorConsts.OFFLINE
    }
  },
  External_Storage_Status: {
    normal: {
      value: 'NORMAL',
      label: 'common_status_normal_label',
      color: ColorConsts.NORMAL
    },
    abnormal: {
      value: 'ABNORMAL',
      label: 'common_off_label',
      color: ColorConsts.OFFLINE
    }
  },
  Model_Status: {
    active: {
      value: true,
      label: 'common_activation_label',
      color: ColorConsts.NORMAL
    },
    deactive: {
      value: false,
      label: 'common_deactive_label',
      color: ColorConsts.OFFLINE
    }
  },
  exportLogStatus: {
    generating: {
      value: 'CREATING',
      color: ColorConsts.RUNNING,
      label: 'common_export_generating_label'
    },
    success: {
      value: 'SUCCESS',
      label: 'common_success_label',
      color: ColorConsts.NORMAL
    },
    fail: {
      value: 'FAIL',
      label: 'common_fail_label',
      color: ColorConsts.ABNORMAL
    }
  },
  Desensitization_generatedType: {
    desensitized: {
      value: 'livemount',
      label: 'common_live_mount_label'
    },
    unDesensitized: {
      value: 'restore',
      label: 'common_restore_label'
    }
  },
  Identification_Status: {
    not_identified: {
      value: 'NOT_IDENTIFIED',
      label: 'explore_not_identified_label',
      color: ColorConsts.OFFLINE
    },
    identifing: {
      value: 'IDENTIFING',
      label: 'explore_identifing_label',
      color: ColorConsts.RUNNING
    },
    identified: {
      value: 'IDENTIFIED',
      label: 'explore_identified_label',
      color: ColorConsts.NORMAL
    },
    failed_identified: {
      value: 'FAILED_IDENTIFIED',
      label: 'explore_failed_identified_label',
      color: ColorConsts.ABNORMAL
    },
    stopping: {
      value: 'ABORTING',
      label: 'common_job_aborting_label',
      color: ColorConsts.RUNNING
    },
    stopped: {
      value: 'ABORTED',
      label: 'common_job_stopped_label',
      color: ColorConsts.OFFLINE
    }
  },
  Desensitization_Status: {
    not_desesitize: {
      value: 'NOT_DESESITIZE',
      label: 'explore_not_desesitize_label',
      color: ColorConsts.OFFLINE
    },
    desesitizing: {
      value: 'DESESITIZING',
      label: 'explore_desesitizing_label',
      color: ColorConsts.RUNNING
    },
    desesitized: {
      value: 'DESESITIZED',
      label: 'explore_desesitized_label',
      color: ColorConsts.NORMAL
    },
    failed_desesitized: {
      value: 'FAILED_DESESITIZED',
      label: 'explore_failed_desesitized_label',
      color: ColorConsts.ABNORMAL
    },
    stopping: {
      value: 'ABORTING',
      label: 'common_job_aborting_label',
      color: ColorConsts.RUNNING
    },
    stopped: {
      value: 'ABORTED',
      label: 'common_job_stopped_label',
      color: ColorConsts.OFFLINE
    }
  },
  Cluster_Host_Type: {
    host: {
      value: false,
      label: 'common_host_label'
    },
    cluster: {
      value: true,
      label: 'protection_cluster_label'
    }
  },
  SQLServer_Auth_Type: {
    os: {
      value: 'using_winAuth',
      label: 'protection_window_auth_label'
    },
    db: {
      value: 'using_sqlAuth',
      label: 'protection_sql_server_auth_label'
    }
  },
  Cluster_Node_Role: {
    common: {
      value: 0,
      label: 'system_node_role_common_label'
    },
    primary: {
      value: 1,
      label: 'system_node_role_primary_label'
    },
    standby: {
      value: 2,
      label: 'system_node_role_standby_label'
    }
  },
  Cluster_Node_Status: {
    unknown: {
      value: 0,
      label: 'common_unknown_label',
      color: ColorConsts.OFFLINE
    },
    normal: {
      value: 1,
      label: 'common_status_normal_label',
      color: ColorConsts.NORMAL
    },
    running: {
      value: 2,
      label: 'common_info_label',
      color: ColorConsts.RUNNING
    },
    online: {
      value: 27,
      label: 'common_online_label',
      color: ColorConsts.NORMAL
    },
    offline: {
      value: 28,
      label: 'common_off_label',
      color: ColorConsts.OFFLINE
    }
  },
  License_Activation_Status: {
    activated: {
      value: 1,
      label: 'common_activation_label',
      color: ColorConsts.NORMAL
    },
    deactivated: {
      value: 0,
      label: 'common_deactive_label',
      color: ColorConsts.OFFLINE
    },
    expired: {
      value: 2,
      label: 'common_active_expired_label',
      color: ColorConsts.OFFLINE
    },
    partiallyActivated: {
      value: 3,
      label: 'common_partially_activated_label',
      color: ColorConsts.OFFLINE
    }
  },
  License_Type: {
    soft_use: {
      value: '88036UQW',
      label: 'common_soft_use_label'
    },
    soft_capacity: {
      value: '88036UQX',
      label: 'common_soft_capacity_label'
    },
    manage_soft_capacity: {
      value: '05221063',
      label: 'common_manage_soft_capacity_label'
    },
    soft_data_manage_feature: {
      value: '88036URA',
      label: 'common_soft_data_manage_feature_label'
    },
    soft_data_security_feature: {
      value: '88036URB',
      label: 'common_soft_data_security_feature_label'
    },
    cyberEngine: {
      value: '88038MEY',
      label: 'common_cyber_engine_soft_capacity_label'
    },
    cyberEngineAllFlash: {
      value: '88038QJJ-88038QJK-88038QJL-88038QJM-88038QJN',
      label: 'common_cyber_engine_all_flash_soft_capacity_label'
    },
    cyberEngineDistributedStorage: {
      value: '88038QJP-88038QJQ-88038QJR-88038QJS-88038QJT',
      label: 'common_cyber_engine_distributed_storage_soft_capacity_label'
    },
    cyberEngineBackupStorage: {
      value: '88038QKC-88038QKD-88038QKE-88038QKF-88038QKG',
      label: 'common_cyber_engine_backup_storage_soft_capacity_label'
    }
  },
  homeLicenseType: {
    cyberEngineAllFlash: {
      value: '88038QJJ-88038QJK-88038QJL-88038QJM-88038QJN',
      label: 'common_cyber_engine_all_flash_soft_label'
    },
    cyberEngineDistributedStorage: {
      value: '88038QJP-88038QJQ-88038QJR-88038QJS-88038QJT',
      label: 'common_cyber_engine_distributed_storage_soft_label'
    },
    cyberEngineBackupStorage: {
      value: '88038QKC-88038QKD-88038QKE-88038QKF-88038QKG',
      label: 'common_backup_storage_label'
    }
  },
  LiveMount_Activation_Status: {
    activated: {
      value: 'activated',
      label: 'common_active_label',
      color: ColorConsts.NORMAL
    },
    disabled: {
      value: 'disabled',
      label: 'common_disable_label',
      color: ColorConsts.OFFLINE
    }
  },
  objectBackupLevel: {
    entire: {
      value: 'false',
      label: 'protection_system_status_copy_label'
    },
    object: {
      value: 'true',
      label: 'protection_system_status_and_object_copy_label'
    }
  },
  CopyData_Backup_Type: {
    full: {
      value: 1,
      label: 'protection_full_label'
    },
    incremental: {
      value: 2,
      label: 'protection_incremental_label'
    },
    diff: {
      value: 3,
      label: 'common_diff_label'
    },
    log: {
      value: 4,
      label: 'common_log_label'
    },
    permanent: {
      value: 5,
      label: 'protection_incremental_forever_full_label'
    },
    snapshot: {
      value: 6,
      label: 'common_snapshot_label'
    }
  },
  generalDbBackupConfig: {
    full: {
      value: 1,
      label: 'full'
    },
    incremental: {
      value: 2,
      label: 'difference_increment'
    },
    diff: {
      value: 3,
      label: 'cumulative_increment'
    },
    log: {
      value: 4,
      label: 'log'
    },
    permanent: {
      value: 5,
      label: 'permanent_increment'
    },
    snapshot: {
      value: 6,
      label: 'snapshot'
    }
  },
  specialBackUpType: {
    full: {
      value: 1,
      label: 'protection_full_label'
    },
    incremental: {
      value: 2,
      label: 'protection_incremental_forever_full_label'
    },
    diff: {
      value: 3,
      label: 'common_diff_label'
    },
    log: {
      value: 4,
      label: 'common_log_label'
    }
  },
  Archive_Trigger: {
    periodArchive: {
      value: 1,
      label: 'protection_period_archive_label'
    },
    immediatelyBackup: {
      value: 2,
      label: 'protection_backup_success_label'
    },
    archiveSpecifiedTime: {
      value: 3,
      label: 'protection_archive_specified_time_label'
    }
  },
  Scheduling_Plan: {
    immediately: {
      value: 'immediately',
      label: 'explore_detected_immediately_label'
    },
    interval: {
      value: 'interval',
      label: 'explore_detected_periodic_label'
    }
  },
  Detecting_Range: {
    last: {
      value: 1,
      label: 'explore_always_latest_label'
    },
    all: {
      value: 0,
      label: 'explore_detect_all_copy_label'
    },
    specified: {
      value: 2,
      label: 'explore_detect_after_time_copy_label'
    }
  },
  Detecting_During_Unit: {
    second: {
      value: 's',
      label: 'common_second_lower_label',
      convertSecond: 1
    },
    minute: {
      value: 'm',
      label: 'common_minute_lower_label',
      convertSecond: 60
    },
    hour: {
      value: 'h',
      label: 'common_hour_lower_label',
      convertSecond: 60 * 60
    },
    day: {
      value: 'd',
      label: 'common_day_lower_label',
      convertSecond: 60 * 60 * 24
    },
    week: {
      value: 'w',
      label: 'common_week_lower_label',
      convertSecond: 60 * 60 * 24 * 7
    },
    month: {
      value: 'M',
      label: 'common_month_lower_label',
      convertSecond: 60 * 60 * 24 * 30
    },
    year: {
      value: 'Y',
      label: 'common_year_lower_label',
      convertSecond: 60 * 60 * 24 * 365
    }
  },
  Detecting_Data_Source: {
    local: {
      value: 'local',
      label: 'explore_backup_copy_label'
    },
    external: {
      value: 'external',
      label: 'common_copy_a_copy_label'
    }
  },
  File_Extension_Type: {
    custom: {
      value: 'CUSTOM',
      label: 'common_customize_label'
    },
    preset: {
      value: 'PRESET',
      label: 'explore_preset_mode_label'
    }
  },
  WhiteList_Type: {
    dir: {
      value: 'DIR',
      label: 'common_directory_label'
    },
    file: {
      value: 'FILE',
      label: 'common_files_label'
    }
  },
  File_Extension_Status: {
    disable: {
      value: 'DISABLED',
      label: 'system_tape_disabled_label',
      color: ColorConsts.OFFLINE
    },
    enable: {
      value: 'ENABLED',
      label: 'system_tape_enabled_label',
      color: ColorConsts.NORMAL
    }
  },
  fileHoneypotStatus: {
    disable: {
      value: 'DISABLED',
      label: 'system_tape_disabled_label',
      color: ColorConsts.OFFLINE
    },
    enable: {
      value: 'ENABLED',
      label: 'system_tape_enabled_label',
      color: ColorConsts.NORMAL
    },
    deploy: {
      value: 'DEPLOYING',
      label: 'explore_honeypot_deploy_label',
      color: ColorConsts.RUNNING
    },
    error: {
      value: 'ERROR',
      label: 'common_status_exception_label',
      color: ColorConsts.ABNORMAL
    },
    close: {
      value: 'CLOSING',
      label: 'explore_honeypot_close_label',
      color: ColorConsts.RUNNING
    }
  },
  fileHoneypotErrorStatus: {
    exceedLimit: {
      value: 'EXCEED_LIMIT',
      label: 'explore_honeypot_exceed_limit_label'
    },
    deleted: {
      value: 'FS_NOT_EXIST',
      label: 'explore_file_system_not_exist_label'
    },
    error: {
      value: 'ERROR',
      label: 'system_no_permission_desc_label'
    }
  },
  File_Extension_Import_Status: {
    init: {
      value: 'INIT',
      label: 'explore_file_extension_init_label',
      color: ColorConsts.NORMAL
    },
    apply: {
      value: 'APPLY',
      label: 'explore_file_extension_apply_label',
      color: ColorConsts.NORMAL
    },
    create: {
      value: 'CREATE',
      label: 'explore_file_extension_create_label',
      color: ColorConsts.RUNNING
    },
    delete: {
      value: 'DELETE',
      label: 'explore_file_extension_delete_label',
      color: ColorConsts.WARN
    },
    cancel: {
      value: 'CANCEL',
      label: 'explore_file_extension_cancel_label',
      color: ColorConsts.WARN
    },
    error: {
      value: 'ERROR',
      label: 'common_status_exception_label',
      color: ColorConsts.ABNORMAL
    },
    deleteError: {
      value: 'DELETE_ERROR',
      label: 'common_status_exception_label',
      color: ColorConsts.ABNORMAL
    }
  },
  Detecting_Resource_Type: {
    virtualMachine: {
      value: 'vim.VirtualMachine',
      label: 'common_vm_virtual_machine_label'
    },
    fusionComputeVm: {
      value: 'FusionCompute',
      label: 'common_fc_vm_label'
    },
    fusionOneComputeVm: {
      value: 'FusionOneCompute',
      label: 'common_fo_vm_label'
    },
    cNwareVm: {
      value: 'CNwareVm',
      label: 'common_cnware_vm_label'
    },
    hypervVm: {
      value: 'HyperV.VM',
      label: 'common_hyper_vm_machine_label'
    },
    hcsContainer: {
      value: 'HCSCloudHost',
      label: 'explore_hcs_cloud_container_label'
    },
    openstackServer: {
      value: 'OpenStackCloudServer',
      label: 'protection_openstack_clouhost_label'
    },
    nasFileSystem: {
      value: 'NasFileSystem',
      label: 'common_nas_file_system_label'
    },
    nasShare: {
      value: 'NasShare',
      label: 'common_nas_shared_label'
    },
    fileset: {
      value: 'Fileset',
      label: 'common_fileset_label'
    }
  },
  Detection_Copy_Status: {
    all: {
      value: -2,
      label: 'common_all_label',
      color: ColorConsts.NORMAL
    },
    uninspected: {
      value: -1,
      label: 'explore_uninspected_label',
      color: ColorConsts.OFFLINE
    },
    ready: {
      value: 0,
      label: 'common_pending_label',
      color: JobColorConsts.PENDING
    },
    detecting: {
      value: 1,
      label: 'explore_detecting_label',
      color: ColorConsts.RUNNING
    },
    uninfected: {
      value: 2,
      label: 'explore_uninfected_label',
      color: ColorConsts.NORMAL,
      resultIcon: 'aui-icon-detection-uninfected'
    },
    infected: {
      value: 3,
      label: 'explore_infected_label',
      color: ColorConsts.ABNORMAL,
      resultIcon: 'aui-icon-detection-infected'
    },
    exception: {
      value: 4,
      label: 'common_status_exception_label',
      color: ColorConsts.WARN,
      resultIcon: 'aui-icon-detection-abnormal'
    }
  },
  Detection_Copy_Type: {
    uninspected: {
      value: 'uninspected_copy_num',
      label: 'explore_uninspected_label',
      isLeaf: true,
      color: ColorConsts.OFFLINE
    },
    ready: {
      value: 'prepare_copy_num',
      label: 'common_ready_label',
      isLeaf: true,
      color: JobColorConsts.PENDING
    },
    detecting: {
      value: 'detecting_copy_num',
      label: 'explore_detecting_label',
      isLeaf: true,
      color: ColorConsts.RUNNING
    },
    uninfected: {
      value: 'uninfected_copy_num',
      label: 'explore_uninfected_label',
      color: ColorConsts.NORMAL,
      isLeaf: true,
      resultIcon: 'aui-icon-detection-uninfected'
    },
    infected: {
      value: 'infected_copy_num',
      label: 'explore_infected_label',
      color: ColorConsts.ABNORMAL,
      isLeaf: true,
      resultIcon: 'aui-icon-detection-infected'
    },
    exception: {
      value: 'abnormal_copy_num',
      label: 'common_status_exception_label',
      color: ColorConsts.WARN,
      isLeaf: true,
      resultIcon: 'aui-icon-detection-abnormal'
    }
  },
  Detecion_Data_Time_Options: {
    week: {
      label: 'common_last_week_label',
      value: 'week',
      isLeaf: true
    },
    month: {
      label: 'common_last_month_label',
      value: 'month',
      isLeaf: true
    },
    halfYear: {
      label: 'common_last_half_year_label',
      value: 'half-year',
      isLeaf: true
    }
  },
  snapshotGeneratedBy: {
    backup: {
      value: 'Backup',
      label: 'explore_data_backup_label'
    }
  },
  Archive_Scope: {
    latest: {
      value: 'latest',
      label: 'protection_archive_latest_label'
    },
    allNoArchiving: {
      value: 'all_no_archiving',
      label: 'protection_archive_copies_label'
    }
  },
  IP_Type: {
    ipv4: {
      value: 'IPV4',
      label: 'IPv4'
    },
    ipv6: {
      value: 'IPV6',
      label: 'IPv6'
    }
  },
  Storage_Cloud_Platform: {
    pacific: {
      value: 0,
      label: 'OceanStor Pacific'
    },
    fusionstorage: {
      value: 1,
      label: 'OceanStor 100D (FusionStorage OBS)'
    },
    obs: {
      value: 3,
      label: 'system_huawei_cloud_obs_label'
    },
    aws: {
      value: 4,
      label: 'AWS S3'
    },
    azure: {
      value: 5,
      label: 'Microsoft Azure Blob Storage'
    }
  },
  azureLinkMode: {
    connection: {
      value: 1,
      label: 'system_azure_connection_string_label'
    },
    standard: {
      value: 0,
      label: 'system_azure_standard_mode_label'
    }
  },
  Cluster_Recovery_Mode: {
    recovery: {
      value: '1',
      label: 'recovery'
    },
    norecovery: {
      value: '0',
      label: 'norecovery'
    }
  },
  Desensitization_Rule_Type: {
    shuffling: {
      value: 'Shuffling',
      label: 'Shuffling',
      desc: 'explore_anonymization_shuffling_label'
    },
    fullMask: {
      value: 'Full-Mask',
      label: 'Full-Mask',
      desc: 'explore_anonymization_full_mask_label'
    },
    partialMask: {
      value: 'Partial-Mask',
      label: 'Partial-Mask',
      desc: 'explore_anonymization_partial_mask_label'
    },
    piitype: {
      value: 'PII-Type',
      label: 'PII-Type',
      desc: 'explore_anonymization_pii_type_label'
    },
    numberic: {
      value: 'Numeric-Range',
      label: 'Numeric-Range',
      desc: 'explore_anonymization_number_range_label'
    },
    fixedNumber: {
      value: 'Fixed-Number',
      label: 'Fixed-Number',
      desc: 'explore_anonymization_fixed_number_label'
    },
    noiseAdd: {
      value: 'Noise-Adding',
      label: 'Noise-Adding',
      desc: 'explore_anonymization_noise_adding_label'
    },
    format: {
      value: 'Format-Preserving',
      label: 'Format-Preserving',
      desc: 'explore_anonymization_format_label'
    }
  },
  System_Backup_Type: {
    manual: {
      value: 0,
      label: 'common_manual_backup_label'
    },
    auto: {
      value: 1,
      label: 'common_auto_backup_label'
    },
    upgrade: {
      value: 2,
      label: 'common_upgrade_backup_label'
    }
  },
  System_Backup_Status: {
    creating: {
      value: 1,
      label: 'common_status_creating_label',
      color: ColorConsts.RUNNING
    },
    available: {
      value: 2,
      label: 'common_status_available_label',
      color: ColorConsts.NORMAL
    },
    invalid: {
      value: 3,
      label: 'common_status_invalid_label',
      color: ColorConsts.OFFLINE
    },
    restoring: {
      value: 4,
      label: 'common_status_restoring_label',
      color: ColorConsts.RUNNING
    },
    backupFailed: {
      value: 5,
      label: 'common_status_backup_failed_label',
      color: ColorConsts.ABNORMAL
    },
    restoreFailed: {
      value: 6,
      label: 'common_status_restore_failed_label',
      color: ColorConsts.ABNORMAL
    }
  },
  System_Backup_DestType: {
    sftp: {
      value: 1,
      label: 'SFTP'
    },
    ftp: {
      value: 2,
      label: 'FTP'
    },
    s3: {
      value: 3,
      label: 'S3'
    }
  },
  System_Backup_DefaultPolicy: {
    default: {
      value: 1,
      label: 'default'
    },
    other: {
      value: 0,
      label: 'other'
    }
  },
  System_Init_Status: {
    failed: {
      value: -1,
      label: 'common_init_failed_label'
    },
    ok: {
      value: 0,
      label: 'common_init_ok_label'
    },
    running: {
      value: 1,
      label: 'common_init_running_label'
    },
    yes: {
      value: 2,
      label: 'common_init_yes_label'
    },
    no: {
      value: 3,
      label: 'common_init_no_label'
    },
    authFailed: {
      value: 4,
      label: 'common_init_auth_failed_label'
    },
    backupFailed: {
      value: 5,
      label: 'common_init_backup_failed_label'
    },
    archiveFailed: {
      value: 6,
      label: '1677935617'
    }
  },
  networkModifyStatus: {
    running: {
      value: 9527,
      label: 'running'
    },
    finish: {
      value: 2233,
      label: 'finish'
    },
    fail: {
      value: 45,
      label: 'fail'
    }
  },
  networkModifyingStatus: {
    normal: {
      value: 1
    },
    modify: {
      value: 2
    },
    modifying: {
      value: 3
    }
  },
  Senesitization_Create_Method: {
    customized: {
      value: 'customized',
      label: 'common_customize_label'
    },
    preset: {
      value: 'preset',
      label: 'explore_preset_mode_label'
    }
  },
  Senesitization_Template_Mode: {
    pci: {
      value: 'PCI-DSS',
      label: 'PCI-DSS'
    },
    hippa: {
      value: 'HIPPA',
      label: 'HIPPA'
    },
    eupii: {
      value: 'EU PII',
      label: 'EU PII'
    },
    gdpr: {
      value: 'GDPR',
      label: 'GDPR'
    }
  },
  Resource_Protected_Status: {
    true: {
      value: true,
      label: 'common_protected_label',
      color: ColorConsts.NORMAL
    },
    false: {
      value: false,
      label: 'common_unprotected_label',
      color: ColorConsts.OFFLINE
    }
  },
  Global_Search_Node_Type: {
    file: {
      value: NodeType.File,
      label: 'common_files_label'
    },
    folder: {
      value: NodeType.Folder,
      label: 'common_directory_label'
    },
    link: {
      value: NodeType.Link,
      label: 'common_link_label'
    }
  },
  Log_Level: {
    info: {
      value: 'INFO',
      label: 'common_info_label'
    },
    warn: {
      value: 'WARN',
      label: 'common_warn_label'
    },
    error: {
      value: 'ERROR',
      label: 'common_error_label'
    },
    debug: {
      value: 'DEBUG',
      label: 'common_debug_label'
    }
  },
  Component_Name: {
    infra: {
      value: 'INFRA',
      label: 'system_infrastructure_label'
    },
    dme: {
      value: 'DME',
      label: 'system_protectengine_e_label'
    },
    dee: {
      value: 'DEE',
      label: 'system_dataenableengine_label'
    },
    pm: {
      value: 'PM',
      label: 'system_protectmanager_label'
    }
  },
  cyberComponentName: {
    infra: {
      value: 'INFRA',
      label: 'system_infrastructure_label'
    },
    dee: {
      value: 'DEE',
      label: 'system_ransomware_detection_engine_label'
    },
    pm: {
      value: 'PM',
      label: 'system_protectmanager_label'
    },
    env: {
      value: 'ENV',
      label: 'system_operating_environment_label'
    }
  },
  Available_Capacity_Status: {
    used: {
      value: 1,
      label: 'common_used_capacity_label'
    },
    unused: {
      value: 0,
      label: 'common_unused_capacity_label'
    }
  },
  Backup_Proxy_File: {
    Upload: {
      value: 'UPLOAD',
      label: 'common_upload_label'
    },
    DownLoad: {
      value: 'DOWNLAOD',
      label: 'common_download_label'
    }
  },
  OS_Type: {
    Windows: {
      value: 'WINDOWS',
      label: 'Windows'
    },
    Linux: {
      value: 'LINUX',
      label: 'Linux'
    },
    Unix: {
      value: 'UNIX',
      label: 'AIX'
    },
    hpux: {
      value: 'HPUX',
      label: 'HP-UX'
    },
    openvm: {
      value: 'OPENVMS',
      label: 'OpenVMS'
    },
    solaris: {
      value: 'SOLARIS',
      label: 'Solaris'
    },
    Others: {
      value: 'OTHERS',
      label: 'common_others_label'
    }
  },
  Backup_Proxy: {
    Host: {
      value: 'HOST',
      label: 'common_host_label'
    },
    VMware: {
      value: 'VM',
      label: 'protection_vmware_agent_label'
    }
  },
  Agent_File: {
    fileName: {
      value: 'common_agent_file_label'
    }
  },
  Job_Log_Level: {
    info: {
      value: 'info',
      label: 'common_alarms_info_label'
    },
    warning: {
      value: 'warning',
      label: 'common_alarms_warning_label'
    },
    error: {
      value: 'error',
      label: 'common_error_label'
    },
    fatal: {
      value: 'fatal',
      label: 'common_fatal_label'
    }
  },
  Alarm_Severity_Type: {
    info: {
      value: 0,
      label: 'common_alarms_info_label'
    },
    warning: {
      value: 1,
      label: 'common_alarms_warning_label' // 告警
    },
    major: {
      value: 3,
      label: 'common_alarms_major_label' // 重要
    },
    critical: {
      value: 4,
      label: 'common_alarms_critical_label' // 紧急
    }
  },
  AlarmSyslogSeverityType: {
    info: {
      value: 2,
      label: 'common_alarms_info_label',
      isLeaf: true
    },
    warning: {
      value: 3,
      label: 'common_alarms_warning_label', // 警告
      isLeaf: true
    },
    major: {
      value: 5,
      label: 'common_alarms_major_label', // 重要
      isLeaf: true
    },
    critical: {
      value: 6,
      label: 'common_alarms_critical_label', // 紧急
      isLeaf: true
    }
  },
  SendAlarmType: {
    alarm: {
      value: 1,
      label: 'common_alarms_label',
      isLeaf: true
    },
    recovery: {
      value: 2,
      label: 'common_alarm_recovery_label',
      isLeaf: true
    },
    event: {
      value: 4,
      label: 'common_events_label',
      isLeaf: true
    }
  },
  AlarmProtocolType: {
    UDP: {
      value: 1,
      label: 'UDP',
      isLeaf: true
    },
    TCP: {
      value: 2,
      label: 'TCP',
      isLeaf: true
    },

    TCPAndSSL: {
      value: 3,
      label: 'TCP+SSL/TLS',
      isLeaf: true
    }
  },
  SysLogLanguage: {
    english: {
      value: 0,
      label: 'common_english_label',
      isLeaf: true
    },
    chinese: {
      value: 1,
      label: 'common_chinese_label',
      isLeaf: true
    }
  },
  File_Download_Status: {
    init: {
      value: 'init',
      label: 'init'
    },
    processing: {
      value: 'processing',
      label: 'processing'
    },
    completed: {
      value: 'completed',
      label: 'completed'
    },
    error: {
      value: 'error',
      label: 'error'
    }
  },
  System_Job_Type: {
    resource_scan: {
      value: 'resource_scan',
      label: 'common_environment_scan_label'
    },
    managementDataBackup: {
      value: 'management',
      label: 'common_management_data_backup_label'
    }
  },
  Protection_Status: {
    protected: {
      value: 1,
      label: 'common_protected_label',
      color: ColorConsts.NORMAL
    },
    not_protected: {
      value: 0,
      label: 'common_unprotected_label',
      color: ColorConsts.OFFLINE
    },
    creating: {
      value: 2,
      label: 'common_status_creating_label',
      color: ColorConsts.RUNNING
    }
  },
  ioDetectEnabled: {
    protected: {
      value: true,
      label: 'common_protected_label',
      color: ColorConsts.NORMAL
    },
    notProtected: {
      value: false,
      label: 'common_unprotected_label',
      color: ColorConsts.OFFLINE
    }
  },
  Install_Languages: {
    chinese: {
      value: 'zh-cn',
      label: 'common_chinese_label'
    },
    english: {
      value: 'en-us',
      label: 'common_english_label'
    }
  },
  Protocol_Version: {
    V3: {
      value: 'V3',
      label: 'SNMPv3'
    },
    V2C: {
      value: 'V2C',
      label: 'SNMPv2c'
    }
  },
  Batch_Result_Status: {
    running: {
      value: 2,
      label: 'common_executing_label',
      color: JobColorConsts.RUNNING
    },
    successful: {
      value: 1,
      label: 'common_success_label',
      color: JobColorConsts.SUCCESSFUL
    },
    fail: {
      value: 0,
      label: 'common_fail_label',
      color: JobColorConsts.FAILED
    }
  },
  batchSendEmailStatus: {
    running: {
      value: 2,
      label: 'common_executing_label',
      color: JobColorConsts.RUNNING
    },
    successful: {
      value: 1,
      label: 'common_send_success_label',
      color: JobColorConsts.SUCCESSFUL
    },
    fail: {
      value: 0,
      label: 'common_send_fail_label',
      color: JobColorConsts.FAILED
    }
  },
  Host_Proxy_Type: {
    DBBackupAgent: {
      value: 'DBBackupAgent',
      label: 'protection_dbbackupagent_native_label'
    },
    VMBackupAgent: {
      value: 'VMBackupAgent',
      label: 'protection_vmbackupagent_native_label'
    },
    DWSBackupAgent: {
      value: 'DWSBackupAgent',
      label: 'protection_dwsbackupagent_native_label'
    },
    UBackupAgent: {
      value: 'UBackupAgent',
      label: 'protection_ubackupagent_native_label'
    },
    SBackupAgent: {
      value: 'SBackupAgent',
      label: 'protection_sbackupagent_native_label'
    }
  },
  Dump_History_Result: {
    unknow: {
      value: 0,
      color: JobColorConsts.INIT,
      label: 'common_unknow_label'
    },
    success: {
      value: 1,
      color: JobColorConsts.SUCCESSFUL,
      label: 'common_success_label'
    },
    fail: {
      value: 2,
      color: JobColorConsts.FAILED,
      label: 'common_fail_label'
    },
    running: {
      value: 3,
      color: JobColorConsts.RUNNING,
      label: 'common_running_label'
    }
  },
  nodeRole: {
    primaryNode: {
      value: 'PRIMARY',
      label: 'system_backup_cluster_primary_node_label'
    },
    memberNode: {
      value: 'MEMBER',
      label: 'system_backup_cluster_member_node_label'
    },
    backupNode: {
      value: 'STANDBY',
      label: 'system_backup_cluster_standby_node_label'
    }
  },
  DistributedClusterRole: {
    management: {
      value: 'management',
      label: 'common_management_role_label'
    },
    storage: {
      value: 'storage',
      label: 'common_storage_label'
    },
    dpcCompute: {
      value: 'dpc_compute',
      label: 'common_dpc_compute_label'
    }
  },
  Target_Cluster_Role: {
    primaryNode: {
      value: 7,
      label: 'system_backup_cluster_primary_node_label'
    },
    memberNode: {
      value: 5,
      label: 'system_backup_cluster_member_node_label'
    },
    backupNode: {
      value: 6,
      label: 'system_backup_cluster_standby_node_label'
    },
    replication: {
      value: 1,
      label: 'system_cluster_role_replication_label'
    },
    management: {
      value: 4,
      label: 'common_management_label'
    },
    managed: {
      value: 2,
      label: 'system_cluster_role_managed_label'
    },
    backupStorage: {
      value: 3,
      label: 'system_backup_storage_cluster_label'
    }
  },
  Report_Type: {
    clientStatus: {
      value: 1,
      label: 'insight_client_status_label'
    },
    storageSpace: {
      value: 'STORAGE',
      label: 'insight_storage_space_label'
    },
    quota: {
      value: 3,
      label: 'insight_quota_label'
    },
    resourceProtect: {
      value: 4,
      label: 'insight_resource_protect_label'
    },
    backupJob: {
      value: 'BACKUP',
      label: 'insight_backup_job_label'
    },
    recoveryJob: {
      value: 'RESTORE',
      label: 'insight_recovery_job_label'
    },
    recoveryDrillJob: {
      value: 'DRILL',
      label: 'insight_recovery_drill_job_label'
    },
    liveMountJob: {
      value: 7,
      label: 'insight_live_mount_job_label'
    },
    replicateJob: {
      value: 8,
      label: 'insight_replicate_job_label'
    },
    archiveJob: {
      value: 9,
      label: 'insight_archive_job_label'
    },
    agentResourceUsed: {
      value: 'AGENT',
      label: 'insight_agent_resource_used_label'
    },
    resourceUsed: {
      value: 'RESOURCE',
      label: 'insight_resource_used_label'
    },
    tapeUsed: {
      value: 'TAPE',
      label: 'insight_tape_used_label'
    }
  },
  Device_Storage_Type: {
    OceanStorDorado_6_1_3: {
      value: 'DoradoV6',
      label: 'OceanStor Dorado 6.x'
    },
    OceanStor_6_1_3: {
      value: 'OceanStorV6',
      label: 'OceanStor 6.x'
    },
    OceanStor_v5: {
      value: 'OceanStorV5',
      label: 'OceanStor V5'
    },
    OceanStorPacific: {
      value: 'OceanStorPacific',
      label: 'OceanStor Pacific'
    },
    OceanProtect: {
      value: 'OceanProtect',
      label: 'OceanProtect'
    },
    NetApp: {
      value: 'NetApp',
      label: 'NetApp ONTAP'
    },
    ndmp: {
      value: 'NDMP-server',
      label: 'protection_ndmp_server_label'
    },
    Other: {
      value: 'StorageOthers',
      label: 'common_others_label'
    }
  },
  poolStorageDeviceType: {
    OceanProtectX: {
      value: 'OceanProtectX',
      label: 'OceanProtect'
    },
    OceanPacific: {
      value: 'OceanPacific',
      label: 'OceanStor Pacific'
    },
    Server: {
      value: 'NormalServer',
      label: 'system_server_label'
    }
  },
  cyberDeviceStorageType: {
    OceanStorDorado: {
      value: 'CyberEngineDoradoV6',
      label: 'OceanStor Dorado & OceanStor'
    },
    OceanStorPacific: {
      value: 'CyberEnginePacific',
      label: 'OceanStor Pacific'
    },
    OceanProtect: {
      value: 'CyberEngineOceanProtect',
      label: 'OceanProtect'
    }
  },
  Shared_Mode: {
    nfs: {
      value: '1',
      label: 'NFS'
    },
    cifs: {
      value: '0',
      label: 'CIFS'
    }
  },
  Cifs_Auth_Mode: {
    everyone: {
      value: '1',
      label: 'protection_no_auth_label'
    },
    password: {
      value: '0',
      label: 'protection_password_auth_label'
    }
  },
  Aggreagte_Level: {
    high: {
      value: 0,
      label: 'common_level_high_label'
    },
    medium: {
      value: 1,
      label: 'common_level_medium_label'
    },
    low: {
      value: 2,
      label: 'common_level_low_label'
    }
  },
  Cluster_Register_Mode: {
    token: {
      value: 6,
      label: 'protection_token_auth_label'
    },
    kubeconfig: {
      value: 8,
      label: 'protection_kubeconfig_auth_label'
    }
  },
  Cluster_Register_Config_Info: {
    upload: {
      value: 1,
      label: 'common_upload_label'
    },
    paste: {
      value: 2,
      label: 'protection_paste_label'
    }
  },
  Dataset_Application_Type: {
    mysql: {
      value: 1,
      label: 'protection_mysql_label'
    },
    others: {
      value: 2,
      label: 'common_others_label'
    }
  },
  HDFS_Cluster_Type: {
    mrs: {
      value: 'MRS',
      label: 'MRS'
    },
    cdh: {
      value: 'CDH',
      label: 'CDH'
    },
    FusionInsight: {
      value: 'FusionInsight',
      label: 'FusionInsight'
    }
  },
  HDFS_Cluster_Auth_Type: {
    system: {
      value: '1',
      label: 'protection_simple_auth_label'
    },
    kerberos: {
      value: '0',
      label: 'protection_kerberos_auth_label'
    }
  },
  HDFS_Clusters_Auth_Type: {
    system: {
      value: 0,
      label: 'protection_simple_auth_label'
    },
    ldap: {
      value: 3,
      label: 'protection_ldap_auth_label'
    },
    kerberos: {
      value: 5,
      label: 'protection_kerberos_auth_label'
    }
  },
  Days_Of_Month_Type: {
    specifiedDate: {
      value: 1,
      label: 'common_specified_date_label'
    },
    lastDay: {
      value: '32',
      label: 'common_lasts_day_label'
    }
  },
  Days_Of_Week: {
    mon: {
      value: 'mon',
      label: 'common_mon_label'
    },
    tue: {
      value: 'tue',
      label: 'common_tue_label'
    },
    wed: {
      value: 'wed',
      label: 'common_wed_label'
    },
    thu: {
      value: 'thu',
      label: 'common_thu_label'
    },
    fri: {
      value: 'fri',
      label: 'common_fri_label'
    },
    sat: {
      value: 'sat',
      label: 'common_sat_label'
    },
    sun: {
      value: 'sun',
      label: 'common_sun_label'
    }
  },

  timeOfDay: {
    0: {
      value: '00:00',
      label: '00:00'
    },
    1: {
      value: '01:00',
      label: '01:00'
    },
    2: {
      value: '02:00',
      label: '02:00'
    },
    3: {
      value: '03:00',
      label: '03:00'
    },
    4: {
      value: '04:00',
      label: '04:00'
    },
    5: {
      value: '05:00',
      label: '05:00'
    },
    6: {
      value: '06:00',
      label: '06:00'
    },
    7: {
      value: '07:00',
      label: '07:00'
    },
    8: {
      value: '08:00',
      label: '08:00'
    },
    9: {
      value: '09:00',
      label: '09:00'
    },
    10: {
      value: '10:00',
      label: '10:00'
    },
    11: {
      value: '11:00',
      label: '11:00'
    },
    12: {
      value: '12:00',
      label: '12:00'
    },
    13: {
      value: '13:00',
      label: '13:00'
    },
    14: {
      value: '14:00',
      label: '14:00'
    },
    15: {
      value: '15:00',
      label: '15:00'
    },
    16: {
      value: '16:00',
      label: '16:00'
    },
    17: {
      value: '17:00',
      label: '17:00'
    },
    18: {
      value: '18:00',
      label: '18:00'
    },
    19: {
      value: '19:00',
      label: '19:00'
    },
    20: {
      value: '20:00',
      label: '20:00'
    },
    21: {
      value: '21:00',
      label: '21:00'
    },
    22: {
      value: '22:00',
      label: '22:00'
    },
    23: {
      value: '23:00',
      label: '23:00'
    }
  },
  timeOfADay: {
    1: {
      value: '01:00',
      label: '01:00'
    },
    2: {
      value: '02:00',
      label: '02:00'
    },
    3: {
      value: '03:00',
      label: '03:00'
    },
    4: {
      value: '04:00',
      label: '04:00'
    },
    5: {
      value: '05:00',
      label: '05:00'
    },
    6: {
      value: '06:00',
      label: '06:00'
    },
    7: {
      value: '07:00',
      label: '07:00'
    },
    8: {
      value: '08:00',
      label: '08:00'
    },
    9: {
      value: '09:00',
      label: '09:00'
    },
    10: {
      value: '10:00',
      label: '10:00'
    },
    11: {
      value: '11:00',
      label: '11:00'
    },
    12: {
      value: '12:00',
      label: '12:00'
    },
    13: {
      value: '13:00',
      label: '13:00'
    },
    14: {
      value: '14:00',
      label: '14:00'
    },
    15: {
      value: '15:00',
      label: '15:00'
    },
    16: {
      value: '16:00',
      label: '16:00'
    },
    17: {
      value: '17:00',
      label: '17:00'
    },
    18: {
      value: '18:00',
      label: '18:00'
    },
    19: {
      value: '19:00',
      label: '19:00'
    },
    20: {
      value: '20:00',
      label: '20:00'
    },
    21: {
      value: '21:00',
      label: '21:00'
    },
    22: {
      value: '22:00',
      label: '22:00'
    },
    23: {
      value: '23:00',
      label: '23:00'
    },
    24: {
      value: '00:00',
      label: '00:00'
    }
  },
  dayOfWeek: {
    mon: {
      value: '1',
      label: 'common_mon_label'
    },
    tue: {
      value: '2',
      label: 'common_tue_label'
    },
    wed: {
      value: '3',
      label: 'common_wed_label'
    },
    thu: {
      value: '4',
      label: 'common_thu_label'
    },
    fri: {
      value: '5',
      label: 'common_fri_label'
    },
    sat: {
      value: '6',
      label: 'common_sat_label'
    },
    sun: {
      value: '7',
      label: 'common_sun_label'
    }
  },
  Year_Time_Range: {
    January: {
      value: '1',
      label: 'common_january_label'
    },
    February: {
      value: '2',
      label: 'common_february_label'
    },
    March: {
      value: '3',
      label: 'common_march_label'
    },
    April: {
      value: '4',
      label: 'common_april_label'
    },
    May: {
      value: '5',
      label: 'common_may_label'
    },
    June: {
      value: '6',
      label: 'common_june_label'
    },
    July: {
      value: '7',
      label: 'common_july_label'
    },
    August: {
      value: '8',
      label: 'common_august_label'
    },
    September: {
      value: '9',
      label: 'common_september_label'
    },
    October: {
      value: '10',
      label: 'common_october_label'
    },
    November: {
      value: '11',
      label: 'common_november_label'
    },
    December: {
      value: '12',
      label: 'common_december_label'
    }
  },
  Month_Time_Range: {
    first: {
      value: 'first',
      label: 'common_first_one_label'
    },
    last: {
      value: 'last',
      label: 'common_last_one_label'
    }
  },
  Archive_Target_Type: {
    archiveAllCopies: {
      value: 1,
      label: 'protection_archive_all_copies_label'
    },
    specifiedDate: {
      value: 2,
      label: 'protection_archive_specified_date_label'
    }
  },
  Hbase_Backup_Type: {
    snapshot: {
      value: 'Snapshot',
      label: 'common_snapshot_label'
    },
    wal: {
      value: 'WAL',
      label: 'WAL'
    }
  },
  HDFS_Name_Mode_Type: {
    ip: {
      value: 'ip',
      label: 'common_ip_address_label'
    },
    domain: {
      value: 'domain',
      label: 'common_domain_label'
    }
  },
  Namespace_Link_Status: {
    normal: {
      value: 0,
      label: 'common_online_label',
      color: ColorConsts.NORMAL
    },
    offline: {
      value: 1,
      label: 'common_off_label',
      color: ColorConsts.OFFLINE
    }
  },
  File_System_LinkStatus: {
    normal: {
      value: '27',
      label: 'common_online_label',
      color: ColorConsts.NORMAL
    },
    offline: {
      value: '28',
      label: 'common_off_label',
      color: ColorConsts.OFFLINE
    },
    invalid: {
      value: '35',
      label: 'common_status_invalid_label',
      color: ColorConsts.OFFLINE
    },
    initializing: {
      value: '53',
      label: 'common_initializing_label',
      color: ColorConsts.RUNNING
    }
  },
  Nas_Share_LinkStatus: {
    normal: {
      value: 1,
      label: 'common_online_label',
      color: ColorConsts.NORMAL
    },
    offline: {
      value: 0,
      label: 'common_off_label',
      color: ColorConsts.OFFLINE
    },
    unknown: {
      value: 2,
      label: 'common_unknown_label',
      color: ColorConsts.OFFLINE
    }
  },
  Nas_Share_Auth_Mode: {
    system: {
      value: 0,
      label: 'protection_no_auth_label'
    },
    password: {
      value: 2,
      label: 'protection_ntml_auth_label'
    },
    kerberos: {
      value: 5,
      label: 'protection_kerberos_auth_label'
    }
  },
  Copy_Format: {
    native: {
      value: 'false',
      label: 'protection_native_copy_label'
    },
    aggregate: {
      value: 'true',
      label: 'protection_aggregate_copy_label'
    }
  },
  Standard_Service_Status: {
    ok: {
      value: 0,
      label: 'common_init_ok_label'
    },
    running: {
      value: 1,
      label: 'system_sftp_start_running_label'
    },
    no: {
      value: 2,
      label: 'common_disable_label'
    },
    failed: {
      value: 6,
      label: 'system_sftp_start_failed_label'
    },
    rollbacking: {
      value: 7,
      label: 'system_sftp_start_rollbacking_label'
    },
    rollbackFailed: {
      value: 8,
      label: 'system_sftp_rollback_failed_label'
    }
  },
  RPC_Protection: {
    integrity: {
      value: 'integrity',
      label: 'integrity'
    },
    privacy: {
      value: 'privacy',
      label: 'privacy'
    },
    authentication: {
      value: 'authentication',
      label: 'authentication'
    }
  },
  Filesystem_Authority_Level: {
    readonly: {
      value: 0,
      label: 'protection_readonly_level_label'
    },
    all: {
      value: 1,
      label: 'protection_root_level_label'
    },
    none: {
      value: 2,
      label: 'protection_no_level_label'
    },
    readWrite: {
      value: 3,
      label: 'protection_read_write_level_label'
    }
  },
  Nfs_Share_Client_Type: {
    host: {
      value: 0,
      label: 'common_host_label'
    },
    networkGroup: {
      value: 1,
      label: 'protection_network_group_label'
    }
  },
  Cifs_Domain_Client_Type: {
    everyone: {
      value: 0,
      label: 'common_everyone_label'
    },
    windows: {
      value: 1,
      label: 'protection_winodws_user_label'
    },
    windowsGroup: {
      value: 2,
      label: 'protection_winodws_user_group_label'
    },
    ad: {
      value: 3,
      label: 'protection_ad_domain_label'
    },
    adGroup: {
      value: 4,
      label: 'protection_ad_domain_group_label'
    }
  },
  Nas_Tenant_Type: {
    system: {
      value: '0',
      label: 'System_vStore'
    }
  },
  Aggregation_Mode: {
    auto: {
      value: 0,
      label: 'protection_aggregation_accord_result_label'
    },
    disable: {
      value: 1,
      label: 'protection_aggregation_no_label'
    },
    enable: {
      value: 2,
      label: 'protection_aggregation_yes_label'
    }
  },
  Livemount_Filesystem_Authority_Level: {
    readonly: {
      value: 0,
      label: 'protection_readonly_level_label'
    },
    readWrite: {
      value: 1,
      label: 'protection_read_write_level_label'
    }
  },
  Deploy_Type: {
    a8000: {
      value: 'a8000',
      label: 'X8000'
    },
    cloudbackup: {
      value: 'cloudbackup',
      label: 'CloudBackup'
    },
    x8000: {
      value: 'd0',
      label: 'X8000'
    },
    cloudbackup2: {
      value: 'd3',
      label: 'Cloudbackup'
    },
    hyperdetect: {
      value: 'd4',
      label: 'Hyperdetect'
    },
    x6000: {
      value: 'd1',
      label: 'X6000'
    },
    x3000: {
      value: 'd2',
      label: 'X3000'
    },
    cyberengine: {
      value: 'd5',
      label: 'OceanCyber'
    },
    x9000: {
      value: 'd6',
      label: 'X9000'
    },
    e6000: {
      value: 'd7',
      label: 'E6000'
    },
    decouple: {
      value: 'd8',
      label: 'decouple'
    },
    openOem: {
      value: 'd9',
      label: 'open-eBackup'
    },
    openServer: {
      value: 'd10',
      label: 'open-eBackup'
    }
  },
  Fileset_Template_Os_Type: {
    linux: {
      value: 'linux',
      label: 'Linux'
    },
    windows: {
      value: 'windows',
      label: 'Windows'
    },
    aix: {
      value: 'aix',
      label: 'AIX'
    },
    hpux: {
      value: 'hp_ux',
      label: 'HP-UX'
    },
    solaris: {
      value: 'solaris',
      label: 'Solaris'
    },
    openvm: {
      value: 'open_vms',
      label: 'OpenVMS'
    }
  },
  Create_Fileset_Mode: {
    manual: {
      value: 0,
      label: 'common_customize_label'
    },
    applicationTemplate: {
      value: 1,
      label: 'common_template_label'
    }
  },
  Archival_Protocol: {
    objectStorage: {
      value: 2,
      label: 'common_object_storage_label'
    },
    tapeLibrary: {
      value: 7,
      label: 'system_archive_device_label'
    }
  },
  Proxy_Type_Options: {
    hostAgentOracle: {
      value: 'HOST_AGENT_ORACLE',
      label: 'protection_dbbackupagent_native_label'
    },
    remoteAgentVmware: {
      value: 'REMOTE_AGENT_VMWARE',
      label: 'protection_vmbackupagent_native_label'
    },
    remoteAgent: {
      value: 'REMOTE_AGENT',
      label: 'protection_ubackupagent_native_label'
    },
    sanclientAgent: {
      value: 'SAN_CLIENT_AGENT',
      label: 'protection_sbackupagent_native_label'
    }
  },
  Global_Search_Type: {
    [SearchRange.COPIES]: {
      value: SearchRange.COPIES,
      label: 'common_copy_data_lowercase_label'
    },
    [SearchRange.RESOURCES]: {
      value: SearchRange.RESOURCES,
      label: 'common_resource_singular_label'
    },
    [SearchRange.LABELS]: {
      value: SearchRange.LABELS,
      label: 'common_tag_label'
    }
  },
  Export_Query_Status: {
    generating: {
      value: 'CREATING',
      color: ColorConsts.RUNNING,
      label: 'common_export_generating_label'
    },
    success: {
      value: 'SUCCESS',
      color: ColorConsts.NORMAL,
      label: 'common_success_label'
    },
    fail: {
      value: 'FAIL',
      color: ColorConsts.ABNORMAL,
      label: 'common_fail_label'
    },
    partSucess: {
      value: 'PART_SUCCESS',
      color: ColorConsts.NORMAL,
      label: 'job_status_partial_success_label'
    }
  },
  Export_Query_Type: {
    log: {
      value: 'LOG',
      label: 'common_log_label'
    },
    config: {
      value: 'STORAGE_DEVICE_CONFIG',
      label: 'common_config_file_label'
    },
    copy: {
      value: 'COPY_FILE',
      label: 'common_copy_file_label'
    },
    detectionReport: {
      value: 'REALTIME_DETECT_ALARM',
      label: 'explore_detection_report_label'
    }
  },
  exportLogType: {
    log: {
      value: 'LOG',
      label: 'common_system_log_label'
    },
    config: {
      value: 'STORAGE_DEVICE_CONFIG',
      label: 'common_config_file_label'
    },
    copy: {
      value: 'COPY_FILE',
      label: 'common_copy_file_label'
    },
    detectionReport: {
      value: 'REALTIME_DETECT_ALARM',
      label: 'explore_detection_report_label'
    },
    agentLog: {
      value: 'AGENT_LOG',
      label: 'common_agent_log_label'
    }
  },
  exportCyberLogType: {
    log: {
      value: 'LOG',
      label: 'common_system_log_label'
    },
    detectionReport: {
      value: 'REALTIME_DETECT_ALARM',
      label: 'explore_detection_report_label'
    }
  },
  exportLogRange: {
    last15: {
      value: 15,
      label: 'common_last_15_day_label',
      isLeaf: true
    },
    all: {
      value: 0,
      label: 'common_all_label',
      isLeaf: true
    }
  },
  Additional_Status: {
    database: {
      value: 'Database Available',
      label: 'protection_database_available_label'
    },
    virtualMachine: {
      value: 'Virtual Machine Available',
      label: 'protection_virtual_machine_available_label'
    }
  },
  NasFileSystem_Protocol: {
    cifs: {
      value: '0',
      label: 'CIFS'
    },
    nfs: {
      value: '1',
      label: 'NFS'
    },
    nfs_cifs: {
      value: '2',
      label: 'NFS+CIFS'
    },
    ndmp: {
      value: '3',
      label: 'protection_ndmp_protocol_label'
    },
    none: {
      value: '-1',
      label: 'common_none_label'
    }
  },
  Deployment_Type: {
    single: {
      value: '1',
      label: 'protection_deployment_single_label'
    },
    standby: {
      value: '3',
      label: 'protection_deployment_standby_label'
    }
  },
  guassdbTDeploymentType: {
    single: {
      value: '1',
      label: 'protection_single_node_deployment_label'
    },
    standby: {
      value: '3',
      label: 'protection_active_standby_deployment_label'
    }
  },
  Opengauss_Deployment_Type: {
    single: {
      value: '1',
      label: 'protection_deployment_single_label'
    },
    standby: {
      value: '3',
      label: 'protection_deployment_standby_label'
    }
  },
  Register_Type: {
    yes: {
      value: 'yes',
      label: 'protection_proxy_mode_label'
    },
    no: {
      value: 'no',
      label: 'common_no_proxy_type_label'
    }
  },
  Mysql_Cluster_Type: {
    master: {
      value: 'AP',
      label: 'protection_deployment_standby_label'
    },
    eapp: {
      value: 'EAPP',
      label: 'protection_deployment_deployment_label'
    },
    pxc: {
      value: 'PXC',
      label: 'PXC(Percona XtraDB Cluster)'
    }
  },
  PostgreSql_Cluster_Type: {
    Pgpool: {
      value: 'AP',
      label: 'protection_deployment_standby_label'
    }
  },
  PostgreSqlDeployType: {
    Pgpool: {
      value: 'Pgpool',
      label: 'Pgpool'
    },
    Patroni: {
      value: 'Patroni',
      label: 'Patroni'
    },
    CLup: {
      value: 'CLup',
      label: 'CLup'
    }
  },
  KingBase_Cluster_Type: {
    Pgpool: {
      value: 'AP',
      label: 'protection_deployment_standby_label'
    }
  },
  Version_Info: {
    six: {
      value: '6.0',
      label: '6.0'
    }
  },
  sync_mode: {
    synchronize: {
      value: '1',
      label: 'protection_synchronize_copy_label'
    },
    asynchronous: {
      value: '2',
      label: 'protection_asynchronous_copy_label'
    }
  },
  Dameng_Type: {
    single: {
      value: 'Dameng-singleNode',
      label: 'protection_deployment_single_label'
    },
    cluster: {
      value: 'Dameng-cluster',
      label: 'common_mpp_as_cluster_label'
    }
  },
  oracleType: {
    single: {
      value: 'Oracle',
      label: 'protection_deployment_single_label'
    },
    cluster: {
      value: 'Oracle-cluster',
      label: 'common_cluster_label'
    }
  },
  CopyDataOpengaussType: {
    dataBase: {
      value: 'OpenGauss-database',
      label: 'common_database_label'
    },
    cluster: {
      value: 'OpenGauss-instance',
      label: 'protection_database_instance_label'
    }
  },
  CopyDataExchangeType: {
    dataBase: {
      value: 'Exchange-database',
      label: 'common_database_label'
    }
  },
  CopyData_DWS_Type: {
    DWS_Cluster: {
      value: 'DWS-cluster',
      label: 'protection_cluster_label'
    },
    DWS_Schema: {
      value: 'DWS-schema',
      label: 'protection_schema_set_label'
    },
    DWS_Table: {
      value: 'DWS-table',
      label: 'protection_table_set_label'
    }
  },
  CopyData_SQL_Server_Type: {
    SQLServerInstance: {
      value: 'SQLServer-instance',
      label: 'protection_single_instance_label'
    },
    SQLServerClusterInstance: {
      value: 'SQLServer-clusterInstance',
      label: 'protection_cluster_instance_label'
    },
    SQLServerGroup: {
      value: 'SQLServer-alwaysOn',
      label: 'protection_availability_group_label'
    },
    SQLServerDatabase: {
      value: 'SQLServer-database',
      label: 'common_database_label'
    }
  },
  copyDataMysqlType: {
    mysqlInstance: {
      value: 'MySQL-instance',
      label: 'protection_single_instance_label'
    },
    mysqlClusterInstance: {
      value: 'MySQL-clusterInstance',
      label: 'protection_cluster_instance_label'
    },
    mysqlDatabase: {
      value: 'MySQL-database',
      label: 'common_database_label'
    }
  },
  copyDataDbTwoType: {
    dbTwoDatabase: {
      value: 'DB2-database',
      label: 'common_database_label'
    },
    dbTwoTableSet: {
      value: 'DB2-tablespace',
      label: 'protection_table_space_set_label'
    }
  },
  copyDataOceanBaseType: {
    oceanBaseCluster: {
      value: 'OceanBase-cluster',
      label: 'protection_cluster_label'
    },
    oceanBaseTenantSet: {
      value: 'OceanBase-tenant',
      label: 'protection_single_tenant_set_label'
    }
  },
  Backup_Type: {
    Roach: {
      value: 0,
      label: 'common_roach_backup_label'
    },
    GDS: {
      value: 1,
      label: 'common_gds_backup_label'
    }
  },
  mysqlDatabaseType: {
    mySql: {
      value: 'MySQL',
      label: 'MySQL'
    },
    mariaDb: {
      value: 'MariaDB',
      label: 'MariaDB'
    },
    greatDB: {
      value: 'GreatSQL',
      label: 'GreatSQL'
    }
  },
  informixDatabaseType: {
    informix: {
      value: 'Informix',
      label: 'Informix'
    },
    gbase: {
      value: 'GBase 8s',
      label: 'GBase 8s'
    }
  },
  resourceFilterType: {
    mix: {
      value: 'MIX',
      label: 'protection_filter_mix_label'
    },
    include: {
      value: 'INCLUDE',
      label: 'common_include_label'
    },
    exclude: {
      value: 'EXCLUDE',
      label: 'protection_filter_exclude_label'
    }
  },
  copyFormat: {
    native: {
      value: 0,
      label: 'protection_native_copy_label'
    },
    aggregate: {
      value: 1,
      label: 'protection_aggregate_copy_label'
    }
  },
  userType: {
    admin: {
      value: 1,
      label: 'common_admin_user_label'
    },
    common: {
      value: -1,
      label: 'common_common_user_label'
    }
  },
  passwordType: {
    yes: {
      value: true,
      label: 'common_yes_label'
    },
    no: {
      value: false,
      label: 'common_no_label'
    }
  },
  timeType: {
    day: {
      value: 'day',
      label: 'common_by_day_label'
    },
    week: {
      value: 'week',
      label: 'common_by_week_label'
    }
  },
  airgapDeviceStatus: {
    online: {
      value: '1',
      label: 'common_online_label',
      color: ColorConsts.NORMAL
    },
    offline: {
      value: '0',
      label: 'common_off_label',
      color: ColorConsts.OFFLINE
    }
  },
  airgapLinkStatus: {
    open: {
      value: 'open',
      label: 'common_connected_status_label',
      color: ColorConsts.NORMAL
    },
    close: {
      value: 'close',
      label: 'common_disconnected_status_label',
      color: ColorConsts.OFFLINE
    },
    unknown: {
      value: 'unknown',
      label: 'common_unknown_label',
      color: ColorConsts.OFFLINE
    }
  },
  airgapPolicyStatus: {
    enable: {
      value: 'enable',
      label: 'explore_file_extension_create_label',
      color: ColorConsts.NORMAL
    },
    disable: {
      value: 'disable',
      label: 'common_not_applied_label',
      color: ColorConsts.OFFLINE
    },
    invalid: {
      value: 'invalid',
      label: 'explore_already_invalidated_label',
      color: ColorConsts.WARN
    }
  },
  airgapPolicyCyberStatus: {
    enable: {
      value: 'enable',
      label: 'explore_file_extension_create_label',
      color: ColorConsts.NORMAL
    },
    disable: {
      value: 'disable',
      label: 'common_not_applied_label',
      color: ColorConsts.OFFLINE
    },
    invalid: {
      value: 'invalid',
      label: 'explore_already_invalidated_label',
      color: ColorConsts.WARN
    }
  },
  lockDatabaseType: {
    limit: {
      value: '1',
      label: 'protection_specified_time_label'
    },
    noLimit: {
      value: '0',
      label: 'protection_on_limit_label'
    }
  },
  datastoreType: {
    local: {
      value: '0',
      label: 'protection_database_type_local_storage_label'
    },
    block: {
      value: '1',
      label: 'protection_database_type_block_storage_label'
    },
    fusionOne: {
      value: '3',
      label: 'protection_database_type_fusionone_label'
    }
  },
  ldapService: {
    ldap: {
      value: 'LDAP',
      label: 'system_ldap_server_label'
    },
    windowsAd: {
      value: 'WINDOWS_AD',
      label: 'system_ldap_service_window_ad_label'
    }
  },
  ldapProtocol: {
    ldap: {
      value: 'LDAP',
      label: 'LDAP'
    },
    ldaps: {
      value: 'LDAPS',
      label: 'LDAPS'
    }
  },
  ldapAddressType: {
    ip: {
      value: 'IP',
      label: 'common_ip_address_lower_label'
    },
    domain: {
      value: 'DOMAIN',
      label: 'common_domain_lower_label'
    }
  },
  loginUserType: {
    local: {
      value: 'COMMON',
      label: 'system_local_user_label'
    },
    ldap: {
      value: 'LDAP',
      label: 'system_ldap_service_user_label'
    },
    ldapGroup: {
      value: 'LDAPGROUP',
      label: 'system_ldap_service_user_group_label'
    },
    saml: {
      value: 'SAML',
      label: 'system_saml_service_user_label'
    },
    hcs: {
      value: 'HCS',
      label: 'system_hcs_user_label'
    },
    adfs: {
      value: 'ADFS',
      label: 'system_adfs_service_user_label'
    }
  },
  loginMethod: {
    password: {
      value: 0,
      label: 'system_login_password_method_label'
    },
    email: {
      value: 1,
      label: 'system_login_password_email_method_label'
    }
  },
  dbTwoType: {
    dpf: {
      value: 'dpf',
      label: 'Database Partitioning Feature'
    },
    standby: {
      value: 'powerHA',
      label: 'powerHA'
    },
    hadr: {
      value: 'hadr',
      label: 'HADR'
    }
  },
  productName: {
    oceanProtect: {
      value: 'OceanProtect',
      label: 'OceanProtect'
    },
    cyberEngine: {
      value: 'OceanCyber',
      label: 'OceanCyber'
    },
    protectManager: {
      value: 'ProtectManager',
      label: 'ProtectManager'
    },
    openSource: {
      value: 'open-eBackup',
      label: 'open-eBackup'
    }
  },
  airGapPortStatus: {
    inactive: {
      value: 'inactive',
      label: ' ',
      color: ColorConsts.OFFLINE
    },
    active: {
      value: 'active',
      label: ' ',
      color: ColorConsts.NORMAL
    },
    unknown: {
      value: 'unknown',
      label: ' ',
      color: ColorConsts.OFFLINE
    }
  },
  userFunction: {
    backup: {
      value: 'backup',
      label: 'common_backup_label'
    },
    replicate: {
      value: 'replicate',
      label: 'common_replicate_label'
    },
    archive: {
      value: 'archive',
      label: 'common_archive_label'
    },
    restore: {
      value: 'restore',
      label: 'common_recovery_label'
    }
  },
  slaReplicationRule: {
    all: {
      value: 1,
      label: 'protection_replicate_all_label'
    },
    specify: {
      value: 2,
      label: 'protection_replicate_specified_date_label'
    }
  },
  lanFreeRunningStatus: {
    unknown: {
      value: '0',
      label: 'common_unknown_label',
      color: ColorConsts.OFFLINE
    },
    normal: {
      value: '1',
      label: 'common_status_normal_label',
      color: ColorConsts.NORMAL
    },
    running: {
      value: '2',
      label: 'common_running_label',
      color: ColorConsts.RUNNING
    },
    connected: {
      value: '10',
      label: 'protection_connected_label',
      color: ColorConsts.NORMAL
    },
    unconnected: {
      value: '11',
      label: 'protection_not_connected_label',
      color: ColorConsts.OFFLINE
    },
    precopy: {
      value: '14',
      label: 'protection_pre_copy_label',
      color: ColorConsts.RUNNING
    },
    refactoring: {
      value: '16',
      label: 'protection_refactor_label',
      color: ColorConsts.RUNNING
    },
    online: {
      value: '27',
      label: 'common_online_label',
      color: ColorConsts.NORMAL
    },
    offline: {
      value: '28',
      label: 'common_off_label',
      color: ColorConsts.OFFLINE
    },
    balancing: {
      value: '32',
      label: 'common_off_label',
      color: ColorConsts.RUNNING
    },
    init: {
      value: '53',
      label: 'common_initializing_label',
      color: ColorConsts.RUNNING
    },
    deleting: {
      value: '106',
      label: 'common_status_deleting_label',
      color: ColorConsts.RUNNING
    }
  },
  generalDbClusterType: {
    single: {
      value: '1',
      label: 'protection_deployment_single_label'
    },
    deploy: {
      value: '3',
      label: 'protection_deploy_cluster_label'
    },
    distribute: {
      value: '5',
      label: 'protection_distribute_cluster_label'
    },
    sharding: {
      value: '4',
      label: 'protection_sharding_cluster_label'
    }
  },
  storageDeviceDetectType: {
    cyberEngine: {
      value: '1',
      label: 'protection_detect_type_cyber_engine_label'
    },
    inDevice: {
      value: '2',
      label: 'protection_detect_type_indevice_label'
    }
  },
  initRole: {
    data: {
      value: 2,
      label: 'common_backup_label'
    },
    copy: {
      value: 4,
      label: 'common_replicate_label'
    },
    dataManage: {
      value: 11,
      label: 'common_archive_label'
    }
  },
  initRoleTable: {
    data: {
      value: 2,
      label: 'common_backup_label'
    },
    copy: {
      value: 4,
      label: 'common_replicate_label'
    },
    dataManage: {
      value: 11,
      label: 'common_archive_label'
    }
  },
  initSupportPortocol: {
    nfsCifs: {
      value: '3',
      label: 'NFS+CIFS'
    },
    nfs: {
      value: '1',
      label: 'NFS'
    }
  },
  initSupportPortocolTable: {
    nfsCifs: {
      value: '3',
      label: 'NFS+CIFS'
    },
    nfs: {
      value: '1',
      label: 'NFS'
    },
    none: {
      value: '0',
      label: '--'
    }
  },
  initHomePortType: {
    ethernet: {
      value: '1',
      label: 'common_ehternet_port_content_label'
    },
    bonding: {
      value: '7',
      label: 'common_bonding_port_label'
    },
    vlan: {
      value: '8',
      label: 'VLAN'
    }
  },
  initControlType: {
    A: {
      value: '0A',
      label: '0A'
    },
    B: {
      value: '0B',
      label: '0B'
    }
  },
  initRuningStatus: {
    unknown: {
      value: '0',
      label: 'common_unknown_label'
    },
    connect: {
      value: '10',
      label: 'system_connected_label'
    },
    unConnet: {
      value: '11',
      label: 'system_not_connected_label'
    },
    toRecovered: {
      value: '33',
      label: 'common_to_be_recovered_label'
    }
  },
  initHeathStatus: {
    unknown: {
      value: '0',
      label: 'common_unknown_label'
    },
    normal: {
      value: '1',
      label: 'common_status_normal_label'
    },
    faulty: {
      value: '2',
      label: 'common_status_abnormal_label'
    },
    errors: {
      value: '7',
      label: 'common_bit_errors_found_label'
    }
  },
  initRouteType: {
    network: {
      value: '0',
      label: 'common_network_segment_route_label',
      isLeaf: true
    },
    client: {
      value: '1',
      label: 'common_client_route_label',
      isLeaf: true
    },
    default: {
      value: '2',
      label: 'common_default_route_label',
      isLeaf: true
    }
  },
  initLogicType: {
    frontEndPort: {
      value: '0',
      label: 'system_front_end_port_label',
      isLeaf: true
    },
    podFrontEndPort: {
      value: '13',
      label: 'system_pod_front_end_port_label',
      isLeaf: true
    }
  },
  goldendbNodeType: {
    ommNode: {
      value: 'ommNode',
      label: 'ommNode'
    },
    dataNode: {
      value: 'dataNode',
      label: 'dataNode'
    },
    managerNode: {
      value: 'managerNode',
      label: 'managerNode'
    },
    gtmNode: {
      value: 'gtmNode',
      label: 'gtmNode'
    }
  },
  detectionMethod: {
    manual: {
      value: false,
      label: 'explore_detection_method_manual_label'
    },
    auto: {
      value: true,
      label: 'explore_detection_method_auto_label'
    }
  },
  detectionSnapshotStatus: {
    infected: {
      value: 3,
      label: 'explore_infected_label',
      color: ColorConsts.ABNORMAL,
      resultIcon: 'aui-icon-detection-infected'
    },
    uninfected: {
      value: 2,
      label: 'explore_uninfected_label',
      color: ColorConsts.NORMAL,
      resultIcon: 'aui-icon-detection-uninfected'
    },
    detecting: {
      value: 1,
      label: 'explore_anti_detecting_label',
      color: ColorConsts.RUNNING
    },
    nodetecte: {
      value: -1,
      label: 'explore_anti_no_detecte_label',
      color: ColorConsts.RUNNING
    }
  },
  snapshotGeneratetype: {
    copyDetect: {
      value: 'COPY_DETECT',
      label: 'explore_intelligent_detection_label'
    },
    ioDetect: {
      value: 'IO_DETECT',
      label: 'explore_real_time_detection_new_label'
    }
  },
  snapshotExpiredType: {
    expired: {
      value: true,
      label: 'explore_snapshot_expired_label'
    },
    unexpired: {
      value: false,
      label: 'explore_snapshot_unexpired_label'
    }
  },
  snapshotCopyStatus: {
    restoring: {
      value: 'Restoring',
      label: 'common_status_restoring_label',
      color: ColorConsts.RUNNING
    },
    deleting: {
      value: 'Deleting',
      label: 'common_status_deleting_label',
      color: ColorConsts.RUNNING
    },
    deleteFailed: {
      value: 'DeleteFailed',
      label: 'common_delete_failed_label',
      color: ColorConsts.ABNORMAL
    },
    mounting: {
      value: 'Mounting',
      label: 'common_status_restoring_label',
      color: ColorConsts.RUNNING
    },
    mounted: {
      value: 'Mounted',
      label: 'common_status_restoring_label',
      color: ColorConsts.RUNNING
    },
    unmounting: {
      value: 'Unmounting',
      label: 'common_status_restoring_label',
      color: ColorConsts.RUNNING
    },
    normal: {
      value: 'Normal',
      label: 'common_status_normal_label',
      color: ColorConsts.NORMAL
    },
    invalid: {
      value: 'Invalid',
      label: 'common_status_invalid_label',
      color: ColorConsts.OFFLINE
    },
    verifying: {
      value: 'Verifying',
      label: 'common_status_verifying_label',
      color: ColorConsts.RUNNING
    }
  },
  snapShotJobStatus: {
    success: {
      value: 'success',
      label: 'common_success_label'
    },
    failed: {
      value: 'failed',
      label: 'common_fail_label'
    }
  },
  hmacSignatureAlgorithm: {
    safe: {
      value: 'safe',
      label: 'hmac-sha2-256'
    },
    compatible: {
      value: 'compatible',
      label: 'hmac-sha2-256,hmac-sha1'
    }
  },
  isBusinessOptions: {
    yes: {
      value: true,
      label: 'common_yes_label'
    },
    no: {
      value: false,
      label: 'common_no_label'
    }
  },
  agentType: {
    external: {
      value: 'external',
      label: 'protection_external_agent_label'
    },
    ecs: {
      value: 'ecs',
      label: 'protection_ecs_agent_label'
    }
  },
  mongodbClusterType: {
    primary: {
      value: '0',
      label: 'protection_mongodb_cluster_primary_label'
    },
    copy: {
      value: '1',
      label: 'protection_mongodb_cluster_copy_label'
    },
    slicing: {
      value: '2',
      label: 'protection_mongodb_cluster_slicing_label'
    }
  },
  mongodbInstanceType: {
    single: {
      value: '3',
      label: 'protection_single_instance_label'
    },
    clusterPrimary: {
      value: '0',
      label: 'protection_mongodb_cluster_primary_instance_label'
    },
    clusterCopy: {
      value: '1',
      label: 'protection_mongodb_cluster_copy_instance_label'
    },
    clusterSlicing: {
      value: '2',
      label: 'protection_mongodb_cluster_slicing_instance_label'
    }
  },
  exchangeGroupType: {
    group: {
      value: 'Exchange-group',
      label: 'protection_checkbox_availability_group_label'
    },
    single: {
      value: 'Exchange-single-node',
      label: 'protection_checkbox_single_node_system_label'
    }
  },
  exchangeClusterstate: {
    normal: {
      value: '1',
      label: 'common_online_label',
      color: ColorConsts.NORMAL
    },
    unavailable: {
      value: '0',
      label: 'common_off_label',
      color: ColorConsts.OFFLINE
    }
  },
  tdsqlInstanceType: {
    distributed: {
      value: '1',
      label: 'protection_tdsql_distributed_instance_label'
    },
    nonDistributed: {
      value: '0',
      label: 'protection_tdsql_non_distributed_instance_label'
    }
  },
  tdsqlInstanceTypeNew: {
    distributed: {
      value: 'TDSQL-clusterGroup',
      label: 'protection_tdsql_distributed_instance_label'
    },
    nonDistributed: {
      value: 'TDSQL-clusterInstance',
      label: 'protection_tdsql_non_distributed_instance_label'
    }
  },
  tdsqlDataNodeType: {
    primary: {
      value: 1,
      label: 'protection_primary_mode_label'
    },
    standby: {
      value: 0,
      label: 'protection_standby_mode_label'
    }
  },
  tdsqlNodePriority: {
    high: {
      value: '1',
      label: 'common_level_high_label'
    },
    medium: {
      value: '2',
      label: 'common_level_medium_label'
    },
    low: {
      value: '3',
      label: 'common_level_low_label'
    }
  },
  tdsqlNodeType: {
    ossNode: {
      value: 'ossNode',
      label: 'ossNode'
    },
    schedulerNode: {
      value: 'schedulerNode',
      label: 'schedulerNode'
    },
    dataNode: {
      value: 'dataNode',
      label: 'dataNode'
    }
  },
  kubernetesDatasetMode: {
    workload: {
      value: 'workloads',
      label: 'protection_by_workload_label'
    },
    tag: {
      value: 'labels',
      label: 'protection_labels_label'
    }
  },
  kubernetesBackupMode: {
    snapshot: {
      value: '0',
      label: 'protection_only_snapshot_label'
    },
    all: {
      value: '1',
      label: 'protection_snapshot_and_data_label'
    }
  },
  selfLearningType: {
    day: {
      value: 0,
      label: 'explore_by_day_label'
    },
    times: {
      value: 1,
      label: 'explore_by_times_label'
    }
  },
  timeUnit: {
    minute: {
      label: 'common_minute_label',
      value: 1,
      isLeaf: true
    },
    hour: {
      label: 'common_hour_label',
      value: 2,
      isLeaf: true
    }
  },
  backupStorageType: {
    none: {
      label: 'protection_no_order_label',
      value: 'none',
      isLeaf: true
    },
    group: {
      label: 'system_backup_storage_unit_group_label',
      value: 'storage_unit_group',
      isLeaf: true
    },
    unit: {
      label: 'system_backup_storage_unit_label',
      value: 'storage_unit',
      isLeaf: true
    }
  },
  storagePoolBackupStorageType: {
    group: {
      label: 'system_backup_storage_unit_group_label',
      value: 'storage_unit_group',
      isLeaf: true
    },
    unit: {
      label: 'system_backup_storage_unit_label',
      value: 'storage_unit',
      isLeaf: true
    }
  },
  backupStorageTypeSla: {
    group: {
      label: 'system_backup_storage_unit_group_label',
      value: 1,
      isLeaf: true
    },
    unit: {
      label: 'system_backup_storage_unit_label',
      value: 2,
      isLeaf: true
    }
  },
  backupStorageGeneratedType: {
    local: {
      value: 2
    },
    nonlocal: {
      value: 1
    }
  },
  newBackupPolicy: {
    reduce: {
      label: 'system_reduce_rate_priority_label',
      content: 'system_reduce_rate_priority_tips_label',
      value: 1
    },
    balance: {
      label: 'system_intelligent_balance_label',
      content: 'system_intelligent_balance_tips_label',
      value: 2
    }
  },
  multiClusterResource: {
    Database: {
      label: 'common_database_label',
      value: 'Database',
      protectedCount: 0
    },
    BigData: {
      label: 'common_bigdata_label',
      value: 'BigData',
      protectedCount: 0
    },
    Virtualization: {
      label: 'common_virtualization_label',
      value: 'Virtualization',
      protectedCount: 0
    },
    Container: {
      label: 'common_container_label',
      value: 'Container',
      protectedCount: 0
    },
    Cloud: {
      label: 'common_huawei_clouds_label',
      value: 'Cloud',
      protectedCount: 0
    },
    FileService: {
      label: 'common_file_systems_label',
      value: 'FileService',
      protectedCount: 0
    },
    BareMetal: {
      label: 'common_bare_metal_label',
      value: 'BareMetal',
      protectedCount: 0
    }
  },
  detectionSoftware: {
    veeam: {
      value: 1,
      label: 'Veeam'
    },
    netbackup: {
      value: 2,
      label: 'NetBackup'
    },
    commvault: {
      value: 3,
      label: 'Commvault'
    }
  },
  snapshotVaryType: {
    old: {
      value: 1,
      label: 'explore_history_type_label'
    },
    new: {
      value: 3,
      label: 'explore_new_type_label'
    }
  },
  switchStatus: {
    on: {
      value: true,
      label: 'switch_status_on_label'
    },
    off: {
      value: false,
      label: 'switch_status_off_label'
    }
  },
  workLoadType: {
    daemonSet: {
      value: 'DaemonSet',
      label: 'DaemonSet'
    },
    deployment: {
      value: 'Deployment',
      label: 'Deployment'
    },
    replicaSet: {
      value: 'ReplicaSet',
      label: 'ReplicaSet'
    },
    statefulSet: {
      value: 'StatefulSet',
      label: 'StatefulSet'
    },
    deploymentconfig: {
      value: 'DeploymentConfig',
      label: 'DeploymentConfig'
    },
    job: {
      value: 'Job',
      label: 'Job'
    },
    cronJob: {
      value: 'CronJob',
      label: 'CronJob'
    }
  },
  sftpPort: {
    IOM0: {
      value: 'IOM0.P3',
      label: 'IOM0.P3',
      isLeaf: true
    },
    IOM1: {
      value: 'IOM1.P3',
      label: 'IOM1.P3',
      isLeaf: true
    }
  },
  wormComplianceMode: {
    compliance: {
      value: 1,
      label: 'common_law_compliance_mode_label',
      isLeaf: true
    },
    enterprise: {
      value: 3,
      label: 'common_enterprise_compliance_mode_label',
      isLeaf: true
    }
  },
  hcsStorageType: {
    san: {
      value: '0',
      label: 'common_san_storage_label'
    },
    block: {
      value: '1',
      label: 'protection_database_type_block_storage_label'
    }
  },
  objectStorageType: {
    pacific: {
      value: 1,
      label: 'OceanStor Pacific'
    },
    hcs: {
      value: 2,
      label: 'protection_hcs_object_storage_label'
    },
    ali: {
      value: 3,
      label: 'protection_ali_object_storage_label'
    }
  },
  protocolType: {
    HTTPS: {
      value: '1',
      label: 'HTTPS',
      isLeaf: true
    },
    HTTP: {
      value: '0',
      label: 'HTTP',
      isLeaf: true
    }
  },
  aclType: {
    enable: {
      value: true,
      label: 'explore_acl_backup_enable_label',
      isLeaf: true
    },
    disable: {
      value: false,
      label: 'explore_acl_backup_disable_label',
      isLeaf: true
    }
  },
  drillType: {
    period: {
      value: 'PERIOD',
      label: 'explore_drill_type_period_label'
    },
    single: {
      value: 'SINGLE',
      label: 'explore_drill_type_single_label'
    }
  },
  drillStatus: {
    running: {
      value: 'RUNNING',
      label: 'explore_drill_status_running_label',
      color: ColorConsts.RUNNING
    },
    waiting: {
      value: 'WAITING',
      label: 'explore_drill_status_waitting_label',
      color: ColorConsts.OFFLINE
    },
    finished: {
      value: 'FINISHED',
      label: 'explore_drill_status_finished_label',
      color: ColorConsts.NORMAL
    },
    disabled: {
      value: 'DISABLED',
      label: 'explore_drill_status_disabled_label',
      color: ColorConsts.OFFLINE
    }
  },
  drillMountConfig: {
    destroy: {
      value: true,
      label: 'explore_destroy_after_drill_label'
    },
    keep: {
      value: false,
      label: 'explore_keep_after_drill_label'
    }
  },
  slaStatus: {
    activate: {
      value: true,
      label: 'protection_sla_status_active_label'
    },
    disabled: {
      value: false,
      label: 'protection_sla_status_disabled_label'
    }
  },
  proxyHostType: {
    external: {
      value: '0',
      label: 'protection_hcs_host_external_label'
    },
    builtin: {
      value: '1',
      label: 'protection_hcs_host_builtin_label'
    }
  },
  vmwareTransferMode: {
    san: {
      value: 'SAN',
      label: 'SAN'
    },
    nbdssl: {
      value: 'NBDSSL',
      label: 'NBDSSL'
    },
    hotadd: {
      value: 'HOT ADD',
      label: 'HOT ADD'
    }
  },
  dataProtocolType: {
    iscsi: {
      label: 'iscsi',
      value: 'iscsi'
    },
    fc: {
      label: 'fc',
      value: 'fc'
    },
    nof: {
      label: 'nof',
      value: 'nof'
    }
  },
  aliDiskType: {
    normal: {
      label: 'protection_cloud_disk_label',
      value: 'cloud'
    },
    efficiency: {
      label: 'protection_cloud_efficiency_disk_label',
      value: 'cloud_efficiency'
    },
    ssd: {
      label: 'protection_cloud_ssd_disk_label',
      value: 'cloud_ssd'
    },
    essd: {
      label: 'protection_cloud_essd_disk_label',
      value: 'cloud_essd'
    },
    pperf: {
      label: 'protection_cloud_pperf_disk_label',
      value: 'cloud_pperf'
    },
    sperf: {
      label: 'protection_cloud_sperf_disk_label',
      value: 'cloud_sperf'
    }
  },
  slaApplicationFilterType: {
    all: {
      value: 'APPLY_TO_ALL'
    },
    new: {
      value: 'APPLY_TO_NEW'
    }
  },
  dagBackupType: {
    active: {
      label: 'protection_exchange_active_backup_label',
      value: 'active'
    },
    inactive: {
      label: 'protection_exchange_inactive_backup_label',
      value: 'passive'
    }
  },
  volumeType: {
    system: {
      value: true,
      label: 'explore_volume_type_system_label',
      isLeaf: true
    },
    nonSystem: {
      value: false,
      label: 'explore_volume_type_non_system_label',
      isLeaf: true
    }
  },
  preallocationType: {
    off: {
      value: 'off',
      label: 'protection_off_preallocation_label'
    },
    falloc: {
      value: 'falloc',
      label: 'protection_falloc_preallocation_label'
    },
    full: {
      value: 'full',
      label: 'protection_full_preallocation_label'
    }
  },
  netplaneStatus: {
    null: {
      value: null,
      label: 'explore_copy_worm_unset_label'
    },
    setting: {
      value: 0,
      label: 'system_net_plane_setting_label'
    },
    settingCompleted: {
      value: 1,
      label: 'explore_copy_worm_setted_label'
    },
    settingFail: {
      value: 2,
      label: 'system_add_failed_label'
    },
    modify: {
      value: 3,
      label: 'common_status_modifing_label'
    },
    modifyFailed: {
      value: 4,
      label: 'system_modify_failed_label'
    },
    delete: {
      value: 5,
      label: 'common_status_deleting_label'
    },
    deleteFailed: {
      value: 6,
      label: 'common_delete_failed_label'
    }
  },
  cnwareDiskType: {
    virtio: {
      value: 'virtio',
      label: 'VIRTIO'
    },
    ide: {
      value: 'ide',
      label: 'IDE'
    },
    scsi: {
      value: 'scsi',
      label: 'SCSI'
    },
    sata: {
      value: 'sata',
      label: 'SATA'
    },
    usb: {
      value: 'usb',
      label: 'USB'
    }
  },
  hypervStatus: {
    running: {
      value: 'Running',
      label: 'common_running_label',
      color: ColorConsts.NORMAL
    },
    paused: {
      value: 'Paused',
      label: 'common_status_pause_label',
      color: ColorConsts.OFFLINE
    },
    saved: {
      value: 'Saved',
      label: 'common_status_saved_label',
      color: ColorConsts.NORMAL
    },
    off: {
      value: 'Off',
      label: 'common_shut_down_label',
      color: ColorConsts.OFFLINE
    }
  },
  gaussDBCanRestoreMode: {
    can: {
      value: true,
      label: 'explore_copy_complete_label'
    },
    disable: {
      value: false,
      label: 'protection_incomplete_label'
    }
  },
  fileReplaceType: {
    overwrite: {
      value: VmFileReplaceStrategy.Overwriting,
      label: 'protection_overwrite_label'
    },
    skip: {
      value: VmFileReplaceStrategy.Skip,
      label: 'protection_skip_label'
    },
    replace: {
      value: VmFileReplaceStrategy.Replace,
      label: 'protection_restore_replace_older_label'
    }
  },
  adfsProvider: {
    adfs: {
      value: 'OpenID Connect/OAuth 2.0',
      label: 'OpenID Connect/OAuth 2.0'
    }
  },
  defaultRoleName: {
    sysAdmin: {
      value: 'Role_SYS_Admin',
      label: 'common_sys_admin_label'
    },
    dpAdmin: {
      value: 'Role_DP_Admin',
      label: 'common_user_label'
    },
    auditor: {
      value: 'Role_Auditor',
      label: 'common_auditor_label'
    },
    rdAdmin: {
      value: 'Role_RD_Admin',
      label: 'common_remote_device_administrator_label'
    },
    drAdmin: {
      value: 'Role_DR_Admin',
      label: 'common_dme_admin_label'
    }
  },
  defaultRoleDescription: {
    sysAdmin: {
      value: 'Role_SYS_Admin',
      label: 'common_sys_admin_desc_label'
    },
    dpAdmin: {
      value: 'Role_DP_Admin',
      label: 'common_user_desc_label'
    },
    auditor: {
      value: 'Role_Auditor',
      label: 'common_auditor_desc_label'
    },
    rdAdmin: {
      value: 'Role_RD_Admin',
      label: 'common_remote_device_administrator_desc_label'
    },
    drAdmin: {
      value: 'Role_DR_Admin',
      label: 'common_dme_admin_desc_label'
    }
  },
  saphanaSwitchStatus: {
    enable: {
      value: 'true',
      label: 'common_enable_label'
    },
    disable: {
      value: 'false',
      label: 'common_disable_label'
    }
  },
  saphanaAuthMethod: {
    hdbuserstore: {
      value: 8,
      label: 'Hdbuserstore'
    },
    db: {
      value: 2,
      label: 'protection_database_auth_label'
    }
  },
  saphanaDatabaseType: {
    systemdb: {
      value: 'SystemDatabase',
      label: 'protection_system_db_label'
    },
    tenantdb: {
      value: 'TenantDatabase',
      label: 'protection_tenant_db_label'
    }
  },
  saphanaDatabaseDeployType: {
    SINGLE: {
      value: '1',
      label: 'database_single_deploy_type_label'
    },
    AA: {
      value: '2',
      label: 'database_aa_deploy_type_label'
    },
    AP: {
      value: '3',
      label: 'database_ap_deploy_type_label'
    },
    SHARDING: {
      value: '4',
      label: 'database_sharding_deploy_type_label'
    },
    DISTRIBUTED: {
      value: '5',
      label: 'database_distributed_deploy_type_label'
    }
  },
  k8sClusterType: {
    k8s: {
      value: 'k8s',
      label: 'protection_k8s_cluster_general_label'
    },
    cce: {
      value: 'cce',
      label: 'protection_k8s_cluster_cce_label'
    },
    openshift: {
      value: 'openshift',
      label: 'protection_k8s_cluster_openshift_label'
    }
  },
  unixPermission: {
    readOnly: {
      value: 0,
      label: 'protection_readonly_level_label'
    },
    readwrite: {
      value: 1,
      label: 'protection_read_write_label'
    },
    none: {
      value: 5,
      label: 'common_none_label'
    }
  },
  permissionLevel: {
    readOnly: {
      value: 0,
      label: 'protection_readonly_level_label'
    },
    fullControl: {
      value: 1,
      label: 'protection_full_control_label'
    },
    forbidden: {
      value: 2,
      label: 'protection_forbidden_label'
    },
    readwrite: {
      value: 5,
      label: 'protection_read_write_label'
    }
  },
  rootPermission: {
    squash: {
      value: 0,
      label: 'protection_root_squash_label'
    },
    noSquash: {
      value: 1,
      label: 'protection_no_root_squash_label'
    }
  },
  antiSwitchStatus: {
    disable: {
      value: false,
      label: 'system_tape_disabled_label',
      color: ColorConsts.OFFLINE
    },
    enable: {
      value: true,
      label: 'system_tape_enabled_label',
      color: ColorConsts.NORMAL
    }
  },
  RepositoryDataTypeEnum: {
    META: {
      value: 0,
      label: 'META'
    },
    DATA: {
      value: 1,
      label: 'DATA'
    },
    CACHE: {
      value: 2,
      label: 'CACHE'
    },
    LOG: {
      value: 3,
      label: 'LOG'
    },
    INDEX: {
      value: 4,
      label: 'INDEX'
    },
    LOG_META: {
      value: 5,
      label: 'LOG_META'
    }
  },
  detectUpperBond: {
    low: {
      value: 7,
      label: 'common_level_low_label'
    },
    medium: {
      value: 6,
      label: 'common_level_medium_label'
    },
    high: {
      value: 5,
      label: 'common_level_high_label'
    }
  },
  mongoDBSingleInstanceType: {
    single: {
      value: 'single',
      label: 'protection_checkbox_single_node_system_label',
      isLeaf: true
    },
    copySet: {
      value: 'single_node_repl',
      label: 'protection_single_node_copy_set_label',
      isLeaf: true
    }
  },
  isWorkspace: {
    yes: {
      value: '1',
      label: 'common_yes_label'
    },
    no: {
      value: '0',
      label: 'common_no_label'
    }
  },
  antiRansomwareType: {
    uninfected: {
      value: 0,
      label: 'common_home_uninfected_label'
    },
    abnormal: {
      value: 1,
      label: 'common_home_abnormal_label'
    },
    infected: {
      value: 2,
      label: 'common_home_infected_label'
    },
    notConfig: {
      value: 3,
      label: 'common_home_not_set_label'
    },
    detecting: {
      value: 4,
      label: 'common_home_detecting_label'
    }
  },
  homeTimeType: {
    all: { label: 'common_all_label', value: 0, isLeaf: true },
    last5Min: { label: 'common_home_last_5_min_label', value: 1, isLeaf: true },
    last30min: {
      label: 'common_home_last_30_min_label',
      value: 2,
      isLeaf: true
    },
    lastDay: { label: 'common_home_last_day_label', value: 3, isLeaf: true },
    lastWeek: { label: 'common_home_last_week_label', value: 4, isLeaf: true },
    lastMonth: {
      label: 'common_home_last_month_label',
      value: 5,
      isLeaf: true
    },
    lastYear: { label: 'common_home_last_year_label', value: 6, isLeaf: true }
  },
  copyDataLimitType: {
    restoreOrigin: {
      value: 'OLD_LOCATION_RESTORE',
      label: 'common_restore_to_origin_location_cyber_label'
    },
    restoreNew: {
      value: 'NEW_LOCATION_RESTORE',
      label: 'system_restore_to_new_label'
    },
    fineGrainedRestore: {
      value: 'FLR_RESTORE',
      label: 'protection_fine_grained_restore_label'
    },
    liveMount: {
      value: 'LIVE_MOUNT',
      label: 'common_live_mount_label'
    },
    liveMountRestore: {
      value: 'LIVE_RESTORE',
      label: 'common_live_restore_job_label'
    },
    donwload: {
      value: 'DOWNLOAD',
      label: 'explore_copy_limit_download_export_label'
    },
    replication: {
      value: 'REPLICATION',
      label: 'common_replicate_label'
    },
    archive: {
      value: 'ARCHIVE',
      label: 'common_archive_label'
    }
  },
  copyTypes: {
    backup: {
      value: 'Backup',
      label: 'explore_backup_copy_label'
    },
    replicate: {
      value: 'Replicated',
      label: 'common_copy_a_copy_label'
    }
  }
};
