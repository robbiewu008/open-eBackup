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
  EventEmitter,
  Input,
  OnInit,
  Output,
  ViewChild
} from '@angular/core';
import { DatatableComponent } from '@iux/live';
import { CommonConsts, DataMap, I18NService, MODAL_COMMON } from 'app/shared';
import { DrawModalService } from 'app/shared/services/draw-modal.service';
import { RememberColumnsService } from 'app/shared/services/remember-columns.service';
import { assign, join, map } from 'lodash';
import { RouteTableComponent } from '../route-table/route-table.component';

@Component({
  selector: 'aui-config-table',
  templateUrl: './config-table.component.html',
  styleUrls: ['./config-table.component.less']
})
export class ConfigTableComponent implements OnInit {
  @Input() isModify;
  @Input() modifying;
  @Input() isLLD;
  @Input() data;
  @Output() onStatusChange = new EventEmitter<any>();
  ip;
  columns = [];
  pageSize = CommonConsts.PAGE_SIZE;
  pageIndex = CommonConsts.PAGE_START;
  pageSizeOptions = CommonConsts.PAGE_SIZE_OPTIONS;
  columnStatus;

  @ViewChild('lvTable', { static: false }) lvTable: DatatableComponent;
  @ViewChild('ipPopover', { static: false }) ipPopover;

  constructor(
    private i18n: I18NService,
    private drawModalService: DrawModalService,
    private rememberColumnsService: RememberColumnsService
  ) {}

  ngOnInit(): void {
    this.columnStatus = this.rememberColumnsService.getColumnsStatus(
      this.data.tableColsKey
    );
  }

  getMtu(item) {
    switch (item.homePortType) {
      case DataMap.initHomePortType.ethernet.value:
        return '--';
      case DataMap.initHomePortType.bonding.value:
        return item?.bondPort?.mtu || '--';
      case DataMap.initHomePortType.vlan.value:
        return item.vlan.mtu;
    }
  }

  getPort(item) {
    switch (item.homePortType) {
      case DataMap.initHomePortType.ethernet.value:
        return item?.homePortName || '--';
      case DataMap.initHomePortType.bonding.value:
        return item?.bondPort?.portNameList.join(',') || '--';
      case DataMap.initHomePortType.vlan.value:
        return item.vlan.portNameList.join(',');
    }
  }

  getRoute(item) {
    if (!item || !item?.length) {
      return '';
    } else {
      const propertyStrings = map(item, obj => {
        return `${obj.TYPE}/${obj.DESTINATION}/${obj.MASK}/${obj.GATEWAY}`;
      });
      return join(propertyStrings, '\n');
    }
  }

  viewRoute(item) {
    if (!item || !item?.length) {
      return;
    }
    this.drawModalService.create(
      assign({}, MODAL_COMMON.generateDrawerOptions(), {
        lvModalKey: 'route-config',
        lvWidth: MODAL_COMMON.largeWidth,
        lvHeader: this.i18n.get('common_route_config_label'),
        lvContent: RouteTableComponent,
        lvOkDisabled: false,
        lvComponentParams: {
          data: item
        },
        lvFooter: [
          {
            label: this.i18n.get('common_close_label'),
            onClick: modal => modal.close()
          }
        ]
      })
    );
  }

  nameChange(e) {
    this.onStatusChange.emit(e);
  }

  operationCallBack(action, data?, item?, name?) {
    if (action === 'modify' && !!item.modifyDisabled) {
      return;
    }
    this.onStatusChange.emit({ action, data, item, name });
  }

  filterChange(options: any) {
    this.lvTable.filter(options);
  }

  searchByIp(e, data) {
    data.isSearch = !!e;
    this.ipPopover.hide();
    this.lvTable.filter({
      key: 'ip',
      value: e,
      filterMode: 'contains'
    });
  }

  trackById(index: number, port: any) {
    return port.id;
  }

  pageChange(page) {
    this.pageSize = page.pageSize;
    this.pageIndex = page.pageIndex;
  }
}
