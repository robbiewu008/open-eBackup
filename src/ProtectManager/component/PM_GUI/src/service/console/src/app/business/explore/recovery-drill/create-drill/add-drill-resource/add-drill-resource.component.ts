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
import { Component, OnInit, ViewChild } from '@angular/core';
import { FormBuilder, FormControl, FormGroup } from '@angular/forms';
import { ModalRef } from '@iux/live';
import {
  ApiService,
  BaseUtilService,
  CommonConsts,
  DataMap,
  I18NService
} from 'app/shared';
import {
  Filters,
  ProTableComponent,
  TableConfig,
  TableData
} from 'app/shared/components/pro-table';
import { AppUtilsService } from 'app/shared/services/app-utils.service';
import {
  assign,
  defer,
  each,
  find,
  includes,
  intersection,
  isArray,
  isEmpty
} from 'lodash';

@Component({
  selector: 'aui-add-drill-resource',
  templateUrl: './add-drill-resource.component.html',
  styleUrls: ['./add-drill-resource.component.less']
})
export class AddDrillResourceComponent implements OnInit {
  formGroup: FormGroup;
  resourceTypeOptions = [];
  tableConfig: TableConfig;
  tableData: TableData;
  selectionData = [];
  selectedResoureType;

  @ViewChild('dataTable', { static: false }) dataTable: ProTableComponent;

  constructor(
    private fb: FormBuilder,
    private modal: ModalRef,
    private i18n: I18NService,
    private exerciseService: ApiService,
    private appUtilsService: AppUtilsService,
    private baseUtilService: BaseUtilService
  ) {}

  ngOnInit(): void {
    this.initOptions();
  }

  disbaleOkBtn() {
    this.modal.getInstance().lvOkDisabled =
      this.formGroup.invalid || isEmpty(this.selectionData);
  }

  clearTable() {
    this.selectionData = [];
    this.dataTable?.setSelections([]);
  }

  initOptions() {
    this.formGroup = this.fb.group({
      resourceType: new FormControl('', {
        validators: [this.baseUtilService.VALID.required()]
      })
    });

    this.formGroup.get('resourceType').valueChanges.subscribe(res => {
      if (res) {
        this.selectedResoureType = find(this.resourceTypeOptions, {
          value: res
        })?.key;
        // exchange只演练数据库
        if (res === 'exchange') {
          this.selectedResoureType = [
            DataMap.Resource_Type.ExchangeDataBase.value
          ];
        }
        if (res === 'saphana') {
          this.selectedResoureType = [
            DataMap.Resource_Type.saphanaDatabase.value
          ];
        }
        defer(() => this.dataTable.fetchData());
      }
      this.clearTable();
      this.disbaleOkBtn();
    });

    const apps = [];
    each(this.appUtilsService.getApplicationConfig().database, app => {
      apps.push(
        assign(app, {
          isLeaf: true,
          value: app.id
        })
      );
    });
    // 新增云平台的GuassDB，应用的exchange，AD域
    each(this.appUtilsService.getApplicationConfig().cloud, app => {
      if (app.id !== 'lightcloudgaussdb') {
        return;
      }
      apps.push(
        assign(app, {
          isLeaf: true,
          value: app.id
        })
      );
    });
    each(this.appUtilsService.getApplicationConfig().application, app => {
      if (!includes(['exchange', 'saphana'], app.id)) {
        return;
      }
      apps.push(
        assign(app, {
          isLeaf: true,
          value: app.id
        })
      );
    });
    this.resourceTypeOptions = apps;

    this.tableConfig = {
      table: {
        compareWith: 'uuid',
        columns: [
          {
            key: 'uuid',
            name: this.i18n.get('protection_resource_id_label'),
            filter: {
              type: 'search',
              filterMode: 'contains'
            }
          },
          {
            key: 'name',
            name: this.i18n.get('common_name_label'),
            filter: {
              type: 'search',
              filterMode: 'contains'
            }
          },
          {
            key: 'path',
            name: this.i18n.get('common_location_label'),
            filter: {
              type: 'search',
              filterMode: 'contains'
            }
          }
        ],
        rows: {
          selectionMode: 'multiple',
          selectionTrigger: 'selector',
          showSelector: true
        },
        scrollFixed: true,
        colDisplayControl: false,
        fetchData: (filters: Filters) => this.getResource(filters),
        selectionChange: selection => {
          this.selectionData = selection;
          this.disbaleOkBtn();
        },
        trackByFn: (_, item) => {
          return item.uuid;
        }
      },
      pagination: {
        winTablePagination: true,
        mode: 'simple'
      }
    };
  }

  getDefaultConditions() {
    const conditions = {
      subType: isArray(this.selectedResoureType)
        ? this.selectedResoureType
        : [this.selectedResoureType]
    };
    return conditions;
  }

  needAddTopInstance(subTypeList: string[]): boolean {
    return !isEmpty(
      intersection(subTypeList, [
        DataMap.Resource_Type.AntDBInstance.value,
        DataMap.Resource_Type.AntDBClusterInstance.value,
        DataMap.Resource_Type.Dameng_cluster.value,
        DataMap.Resource_Type.Dameng_singleNode.value,
        DataMap.Resource_Type.MySQLInstance.value,
        DataMap.Resource_Type.MySQLClusterInstance.value,
        DataMap.Resource_Type.PostgreSQLInstance.value,
        DataMap.Resource_Type.PostgreSQLClusterInstance.value,
        DataMap.Resource_Type.informixInstance.value,
        DataMap.Resource_Type.informixClusterInstance.value,
        DataMap.Resource_Type.KingBaseInstance.value,
        DataMap.Resource_Type.KingBaseClusterInstance.value
      ])
    );
  }

  getResource(filters) {
    if (isEmpty(this.selectedResoureType)) {
      return;
    }
    const params = {
      pageNo: filters.paginator.pageIndex + 1,
      pageSize: filters.paginator.pageSize,
      subTypeList: isArray(this.selectedResoureType)
        ? this.selectedResoureType
        : [this.selectedResoureType]
    };

    // 一些应用需要加顶层资源标签
    if (this.needAddTopInstance(params.subTypeList)) {
      assign(params, {
        isTopInstance: '1'
      });
    }

    if (!isEmpty(filters.conditions)) {
      const conditionsTemp = JSON.parse(filters.conditions);
      assign(params, conditionsTemp);
    }

    this.exerciseService
      .queryExerciseResourcesUsingGET(params)
      .subscribe((res: any) => {
        this.tableData = {
          data: res?.records || [],
          total: res?.total || 0
        };
      });
  }
}
