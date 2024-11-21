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
  DatabasesService,
  DataMap,
  DataMapService,
  extendSlaInfo,
  I18NService,
  ProtectedResourceApiService,
  ProtectResourceCategory
} from 'app/shared';
import { assign, cloneDeep, each, filter, isEmpty, size, trim } from 'lodash';
import { Subject } from 'rxjs';

@Pipe({ name: 'selectionPipe' })
export class SelectionPipe implements PipeTransform {
  transform(value: any[], exponent: string = 'sla_id') {
    return filter(
      value,
      item =>
        isEmpty(item[exponent]) &&
        item.link_status ===
          DataMap.Database_Resource_LinkStatus.normal.value &&
        item.verify_status
    );
  }
}

@Component({
  selector: 'aui-select-database-list',
  templateUrl: './select-database-list.component.html',
  styleUrls: ['./select-database-list.component.less']
})
export class SelectDatabaseListComponent implements OnInit {
  name;
  parent_name;
  checkName;
  checkParentName;
  activeIndex = 1;
  resourceData;
  resourceType;
  selectionData = [];
  tabActiveIndex = 0;
  totalTableData = [];
  selectedTableData = [];
  filterParams: any = {};
  total = 0;
  pageIndex = CommonConsts.PAGE_START;
  pageSize = CommonConsts.PAGE_SIZE_SMALL;
  sizeOptions = CommonConsts.PAGE_SIZE_OPTIONS;
  protectResourceCategory = ProtectResourceCategory;
  columns = [
    {
      label: this.i18n.get('common_name_label'),
      key: 'name'
    },
    {
      label: this.i18n.get('common_type_label'),
      key: 'type'
    },
    {
      label: this.i18n.get('common_sla_label'),
      key: 'sla_name'
    }
  ];
  @ViewChild('namePopover', { static: false }) namePopover;
  @ViewChild('parentNamePopover', { static: false }) parentNamePopover;
  @ViewChild('page', { static: false }) page;
  valid$ = new Subject<boolean>();
  dataMap = DataMap;

  constructor(
    private i18n: I18NService,
    private protectedResourceApiService: ProtectedResourceApiService
  ) {}

  ngOnInit() {
    this.getDatabase();
  }

  getDatabase() {
    const params = {
      pageNo: this.pageIndex,
      pageSize: this.pageSize
    };

    each(this.filterParams, (value, key) => {
      if (isEmpty(value)) {
        delete this.filterParams[key];
      }
    });

    this.filterParams = {
      ...this.filterParams,
      subType: [
        DataMap.Resource_Type.oracle.value,
        DataMap.Resource_Type.oracleCluster.value
      ]
    };

    if (!isEmpty(this.filterParams)) {
      assign(params, {
        conditions: JSON.stringify({ ...this.filterParams })
      });
    }
    this.protectedResourceApiService.ListResources(params).subscribe(res => {
      each(res.records, (item: any) => {
        item.sub_type = item.subType;
        item.link_status = item.extendInfo?.linkStatus;
        item.verify_status = item.extendInfo?.verify_status === 'true';
        extendSlaInfo(item);
      });
      this.total = res.totalCount;
      this.totalTableData = res.records;
    });
  }

  searchByName(event) {
    if (this.namePopover) {
      this.namePopover.hide();
    }
    assign(this.filterParams, {
      name: trim(this.name)
    });
    this.getDatabase();
  }

  searchByParentName(event) {
    if (this.parentNamePopover) {
      this.parentNamePopover.hide();
    }
    assign(this.filterParams, {
      parent_name: trim(this.parent_name)
    });
    this.getDatabase();
  }

  selectionChange() {
    this.valid$.next(!!size(this.selectionData));
  }

  pageChange(page) {
    this.pageSize = page.pageSize;
    this.pageIndex = page.pageIndex;
    this.getDatabase();
  }

  filterChange = e => {
    assign(this.filterParams, {
      [e.key]: trim(e.value)
    });
    this.getDatabase();
  };

  beforeChange = (origin, active) => {};

  initData(data: any, resourceType: string) {
    this.resourceData = data;
    this.resourceType = resourceType;
    this.selectionData = cloneDeep(data);
  }

  onOK() {
    return { selectedList: this.selectionData };
  }
}
