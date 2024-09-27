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
  AfterViewInit,
  Component,
  OnDestroy,
  OnInit,
  ViewChild
} from '@angular/core';
import {
  CookieService,
  DataMap,
  DataMapService,
  GlobalService,
  I18NService,
  JOB_ORIGIN_TYPE,
  SwitchService,
  UsersApiService
} from 'app/shared';
import {
  assign,
  each,
  find,
  first,
  get,
  includes,
  isEmpty,
  map,
  reject
} from 'lodash';
import { Subscription } from 'rxjs';
import { JobTableComponent } from '../job-table/job-table.component';

@Component({
  selector: 'aui-job-resource',
  templateUrl: './job-resource.component.html',
  styleUrls: ['./job-resource.component.less']
})
export class JobResourceComponent implements OnInit, AfterViewInit, OnDestroy {
  showExpiredCopy = true;
  disableCheckBox = false;
  JobStatisticsEvent = new Subscription();
  selectedResource;
  activeIndex = JOB_ORIGIN_TYPE.EXE;
  statusList = [];
  isOceanProtect = !includes(
    [
      DataMap.Deploy_Type.cloudbackup2.value,
      DataMap.Deploy_Type.cloudbackup.value,
      DataMap.Deploy_Type.hyperdetect.value,
      DataMap.Deploy_Type.cyberengine.value
    ],
    this.i18n.get('deploy_type')
  );
  tabs = [
    {
      id: JOB_ORIGIN_TYPE.EXE,
      label: this.i18n.get('common_exeing_job_label'),
      statusList: this.isOceanProtect
        ? [
            DataMap.Job_status.running.value,
            DataMap.Job_status.initialization.value,
            DataMap.Job_status.pending.value,
            DataMap.Job_status.aborting.value,
            DataMap.Job_status.dispatching.value,
            DataMap.Job_status.redispatch.value
          ]
        : [
            DataMap.Job_status.running.value,
            DataMap.Job_status.initialization.value,
            DataMap.Job_status.pending.value,
            DataMap.Job_status.aborting.value
          ]
    },
    {
      id: JOB_ORIGIN_TYPE.HISTORIC,
      label: this.i18n.get('common_historic_job_label'),
      statusList: map(
        reject(this.dataMapService.toArray('Job_status'), item =>
          includes(
            [
              DataMap.Job_status.running.value,
              DataMap.Job_status.initialization.value,
              DataMap.Job_status.pending.value,
              DataMap.Job_status.aborting.value,
              DataMap.Job_status.dispatching.value,
              DataMap.Job_status.redispatch.value,
              DataMap.Job_status.dispatch_failed.value
            ],
            item.value
          )
        ),
        'value'
      )
    }
  ];
  detailSubType;

  cacheFilter: any;

  @ViewChild(JobTableComponent, { static: false })
  jobTableComponent: JobTableComponent;

  constructor(
    public i18n: I18NService,
    private globalService: GlobalService,
    private dataMapService: DataMapService,
    private switchSerivce: SwitchService,
    private cookieService: CookieService,
    private usersApiService: UsersApiService
  ) {}

  ngOnDestroy() {
    this.JobStatisticsEvent.unsubscribe();
  }

  ngOnInit() {
    this.getCurrentUser();
    this.watchStore();
  }

  ngAfterViewInit() {
    if (!this.selectedResource) {
      this.getCopyExpiredConfig();
    } else {
      this.jobTableComponent?.dataTable.fetchData();
    }
  }

  getData() {
    this.jobTableComponent?.dataTable.fetchData();
  }

  initDetailData(data: any) {
    this.selectedResource = data;
    this.getData();
  }

  afterChange = item => {
    setTimeout(() => {
      if (!isEmpty(this.statusList)) {
        this.upDateTableFilter(true);
      } else {
        this.upDateTableFilter();
      }
    });
  };

  watchStore() {
    this.JobStatisticsEvent = this.globalService
      .getState('JobStatisticsEvent')
      .subscribe(res => {
        let { activeIndex, statusList } = res;

        this.statusList = statusList;
        if (this.activeIndex === activeIndex) {
          this.upDateTableFilter(true);
        } else {
          this.activeIndex = activeIndex;
        }
      });
  }

  showCopyExpired() {
    if (this.disableCheckBox) {
      return;
    }

    this.switchSerivce
      .UpdateSystemSwitchApi({
        UpdateSystemSwitchApiRequestBody: {
          name: 'COPY_EXPIRE_JOB_EXHIBITION' as any,
          status: !this.showExpiredCopy ? 1 : 0
        }
      })
      .subscribe(res => {
        this.showExpiredCopy = !this.showExpiredCopy;
        this.jobTableComponent.showExpiredCopy = this.showExpiredCopy;
        this.upDateTableFilter();
      });
  }

  upDateTableFilter(jobStatisticsEvent = false) {
    this.jobTableComponent.showExpiredCopy = this.showExpiredCopy;
    each(this.jobTableComponent?.tableConfig.table.columns, item => {
      if (item.key === 'type') {
        assign(item, {
          filter: {
            type: 'select',
            isMultiple: true,
            showCheckAll: true,
            showSearch: true,
            options: this.jobTableComponent?.getTypeOptions()
          }
        });
      }
    });

    if (jobStatisticsEvent) {
      this.jobTableComponent.dataTable.setFilterMap(
        assign(this.jobTableComponent.dataTable.filterMap, {
          filters: [
            {
              caseSensitive: false,
              key: 'status',
              value: [...this.statusList]
            }
          ]
        })
      );

      this.statusList = [];
    }
    this.jobTableComponent?.dataTable.fetchData();
  }

  getCopyExpiredConfig() {
    this.switchSerivce.ListSystemSwitchApi({}).subscribe({
      next: res => {
        const switches = get(res, 'switches', []);
        this.showExpiredCopy = !!get(
          find(switches, item => {
            return get(item, 'name') === 'COPY_EXPIRE_JOB_EXHIBITION';
          }),
          'status'
        );
        this.upDateTableFilter();
      },
      error: () => {
        this.showExpiredCopy = false;
        this.upDateTableFilter();
      }
    });
  }

  getCurrentUser() {
    const userId = this.cookieService.get('userId');
    this.usersApiService
      .getUsingGET2({ userId: userId, akDoException: false })
      .subscribe(res => {
        this.disableCheckBox =
          Number(get(first(get(res, 'rolesSet', [])), 'roleId')) !== 1;
      });
  }
}
