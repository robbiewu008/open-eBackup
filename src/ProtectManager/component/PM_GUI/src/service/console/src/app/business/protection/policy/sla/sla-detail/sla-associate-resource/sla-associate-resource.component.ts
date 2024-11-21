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
import {
  CommonConsts,
  CopiesService,
  CopyControllerService,
  DataMap,
  DataMapService,
  extendSummaryCopiesParams,
  I18NService,
  isSlaResourceSubType,
  ProjectedObjectApiService
} from 'app/shared';
import { assign, each, filter, includes, isEmpty, trim } from 'lodash';

@Component({
  selector: 'aui-sla-associate-resource',
  templateUrl: './sla-associate-resource.component.html',
  styleUrls: ['./sla-associate-resource.component.less']
})
export class SlaAssociateResourceComponent implements OnInit {
  name;
  path;
  orderBy;
  orderType;
  tableData = [];
  filterParams = {};
  isReplica;
  orders = ['-display_timestamp'];
  total = CommonConsts.PAGE_TOTAL;
  startPage = CommonConsts.PAGE_START;
  pageSize = CommonConsts.PAGE_SIZE;
  sizeOptions = CommonConsts.PAGE_SIZE_OPTIONS;
  isHyperdetect =
    this.i18n.get('deploy_type') === DataMap.Deploy_Type.hyperdetect.value;
  columns = [
    {
      key: 'name',
      label: this.i18n.get('common_name_label')
    },
    {
      key: 'subType',
      label: this.i18n.get('common_type_label'),
      filter: !this.isHyperdetect,
      filterMap: this.dataMapService
        .toArray('Job_Target_Type', [
          DataMap.Job_Target_Type.LocalFileSystem.value
        ])
        .filter(item => isSlaResourceSubType(item.value))
    },
    {
      key: 'slaCompliance',
      label: this.i18n.get('common_sla_compliance_label'),
      filter: true,
      filterMap: this.dataMapService.toArray('Sla_Compliance')
    },
    {
      key: 'path',
      label: this.i18n.get('common_location_label')
    }
  ];
  filter = filter;
  replicaColumns = [
    {
      label: this.i18n.get('common_resource_label'),
      expanded: true,
      disabled: true,
      show: true,
      children: [
        {
          key: 'resource_name',
          show: true,
          isLeaf: true,
          label: this.i18n.get('common_name_label')
        },
        {
          key: 'resource_sub_type',
          show: true,
          filter: true,
          isLeaf: true,
          label: this.i18n.get('common_type_label'),
          filterMap: this.dataMapService
            .toArray('Job_Target_Type', [
              DataMap.Job_Target_Type.LocalFileSystem.value
            ])
            .filter(item => isSlaResourceSubType(item.value))
        },
        {
          key: 'resource_location',
          show: true,
          isLeaf: true,
          label: this.i18n.get('common_location_label')
        },
        {
          key: 'resource_status',
          show: true,
          filter: true,
          isLeaf: true,
          label: this.i18n.get('common_status_label'),
          filterMap: this.dataMapService.toArray('Resource_Status')
        }
      ]
    },
    {
      label: this.i18n.get('common_copies_label'),
      expanded: true,
      disabled: true,
      show: true,
      children: [
        {
          key: 'copy_count',
          show: true,
          isLeaf: true,
          label: this.i18n.get('common_amount_label'),
          sort: true
        }
      ]
    }
  ];

  @Input() sla;
  @ViewChild('namePopover', { static: false }) namePopover;
  @ViewChild('pathPopover', { static: false }) pathPopover;

  constructor(
    private i18n: I18NService,
    private dataMapService: DataMapService,
    private copiesApiService: CopiesService,
    private copyControllerService: CopyControllerService,
    private projectedObjectApiService: ProjectedObjectApiService
  ) {}

  ngOnInit() {
    this.isReplica =
      this.sla.application === DataMap.Application_Type.Replica.value;
    this.getResource();
  }

  getResource() {
    if (this.isReplica) {
      const params = {
        pageNo: this.startPage,
        pageSize: this.pageSize
      };

      each(this.filterParams, (value, key) => {
        if (isEmpty(value)) {
          delete this.filterParams[key];
        } else {
          params[key] = value;
        }
      });

      assign(params, {
        orders: this.orders
      });

      assign(params, {
        conditions: JSON.stringify({
          ...this.filterParams,
          protectedSlaId: this.sla.uuid
        })
      });

      this.copyControllerService
        .queryCopySummaryResourceV2(params)
        .subscribe(res => {
          each(res.records, item => extendSummaryCopiesParams(item));
          this.total = res.totalCount;
          this.tableData = res.records;
        });
    } else {
      const params = {
        slaId: this.sla.uuid,
        pageNo: this.startPage,
        pageSize: this.pageSize
      };

      each(this.filterParams, (value, key) => {
        if (isEmpty(value)) {
          delete this.filterParams[key];
        } else {
          params[key] = value;
        }
      });

      this.projectedObjectApiService
        .pageQueryV1ProtectedObjectsGet(params)
        .subscribe(res => {
          this.total = res.total;
          this.tableData = res.items;
        });
    }
  }

  pageChange = event => {
    this.pageSize = event.pageSize;
    this.startPage = event.pageIndex;
    this.getResource();
  };

  filterChange(e) {
    assign(this.filterParams, {
      [e.key]: e.value
    });
    if (
      e.key === 'slaCompliance' &&
      includes(e.value, true) &&
      includes(e.value, false)
    ) {
      delete this.filterParams['slaCompliance'];
    }
    this.getResource();
  }

  sortChange(source) {
    this.orderBy = source.key;
    this.orderType = source.direction;
    this.orders = [];
    this.orders.push((source.direction === 'asc' ? '+' : '-') + source.key);
    this.getResource();
  }

  searchByName() {
    if (this.namePopover) {
      this.namePopover.hide();
    }
    assign(this.filterParams, {
      [this.isReplica ? 'resourceName' : 'name']: trim(this.name)
    });
    this.getResource();
  }

  searchByPath() {
    if (this.pathPopover) {
      this.pathPopover.hide();
    }
    assign(this.filterParams, {
      [this.isReplica ? 'resourceLocation' : 'path']: trim(this.path)
    });
    this.getResource();
  }
}
