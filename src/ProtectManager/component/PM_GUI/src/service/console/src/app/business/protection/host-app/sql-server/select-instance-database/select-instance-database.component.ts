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
import { FormBuilder, FormControl } from '@angular/forms';
import {
  BaseUtilService,
  DataMap,
  extendSlaInfo,
  I18NService,
  ProtectedResourceApiService
} from 'app/shared';
import {
  assign,
  each,
  isEmpty,
  size,
  map,
  first,
  isArray,
  get,
  includes
} from 'lodash';
import { Subject } from 'rxjs';
import { map as _map } from 'rxjs/operators';

@Component({
  selector: 'aui-select-instance-database-sql-server',
  templateUrl: './select-instance-database.component.html',
  styleUrls: ['./select-instance-database.component.less'],
  changeDetection: ChangeDetectionStrategy.OnPush
})
export class SelectInstanceDatabaseComponent implements OnInit {
  allTableData = {};
  selectionData = [];
  resourceData = [];
  valid$ = new Subject<boolean>();
  title = this.i18n.get('protection_selected_pvc_number_label', ['']);

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
      key: 'clusterOrHostName',
      name: this.i18n.get('protection_host_cluster_name_label'),
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

  constructor(
    private i18n: I18NService,
    private fb: FormBuilder,
    private cdr: ChangeDetectorRef,
    public baseUtilService: BaseUtilService,
    private protectedResourceApiService: ProtectedResourceApiService
  ) {}

  ngOnInit() {}

  updateTable(filters) {
    const params = {
      pageNo: filters.paginator.pageIndex,
      pageSize: filters.paginator.pageSize
    };
    const subType = isArray(this.resourceData)
      ? first(this.resourceData).subType
      : get(this.resourceData, 'subType');
    const defaultConditions = {
      subType: includes(
        [
          DataMap.Resource_Type.SQLServerClusterInstance.value,
          DataMap.Resource_Type.SQLServerInstance.value
        ],
        subType
      )
        ? [
            DataMap.Resource_Type.SQLServerClusterInstance.value,
            DataMap.Resource_Type.SQLServerInstance.value
          ]
        : [subType]
    };

    if (subType === DataMap.Resource_Type.SQLServerDatabase.value) {
      assign(defaultConditions, {
        agId: [['=='], '']
      });
    }

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
        _map(res => {
          each(res.records, item => {
            this.dataProcess(subType, item);
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

  private dataProcess(subType, item) {
    switch (subType) {
      case DataMap.Resource_Type.SQLServerInstance.value:
      case DataMap.Resource_Type.SQLServerClusterInstance.value:
        assign(item, {
          clusterOrHostName: item.environment?.name
        });
        break;
      case DataMap.Resource_Type.SQLServerGroup.value:
        assign(item, {
          clusterOrHostName: item.parentName
        });
        break;
      case DataMap.Resource_Type.SQLServerDatabase.value:
        assign(item, {
          clusterOrHostName: item.extendInfo?.hostName
        });
        break;
    }
    extendSlaInfo(item);
    assign(item, {
      sub_type: item.subType,
      ip: item.extendInfo?.ip,
      disabled: !!get(item, 'sla_id')
    });
  }

  dataChange(selection) {
    this.selectionData = selection;
    this.valid$.next(!!size(this.selectionData));
  }

  initData(data: any) {
    this.resourceData = data;
    this.selectionData = data;
  }

  onOK() {
    return {
      selectedList: this.selectionData
    };
  }
}
