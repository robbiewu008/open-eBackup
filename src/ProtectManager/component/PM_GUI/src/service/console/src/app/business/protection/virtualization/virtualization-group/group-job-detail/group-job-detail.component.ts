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
import { DatePipe } from '@angular/common';
import {
  Component,
  Input,
  OnDestroy,
  OnInit,
  TemplateRef,
  ViewChild
} from '@angular/core';
import { MessageService, ModalRef } from '@iux/live';
import {
  DataMap,
  DataMapService,
  I18NService,
  JobAPIService,
  SYSTEM_TIME
} from 'app/shared';
import { SystemTimeService } from 'app/shared/services/system-time.service';
import { chunk, cloneDeep, each, first, includes, values } from 'lodash';
import { Subject, Subscription, timer } from 'rxjs';
import { map, switchMap, takeUntil } from 'rxjs/operators';
import { BaseTableComponent } from '../../virtualization-base/base-table/base-table.component';
import { AppUtilsService } from 'app/shared/services/app-utils.service';

@Component({
  selector: 'aui-group-job-detail',
  templateUrl: './group-job-detail.component.html',
  styleUrls: ['./group-job-detail.component.less'],
  providers: [DatePipe]
})
export class GroupJobDetailComponent implements OnInit, OnDestroy {
  job;
  isSelectedResource;
  detailData;
  dataMap = DataMap;
  timeZone = SYSTEM_TIME.timeZone;
  jobForms = {};
  _values = values;
  jobDestroy$ = new Subject();
  jobSubscription$ = new Subscription();

  baseTableComponent: BaseTableComponent;

  @ViewChild('headerTpl', { static: true }) headerTpl: TemplateRef<any>;

  constructor(
    private i18n: I18NService,
    private datePipe: DatePipe,
    private dataMapService: DataMapService,
    private jobApiService: JobAPIService,
    private systemTimeService: SystemTimeService,
    public modal: ModalRef,
    public messageService: MessageService,
    private appUtilsService: AppUtilsService
  ) {}
  ngOnDestroy(): void {
    this.jobDestroy$.next(true);
    this.jobDestroy$.complete();
  }

  ngOnInit(): void {
    if (this.isSelectedResource) {
      this.modal.setProperty({ lvHeader: this.headerTpl });
    }
    this.getJob();
  }

  optCallback = data => {
    return this.detailData.optItems || [];
  };

  getModalHeader() {
    this.modal.setProperty({ lvHeader: this.headerTpl });
  }
  getJob() {
    if (this.jobSubscription$) {
      this.jobSubscription$.unsubscribe();
    }
    this.jobSubscription$ = timer(0, 5 * 1e3)
      .pipe(
        switchMap(index => {
          return this.jobApiService
            .queryJobsUsingGET({
              jobId: this.job?.jobId,
              akLoading: !index
            })
            .pipe(
              map(res => {
                return first(res.records);
              })
            );
        }),
        takeUntil(this.jobDestroy$)
      )
      .subscribe(result => {
        this.job = {
          ...cloneDeep(this.job),
          ...result
        };
        this.initJobForms();
      });
  }

  initJobForms() {
    this.jobForms = {
      basicInfo: {
        title: this.i18n.get('common_basic_info_label'),
        keys: ['jobId', 'status', 'startTime', 'endTime', 'durationTime'],
        values: []
      },
      targetInfo: {
        title: this.i18n.get('insight_job_target_object_label'),
        keys: ['sourceName', 'sourceSubType'],
        values: []
      }
    };

    for (const key in this.jobForms) {
      const array = this.jobForms[key];
      each(array.keys, prop => {
        this.jobForms[key]['values'].push({
          key: prop,
          value: this.getValue(prop, this.job[prop]),
          label: this.i18n.get(`insight_job_${prop.toLowerCase()}_label`)
        });
      });
      array.values = chunk(array.values, key === 'targetInfo' ? 2 : 5);
    }
  }

  getValue(key, value) {
    switch (key) {
      case 'startTime':
      case 'endTime':
        value = this.datePipe.transform(
          value,
          'yyyy/MM/dd HH:mm:ss',
          this.timeZone
        );
        break;
      case 'sourceSubType':
        value = this.dataMapService.getLabel('Job_Target_Type', value);
        break;
      case 'durationTime':
        const systemTime = this.appUtilsService.getCurrentSysTime();
        value = this.job.getDuration(
          includes(
            [
              DataMap.Job_status.running.value,
              DataMap.Job_status.initialization.value,
              DataMap.Job_status.pending.value,
              DataMap.Job_status.aborting.value
            ],
            this.job.status
          )
            ? systemTime - this.job.startTime < 0
              ? 0
              : systemTime - this.job.startTime
            : this.job.endTime
            ? this.job.endTime - this.job.startTime
            : 0
        );
        break;
      default:
        break;
    }
    return value;
  }
}
