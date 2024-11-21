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
  Input,
  OnInit,
  ViewChild
} from '@angular/core';
import {
  CommonConsts,
  CopyControllerService,
  DataMap,
  DataMapService,
  I18NService,
  WarningMessageService
} from 'app/shared';
import { CopiesService } from 'app/shared/api/services/copies.service';
import { ResourceDetailService } from 'app/shared/services/resource-detail.service';
import { VirtualScrollService } from 'app/shared/services/virtual-scroll.service';
import { assign, each, includes, isEmpty, map, trim } from 'lodash';

@Component({
  selector: 'aui-resource-list',
  templateUrl: './resource-list.component.html',
  styleUrls: ['./resource-list.component.less'],
  changeDetection: ChangeDetectionStrategy.OnPush
})
export class ResourceListComponent implements OnInit {
  slaName;
  resourceName;
  resourceLocation;
  tableData = [];
  pageIndex = CommonConsts.PAGE_START;
  pageSize = CommonConsts.PAGE_SIZE;
  total = CommonConsts.PAGE_TOTAL;
  sizeOptions = CommonConsts.PAGE_SIZE_OPTIONS;
  orders = ['-display_timestamp'];
  filterParams = {};
  columns = [];

  isHyperdetect = includes(
    [
      DataMap.Deploy_Type.hyperdetect.value,
      DataMap.Deploy_Type.cloudbackup.value,
      DataMap.Deploy_Type.cloudbackup2.value
    ],
    this.i18n.get('deploy_type')
  );

  @Input() resourceType;
  @Input() childResourceType;
  @ViewChild('sourceNamePopover', { static: false }) sourceNamePopover;
  @ViewChild('resourceLocationPopover', { static: false })
  resourceLocationPopover;
  @ViewChild('slaNamePopover', { static: false }) slaNamePopover;

  constructor(
    private i18n: I18NService,
    private detailService: ResourceDetailService,
    private copiesApiService: CopiesService,
    private dataMapService: DataMapService,
    public warningMessageService: WarningMessageService,
    public virtualScroll: VirtualScrollService,
    private cdr: ChangeDetectorRef,
    private copyControllerService: CopyControllerService
  ) {}

  ngOnInit() {
    this.getColumns();
    this.getResource();
    this.virtualScroll.getScrollParam(this.isHyperdetect ? 320 : 420);
  }

  getColumns() {
    this.columns = [
      {
        key: 'resourceName',
        label: this.i18n.get('common_name_label')
      },
      {
        key: 'resourceLocation',
        label: this.i18n.get('common_location_label')
      },
      {
        key: 'resourceStatus',
        filter: true,
        label: this.i18n.get('common_status_label'),
        filterMap: this.dataMapService.toArray('Resource_Status')
      },
      {
        key: 'copyCount',
        label: this.i18n.get('common_amount_label'),
        sort: true
      }
    ];
  }

  getResource() {
    assign(this.filterParams, {
      resourceSubType: this.childResourceType
    });
    each(this.filterParams, (value, key) => {
      if (isEmpty(value)) {
        delete this.filterParams[key];
      }
    });
    const params = {
      pageNo: this.pageIndex,
      pageSize: this.pageSize
    };
    if (!isEmpty(this.orders)) {
      assign(params, {
        orders: this.orders
      });
    }
    if (!isEmpty(this.filterParams)) {
      assign(params, {
        conditions: JSON.stringify(this.filterParams)
      });
    }
    this.copyControllerService
      .queryCopySummaryResourceV2(params)
      .subscribe(res => {
        this.tableData = map(res.records, item => {
          assign(item, {
            resource_id: item.resourceId,
            protected_sla_id: item.protectedSlaId,
            sla_id: item.protectedSlaId
          });
          return item;
        });
        this.total = res.totalCount;
        this.cdr.detectChanges();
      });
  }

  pageChange(page) {
    this.pageSize = page.pageSize;
    this.pageIndex = page.pageIndex;
    this.getResource();
  }

  searchByName(value) {
    if (this.sourceNamePopover) {
      this.sourceNamePopover.hide();
    }
    assign(this.filterParams, {
      resourceName: trim(this.resourceName)
    });
    this.getResource();
  }

  searchByLocation(value) {
    if (this.resourceLocationPopover) {
      this.resourceLocationPopover.hide();
    }
    assign(this.filterParams, {
      resourceLocation: trim(value)
    });
    this.getResource();
  }

  searchBySla(value) {
    if (this.slaNamePopover) {
      this.slaNamePopover.hide();
    }
    assign(this.filterParams, {
      slaName: trim(value)
    });
    this.getResource();
  }

  getResourceDetail(item) {
    const resource = JSON.parse(item.resourceProperties);
    this.detailService.openDetailModal(item.resourceSubType, {
      lvHeader: item.name || item.resourceName,
      data: assign(
        {
          uuid: item.resource_id,
          name: item.resourceName,
          ip:
            resource.sub_type === DataMap.Resource_Type.virtualMachine.value
              ? ''
              : resource.environment_endpoint
        },
        resource,
        item
      )
    });
  }

  filterChange(e) {
    assign(this.filterParams, {
      [e.key]: e.value
    });
    this.getResource();
  }

  sortChange(source) {
    this.orders = [];
    this.orders.push((source.direction === 'asc' ? '+' : '-') + source.key);
    this.getResource();
  }
}
