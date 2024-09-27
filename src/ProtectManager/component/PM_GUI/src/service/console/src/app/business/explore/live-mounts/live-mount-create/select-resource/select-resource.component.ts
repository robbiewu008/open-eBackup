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
import {
  CommonConsts,
  CopiesService,
  CopyControllerService,
  DataMapService,
  I18NService
} from 'app/shared';
import {
  assign,
  difference,
  each,
  filter,
  first,
  includes,
  isEmpty,
  map,
  size,
  trim
} from 'lodash';
import { DataMap } from 'app/shared/consts/data-map.config';

@Component({
  selector: 'aui-select-resource',
  templateUrl: './select-resource.component.html',
  styleUrls: ['./select-resource.component.less']
})
export class SelectResourceComponent implements OnInit {
  resource_name;
  resource_location;
  resource_environment_name;
  resource_environment_ip;
  columns = [];
  tableData = [];
  filterParams = {};
  orders = [];
  pageIndex = CommonConsts.PAGE_START;
  pageSize = CommonConsts.PAGE_SIZE;
  total = CommonConsts.PAGE_TOTAL;
  @Input() activeIndex;
  @Input() componentData;
  @ViewChild('lvTable', { static: false }) lvTable: DatatableComponent;
  @ViewChild('namePopover', { static: false }) namePopover;
  @ViewChild('ipPopover', { static: false }) ipPopover;
  @ViewChild('environmentNamePopover', { static: false })
  environmentNamePopover;
  @ViewChild('locationPopover', { static: false })
  locationPopover;
  @Output() selectResourceChange = new EventEmitter<any>();

  constructor(
    private i18n: I18NService,
    private dataMapService: DataMapService,
    private copiesApiService: CopiesService,
    private copyControllerService: CopyControllerService
  ) {}

  ngOnInit() {
    this.getColumns();
    this.getResource();
  }

  getColumns() {
    this.columns = [
      {
        label: includes(
          [
            DataMap.Resource_Type.KubernetesMySQL.value,
            DataMap.Resource_Type.KubernetesCommon.value
          ],
          this.componentData.childResourceType
        )
          ? this.i18n.get('protection_dataset_name_label')
          : this.i18n.get('common_name_label'),
        key: 'resource_name'
      },
      {
        label: this.i18n.get('common_type_label'),
        key: 'resourceType',
        childResourceType: [
          DataMap.Resource_Type.KubernetesMySQL.value,
          DataMap.Resource_Type.KubernetesCommon.value
        ]
      },
      {
        label: this.i18n.get('protection_cluster_label'),
        key: 'resourceCluster',
        childResourceType: [
          DataMap.Resource_Type.KubernetesMySQL.value,
          DataMap.Resource_Type.KubernetesCommon.value
        ]
      },
      {
        label: this.i18n.get('common_ip_address_label'),
        key: 'resource_environment_ip',
        childResourceType: [
          DataMap.Resource_Type.oracle.value,
          DataMap.Resource_Type.MySQL.value,
          DataMap.Resource_Type.KubernetesMySQL.value,
          DataMap.Resource_Type.KubernetesCommon.value
        ]
      },
      {
        label: this.i18n.get('common_location_label'),
        key: 'resource_location',
        childResourceType: [
          DataMap.Resource_Type.virtualMachine.value,
          DataMap.Resource_Type.NASFileSystem.value,
          DataMap.Resource_Type.NASShare.value
        ]
      },
      {
        label: this.i18n.get('protection_host_cluster_label'),
        key: 'resource_environment_name',
        childResourceType: [
          DataMap.Resource_Type.oracle.value,
          DataMap.Resource_Type.MySQL.value
        ]
      },
      {
        label: this.i18n.get('protection_name_space_label'),
        key: 'resourceNamespace',
        childResourceType: [
          DataMap.Resource_Type.KubernetesMySQL.value,
          DataMap.Resource_Type.KubernetesCommon.value
        ]
      }
    ];

    this.columns = filter(this.columns, col => {
      return !(
        col.childResourceType &&
        size(
          difference(
            this.componentData.childResourceType,
            col.childResourceType
          )
        ) === size(this.componentData.childResourceType)
      );
    });
  }

  getResource() {
    this.copyControllerService
      .queryCopySummaryResourceV2({ ...this.getParams() })
      .subscribe(res => {
        each(res.records, item => {
          assign(item, {
            resource_id: item.resourceId,
            resource_name: item.resourceName,
            resource_location: item.resourceLocation,
            resource_environment_ip: item.resourceEnvironmentIp,
            resource_environment_name: item.resourceEnvironmentName,
            resource_properties: item.resourceProperties,
            resource_sub_type: item.resourceSubType,
            resource_type: item.resourceType,
            resource_status: item.resourceStatus,
            sla_name: item.slaName
          });
        });
        if (
          this.componentData.childResourceType.includes(
            DataMap.Resource_Type.oracle.value
          )
        ) {
          this.tableData = map(res.records, item => {
            const resourceProperties =
              item?.resourceProperties && JSON.parse(item.resourceProperties);
            assign(item, {
              disabled:
                !resourceProperties ||
                resourceProperties?.environment_os_type !==
                  DataMap.Os_Type.linux.value
            });
            return item;
          });
          this.total = res.totalCount;
        } else {
          this.tableData = res.records;
          this.total = res.totalCount;
        }
      });
  }

  getParams() {
    assign(this.filterParams, {
      resourceSubType: this.componentData.childResourceType.includes(
        DataMap.Resource_Type.oracle.value
      )
        ? [
            DataMap.Resource_Type.oracle.value,
            DataMap.Resource_Type.oracleCluster.value
          ]
        : this.componentData.childResourceType
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

    return params;
  }

  searchByName(value) {
    if (this.namePopover) {
      this.namePopover.hide();
    }
    assign(this.filterParams, {
      resourceName: trim(this.resource_name)
    });
    this.getResource();
  }

  searchByIp(value) {
    if (this.ipPopover) {
      this.ipPopover.hide();
    }
    assign(this.filterParams, {
      resourceEnvironmentIp: trim(this.resource_environment_ip)
    });
    this.getResource();
  }

  searchByEnvironmentName(value) {
    if (this.environmentNamePopover) {
      this.environmentNamePopover.hide();
    }
    assign(this.filterParams, {
      resourceEnvironmentName: trim(this.resource_environment_name)
    });
    this.getResource();
  }

  searchByLocation(value) {
    if (this.locationPopover) {
      this.locationPopover.hide();
    }
    assign(this.filterParams, {
      resourceLocation: trim(this.resource_location)
    });
    this.getResource();
  }

  pageChange(page) {
    this.pageSize = page.pageSize;
    this.pageIndex = page.pageIndex;
    this.getResource();
  }

  filterChange(source) {
    assign(this.filterParams, {
      [source.key]: source.value
    });
    this.getResource();
  }

  selectionRow(source) {
    this.lvTable.toggleSelection(source);
    this.selectResourceChange.emit(this.lvTable.getSelection());
  }

  search(name) {
    this.resource_name = name;
    this.searchByName(name);
  }

  getComponentData() {
    const selectionResource = first(this.lvTable.getSelection());
    return assign(this.componentData, {
      requestParams: { source_resource_id: selectionResource.resourceId },
      selectionResource: assign(selectionResource, {
        environment_is_cluster: selectionResource.resourceProperties
          ? JSON.parse(selectionResource.resourceProperties)
              .environment_is_cluster
          : true
      })
    });
  }
}
