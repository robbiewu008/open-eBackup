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
import { Component, OnInit, TemplateRef, ViewChild } from '@angular/core';
import {
  CommonConsts,
  DataMapService,
  I18NService,
  ProtectedResourceApiService
} from 'app/shared';
import {
  TableCols,
  TableConfig,
  TableData
} from 'app/shared/components/pro-table';
import { assign, defer, each, find, isEmpty, size } from 'lodash';

@Component({
  selector: 'aui-openstack-summary',
  templateUrl: './summary.component.html',
  styleUrls: ['./summary.component.less']
})
export class SummaryComponent implements OnInit {
  source;
  tableConfig: TableConfig;
  tableData: TableData;
  vmType: string;
  network: string;

  @ViewChild('sizeTpl', { static: true }) sizeTpl: TemplateRef<any>;

  constructor(
    private i18n: I18NService,
    private dataMapService: DataMapService,
    private protectedResourceApiService: ProtectedResourceApiService
  ) {}

  ngOnInit(): void {
    this.initConfig();
    this.getResource();
  }

  initDetailData(data: any) {
    this.source = data || {};
    if (!this.source?.subType) {
      assign(this.source, {
        subType: this.source.sub_type,
        status: this.source?.extendInfo?.status
      });
    }
  }

  initConfig() {
    const cols: TableCols[] = [
      {
        key: 'nameId',
        name: this.i18n.get('common_name_label'),
        filter: {
          type: 'search',
          filterMode: 'contains'
        }
      },
      {
        key: 'bootable',
        name: this.i18n.get('common_type_label'),
        filter: {
          type: 'select',
          isMultiple: true,
          showCheckAll: true,
          options: this.dataMapService.toArray('Disk_Mode')
        },
        cellRender: {
          type: 'status',
          config: this.dataMapService.toArray('Disk_Mode')
        }
      },
      {
        key: 'volumeType',
        name: this.i18n.get('explore_disk_type_label')
      },
      {
        key: 'sla',
        name: this.i18n.get('protection_associate_sla_label'),
        filter: {
          type: 'select',
          isMultiple: true,
          showCheckAll: true,
          options: this.dataMapService.toArray('slaAssociateStatus')
        },
        cellRender: {
          type: 'status',
          config: this.dataMapService.toArray('slaAssociateStatus')
        }
      },
      {
        key: 'size',
        name: this.i18n.get('common_capacity_label'),
        sort: true,
        cellRender: this.sizeTpl
      }
    ];

    this.tableConfig = {
      table: {
        async: false,
        size: 'small',
        columns: cols,
        compareWith: 'uuid',
        colDisplayControl: false,
        trackByFn: (_, item) => {
          return item.uuid;
        }
      },
      pagination: {
        mode: 'simple',
        pageSize: CommonConsts.PAGE_SIZE_SMALL,
        showPageSizeOptions: false,
        winTablePagination: true
      }
    };

    defer(() => this.getDisk());
  }

  getResource() {
    this.protectedResourceApiService
      .ShowResource({
        resourceId: this.source?.parentUuid || this.source?.parent_uuid
      })
      .subscribe(res => {
        const flavor = JSON.parse(res.extendInfo?.flavor || '[]');
        this.vmType = find(
          flavor,
          item => item.id === this.source?.extendInfo?.flavorId
        )?.name;
      });
  }

  getDisk() {
    let selectAll = false;
    let selectDisk = [];
    if (!isEmpty(this.source?.protectedObject)) {
      const diskInfo = this.source.protectedObject?.extParameters?.disk_info;
      if (isEmpty(diskInfo)) {
        selectAll = this.source.protectedObject?.extParameters?.all_disk;
      } else {
        selectDisk = diskInfo;
      }
    }
    const disks = JSON.parse(this.source.extendInfo?.volInfo || '{}');
    if (disks.length) {
      each(disks, item => {
        assign(item, {
          sla: selectAll ? true : selectDisk.includes(item.id),
          nameId: `${item.name || '--'}(${item.id})`
        });
      });
      this.tableData = {
        data: disks,
        total: size(disks)
      };
    }
  }
}
