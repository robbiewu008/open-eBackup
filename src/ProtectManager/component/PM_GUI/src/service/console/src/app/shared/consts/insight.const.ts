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
export enum JOB_ORIGIN_TYPE {
  EXE = 1, // 进行中
  HISTORIC, // 历史
  HANDLED // 已处理，批量重试
}

// 告警事件type
export enum ALARM_EVENT_EVENT_TYPE {
  RESTORE_ALARM = 1,
  EVENT = 5,
  OPLOG
}

export enum ALARM_EVENT_EVENT_STATUS {
  UNCONFIRMED,
  CONFIRMED,
  RECOVERY = 3
}

// 告警类型
export enum ALARM_EVENT_TYPE {
  ALARM,
  EVENT
}

// 告警级别枚举定义
export enum ALARM_SEVERITY {
  CRITICAL = 'critical',
  MAJOR = 'major',
  WARNING = 'warning',
  INFO = 'info'
}

// 报告定期生成规则
export enum REPORT_RULE_TYPE {
  REPORT,
  RULE
}

// 报告生成规则动作
export enum REPORT_RULE_Action {
  Create,
  Modify
}

export enum JOB_TYPE {
  BACKUP,
  RESTORE,
  INSTANT_RESTORE,
  live_mount,
  COPY_REPLICATION,
  ARCHIVE,
  COPY_DELETE,
  COPY_EXPIRE,
  unmount
}

export enum JOB_STATUS {
  READY,
  PENDING,
  RUNNING,
  SUCCESS,
  PARTIAL_SUCCESS,
  ABORTED,
  ABORTING,
  FAIL,
  ABNORMAL
}

export const ALARM_NAVIGATE_STATUS = {
  sequence: '',
  entityIdList: [],
  alarmId: '',
  location: ''
};
