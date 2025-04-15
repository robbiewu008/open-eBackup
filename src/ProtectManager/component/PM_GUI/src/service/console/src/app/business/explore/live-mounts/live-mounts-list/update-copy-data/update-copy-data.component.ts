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
import { Component, OnInit, ViewChild } from '@angular/core';
import { FormBuilder, FormControl, FormGroup } from '@angular/forms';
import { DatatableComponent, MessageboxService, ModalRef } from '@iux/live';
import {
  BaseUtilService,
  CommonConsts,
  CopiesService,
  DataMap,
  DataMapService,
  I18NService,
  LiveMountApiService,
  LiveMountUpdateModal,
  SYSTEM_TIME,
  WarningMessageService
} from 'app/shared';
import {
  assign,
  cloneDeep,
  each,
  first,
  get,
  includes,
  isEmpty,
  set,
  size,
  trim
} from 'lodash';
import { Observable, Observer } from 'rxjs';
import { AppUtilsService } from 'app/shared/services/app-utils.service';

@Component({
  selector: 'aui-update-copy-data',
  templateUrl: './update-copy-data.component.html',
  styleUrls: ['./update-copy-data.component.less'],
  providers: [DatePipe]
})
export class UpdateCopyDataComponent implements OnInit {
  item;
  location;
  columns = [];
  tableData = [];
  filterParams = {};
  orders = ['-display_timestamp'];
  activeSort = {};
  formGroup: FormGroup;
  pageSize = CommonConsts.PAGE_SIZE;
  pageNo = CommonConsts.PAGE_START;
  total = CommonConsts.PAGE_TOTAL;
  liveMountUpdateModal = LiveMountUpdateModal;

  @ViewChild('locationPopover', { static: false }) locationPopover;
  @ViewChild('lvTable', { static: false }) lvTable: DatatableComponent;

  constructor(
    private i18n: I18NService,
    public fb: FormBuilder,
    public modal: ModalRef,
    public datePipe: DatePipe,
    public baseUtilService: BaseUtilService,
    public dataMapService: DataMapService,
    private copiesApiService: CopiesService,
    private messageboxService: MessageboxService,
    private liveMountApiService: LiveMountApiService,
    private warningMessageService: WarningMessageService,
    public appUtilsService: AppUtilsService
  ) {}

  ngOnInit() {
    this.initForm();
    this.getColumns();
  }

  initForm() {
    this.formGroup = this.fb.group({
      mode: new FormControl(this.liveMountUpdateModal.Latest)
    });

    this.formGroup.get('mode').valueChanges.subscribe(res => {
      this.modal.getInstance().lvOkDisabled =
        res === this.liveMountUpdateModal.Specified;
      if (res === this.liveMountUpdateModal.Specified) {
        this.getCopies();
      }
    });
  }

