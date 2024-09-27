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
// copydata的explore页面的图表
export const COPY_DATA_ECHARTS_OPTION = {
  normal: {
    color: '#91E6B1',
    i18n: 'common_status_normal_label'
  },
  restoring: {
    color: '#F7E08C',
    i18n: 'common_status_restoring_label'
  },
  invalid: {
    color: '#E6EBF5',
    i18n: 'common_status_invalid_label'
  }
};

// copydata页面vm列表过滤项
export const COPY_DATA_VM_LIST_FILTER_TYPE = {
  vsphere: {
    i18n: 'explore_list_vm_filter_type_vsphere_label',
    value: 'vsphere'
  },
  vcd: {
    i18n: 'explore_list_vm_filter_type_vcd_label',
    value: 'vcd'
  },
  hyper: {
    i18n: 'common_hyper_vm_machine_label',
    value: 'hyper'
  }
};

// copydata页面database列表过滤项
export const COPY_DATA_DATABASE_LIST_FILTER_TYPE = {
  vsphere: {
    i18n: 'common_oracle_label',
    value: 'oracle'
  },
  vcd: {
    i18n: 'protection_sql_server_label',
    value: 'sqlServer'
  }
};

// copydata页面host列表过滤项
export const COPY_DATA_HOST_LIST_FILTER_TYPE = {
  host: {
    i18n: 'common_host_label',
    value: 'host'
  },
  fileset: {
    i18n: 'protection_fileset_label',
    value: 'fileset'
  }
};

export const DESENSITIZATION_POLICY_DESC_MAP = {
  'PII for the Middle East': 'explore_middle_east_pii_desc_label',
  HIPAA: 'explore_hipaa_desc_label',
  'PII for Africa': 'explore_africa_pii_desc_label',
  'PCI-DSS': 'explore_pci_dss_desc_label',
  'PII for the Asia Pacific region': 'explore_asia_pacific_pii_desc_label',
  'PII for North America': 'explore_north_america_pii_desc_label',
  'PII for Latin America': 'explore_latin_america_pii_desc_label',
  'PII for Europe': 'explore_europe_pii_desc_label'
};

// 虚拟机副本侦测敏感度
export const DETECT_UPPER_BOUND_POINTER = [
  {
    value: 1,
    label: 'common_level_low_label',
    textStyle: {
      'margin-left': '5px'
    }
  },
  {
    value: 2,
    label: 'common_level_medium_label',
    pointStyle: {
      'border-color': '#779bfa',
      'background-color': '#779bfa'
    },
    textStyle: {
      'margin-left': '5px'
    }
  },
  {
    value: 3,
    label: 'common_level_high_label',
    textStyle: {
      'margin-left': '5px'
    }
  }
];

// 副本选择策略
export enum CopyDataSelectionPolicy {
  LastHour = 'last_hour', // 最近一小时
  LastDay = 'last_day', // 最近一天
  LastWeek = 'last_week', // 最近一周
  LastMonth = 'last_month', // 最近一月
  Latest = 'latest' // 最新的副本
}

// 策略调度周期
export enum SchedulePolicy {
  PeriodSchedule = 'period_schedule', // 周期调度
  AfterBackupDone = 'after_backup_done' // 备份完成后立刻执行
}

// 副本保留策略
export enum RetentionPolicy {
  Permanent = 'permanent', // 永久保留
  FixedTime = 'fixed_time', // 按固定时间保留
  LatestOne = 'latest_one' // 只保留最后一个
}

// 副本数据选择类型
export enum CopyDataSelectionType {
  Secified = 'specified',
  Automatic = 'automatic'
}

// 挂载的目标位置
export enum MountTargetLocation {
  Original = 'original',
  Others = 'others'
}

// LiveMount更新模式
export enum LiveMountUpdateModal {
  Latest = 'latest',
  Specified = 'specified'
}

// LiveMount资源动作
export enum LiveMountAction {
  View = 'LiveMountView',
  Create = 'LiveMountCreate',
  SelectResource = 'SelectResource'
}

// 防勒索界面副本动作
export enum DetectionCopyAction {
  View = 'CopyView',
  DetectionSelect = 'DetectionSelect',
  FeedbackSelect = 'FeedbackSelect'
}

// 防勒索检测列表字段
export enum DetectionConfigField {
  CopyDetect = 'COPY_DETECT', // 事后
  IoDetect = 'IO_DETECT', // 事中
  FileExtensionFilter = 'FILE_EXTENSION_FILTER'
}

// 快照勒索检测字段
export enum DetectionGroupField {
  TenantId = 'tenantId'
}

export enum TargetCPU {
  OriginalConfig,
  SpecifyConfig
}

export enum TargetMemory {
  OriginalConfig,
  SpecifyConfig
}

export enum ExpressionMoal {
  Regx,
  Dictionary
}

export enum DesensitizationMode {
  TemplateBased,
  Custom
}

export enum DesensitizationSourceType {
  GaussDB = 'gaussdb',
  REDIS = 'redis',
  SQLITE = 'sqlite',
  MySQL = 'mysql',
  POSTGRES = 'postgres',
  Oracle = 'oracle',
  SQLServer = 'mssql',
  ES = 'ElasticSearch'
}

export enum StorageLocation {
  Different = 'different_datastore',
  Same = 'same_datastore'
}

export enum SoftwareType {
  VEEAM = 1,
  NBU,
  CV
}

export enum DrillExecuteType {
  Immediately,
  Specified
}
