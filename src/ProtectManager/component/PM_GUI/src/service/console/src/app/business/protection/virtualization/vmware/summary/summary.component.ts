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
import { endWith } from 'rxjs/operators';
import { Component, OnInit, ViewChild } from '@angular/core';
import { DatatableComponent, ModalRef, PaginatorComponent } from '@iux/live';
import {
  CommonConsts,
  DataMapService,
  ResourceType,
  VirtualResourceService
} from 'app/shared';
import { assign, each, isEmpty, sortBy, trim, endsWith, defer } from 'lodash';

@Component({
  selector: 'aui-vmware-summary',
  templateUrl: './summary.component.html',
  styleUrls: ['./summary.component.less']
})
export class SummaryComponent implements OnInit {
  item;
  resourceType = ResourceType;
  type = ResourceType.HOST; // 主机和集群的总览页面相似，同一个组件，type用于区分是主机还是集群

  basicInfo;
  slaInfo;

  vmPageIndex = CommonConsts.PAGE_START;
  vmPageSize = CommonConsts.PAGE_SIZE_SMALL;
  vmTableData = [];
  vmTotal = 0;
  queryVmName;
  queryVmSlaName;
  slaStatusFilterMap = this.dataMapService.toArray('Sla_Status');

  // 槽位号的枚举值
  diskTableData = [];
  queryDiskName;
  queryDiskSlaName;
  diskPageSize = CommonConsts.PAGE_SIZE_SMALL;

  hostTableData = [];
  queryHostName;
  hostPageIndex = CommonConsts.PAGE_START;
  hostPageSize = CommonConsts.PAGE_SIZE_SMALL;
  hostTotal;

  filterParams = {};
  tabActiveIndex = ResourceType.VM;

  @ViewChild('headerTpl', { static: true }) headerTpl;

  @ViewChild(DatatableComponent, { static: false }) lvTable: DatatableComponent;
  @ViewChild(PaginatorComponent, { static: false }) lvPage: PaginatorComponent;

  constructor(
    private dataMapService: DataMapService,
    private modal: ModalRef,
    private virtualResourceService: VirtualResourceService
  ) {}

  ngOnInit() {
    this.getModalHeader();
    this.getBasicInfo();
    if (this.item.resType === ResourceType.VM) {
      this.getDiskData();
      defer(() => this.tabChange(this.tabActiveIndex));
    }
  }

  tabChange(event) {
    this.filterParams = {};
    this.lvTable.removeFilter();
    this.slaStatusFilterMap.forEach(filter => {
      filter['selected'] = false;
    });
    this.slaStatusFilterMap = [...this.slaStatusFilterMap];
    if (event === ResourceType.VM) {
      this.queryVmName = '';
      this.queryVmSlaName = '';
      this.vmPageIndex = CommonConsts.PAGE_START;
      this.vmPageSize = CommonConsts.PAGE_SIZE_SMALL;
      this.getVmData();
    } else if (event === ResourceType.HOST) {
      this.queryHostName = '';
      this.hostPageIndex = CommonConsts.PAGE_START;
      this.hostPageSize = CommonConsts.PAGE_SIZE_SMALL;
      this.getHostData();
    } else {
      this.queryDiskName = '';
      this.queryDiskSlaName = '';
    }
  }

  getBasicInfo() {
    this.basicInfo = {
      icon:
        this.type === ResourceType.HOST
          ? 'aui-icon-summary-host'
          : 'aui-sla-cluster',
      name: this.item.name,
      type: this.item.type,
      linkStatus: this.item.link_status
    };

    this.slaInfo = {
      icon: 'aui-icon-summary-sla',
      activation: this.item.sla_status,
      sla: this.item.sla_name,
      slaCompliance: this.item.sla_compliance
    };
  }

  getParams() {
    each(this.filterParams, (value, key) => {
      if (isEmpty(value)) {
        delete this.filterParams[key];
      }
    });
  }