  getColumns() {
    this.columns = [
      {
        key: 'display_timestamp',
        label: this.i18n.get('common_time_stamp_label'),
        showSort: true,
        disabled: true,
        show: true,
        isLeaf: true
      },
      {
        key: 'generation',
        label: this.i18n.get('protection_copy_generation_label'),
        resourceType: [
          DataMap.Resource_Type.oracle.value,
          DataMap.Resource_Type.virtualMachine.value
        ],
        filter: true,
        filterMap: this.dataMapService.toArray('CopyData_Generation'),
        show: true,
        isLeaf: true,
        width: '140px'
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
                this.item.resource_sub_type
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
  }

  getCopies() {
    assign(this.filterParams, {
      resource_sub_type: this.item.resource_sub_type,
      resource_id: this.item.resource_id
    });
    each(this.filterParams, (value, key) => {
      if (isEmpty(value)) {
        delete this.filterParams[key];
      }
    });
    const params = {
      pageNo: this.pageNo,
      pageSize: this.pageSize
    };
    if (!isEmpty(this.orders)) {
      assign(params, {
        orders: this.orders
      });
    }
    if (!isEmpty(this.filterParams)) {
      const filter = cloneDeep(this.filterParams);

      if (!get(filter, 'generated_by')) {
        if (
          includes(
            [
              DataMap.Resource_Type.MySQLInstance.value,
              DataMap.Resource_Type.tdsqlInstance.value
            ],
            this.item.resource_sub_type
          )
        ) {
          set(filter, 'generated_by', [
            DataMap.CopyData_generatedType.backup.value
          ]);
        } else {
          set(filter, 'generated_by', [
            DataMap.CopyData_generatedType.replicate.value,
            DataMap.CopyData_generatedType.backup.value,
            DataMap.CopyData_generatedType.liveMount.value,
            DataMap.CopyData_generatedType.cascadedReplication.value,
            DataMap.CopyData_generatedType.reverseReplication.value
          ]);
        }
      }

      if (
        includes(
          [
            DataMap.Resource_Type.MySQLInstance.value,
            DataMap.Resource_Type.tdsqlInstance.value
          ],
          this.item.resource_sub_type
        )
      ) {
        set(filter, 'backup_type', [
          DataMap.CopyData_Backup_Type.full.value,
          DataMap.CopyData_Backup_Type.incremental.value,
          DataMap.CopyData_Backup_Type.diff.value
        ]);
      }

      assign(params, {
        conditions: JSON.stringify(filter)
      });
    }
    this.copiesApiService.queryResourcesV1CopiesGet(params).subscribe(res => {
      if (this.item.resource_type === DataMap.Resource_Type.fileset.value) {
        each(res.items, item => {
          const properties = JSON.parse(item.properties);

          assign(item, {
            disabled: properties.format === DataMap.copyFormat.aggregate.value
          });
        });
      }
      this.total = res.total;
      this.tableData = res.items;
    });
  }

  filterChange(e) {
    assign(this.filterParams, {
      [e.key]: e.value
    });
    this.getCopies();
  }

  pageChange(page) {
    this.pageSize = page.pageSize;
    this.pageNo = page.pageIndex;
    this.getCopies();
  }

  sortChange(source) {
    this.orders = [];
    this.orders.push((source.direction === 'asc' ? '+' : '-') + source.key);
    this.getCopies();
  }

  searchByLocation(value) {
    if (this.locationPopover) {
      this.locationPopover.hide();
    }
    assign(this.filterParams, {
      location: trim(value)
    });
    this.getCopies();
  }

  selectionRow(source) {
    if (
      (source.generated_by === DataMap.CopyData_generatedType.liveMount.value
        ? source.generation > DataMap.CopyData_Generation.two.value
        : source.generation >= DataMap.CopyData_Generation.two.value) ||
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
      this.lvTable.toggleSelection(source);
      this.modal.getInstance().lvOkDisabled = true;
      return;
    }
    if (
      this.item.resource_type === DataMap.Resource_Type.fileset.value &&
      source.disabled
    ) {
      return;
    }
    this.lvTable.toggleSelection(source);
    this.modal.getInstance().lvOkDisabled = !size(this.lvTable.getSelection());
  }

  onOK(): Observable<void> {
    return new Observable<any>((observer: Observer<void>) => {
      if (this.formGroup.invalid) {
        return;
      }

      const liveMountUpdate = {
        mode: this.formGroup.value.mode
      };
      if (this.formGroup.value.mode === this.liveMountUpdateModal.Specified) {
        assign(liveMountUpdate, {
          copy_id: first(this.lvTable.getSelection()).uuid
        });
      }
      if (this.formGroup.value.mode === this.liveMountUpdateModal.Latest) {
        this.warningMessageService.create({
          content: this.i18n.get('explore_update_livemount_latest_label'),
          onOK: () => {
            this.liveMountApiService
              .updateLiveMountUsingPUT({
                liveMountId: this.item.id,
                liveMountUpdate
              })
              .subscribe({
                next: () => {
                  observer.next();
                  observer.complete();
                },
                error: error => {
                  observer.error(error);
                  observer.complete();
                }
              });
          },
          onCancel: () => {
            observer.error(null);
            observer.complete();
          },
          lvAfterClose: result => {
            if (result && result.trigger === 'close') {
              observer.error(null);
              observer.complete();
            }
          }
        });
        return;
      }
      this.liveMountApiService
        .updateLiveMountUsingPUT({
          liveMountId: this.item.id,
          liveMountUpdate
        })
        .subscribe({
          next: () => {
            observer.next();
            observer.complete();
          },
          error: error => {
            observer.error(error);
            observer.complete();
          }
        });
    });
  }
}
