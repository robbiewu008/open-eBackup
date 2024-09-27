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
import { Injectable } from '@angular/core';
import { findIndex, first } from 'lodash';
import { DataMap } from '../consts/data-map.config';

@Injectable({
  providedIn: 'root'
})
export class SlaValidatorService {
  constructor() {}

  // 备份频率必须小于保留时间
  validBackupInterval(
    intervalControlName,
    intervalUnitControlName,
    retentionControlName,
    retentionUnitControlName,
    backupTeam
  ) {
    let interval = backupTeam[intervalControlName];
    const interval_unit = backupTeam[intervalUnitControlName];
    let retention_duration = backupTeam[retentionControlName];
    const duration_unit = backupTeam[retentionUnitControlName];
    if (
      !interval ||
      !interval_unit ||
      !retention_duration ||
      !duration_unit ||
      duration_unit === DataMap.Interval_Unit.persistent.value
    ) {
      return null;
    }

    interval =
      interval_unit === DataMap.Interval_Unit.hour.value
        ? +interval * 60
        : interval_unit === DataMap.Interval_Unit.day.value
        ? +interval * 24 * 60
        : interval_unit === DataMap.Interval_Unit.week.value
        ? +interval * 7 * 24 * 60
        : interval_unit === DataMap.Interval_Unit.month.value
        ? +interval * 30 * 24 * 60
        : +interval;
    retention_duration =
      duration_unit === DataMap.Interval_Unit.hour.value
        ? +retention_duration * 60
        : duration_unit === DataMap.Interval_Unit.day.value
        ? +retention_duration * 24 * 60
        : duration_unit === DataMap.Interval_Unit.week.value
        ? +retention_duration * 7 * 24 * 60
        : duration_unit === DataMap.Interval_Unit.month.value
        ? +retention_duration * 30 * 24 * 60
        : duration_unit === DataMap.Interval_Unit.year.value
        ? +retention_duration * 365 * 24 * 60
        : +retention_duration;

    return interval < retention_duration
      ? null
      : 'protection_backup_interval_retention_label';
  }

  // 归档频率必须大于等待时间
  validArchivalInterval(archivalTeam) {
    let interval = archivalTeam['interval'];
    const interval_unit = archivalTeam['interval_unit'];
    const auto_retry = archivalTeam['auto_retry'];
    const auto_retry_wait_minutes = archivalTeam['auto_retry_wait_minutes'];
    if (
      !interval ||
      !interval_unit ||
      !auto_retry ||
      !auto_retry_wait_minutes
    ) {
      return null;
    }

    interval =
      interval_unit === DataMap.Interval_Unit.hour.value
        ? +interval * 60
        : interval_unit === DataMap.Interval_Unit.day.value
        ? +interval * 24 * 60
        : interval_unit === DataMap.Interval_Unit.week.value
        ? +interval * 7 * 24 * 60
        : interval_unit === DataMap.Interval_Unit.month.value
        ? +interval * 30 * 24 * 60
        : +interval;

    return interval > +auto_retry_wait_minutes
      ? null
      : 'protection_archival_interval_wait_minutes_label';
  }

  // 日志备份频率必须小于数据备份频率
  validLogInterval(logItem, backupItem) {
    let log_interval = logItem['interval'];
    const log_interval_unit = logItem['interval_unit'];
    let backup_interval = backupItem['interval'];
    const backup_interval_unit = backupItem['interval_unit'];
    if (
      !log_interval ||
      !log_interval_unit ||
      !backup_interval ||
      !backup_interval_unit
    ) {
      return null;
    }

    log_interval =
      log_interval_unit === DataMap.Interval_Unit.hour.value
        ? +log_interval * 60
        : log_interval_unit === DataMap.Interval_Unit.day.value
        ? +log_interval * 24 * 60
        : log_interval_unit === DataMap.Interval_Unit.week.value
        ? +log_interval * 7 * 24 * 60
        : log_interval_unit === DataMap.Interval_Unit.month.value
        ? +log_interval * 30 * 24 * 60
        : +log_interval;
    backup_interval =
      backup_interval_unit === DataMap.Interval_Unit.hour.value
        ? +backup_interval * 60
        : backup_interval_unit === DataMap.Interval_Unit.day.value
        ? +backup_interval * 24 * 60
        : backup_interval_unit === DataMap.Interval_Unit.week.value
        ? +backup_interval * 7 * 24 * 60
        : backup_interval_unit === DataMap.Interval_Unit.month.value
        ? +backup_interval * 30 * 24 * 60
        : +backup_interval;

    return log_interval < backup_interval
      ? null
      : 'protection_log_backup_interval_valid_label';
  }

  // 首次执行备份时间应按照如下顺序：全量备份 < 增量备份 < 日志备份
  validFullBackupStartTimeSort(fullItem, incrementItem) {
    if (
      !(
        fullItem &&
        fullItem['start_time'] &&
        fullItem['window_start'] &&
        incrementItem &&
        incrementItem['start_time'] &&
        incrementItem['window_start']
      )
    ) {
      return null;
    }

    const incrementDate =
      `${incrementItem['start_time'].getFullYear()}` +
      '/' +
      (+`${incrementItem['start_time'].getMonth()}` + 1) +
      '/' +
      `${incrementItem['start_time'].getDate()}` +
      ' ' +
      `${incrementItem['window_start'].getHours()}` +
      ':' +
      `${incrementItem['window_start'].getMinutes()}` +
      ':' +
      `${incrementItem['window_start'].getSeconds()}`;

    const fullDate =
      `${fullItem['start_time'].getFullYear()}` +
      '/' +
      (+`${fullItem['start_time'].getMonth()}` + 1) +
      '/' +
      `${fullItem['start_time'].getDate()}` +
      ' ' +
      `${fullItem['window_start'].getHours()}` +
      ':' +
      `${fullItem['window_start'].getMinutes()}` +
      ':' +
      `${fullItem['window_start'].getSeconds()}`;

    if (Date.parse(incrementDate) <= Date.parse(fullDate)) {
      return 'protection_valid_backup_start_time_label';
    }

    return null;
  }