  hostPageChange(page) {
    this.hostPageSize = page.pageSize;
    this.hostPageIndex = page.pageIndex;
    this.getHostData();
  }

  getHostData() {
    this.getParams();
    this.virtualResourceService
      .queryResourcesV1VirtualResourceGet({
        pageNo: this.hostPageIndex,
        pageSize: this.hostPageSize,
        conditions: JSON.stringify({
          parent_uuid: this.item.uuid,
          type: ResourceType.HOST,
          ...this.filterParams
        })
      })
      .subscribe(res => {
        this.hostTableData = res.items;
        this.hostTotal = res.total;
      });
  }

  getVmData() {
    this.getParams();
    this.virtualResourceService
      .queryResourcesV1VirtualResourceGet({
        pageNo: this.vmPageIndex,
        pageSize: this.vmPageSize,
        conditions: JSON.stringify({
          path: endsWith(this.item.path, '/')
            ? this.item.path
            : `${this.item.path}/`,
          type: ResourceType.VM,
          ...this.filterParams
        })
      })
      .subscribe(res => {
        this.vmTableData = res.items;
        this.vmTotal = res.total;
      });
  }

  vmPageChange(page) {
    this.vmPageSize = page.pageSize;
    this.vmPageIndex = page.pageIndex;
    this.getVmData();
  }

  vmFilterChange(e) {
    assign(this.filterParams, { [e.key]: e.value });
    this.getVmData();
  }

  vmOptsCallback = data => {
    return this.item.optItemFunc(data, () => {
      this.getVmData();
    });
  };

  getDiskData() {
    this.diskTableData = [];
    //  IDE 4个槽位号 0:0 0:1 1:0 1:1
    //  SCSI 64个槽位号 0:0-0:15 1:0-1:15 2:0-2:15 3:0-3:15
    //  SATA 120个槽位号 0:0-0:29 1:0-1:29 2:0-2:29 3:0-3:29
    //  NVME 60个槽位号 0:0-0:14 1:0-1:14 2:0-2:14 3:0-3:14
    const diskList = [
      { label: 'IDE', x: 2, y: 2 },
      { label: 'SATA', x: 4, y: 30 },
      { label: 'SCSI', x: 4, y: 16 },
      { label: 'NVME', x: 4, y: 15 }
    ];
    const tableData = [];
    const diskFilters = this.item.ext_parameters
      ? this.item.ext_parameters.disk_filters[0].values
      : [];
    each(diskList, item => {
      for (let i = 0; i < item.x; i++) {
        for (let j = 0; j < item.y; j++) {
          tableData.push({
            slot: `${item.label}(${i}:${j})`,
            sla_name:
              diskFilters[0] === '*' ||
              diskFilters.includes(`${item.label}(${i}:${j})`)
                ? this.item.sla_name
                : ''
          });
        }
      }
    });
    this.diskTableData = sortBy(tableData, item => !item.sla_name);
  }

  searchByHostName(queryHostName) {
    assign(this.filterParams, {
      name: trim(queryHostName)
    });
    this.getHostData();
  }

  searchByVmName(queryVmName) {
    assign(this.filterParams, {
      name: trim(queryVmName)
    });
    this.getVmData();
  }

  searchByVmSlaName(queryVmSlaName) {
    assign(this.filterParams, {
      sla_name: trim(queryVmSlaName)
    });
    this.getVmData();
  }

  searchByDiskName(queryDiskName) {
    this.lvTable.filter({
      key: 'slot',
      value: trim(queryDiskName),
      filterMode: 'contains'
    });
    this.lvPage.jumpToFisrtPage();
  }

  searchByDiskSlaName(queryDiskSlaName) {
    this.lvTable.filter({
      key: 'sla_name',
      value: trim(queryDiskSlaName),
      filterMode: 'contains'
    });
    this.lvPage.jumpToFisrtPage();
  }

  getModalHeader() {
    this.modal.setProperty({ lvHeader: this.headerTpl });
  }
}
