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
  OnInit
} from '@angular/core';
import { DataMap, I18NService, ProtectedResourceApiService } from 'app/shared';
import { assign, each, isEmpty, size, isArray, find } from 'lodash';
import { Subject } from 'rxjs';
import { map } from 'rxjs/operators';

@Component({
  selector: 'aui-select-nas-shared-list',
  templateUrl: './select-nas-shared-list.component.html',
  styleUrls: ['./select-nas-shared-list.component.less'],
  changeDetection: ChangeDetectionStrategy.OnPush
})
export class SelectNasSharedListComponent implements OnInit {
  title = this.i18n.get('protection_selected_nas_label');
  allTableData = {};
  columns = [
    {
      key: 'name',
      name: this.i18n.get('common_name_label'),
      filter: {
        type: 'search',
        filterMode: 'contains'
      }
    },
    {
      key: 'ip',
      name: this.i18n.get('protection_shared_ip_label'),
      filter: {
        type: 'search',
        filterMode: 'contains'
      }
    },
    {
      key: 'sla_name',
      name: this.i18n.get('common_sla_label')
    }
  ];
  isNfsShareMode = false;
  resourceData = [];
  selectionData = [];
  valid$ = new Subject<boolean>();

  constructor(
    private i18n: I18NService,
    private cdr: ChangeDetectorRef,
    private protectedResourceApiService: ProtectedResourceApiService
  ) {}

  ngOnInit() {}

  updateTable(filters) {
    const params = {
      pageNo: filters.paginator.pageIndex,
      pageSize: filters.paginator.pageSize
    };

    const defaultConditions = {
      subType: [DataMap.Resource_Type.NASShare.value]
    };

    if (!isEmpty(filters.conditions_v2)) {
      assign(defaultConditions, JSON.parse(filters.conditions_v2));
    }

    assign(params, { conditions: JSON.stringify(defaultConditions) });

    if (!!size(filters.sort)) {
      assign(params, { orders: filters.orders });
    }

    this.protectedResourceApiService
      .ListResources(params)
      .pipe(
        map(res => {
          each(res.records, item => {
            assign(item, {
              sub_type: item.subType,
              ip: item.extendInfo?.ip
            });
          });
          return res;
        })
      )
      .subscribe(res => {
        this.allTableData = {
          total: res.totalCount,
          data: res.records
        };
        this.cdr.detectChanges();
      });
  }

  dataChange(selection) {
    this.selectionData = selection;
    this.valid$.next(!!size(this.selectionData));
  }

  initData(data: any) {
    this.resourceData = data;
    this.selectionData = data;
    this.isNfsShareMode = isArray(data)
      ? find(data, item => item.extendInfo?.shareMode === '1')
      : data.extendInfo?.shareMode === '1';
  }

  onOK() {
    return { selectedList: this.selectionData };
  }
}
