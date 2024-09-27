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
import { AfterViewInit, Component, OnInit, ViewChild } from '@angular/core';
import {
  CommonConsts,
  DataMapService,
  getPermissionMenuItem,
  I18NService,
  MODAL_COMMON,
  ModelManagementService,
  OperateItems,
  WarningMessageService
} from 'app/shared';
import {
  Filters,
  ProTableComponent,
  TableCols,
  TableConfig,
  TableData
} from 'app/shared/components/pro-table';
import { DrawModalService } from 'app/shared/services/draw-modal.service';
import { VirtualScrollService } from 'app/shared/services/virtual-scroll.service';
import { each, find, first, isEmpty, size } from 'lodash';
import { AddModelComponent } from './add-model/add-model.component';

@Component({
  selector: 'aui-detection-model',
  templateUrl: './detection-model.component.html',
  styleUrls: ['./detection-model.component.less']
})
export class DetectionModelComponent implements OnInit, AfterViewInit {
  tableConfig: TableConfig;
  tableData: TableData;

  @ViewChild('dataTable', { static: false }) dataTable: ProTableComponent;

  constructor(
    private i18n: I18NService,
    private dataMapService: DataMapService,
    private drawModalService: DrawModalService,
    private virtualScroll: VirtualScrollService,
    private warningMessageService: WarningMessageService,
    private modelManagementService: ModelManagementService
  ) {}

  ngAfterViewInit(): void {
    this.dataTable.fetchData();
  }

  ngOnInit(): void {
    this.initConfig();
  }

  initConfig() {
    const cols: TableCols[] = [
      {
        key: 'version',
        name: this.i18n.get('common_version_label')
      },
      {
        key: 'updateTime',
        name: this.i18n.get('common_import_time_label')
      },
      {
        key: 'signatureInfo',
        name: this.i18n.get('explore_signature_info_label')
      },
      {
        key: 'desc',
        name: this.i18n.get('common_desc_label')
      },
      {
        key: 'status',
        name: this.i18n.get('common_status_label'),
        filter: {
          type: 'select',
          isMultiple: true,
          showCheckAll: true,
          options: this.dataMapService.toArray('Model_Status')
        },
        cellRender: {
          type: 'status',
          config: this.dataMapService.toArray('Model_Status')
        }
      },
      {
        key: 'operation',
        width: 130,
        hidden: 'ignoring',
        name: this.i18n.get('common_operation_label'),
        cellRender: {
          type: 'operation',
          config: {
            maxDisplayItems: 1,
            items: getPermissionMenuItem([
              {
                id: 'active',
                permission: OperateItems.DeleteResource,
                label: this.i18n.get('common_active_label'),
                disableCheck: ([data]) => {
                  return data.status;
                },
                onClick: ([data]) => {
                  this.active(data);
                }
              },
              {
                id: 'delete',
                permission: OperateItems.DeleteResource,
                label: this.i18n.get('common_delete_label'),
                disableCheck: ([data]) => {
                  return data.status;
                },
                onClick: ([data]) => {
                  this.delete(data);
                }
              }
            ])
          }
        }
      }
    ];
    this.tableConfig = {
      table: {
        compareWith: 'id',
        columns: cols,
        showLoading: false,
        colDisplayControl: {
          ignoringColsType: 'hide'
        },
        scrollFixed: true,
        fetchData: (filter: Filters) => {
          this.getData(filter);
        },
        trackByFn: (index, item) => {
          return item.id;
        }
      }
    };
  }

  getData(filters: Filters) {
    const params = {
      pageNum: filters.paginator.pageIndex + 1,
      pageSize: filters.paginator.pageSize
    };

    each(filters.filters, filter => {
      if (filter.value && size(filter.value)) {
        if (filter.key === 'status' && filter.value.length === 2) {
          return;
        } else {
          params[filter.key] =
            filter.key === 'status' ? filter.value[0] : filter.value;
        }
      }
    });

    this.modelManagementService.getModelListUsingGET(params).subscribe(res => {
      this.tableData = {
        data: res.records,
        total: res.totalCount
      };
    });
  }

  addModel() {
    this.drawModalService.create({
      ...MODAL_COMMON.generateDrawerOptions(),
      lvHeader: this.i18n.get('common_add_label'),
      lvModalKey: 'add_detection_model_key',
      lvWidth: MODAL_COMMON.normalWidth,
      lvContent: AddModelComponent,
      lvOkDisabled: true,
      lvOk: modal => {
        return new Promise(resolve => {
          const content = modal.getContentComponent() as AddModelComponent;
          content.onOK().subscribe({
            next: () => {
              resolve(true);
              this.dataTable.fetchData();
            },
            error: error => resolve(false)
          });
        });
      }
    });
  }

  active(data) {
    const _sendRequest = () => {
      this.modelManagementService
        .activateModelUsingPUT({
          activateModelRequest: { id: data.id }
        })
        .subscribe(() => this.dataTable.fetchData());
    };
    // 查询已激活模型
    this.modelManagementService
      .getModelListUsingGET({
        pageNum: 1,
        pageSize: CommonConsts.PAGE_SIZE,
        status: true
      })
      .subscribe(res => {
        if (
          !isEmpty(res.records) &&
          find(res.records, item => item.version !== data.version)
        ) {
          this.warningMessageService.create({
            content: this.i18n.get('explore_active_cyber_anti_model_label', [
              data.version,
              first(res.records).version
            ]),
            onOK: () => _sendRequest()
          });
        } else {
          _sendRequest();
        }
      });
  }

  delete(data) {
    this.warningMessageService.create({
      content: this.i18n.get('explore_delete_cyber_anti_model_label', [
        data.version
      ]),
      onOK: () => {
        this.modelManagementService
          .deleteModelUsingDELETE({
            deleteModelRequest: { id: data.id }
          })
          .subscribe(res => {
            this.dataTable.fetchData();
          });
      }
    });
  }
}