  // 首次执行备份时间应按照如下顺序：全量备份 < 增量备份 < 日志备份
  validLogBackupStartTimeSort(logItem, fullItem, incrementItem) {
    if (fullItem && logItem) {
      // 周期性
      if (
        fullItem['start_time'] &&
        fullItem['window_start'] &&
        fullItem['trigger'] === 1 &&
        logItem['start_time']
      ) {
        const fullDate =
          `${fullItem['start_time'].getFullYear()}` +
          '/' +
          (+`${fullItem['start_time'].getMonth()}` + 1) +
          '/' +
          `${fullItem['start_time'].getDate()}` +
          ' ' +
          `${fullItem['window_start'].getHours()}` +
          ':' +
          `${fullItem['window_start'].getMinutes()}` +
          ':' +
          `${fullItem['window_start'].getSeconds()}`;

        if (Date.parse(fullDate) >= Date.parse(logItem['start_time'])) {
          return 'protection_valid_backup_start_time_label';
        }
      }

      // 指定时间
      if (
        fullItem['specified_window_start'] &&
        fullItem['trigger'] === 4 &&
        logItem['start_time']
      ) {
        const sysTime = new Date();
        let fullDate;
        let year = sysTime.getFullYear();
        let month = sysTime.getMonth();
        let date = sysTime.getDate();

        if (fullItem['trigger_action'] === 'year') {
          month = fullItem['days_of_year'].getMonth();
          date = fullItem['days_of_year'].getDate();

          // 选择同一天的情况
          if (month === sysTime.getMonth() && date === sysTime.getDate()) {
            if (fullItem['specified_window_start'] <= sysTime) {
              year++;
            }
          } else {
            if (month < sysTime.getMonth()) {
              year++;
            } else if (month === sysTime.getMonth()) {
              if (date < sysTime.getDate()) {
                year++;
              }
            }
          }
        } else if (fullItem['trigger_action'] === 'month') {
          if (
            findIndex(fullItem['days_of_months'], val => val === date) !== -1
          ) {
            // 选择同一天的情况
            if (fullItem['specified_window_start'] <= sysTime) {
              const idx = findIndex(
                fullItem['days_of_months'],
                val => val > date
              );

              if (idx === -1) {
                date = first(fullItem['days_of_months']);
                month++;
              } else {
                date = fullItem['days_of_months'][idx];
              }
            }
          } else {
            const idx = findIndex(
              fullItem['days_of_months'],
              val => val > date
            );

            if (idx === -1) {
              date = first(fullItem['days_of_months']);
              month++;
            } else {
              date = fullItem['days_of_months'][idx];
            }
          }
        } else if (fullItem['trigger_action'] === 'week') {
          const weekVal = {
            mon: 1,
            tue: 2,
            wed: 3,
            thu: 4,
            fri: 5,
            sat: 6,
            sun: 0
          };
          const today = sysTime.getDay();
          let backupDate;

          if (
            findIndex(
              fullItem['days_of_week'],
              val => weekVal[val as string] === today
            ) !== -1
          ) {
            // 选择同一天的情况
            if (fullItem['specified_window_start'] <= sysTime) {
              const idx = findIndex(
                fullItem['days_of_week'],
                val => weekVal[val as string] > today
              );

              idx === -1
                ? (backupDate =
                    weekVal[first(fullItem['days_of_week']) as string])
                : (backupDate = weekVal[fullItem['days_of_week'][idx]]);

              if (today <= backupDate) {
                date += backupDate - today;
              } else {
                date += 7 + backupDate - today;
              }
            }
          } else {
            const idx = findIndex(
              fullItem['days_of_week'],
              val => weekVal[val as string] > today
            );

            idx === -1
              ? (backupDate =
                  weekVal[first(fullItem['days_of_week']) as string])
              : (backupDate = weekVal[fullItem['days_of_week'][idx]]);

            if (today <= backupDate) {
              date += backupDate - today;
            } else {
              date += 7 + backupDate - today;
            }
          }
        }

        fullDate =
          `${year}` +
          '/' +
          (+`${month}` + 1) +
          '/' +
          `${date}` +
          ' ' +
          `${fullItem['specified_window_start'].getHours()}` +
          ':' +
          `${fullItem['specified_window_start'].getMinutes()}` +
          ':' +
          `${fullItem['specified_window_start'].getSeconds()}`;

        if (Date.parse(fullDate) >= Date.parse(logItem['start_time'])) {
          return 'protection_valid_backup_start_time_label';
        }
      }
    }

    if (incrementItem && logItem) {
      if (
        incrementItem['start_time'] &&
        incrementItem['window_start'] &&
        logItem['start_time']
      ) {
        const incrementDate =
          `${incrementItem['start_time'].getFullYear()}` +
          '/' +
          (+`${incrementItem['start_time'].getMonth()}` + 1) +
          '/' +
          `${incrementItem['start_time'].getDate()}` +
          ' ' +
          `${incrementItem['window_start'].getHours()}` +
          ':' +
          `${incrementItem['window_start'].getMinutes()}` +
          ':' +
          `${incrementItem['window_start'].getSeconds()}`;

        if (Date.parse(incrementDate) >= Date.parse(logItem['start_time'])) {
          return 'protection_valid_backup_start_time_label';
        }
      }
    }

    return null;
  }
}
