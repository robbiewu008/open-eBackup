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
  AfterViewInit,
  ChangeDetectorRef,
  Component,
  OnInit,
  TemplateRef,
  ViewChild
} from '@angular/core';
import { MessageboxService } from '@iux/live';
import {
  ConfigManagementService,
  CookieService,
  DataMap,
  DataMapService,
  getPermissionMenuItem,
  I18NService,
  MODAL_COMMON,
  ModelManagementService,
  RoleType,
  WarningMessageService
} from 'app/shared';
import { ProButton } from 'app/shared/components/pro-button/interface';
import {
  Filters,
  ProTableComponent,
  TableCols,
  TableConfig,
  TableData
} from 'app/shared/components/pro-table';
import { DrawModalService } from 'app/shared/services/draw-modal.service';
import { each, find, includes, isUndefined, size } from 'lodash';
import { AddDetectionModelComponent } from './add-detection-model/add-detection-model.component';

@Component({
  selector: 'aui-detection-model-list',
  templateUrl: './detection-model-list.component.html',
  styleUrls: ['./detection-model-list.component.less']
})
export class DetectionModelListComponent implements OnInit, AfterViewInit {
  tableConfig: TableConfig;
  tableData: TableData;
  optsConfig;
  selectionData = [];
  isDataBackup = includes(
    [
      DataMap.Deploy_Type.a8000.value,
      DataMap.Deploy_Type.x3000.value,
      DataMap.Deploy_Type.x6000.value,
      DataMap.Deploy_Type.x8000.value,
      DataMap.Deploy_Type.x9000.value
    ],
    this.i18n.get('deploy_type')
  );
  loading = false;
  dynamicDetection = false;
  isHyperDetect =
    this.i18n.get('deploy_type') === DataMap.Deploy_Type.hyperdetect.value;

  @ViewChild('dataTable', { static: false }) dataTable: ProTableComponent;
  @ViewChild('tipContentTpl', { static: false }) tipContentTpl: TemplateRef<
    any
  >;

  constructor(
    private i18n: I18NService,
    private cdr: ChangeDetectorRef,
    private cookieService: CookieService,
    private messageBox: MessageboxService,
    private dataMapService: DataMapService,
    private drawModalService: DrawModalService,
    private modelManagementService: ModelManagementService,
    private warningMessageService: WarningMessageService,
    private configManagementService: ConfigManagementService
  ) {}

  ngAfterViewInit() {
    this.dataTable.fetchData();
  }

  ngOnInit() {
    this.initConfig();
    if (!this.isHyperDetect) {
      this.initSettingData();
    }
  }

  initConfig() {
    const opts: ProButton[] = [
      {
        id: 'add',
        type: 'primary',
        label: this.i18n.get('common_import_label'),
        displayCheck: data => {
          return this.cookieService.role === RoleType.SysAdmin;
        },
        onClick: () => {
          this.addModel();
        }
      }
    ];
    this.optsConfig = getPermissionMenuItem(opts);

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
        name: this.i18n.get('common_operation_label'),
        width: 144,
        cellRender: {
          type: 'operation',
          config: {
            maxDisplayItems: 1,
            items: [
              {
                id: 'active',
                label: this.i18n.get('common_active_label'),
                disableCheck: data => {
                  return data[0].status;
                },
                onClick: data => {
                  this.activeModel(data);
                }
              },
              {
                id: 'delete',
                label: this.i18n.get('common_delete_label'),
                disableCheck: data => {
                  return data[0].status;
                },
                onClick: data => {
                  this.delete(data);
                }
              }
            ]
          }
        }
      }
    ];

    if (this.cookieService.role !== RoleType.SysAdmin) {
      cols.splice(5, 1);
    }

    this.tableConfig = {
      table: {
        compareWith: 'id',
        columns: cols,
        scrollFixed: true,
        colDisplayControl: false,
        fetchData: (filter: Filters) => {
          this.getData(filter);
        }
      }
    };
  }

  getData(filters?: Filters) {
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
      this.cdr.detectChanges();
    });
  }

  addModel(data?) {
    this.drawModalService.create({
      ...MODAL_COMMON.generateDrawerOptions(),
      lvHeader: this.i18n.get('common_import_label'),
      lvModalKey: 'add_detection_model',
      lvWidth: MODAL_COMMON.normalWidth,
      lvContent: AddDetectionModelComponent,
      lvOkDisabled: true,
      lvAfterOpen: modal => {
        const content = modal.getContentComponent() as AddDetectionModelComponent;
        content.valid$.subscribe(res => {
          modal.lvOkDisabled = !res;
        });
      },
      lvComponentParams: {
        data
      },
      lvOk: modal => {
        return new Promise(resolve => {
          const content = modal.getContentComponent() as AddDetectionModelComponent;
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

  activeModel(data) {
    const activeModel = find(this.tableData.data, {
      status: DataMap.Model_Status.active.value
    });
    if (!isUndefined(activeModel) && activeModel.version !== data[0].version) {
      this.warningMessageService.create({
        content: this.i18n.get('explore_active_anti_model_label', [
          data[0].version,
          activeModel.version
        ]),
        onOK: () => {
          this.modelManagementService
            .activateModelUsingPUT({
              activateModelRequest: { id: data[0].id }
            })
            .subscribe(res => {
              this.dataTable.fetchData();
            });
        }
      });
      return;
    }
    this.modelManagementService
      .activateModelUsingPUT({ activateModelRequest: { id: data[0].id } })
      .subscribe(res => {
        this.dataTable.fetchData();
      });
  }

  delete(data) {
    this.warningMessageService.create({
      content: this.i18n.get('explore_delete_anti_model_label', [
        data[0].version
      ]),
      onOK: () => {
        this.modelManagementService
          .deleteModelUsingDELETE({
            deleteModelRequest: { id: data[0].id }
          })
          .subscribe(res => {
            this.dataTable.fetchData();
          });
      }
    });
  }

  initSettingData() {
    this.configManagementService
      .getConfigUsingGET({ akOperationTips: false, akLoading: false })
      .subscribe(res => {
        this.dynamicDetection = res.dynamicDetection;
      });
  }

  switchDynamic() {
    if (!this.dynamicDetection) {
      this.loading = true;
      this.messageBox.info({
        lvHeader: this.i18n.get('common_alarms_info_label'),
        lvContent: this.tipContentTpl,
        lvFooter: [
          {
            label: this.i18n.get('common_ok_label'),
            onClick: modal => {
              this.loading = false;
              this.save();
              modal.close();
            }
          },
          {
            label: this.i18n.get('common_cancel_label'),
            onClick: modal => {
              this.dynamicDetection = true;
              this.loading = false;
              modal.close();
            }
          }
        ],
        lvAfterClose: result => {
          if (result && result.trigger === 'close') {
            this.loading = false;
          }
        }
      });
    } else {
      this.save();
    }
  }

  save() {
    this.configManagementService
      .setConfigUsingPUT({
        config: {
          dynamicDetection: this.dynamicDetection
        }
      })
      .subscribe(res => {
        this.dynamicDetection = res.dynamicDetection;
      });
  }
}
