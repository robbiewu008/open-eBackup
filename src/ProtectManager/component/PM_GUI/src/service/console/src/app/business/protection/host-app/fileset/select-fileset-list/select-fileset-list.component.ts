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
  Component,
  OnInit,
  Pipe,
  PipeTransform,
  ViewChild
} from '@angular/core';
import { ModalRef } from '@iux/live';
import {
  CommonConsts,
  DataMap,
  extendSlaInfo,
  I18NService,
  ProtectedResourceApiService,
  ProtectResourceCategory
} from 'app/shared';
import {
  assign,
  each,
  filter,
  find,
  isEmpty,
  size,
  trim,
  cloneDeep,
  reject
} from 'lodash';
import { Subject } from 'rxjs';

@Pipe({ name: 'selectionPipe' })
export class SelectionPipe implements PipeTransform {
  transform(value: any[], exponent: string = 'sla_id') {
    return filter(value, item => isEmpty(item[exponent]));
  }
}

@Component({
  selector: 'aui-select-fileset-list',
  templateUrl: './select-fileset-list.component.html',
  styleUrls: ['./select-fileset-list.component.less']
})
export class SelectFilesetListComponent implements OnInit {
  name;
  environmentIp;
  resourceData;
  resourceType;
  selectionData = [];
  tabActiveIndex = 0;
  totalTableData = [];
  selectedTableData = [];
  total = 0;
  pageIndex = CommonConsts.PAGE_START;
  pageSize = CommonConsts.PAGE_SIZE;
  sizeOptions = CommonConsts.PAGE_SIZE_OPTIONS;
  protectResourceCategory = ProtectResourceCategory;
  columns = [
    {
      label: this.i18n.get('common_name_label'),
      key: 'name'
    },
    {
      label: this.i18n.get('common_ip_address_label'),
      key: 'environment_endpoint'
    },
    {
      label: this.i18n.get('common_sla_label'),
      key: 'sla_name'
    }
  ];
  @ViewChild('namePopover', { static: false }) namePopover;
  @ViewChild('environmentIpPopover', { static: false }) environmentIpPopover;
  filterParams = { subType: [DataMap.Resource_Type.fileset.value] };
  valid$ = new Subject<boolean>();

  constructor(
    private i18n: I18NService,
    private modal: ModalRef,
    private protectedResourceApiService: ProtectedResourceApiService
  ) {}

  ngOnInit() {
    this.getFileset();
  }

  getFileset() {
    const params = {
      pageNo: this.pageIndex,
      pageSize: this.pageSize
    };

    each(this.filterParams, (value, key) => {
      if (isEmpty(value)) {
        delete this.filterParams[key];
      }
    });

    if (!isEmpty(this.filterParams)) {
      assign(params, {
        conditions: JSON.stringify(this.filterParams)
      });
    }
    this.protectedResourceApiService.ListResources(params).subscribe(res => {
      each(res.records, item => {
        assign(item, {
          sub_type: item.subType,
          environment_name: item.environment?.name,
          environment_endpoint: item.environment?.endpoint,
          template_name: item.extendInfo?.templateName
        });
        extendSlaInfo(item);
      });
      this.totalTableData = res.records;
      this.total = res.totalCount;
    });
  }

  searchByName(event) {
    if (this.namePopover) {
      this.namePopover.hide();
    }
    assign(this.filterParams, {
      name: trim(this.name)
    });
    this.getFileset();
  }

  searchByEnvironmentIp(event) {
    if (this.environmentIpPopover) {
      this.environmentIpPopover.hide();
    }
    assign(this.filterParams, {
      environment_endpoint: trim(this.environmentIp)
    });
    this.getFileset();
  }

  selectionChange() {
    this.valid$.next(!!size(this.selectionData));
  }

  pageChange(page) {
    this.pageSize = page.pageSize;
    this.pageIndex = page.pageIndex;
    this.getFileset();
  }

  optsCallback = item => {
    return [
      {
        id: 'delete',
        label: this.i18n.get('common_remove_label'),
        onClick: () => {
          this.selectionData = reject(this.selectionData, value => {
            return value.uuid === item.uuid;
          });
          this.selectionChange();
        }
      }
    ];
  };

  filterChange = e => {};

  beforeChange = (origin, active) => {};

  initData(data: any, resourceType: string) {
    this.resourceData = data;
    this.resourceType = resourceType;
    this.selectionData = cloneDeep(data);
    if (data.length <= 20) {
      this.pageSize = 20;
    } else if (data.length > 20 && data.length <= 50) {
      this.pageSize = 50;
    } else {
      this.pageSize = 100;
    }
  }

  onOK() {
    return { selectedList: this.selectionData };
  }
}
