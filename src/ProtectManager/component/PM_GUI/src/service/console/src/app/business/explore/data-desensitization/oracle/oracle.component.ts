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
  ChangeDetectionStrategy,
  ChangeDetectorRef,
  Component,
  OnDestroy,
  OnInit,
  ViewChild
} from '@angular/core';
import {
  CommonConsts,
  DataMap,
  I18NService,
  ProtectedResourceApiService,
  ResourceType,
  extendDesesitizationInfo,
  extendSlaInfo
} from 'app/shared';
import { assign, each, isEmpty } from 'lodash';
import { Subject, Subscription, timer } from 'rxjs';
import { map, switchMap, takeUntil } from 'rxjs/operators';
import { DataDesensitizationListComponent } from '../data-desensitization-list/data-desensitization-list.component';

@Component({
  selector: 'aui-oracle',
  templateUrl: './oracle.component.html',
  styleUrls: ['./oracle.component.less'],
  changeDetection: ChangeDetectionStrategy.OnPush
})
export class OracleComponent implements OnInit, OnDestroy {
  header = this.i18n.get('common_oracle_label');
  resourceType = ResourceType.DATABASE;
  childResourceType = [DataMap.Resource_Type.oracle.value];

  timeSub$: Subscription;
  destroy$ = new Subject();

  @ViewChild(DataDesensitizationListComponent, { static: true })
  component: DataDesensitizationListComponent;

  constructor(
    private i18n: I18NService,
    private cdr: ChangeDetectorRef,
    private protectedResourceApiService: ProtectedResourceApiService
  ) {}

  ngOnDestroy() {
    this.destroy$.next(true);
    this.destroy$.complete();
  }

  ngOnInit() {
    this.getResource();
  }

  getResource() {
    const params = {
      pageNo: this.component.pageIndex,
      pageSize: this.component.pageSize
    };

    each(this.component.filterParams, (value, key) => {
      if (isEmpty(value)) {
        delete this.component.filterParams[key];
      }
    });

    this.component.filterParams = {
      ...this.component.filterParams,
      subType: [
        DataMap.Resource_Type.oracle.value,
        DataMap.Resource_Type.oracleCluster.value
      ],
      isDesesitization: 'true'
    };

    if (!isEmpty(this.component.filterParams)) {
      assign(params, {
        conditions: JSON.stringify(this.component.filterParams)
      });
    }

    if (this.timeSub$) {
      this.timeSub$.unsubscribe();
    }

    this.timeSub$ = timer(0, CommonConsts.TIME_INTERVAL)
      .pipe(
        switchMap(index => {
          return this.protectedResourceApiService
            .ListResources({
              akLoading: !index,
              ...params
            })
            .pipe(
              map(res => {
                each(res.records, (item: any) => {
                  item.sub_type = DataMap.Resource_Type.oracle.value;
                  item.link_status = item.extendInfo?.linkStatus;
                  item.verify_status =
                    item.extendInfo?.verify_status === 'true';
                  item['instance_names'] = item.extendInfo?.inst_name;
                  item['environment_endpoint'] = item.environment?.endpoint;
                  extendSlaInfo(item);
                  extendDesesitizationInfo(item);
                });
                this.cdr.detectChanges();
                return res;
              })
            );
        }),
        takeUntil(this.destroy$)
      )
      .subscribe(res => {
        this.component.total = res.totalCount;
        this.component.tableData = res.records;
        this.cdr.detectChanges();
      });
  }
}
