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
import { DatePipe } from '@angular/common';
import {
  Component,
  EventEmitter,
  Input,
  OnInit,
  Output,
  ViewChild
} from '@angular/core';
import { FormBuilder } from '@angular/forms';
import { DatatableComponent, MessageboxService } from '@iux/live';
import {
  CommonConsts,
  CopiesService,
  DataMap,
  DataMapService,
  I18NService,
  LANGUAGE,
  LiveMountPolicyApiService,
  ResourceType,
  RetentionPolicy,
  SchedulePolicy,
  StorageUnitService
} from 'app/shared';
import { AppUtilsService } from 'app/shared/services/app-utils.service';
import {
  assign,
  difference,
  each,
  filter,
  find,
  first,
  includes,
  isEmpty,
  size,
  trim
} from 'lodash';

@Component({
  selector: 'aui-select-copy-data',
  templateUrl: './select-copy-data.component.html',
  styleUrls: ['./select-copy-data.component.less'],
  providers: [DatePipe]
})
export class SelectCopyDataComponent implements OnInit {
  policyName;
  copyLocation;
  copyName;
  copyOrders = ['-display_timestamp'];
  copyActiveSort = {};
  copyFilterParams = {};
  policyFilterParams = {};
  copyColumns = [];
  policyColumns = [];
  copyTableData = [];
  policyTableData = [];
  policyOrders = ['-created_time'];
  copyPageIndex = CommonConsts.PAGE_START;
  copyPageSize = CommonConsts.PAGE_SIZE_SMALL;
  copyTotal = CommonConsts.PAGE_TOTAL;
  policyPageIndex = CommonConsts.PAGE_START;
  policyPageSize = CommonConsts.PAGE_SIZE_SMALL;
  policyTotal = CommonConsts.PAGE_TOTAL;
  retentionPolicy = RetentionPolicy;
  schedulePolicy = SchedulePolicy;
  resourceType = ResourceType;
  storageUnitOptions = [];

  @Input() activeIndex;
  @Input() componentData;
  @ViewChild('copyLocationPopover', { static: false }) copyLocationPopover;
  @ViewChild('copyNamePopover', { static: false }) copyNamePopover;
  @ViewChild('copyTable', { static: false }) copyTable: DatatableComponent;
  @ViewChild('policyTable', { static: false }) policyTable: DatatableComponent;
  @ViewChild('policyNamePopover', { static: false }) policyNamePopover;
  @Output() selectCopyDataChange = new EventEmitter<any>();

  spaceLabel = this.i18n.language === LANGUAGE.CN ? '' : ' ';
  executionPeriodLabel = this.i18n.get(
    'protection_execution_period_label',
    [],
    true
  );
  firstExecuteTimeLabel = this.i18n.get(
    'explore_first_execute_label',
    [],
    true
  );

  constructor(
    private fb: FormBuilder,
    private i18n: I18NService,
    private dataMapService: DataMapService,
    private copiesApiService: CopiesService,
    private messageboxService: MessageboxService,
    private storageUnitService: StorageUnitService,
    private liveMountPolicyApiService: LiveMountPolicyApiService,
    public datePipe: DatePipe,
    public appUtilsService: AppUtilsService
  ) {}

  ngOnInit() {
    this.initForm();
    this.getColumns();
  }

  getStorageUnit() {
    this.storageUnitService
      .queryBackUnitGET({
        pageNo: 0,
        pageSize: 200
      })
      .subscribe(res => {
        this.storageUnitOptions = res.records.map(item => {
          return assign(item, {
            deviceType:
              item.deviceType === 'BasicDisk'
                ? DataMap.poolStorageDeviceType.Server.value
                : item.deviceType
          });
        });
        this.getCopies();
      });
  }

  initForm() {
    this.getTableData();
    this.selectCopyDataChange.emit([]);
  }

