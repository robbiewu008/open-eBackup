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
import {
  DataMap,
  I18NService,
  ProtectedResourceApiService,
  CookieService,
  IODETECTFILESYSTEMService
} from 'app/shared';
import { assign, each, isArray, isEmpty, size } from 'lodash';
import { Subject } from 'rxjs';
import { map } from 'rxjs/operators';

@Component({
  selector: 'aui-select-dorado-list',
  templateUrl: './select-dorado-list.component.html',
  styleUrls: ['./select-dorado-list.component.less'],
  changeDetection: ChangeDetectionStrategy.OnPush
})
export class SelectDoradoListComponent implements OnInit {
  title = this.i18n.get('protection_selected_file_system_label');
  allTableData = {};
  resourceData: any = [];
  selectionData = [];
  valid$ = new Subject<boolean>();
  isRealDetection = false;
  resourceType;

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
      key: 'parentName',
      name: this.i18n.get('protection_equipment_name_label')
    },
    {
      key: 'tenantName',
      name: this.i18n.get('common_tenant_label'),
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

  isCyberEngine =
    this.i18n.get('deploy_type') === DataMap.Deploy_Type.cyberengine.value;

  constructor(
    private i18n: I18NService,
    private cdr: ChangeDetectorRef,
    private cookieService: CookieService,
    private detectFilesystemService: IODETECTFILESYSTEMService,
    private protectedResourceApiService: ProtectedResourceApiService
  ) {}

  ngOnInit() {
    this.initTitle();
    this.initColumns();
  }

  initTitle() {
    if (this.isCyberEngine) {
      this.title = this.i18n.get('common_selected_label');
    }
  }

  initColumns() {
    if (this.isRealDetection) {
      this.columns = [
        {
          key: 'fsName',
          name: this.i18n.get('common_name_label'),
          filter: {
            type: 'search',
            filterMode: 'contains'
          }
        },
        {
          key: 'deviceName',
          name: this.i18n.get('protection_storage_device_label'),
          filter: {
            type: 'search',
            filterMode: 'contains'
          }
        },
        {
          key: 'vstoreName',
          name: this.i18n.get('common_tenant_label'),
          filter: {
            type: 'search',
            filterMode: 'contains'
          }
        }
      ];
    }
  }

  updateTable(filters) {
    const params = {
      pageNo: filters.paginator.pageIndex,
      pageSize: filters.paginator.pageSize
    };

    if (this.isRealDetection) {
      this.detectFilesystemService
        .pageQueryProtectObject(params)
        .subscribe(res => {
          each(res.records, item => {
            assign(item, {
              isRealDetection: true,
              name: item.fsName,
              uuid: item.id
            });
          });
          this.allTableData = {
            data: res.records,
            total: res.totalCount
          };
        });
    } else {
      let subType = [];
      if (this.cookieService.isCloudBackup || this.isCyberEngine) {
        subType =
          this.resourceType === DataMap.Resource_Type.LocalLun.value
            ? [this.resourceType]
            : [DataMap.Resource_Type.LocalFileSystem.value];
      } else {
        subType = [
          DataMap.Resource_Type.NASFileSystem.value,
          DataMap.Resource_Type.ndmp.value
        ];
      }
      const defaultConditions = {
        subType: subType
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
                tenantName: item.extendInfo?.tenantName,
                sub_type: item.subType
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
  }

  dataChange(selection) {
    this.selectionData = selection;
    this.valid$.next(!!size(this.selectionData));
  }

  initData(data: any, resourceType: string) {
    this.resourceType = resourceType;
    if (this.resourceType === DataMap.Resource_Type.LocalLun.value) {
      this.title = this.i18n.get('protection_selected_lun_label');
    }
    this.resourceData = data;
    this.selectionData = data;
    this.isRealDetection = isArray(this.resourceData)
      ? this.resourceData[0]?.isRealDetection === true
      : this.resourceData.isRealDetection === true;
  }

  onOK() {
    return { selectedList: this.selectionData };
  }
}
