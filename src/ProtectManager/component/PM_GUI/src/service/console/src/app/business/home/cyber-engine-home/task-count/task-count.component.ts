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
import { Component, OnInit } from '@angular/core';
import { eq, get, remove, values } from 'lodash';
import {
  JobAPIService,
  DataMap,
  I18NService,
  JobColorConsts,
  RouterUrl
} from 'app/shared';
import { Router } from '@angular/router';
interface IItem {
  id: string;
  label: string;
  value: number;
  statusList: any[];
  color?: string;
}

@Component({
  selector: 'app-task-count',
  templateUrl: './task-count.component.html',
  styleUrls: ['./task-count.component.less']
})
export class TaskCountComponent implements OnInit {
  items = {
    total: {
      id: 'total',
      label: this.i18n.get('common_all_label'),
      value: 0,
      statusList: []
    },
    pending: {
      id: 'pending',
      label: this.i18n.get('common_pending_label'),
      value: 0,
      statusList: [DataMap.Job_status.pending.value],
      color: JobColorConsts.PENDING
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
      color: JobColorConsts.RUNNING
    },
    success: {
      id: 'success',
      label: `${this.i18n.get('common_success_label')}`,
      value: 0,
      statusList: [
        DataMap.Job_status.success.value,
        DataMap.Job_status.partial_success.value
      ],
      color: JobColorConsts.SUCCESSFUL
    },
    fail: {
      id: 'fail',
      label: this.i18n.get('common_fail_label'),
      value: 0,
      statusList: [
        DataMap.Job_status.failed.value,
        DataMap.Job_status.abnormal.value,
        DataMap.Job_status.abort_failed.value
      ],
      color: JobColorConsts.FAILED
    },
    aborted: {
      id: 'aborted',
      label: this.i18n.get('common_job_abort_label'),
      value: 0,
      statusList: [
        DataMap.Job_status.aborted.value,
        DataMap.Job_status.cancelled.value
      ],
      color: JobColorConsts.ABORTED
    }
  };

  get vals(): IItem[] {
    return remove(values(this.items), item => !eq(item.id, 'total'));
  }

  constructor(
    private jobApiService: JobAPIService,
    public i18n: I18NService,
    private router: Router
  ) {}

  ngOnInit() {
    this.loadData();
  }

  loadData(loading: boolean = true) {
    this.jobApiService
      .summaryUsingGET({ akLoading: loading })
      .subscribe(res => {
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
          if (key === 'abnormal' || key === 'abort_failed') {
            this.items['fail']['value'] =
              res[key] + this.items['fail']['value'];
          }
          if (key === 'cancelled') {
            this.items['aborted']['value'] =
              res[key] + this.items['aborted']['value'];
          }
        }
      });
  }

  getTotal() {
    return this.items.total.value;
  }

  widthComputer(key: string) {
    return `${(get(this.items, [key, 'value'], 0) / this.getTotal()) * 100}%`;
  }

  gotoTask() {
    this.router.navigateByUrl(RouterUrl.InsightJobs);
  }
}