  getColumns() {
    this.copyColumns = [
      {
        key: 'name',
        label: this.i18n.get('common_name_label'),
        disabled: true,
        show: true,
        isLeaf: true
      },
      {
        key: 'display_timestamp',
        label: this.i18n.get('common_time_stamp_label'),
        showSort: true,
        show: true,
        isLeaf: true
      },
      {
        key: 'application_type',
        label: this.i18n.get('common_application_type_label'),
        resourceType: [
          DataMap.Resource_Type.KubernetesMySQL.value,
          DataMap.Resource_Type.KubernetesCommon.value
        ],
        show: true,
        isLeaf: true
      },
      {
        key: 'generation',
        label: this.i18n.get('protection_copy_generation_label'),
        resourceType: [
          DataMap.Resource_Type.oracle.value,
          DataMap.Resource_Type.virtualMachine.value,
          DataMap.Resource_Type.NASFileSystem.value,
          DataMap.Resource_Type.NASShare.value
        ],
        filter: true,
        filterMap: this.dataMapService.toArray('CopyData_Generation'),
        show: true,
        isLeaf: true,
        width: '120px'
      },
      {
        key: 'status',
        label: this.i18n.get('common_status_label'),
        filter: true,
        filterMap: this.dataMapService.toArray('copydata_validStatus'),
        show: true,
        isLeaf: true,
        width: '140px'
      },
      {
        key: 'version',
        label: this.i18n.get('common_version_label'),
        show: true,
        isLeaf: true,
        resourceType: [DataMap.Resource_Type.oracle.value],
        width: '100px'
      },
      {
        key: 'location',
        label: this.i18n.get('common_location_label'),
        show: true,
        isLeaf: true,
        width: '140px'
      },
      {
        key: 'generated_by',
        filter: true,
        label: this.i18n.get('common_generated_type_label'),
        filterMap: this.dataMapService
          .toArray('CopyData_generatedType')
          .filter(v => {
            if (
              includes(
                [
                  DataMap.Resource_Type.MySQLInstance.value,
                  DataMap.Resource_Type.tdsqlInstance.value
                ],
                first(this.componentData.childResourceType)
              )
            ) {
              return includes(
                [DataMap.CopyData_generatedType.backup.value],
                v.value
              );
            } else if (this.appUtilsService.isDistributed) {
              return includes(
                [
                  DataMap.CopyData_generatedType.backup.value,
                  DataMap.CopyData_generatedType.liveMount.value
                ],
                v.value
              );
            } else {
              return includes(
                [
                  DataMap.CopyData_generatedType.replicate.value,
                  DataMap.CopyData_generatedType.backup.value,
                  DataMap.CopyData_generatedType.liveMount.value,
                  DataMap.CopyData_generatedType.cascadedReplication.value,
                  DataMap.CopyData_generatedType.reverseReplication.value
                ],
                v.value
              );
            }
          }),
        show: true,
        isLeaf: true,
        width: '140px'
      }
    ];
    this.policyColumns = [
      {
        key: 'name',
        label: this.i18n.get('common_name_label')
      },
      {
        key: 'copyDataSelectionPolicy',
        label: this.i18n.get('common_copy_data_label')
      },
      {
        key: 'scheduleInterval',
        label: this.i18n.get('common_scheduled_label'),
        width: '240px'
      },
      {
        key: 'retentionValue',
        label: this.i18n.get('common_retention_label')
      },
      {
        width: '110px',
        key: 'liveMountCount',
        label: this.i18n.get('explore_account_of_object_label')
      }
    ];

    this.copyColumns = filter(this.copyColumns, children => {
      return !(
        children.resourceType &&
        size(
          difference(
            this.componentData.childResourceType,
            children.resourceType
          )
        ) === size(this.componentData.childResourceType)
      );
    });
  }

  getTableData() {
    if (this.appUtilsService.isDistributed) {
      this.getStorageUnit();
    } else {
      this.getCopies();
    }
    this.getPolicies();
  }

