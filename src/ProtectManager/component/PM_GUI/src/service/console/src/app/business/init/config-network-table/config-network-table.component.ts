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
import { Component, Input, OnInit, ViewChild } from '@angular/core';
import { DatatableComponent } from '@iux/live';
import {
  BaseUtilService,
  DataMap,
  I18NService,
  SystemApiService
} from 'app/shared';
import { AppUtilsService } from 'app/shared/services/app-utils.service';
import { VirtualScrollService } from 'app/shared/services/virtual-scroll.service';
import {
  assign,
  each,
  every,
  find,
  isArray,
  isEmpty,
  size,
  trim
} from 'lodash';

@Component({
  selector: 'aui-config-network-table',
  templateUrl: './config-network-table.component.html',
  styleUrls: ['./config-network-table.component.less']
})
export class ConfigNetworkTableComponent implements OnInit {
  @Input() serviceType;
  @Input() selectionData;
  @Input() isManual = true;
  @Input() lldTableData;
  componentData = {};
  isCertify = true;
  dataMap = DataMap;
  isEverySelect = false;
  activeIndex = 0;

  tableData = [];
  selection = [];
  queryManageIp;
  queryPort;
  queryIpAdress;
  filterParams: any = {};
  filterListParams: any = {};
  backupSelectedData = {};
  replicationSelectedData = {};
  archivedSelectedData = {};

  @ViewChild('lvTable', { static: false }) lvTable: DatatableComponent;
  @ViewChild('lvSubTable', { static: false }) lvSubTable: DatatableComponent;
  @ViewChild('manageIpPopover', { static: false }) manageIpPopover;

  constructor(
    public baseUtilService: BaseUtilService,
    private i18n: I18NService,
    private systemApiService: SystemApiService,
    public virtualScroll: VirtualScrollService,
    public appUtilsService: AppUtilsService
  ) {}

  ngOnInit() {
    if (this.appUtilsService.isDistributed && this.serviceType === 'archived') {
      this.isEverySelect = true;
      assign(this.selectionData, {
        [this.serviceType]: [],
        [this.serviceType + 'Valid']: this.isEverySelect
      });
    }
    if (!this.isManual) {
      const arr = [];
      each(this.lldTableData.pacificInitNetWorkInfoList, item => {
        const tempIpPoolDtoList = item.ipInfoList.map(val => {
          return assign(val, {
            iface_name: val.ifaceName,
            ip_address: val.ipAddress
          });
        });
        arr.push({
          manageIp: item.manageIp,
          port: tempIpPoolDtoList?.length,
          expand: false,
          selection: tempIpPoolDtoList,
          ipPoolDtoList: tempIpPoolDtoList
        });
      });
      this.tableData = arr;
      this.selectionChange();
    } else {
      this.queryNetworkInfo();
    }
  }

  refresh() {
    this.queryNetworkInfo();
  }

  checkChange(source, manageIp) {
    const targetData = find(this.tableData, v => v.manageIp === manageIp);
    if (!isEmpty(source)) {
      if (isArray(source)) {
        targetData.port += size(source);
      } else {
        targetData.port++;
      }
    } else {
      if (isArray(source)) {
        targetData.port -= size(targetData.ipPoolDtoList);
      } else {
        targetData.port--;
      }
    }
  }

  selectionChange(source?) {
    if (this.serviceType === 'backup') {
      this.isEverySelect = every(
        this.tableData,
        item => size(item.selection) > 0
      );
    } else if (
      this.appUtilsService.isDistributed &&
      this.serviceType === 'archived'
    ) {
      this.isEverySelect =
        every(this.tableData, item => {
          return size(item.selection) > 0;
        }) ||
        every(this.tableData, item => {
          return size(item.selection) === 0;
        });
    } else {
      this.isEverySelect = true;
    }

    assign(this.selectionData, {
      [this.serviceType]: this.getSelectionData(),
      [this.serviceType + 'Valid']: this.isEverySelect
    });
  }
  getSelectionData() {
    const arr = [];
    each(this.tableData, item => {
      if (size(item.selection)) {
        const selectArr = [];
        each(item.selection, v => {
          selectArr.push({
            ipAddress: v.ip_address,
            ifaceName: v.iface_name
          });
        });
        arr.push({
          manageIp: item.manageIp,
          ipInfoList: selectArr
        });
      }
    });
    return arr;
  }

  searchByManageIp(value) {
    if (this.manageIpPopover) {
      this.manageIpPopover.hide();
    }
    if (this.isManual) {
      assign(this.filterParams, {
        manageIp: trim(value)
      });
      this.queryNetworkInfo();
    } else {
      this.lvTable.filter({
        key: 'manageIp',
        value: value,
        filterMode: 'contains'
      });
    }
  }

  searchByPort(value, item) {
    item.searchPort = value;
    if (this.isManual) {
      assign(this.filterListParams, {
        manageIp: trim(item.manageIp),
        ifaceName: trim(value)
      });
      this.queryNodeNetwokrInfo(item);
    } else {
      this.lvSubTable.filter({
        key: 'iface_name',
        value: value,
        filterMode: 'contains'
      });
    }
  }

  clearSearchByPort(_, item) {
    if (!item.searchPort) return;
    item.searchPort = '';
    if (this.isManual) {
      delete this.filterListParams.ifaceName;
      this.queryNodeNetwokrInfo(item);
    } else {
      this.lvSubTable.filter({
        key: 'iface_name',
        value: '',
        filterMode: 'contains'
      });
    }
  }

  getfilterPortValue(item) {
    return item.searchPort;
  }

  searchByIpAdress(value, item) {
    item.searchIpAdress = value;
    if (this.isManual) {
      assign(this.filterListParams, {
        manageIp: trim(item.manageIp),
        ipAddress: trim(value)
      });
      this.queryNodeNetwokrInfo(item);
    } else {
      this.lvSubTable.filter({
        key: 'ip_address',
        value: value,
        filterMode: 'contains'
      });
    }
  }

  clearSearchByIpAdress(_, item) {
    if (!item.searchIpAdress) return;
    item.searchIpAdress = '';
    if (this.isManual) {
      delete this.filterListParams.ipAddress;
      this.queryNodeNetwokrInfo(item);
    } else {
      this.lvSubTable.filter({
        key: 'ip_address',
        value: '',
        filterMode: 'contains'
      });
    }
  }

  getfilterIpValue(item) {
    return item.searchIpAdress;
  }

  queryNetworkInfo() {
    this.systemApiService.getNetworkInfo(this.filterParams).subscribe(res => {
      const arr = [];
      each(res.nodeNetworkInfoList, item => {
        arr.push({
          manageIp: item.manageIp,
          port: 0,
          expand: false,
          selection: [],
          ipPoolDtoList: item.ipPoolDtoList
        });
      });
      this.tableData = arr;
    });
  }

  queryNodeNetwokrInfo(item) {
    this.systemApiService
      .getNodeNetworkInfo(this.filterListParams)
      .subscribe(res => {
        item.ipPoolDtoList = res.ipPoolDtoList;
        item.port = 0;
        item.selection = [];
        this.selectionChange(null);
      });
  }
}
