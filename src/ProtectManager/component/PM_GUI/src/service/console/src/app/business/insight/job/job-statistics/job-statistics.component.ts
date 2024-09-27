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
import { Component, OnDestroy, OnInit } from '@angular/core';
import {
  CommonConsts,
  DataMap,
  GlobalService,
  I18NService,
  JOB_ORIGIN_TYPE,
  JobAPIService,
  JobColorConsts
} from 'app/shared';
import { includes, intersection, size, values } from 'lodash';
import { Subject, Subscription } from 'rxjs';

@Component({
  selector: 'aui-job-statistics',
  templateUrl: './job-statistics.component.html',
  styleUrls: ['./job-statistics.component.less']
})
export class JobStatisticsComponent implements OnInit, OnDestroy {
  _values = values;
  isOceanProtect = !includes(
    [
      DataMap.Deploy_Type.cloudbackup2.value,
      DataMap.Deploy_Type.cloudbackup.value,
      DataMap.Deploy_Type.hyperdetect.value,
      DataMap.Deploy_Type.cyberengine.value
    ],
    this.i18n.get('deploy_type')
  );
  items = {
    total: {
      id: 'total',
      label: this.i18n.get('common_all_label'),
      value: 0,
      statusList: [],
      click: item => {
        this.emitStore(item);
      }
    },
    dispatching: {
      id: 'dispatching',
      label: this.i18n.get('common_dispatch_label'),
      value: 0,
      statusList: [
        DataMap.Job_status.dispatching.value,
        DataMap.Job_status.redispatch.value
      ],
      color: JobColorConsts.PENDING,
      click: item => this.emitStore(item)
    },
    pending: {
      id: 'pending',
      label: this.i18n.get('common_pending_label'),
      value: 0,
      statusList: [DataMap.Job_status.pending.value],
      color: JobColorConsts.PENDING,
      click: item => this.emitStore(item)
    },
    running: {
      id: 'running',
      label: this.i18n.get('common_running_label'),
      value: 0,
      statusList: [
        DataMap.Job_status.running.value,
        DataMap.Job_status.initialization.value,
        DataMap.Job_status.aborting.value
      ],
      color: JobColorConsts.RUNNING,
      click: item => this.emitStore(item)
    },
    success: {
      id: 'success',
      label: `${this.i18n.get('common_success_label')}/${this.i18n.get(
        'common_partial_success_label'
      )}`,
      value: 0,
      statusList: [
        DataMap.Job_status.success.value,
        DataMap.Job_status.partial_success.value
      ],
      color: JobColorConsts.SUCCESSFUL,
      click: item => this.emitStore(item)
    },
    fail: {
      id: 'fail',
      label: this.i18n.get('common_fail_label'),
      value: 0,
      statusList: [
        DataMap.Job_status.failed.value,
        DataMap.Job_status.abnormal.value,
        DataMap.Job_status.abort_failed.value,
        DataMap.Job_status.dispatch_failed.value
      ],
      color: JobColorConsts.FAILED,
      click: item => this.emitStore(item)
    },
    aborted: {
      id: 'aborted',
      label: this.i18n.get('common_job_stopped_label'),
      value: 0,
      statusList: [
        DataMap.Job_status.aborted.value,
        DataMap.Job_status.cancelled.value
      ],
      color: JobColorConsts.ABORTED,
      click: item => this.emitStore(item)
    }
  };

  selectedStatus = 'total';

  summarySub$: Subscription;
  destroy$ = new Subject();
  summaryTimeOut;

  constructor(
    private jobAPIService: JobAPIService,
    private i18n: I18NService,
    private globalService: GlobalService
  ) {}

  ngOnDestroy() {
    this.destroy$.next(true);
    this.destroy$.complete();
    clearTimeout(this.summaryTimeOut);
  }

  ngOnInit() {
    if (!this.isOceanProtect) {
      delete this.items.dispatching;
    } else {
      this.items.fail.statusList.push(DataMap.Job_status.dispatch_failed.value);
    }
    this.getData();
  }

  getData(unLoading?) {
    clearTimeout(this.summaryTimeOut);
    this.jobAPIService.summaryUsingGET({ akLoading: !unLoading }).subscribe(
      res => {
        for (const key in res) {
          this.items[key] && (this.items[key]['value'] = res[key]);
          if (key === 'ready' || key === 'aborting') {
            this.items['running']['value'] =
              res[key] + this.items['running']['value'];
          }
          if (key === 'partial_success') {
            this.items['success']['value'] =
              res[key] + this.items['success']['value'];
          }
          if (
            key === 'abnormal' ||
            key === 'abort_failed' ||
            key === 'dispatch_failed'
          ) {
            this.items['fail']['value'] =
              res[key] + this.items['fail']['value'];
          }
          if (key === 'cancelled') {
            this.items['aborted']['value'] =
              res[key] + this.items['aborted']['value'];
          }
          if (key === 'redispatch' && this.isOceanProtect) {
            this.items['dispatching']['value'] =
              res[key] + this.items['dispatching']['value'];
          }
        }
        this.summaryTimeOut = setTimeout(() => {
          this.getData(true);
          clearTimeout(this.summaryTimeOut);
        }, CommonConsts.TIME_INTERVAL);
      },
      () => {
        this.summaryTimeOut = setTimeout(() => {
          this.getData(true);
          clearTimeout(this.summaryTimeOut);
        }, CommonConsts.TIME_INTERVAL);
      }
    );
  }

  emitStore(item) {
    this.selectedStatus = item.id;
    const params = {
      statusList: item.statusList
    };
    if (!size(item.statusList)) {
      params['activeIndex'] = JOB_ORIGIN_TYPE.EXE;
    } else if (
      !!size(
        intersection(item.statusList, [
          DataMap.Job_status.running.value,
          DataMap.Job_status.initialization.value,
          DataMap.Job_status.pending.value,
          DataMap.Job_status.aborting.value,
          DataMap.Job_status.dispatching.value,
          DataMap.Job_status.redispatch.value
        ])
      )
    ) {
      params['activeIndex'] = JOB_ORIGIN_TYPE.EXE;
    } else {
      params['activeIndex'] = JOB_ORIGIN_TYPE.HISTORIC;
    }

    this.globalService.emitStore({
      action: 'JobStatisticsEvent',
      state: params
    });
  }
}