  getPolicies() {
    if (!this.componentData.selectionResource.resource_id) {
      return;
    }
    const params = {
      page: this.policyPageIndex,
      size: this.policyPageSize
    };
    each(this.policyFilterParams, (value, key) => {
      if (isEmpty(value)) {
        delete this.policyFilterParams[key];
      }
    });
    if (!isEmpty(this.policyOrders)) {
      assign(params, {
        orders: this.policyOrders
      });
    }
    if (!isEmpty(this.policyFilterParams)) {
      assign(params, {
        conditions: JSON.stringify(this.policyFilterParams)
      });
    }

    this.liveMountPolicyApiService
      .getPoliciesUsingGET(params)
      .subscribe(res => {
        if (this.policyTable) {
          this.policyTable.clearSelection();
        }
        this.policyTotal = res.total;
        this.policyTableData = res.items;
      });
  }

  getCopies() {
    if (!this.componentData.selectionResource.resource_id) {
      return;
    }
    assign(this.copyFilterParams, {
      resource_sub_type: includes(
        this.componentData.childResourceType,
        DataMap.Resource_Type.oracle.value
      )
        ? [
            DataMap.Resource_Type.oracle.value,
            DataMap.Resource_Type.oracleCluster.value
          ]
        : this.componentData.childResourceType,
      resource_id: this.componentData.selectionResource.resource_id
    });
    if (isEmpty(this.copyFilterParams['generated_by'])) {
      if (
        includes(
          [
            DataMap.Resource_Type.MySQLInstance.value,
            DataMap.Resource_Type.tdsqlInstance.value
          ],
          first(this.componentData.childResourceType)
        )
      ) {
        assign(this.copyFilterParams, {
          generated_by: [DataMap.CopyData_generatedType.backup.value]
        });
      } else if (this.appUtilsService.isDistributed) {
        assign(this.copyFilterParams, {
          generated_by: [
            DataMap.CopyData_generatedType.backup.value,
            DataMap.CopyData_generatedType.liveMount.value
          ]
        });
      } else {
        assign(this.copyFilterParams, {
          generated_by: [
            DataMap.CopyData_generatedType.replicate.value,
            DataMap.CopyData_generatedType.backup.value,
            DataMap.CopyData_generatedType.liveMount.value,
            DataMap.CopyData_generatedType.cascadedReplication.value,
            DataMap.CopyData_generatedType.reverseReplication.value
          ]
        });
      }
    }
    if (
      includes(
        [
          DataMap.Resource_Type.MySQLInstance.value,
          DataMap.Resource_Type.MySQLClusterInstance.value,
          DataMap.Resource_Type.MySQLDatabase.value,
          DataMap.Resource_Type.tdsqlInstance.value
        ],
        first(this.componentData.childResourceType)
      )
    ) {
      assign(this.copyFilterParams, {
        backup_type: [
          DataMap.CopyData_Backup_Type.full.value,
          DataMap.CopyData_Backup_Type.incremental.value,
          DataMap.CopyData_Backup_Type.diff.value
        ]
      });
    }
    if (
      includes(
        this.componentData.childResourceType,
        DataMap.Resource_Type.oracle.value
      )
    ) {
      assign(this.copyFilterParams, {
        backup_type: [
          DataMap.CopyData_Backup_Type.full.value,
          DataMap.CopyData_Backup_Type.incremental.value,
          DataMap.CopyData_Backup_Type.diff.value,
          DataMap.CopyData_Backup_Type.permanent.value
        ]
      });
    }
    each(this.copyFilterParams, (value, key) => {
      if (isEmpty(value)) {
        delete this.copyFilterParams[key];
      }
    });
    const params = {
      pageNo: this.copyPageIndex,
      pageSize: this.copyPageSize
    };
    if (!isEmpty(this.copyOrders)) {
      assign(params, {
        orders: this.copyOrders
      });
    }
    if (!isEmpty(this.copyFilterParams)) {
      assign(params, {
        conditions: JSON.stringify(this.copyFilterParams)
      });
    }
    this.copiesApiService.queryResourcesV1CopiesGet(params).subscribe(res => {
      this.copyTable.clearSelection();
      this.copyTotal = res.total;

      if (
        includes(
          [
            DataMap.Resource_Type.NASShare.value,
            DataMap.Resource_Type.fileset.value
          ],
          first(this.componentData.childResourceType)
        )
      ) {
        each(res.items, item => {
          const properties = JSON.parse(item.properties);

          assign(item, {
            disabled: properties.format === DataMap.copyFormat.aggregate.value
          });
        });
      }

      // E6000本地盘不支持挂载
      if (this.appUtilsService.isDistributed) {
        each(res.items, item => {
          this.validStorageUnit(item);
        });
      }
      this.copyTableData = res.items;
    });
  }

  private validStorageUnit(item: any) {
    if (
      find(this.storageUnitOptions, {
        name: item.storage_unit_name
      })?.deviceType === DataMap.poolStorageDeviceType.Server.value
    ) {
      assign(item, {
        disabled: true
      });
    }
  }

  getVersion(item) {
    const properties = JSON.parse(item.resource_properties || '{}');
    return properties.version || '--';
  }

  copyFilterChange(e) {
    assign(this.copyFilterParams, {
      [e.key]: e.value
    });
    this.getCopies();
  }

  copyPageChange(page) {
    this.copyPageSize = page.pageSize;
    this.copyPageIndex = page.pageIndex;
    this.getCopies();
  }

  policyPageChange(page) {
    this.policyPageSize = page.pageSize;
    this.policyPageIndex = page.pageIndex;
    this.getPolicies();
  }

  copySortChange(source) {
    this.copyOrders = [];
    this.copyOrders.push((source.direction === 'asc' ? '+' : '-') + source.key);
    this.getCopies();
  }

  policySortChange(source) {
    this.policyOrders = [];
    this.policyOrders.push(
      (source.direction === 'asc' ? '+' : '-') +
        (source.key === 'liveMountCount' ? 'live_mount_count' : source.key)
    );
    this.getPolicies();
  }

  searchByPolicyName(value) {
    if (this.policyNamePopover) {
      this.policyNamePopover.hide();
    }
    assign(this.policyFilterParams, {
      name: trim(value)
    });
    this.getPolicies();
  }

  searchByCopyName(value) {
    if (this.copyNamePopover) {
      this.copyNamePopover.hide();
    }
    assign(this.copyFilterParams, {
      name: trim(value)
    });
    this.getCopies();
  }

  searchByCopyLocation(value) {
    if (this.copyLocationPopover) {
      this.copyLocationPopover.hide();
    }
    assign(this.copyFilterParams, {
      location: trim(value)
    });
    this.getCopies();
  }

  selectionCopyRow(source) {
    if (
      (source.generated_by === DataMap.CopyData_generatedType.liveMount.value
        ? source.generation > DataMap.CopyData_Generation.two.value
        : source.generation >= DataMap.CopyData_Generation.two.value &&
          !includes(
            [
              DataMap.CopyData_generatedType.cascadedReplication.value,
              DataMap.CopyData_generatedType.reverseReplication.value
            ],
            source.generated_by
          )) ||
      source.status !== DataMap.copydata_validStatus.normal.value
    ) {
      this.messageboxService.info(
        this.i18n.get('explore_generation_by_select_label', [
          this.datePipe.transform(
            source.display_timestamp,
            'yyyy-MM-dd HH:mm:ss'
          )
        ])
      );
      this.copyTable.toggleSelection(source);
      this.selectCopyDataChange.emit([]);
      return;
    }

    if (source.disabled) {
      return;
    }

    this.copyTable.toggleSelection(source);
    this.selectCopyDataChange.emit(this.copyTable.getSelection());
  }

  selectionPolicyRow(source) {
    this.policyTable.toggleSelection(source);
  }

  getComponentData() {
    const policySelectionData =
      this.componentData.resourceType !== ResourceType.Storage
        ? this.policyTable.getSelection()
        : [];
    if (!!size(policySelectionData)) {
      assign(this.componentData.requestParams, {
        policy_id: first(policySelectionData).policyId
      });
    }

    const copySelectionData = this.copyTable.getSelection();
    if (!!size(copySelectionData)) {
      assign(this.componentData.requestParams, {
        copy_id: first(copySelectionData).uuid
      });
    }

    return assign(this.componentData, {
      requestParams: this.componentData.requestParams,
      selectionCopy: first(copySelectionData) || {},
      selectionPolicy: first(policySelectionData) || {}
    });
  }
}
