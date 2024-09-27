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
  OnDestroy,
  OnInit,
  TemplateRef,
  ViewChild
} from '@angular/core';
import { MessageboxService, MessageService } from '@iux/live';
import {
  CommonConsts,
  ConfigManagementService,
  DataMap,
  DataMapService,
  getPermissionMenuItem,
  HoneypotService,
  I18NService,
  MODAL_COMMON,
  ProtectedResourceApiService,
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
import { VirtualScrollService } from 'app/shared/services/virtual-scroll.service';
import {
  assign,
  cloneDeep,
  defer,
  each,
  filter as _filter,
  includes,
  isEmpty,
  isUndefined,
  size,
  trim,
  values,
  map as _map
} from 'lodash';
import { map } from 'rxjs/operators';
import { FileDetailComponent } from './file-detail/file-detail.component';
import { HoneypotFileWarningComponent } from './honeypot-file-warning/honeypot-file-warning.component';
import { HoneypotSettingComponent } from './honeypot-setting/honeypot-setting.component';

@Component({
  selector: 'aui-honey-pot',
  templateUrl: './honey-pot.component.html',
  styleUrls: ['./honey-pot.component.less']
})
export class HoneyPotComponent implements OnInit, AfterViewInit, OnDestroy {
  tableConfig: TableConfig;
  tableData: TableData;
  pageIndex = CommonConsts.PAGE_START;
  pageSize = CommonConsts.PAGE_SIZE;
  dataMap = DataMap;
  optItems;
  optsConfig;
  selectionData = [];
  name;
  realTimeStatus = false;
  vstoreName = [];
  currentNum = 0;
  autoPolling;
  autoPollingStatus;
  isSearch = false;

  noDataTip = this.i18n.get('explore_honeypot_no_data_info_label');
  numTip = this.i18n.get('explore_honeypot_start_num_info_label', [
    10,
    this.currentNum
  ]);

  @ViewChild('dataTable', { static: false }) dataTable: ProTableComponent;
  @ViewChild('periodTpl', { static: true }) periodTpl: TemplateRef<any>;
  @ViewChild('statusTpl', { static: true }) statusTpl: TemplateRef<any>;

  constructor(
    public virtualScroll: VirtualScrollService,
    public warningMessageService: WarningMessageService,
    private cdr: ChangeDetectorRef,
    private i18n: I18NService,
    private messageBox: MessageboxService,
    private messageService: MessageService,
    private drawModalService: DrawModalService,
    private dataMapService: DataMapService,
    private configManagementService: ConfigManagementService,
    private protectedResourceApiService: ProtectedResourceApiService,
    private honeypotService: HoneypotService
  ) {}

  ngOnInit(): void {
    this.initConfig();
    this.autoPolling = setInterval(() => {
      this.getNum();
    }, CommonConsts.TIME_INTERVAL);
  }

  ngOnDestroy(): void {
    clearInterval(this.autoPolling);
    if (this.autoPollingStatus) {
      clearTimeout(this.autoPollingStatus);
      this.autoPollingStatus = null;
    }
  }

  ngAfterViewInit() {
    this.getStatus();
  }

  getNum() {
    this.honeypotService
      .ListHoneypotNum({ akLoading: false })
      .subscribe(res => {
        this.currentNum = res.records?.num || 0;
        this.numTip = this.i18n.get('explore_honeypot_start_num_info_label', [
          10,
          this.currentNum
        ]);
      });
  }

  getStatus() {
    const params = {
      pageNum: this.pageIndex + 1,
      pageSize: CommonConsts.PAGE_SIZE
    };
    assign(params, {
      ioDetectConfigStatus: [DataMap.fileHoneypotStatus.enable.value]
    });
    this.configManagementService
      .getVstoreDetectConfigs(params)
      .subscribe(res => {
        this.realTimeStatus = res.totalCount > 0;
        if (this.realTimeStatus) {
          this.getNum();
          defer(() => this.dataTable?.fetchData());
        }
      });
  }

  initConfig() {
    const opts: { [key: string]: ProButton } = {
      startDetect: {
        id: 'startDetection',
        label: this.i18n.get('explore_honeypot_start_detection_label'),
        disableCheck: data => {
          return (
            size(
              _filter(data, val => {
                return val.status === DataMap.fileHoneypotStatus.disable.value;
              })
            ) !== size(data) || !size(data)
          );
        },
        onClick: data => {
          if (size(data) + this.currentNum > 10) {
            this.messageBox.info({
              lvDialogIcon: 'lv-icon-popup-danger-48',
              lvContent: this.i18n.get('explore_honeypot_start_warning_label', [
                10
              ]),
              lvWidth: MODAL_COMMON.smallWidth + 80,
              lvHeight: 240,
              lvOkType: 'primary'
            });
          } else {
            this.startDetection(data);
          }
        },
        type: 'primary'
      },
      stopDetection: {
        id: 'stopDetection',
        label: this.i18n.get('explore_honeypot_stop_detection_label'),
        divide: true,
        disableCheck: data => {
          return (
            size(
              _filter(data, val => {
                return includes(
                  [
                    DataMap.fileHoneypotStatus.enable.value,
                    DataMap.fileHoneypotStatus.error.value
                  ],
                  val.status
                );
              })
            ) !== size(data) || !size(data)
          );
        },
        onClick: data => {
          this.stopDetection(data);
        },
        type: 'primary'
      },
      setting: {
        id: 'honeypotSetting',
        label: this.i18n.get('explore_honeypot_setting_label'),
        disableCheck: data => {
          return !includes(
            [
              DataMap.fileHoneypotStatus.deploy.value,
              DataMap.fileHoneypotStatus.enable.value
            ],
            data[0].status
          );
        },
        onClick: data => {
          this.startDetection(data);
        }
      },
      detail: {
        id: 'getDetail',
        label: this.i18n.get('explore_honeypot_file_detail_label'),
        disableCheck: data => {
          return (
            includes(
              [
                DataMap.fileHoneypotStatus.disable.value,
                DataMap.fileHoneypotStatus.deploy.value,
                DataMap.fileHoneypotStatus.close.value
              ],
              data[0].status
            ) || isUndefined(data[0]?.status)
          );
        },
        onClick: data => {
          this.getDetail(data);
        }
      }
    };

    this.optItems = cloneDeep(getPermissionMenuItem(values(opts)));
    this.optsConfig = getPermissionMenuItem([
      opts.startDetect,
      opts.stopDetection
    ]);

    const cols: TableCols[] = [
      {
        key: 'resourceId',
        name: this.i18n.get('explore_file_system_id_label'),
        hidden: true,
        filter: {
          type: 'search',
          filterMode: 'contains'
        }
      },
      {
        key: 'name',
        name: this.i18n.get('common_name_label'),
        filter: {
          type: 'search',
          filterMode: 'contains'
        }
      },
      {
        key: 'tenantName',
        name: this.i18n.get('common_tenant_label'),
        filter: {
          type: 'search',
          filterMode: 'contains'
        }
      },
      {
        key: 'status',
        name: this.i18n.get('explore_honeypot_detection_status_label'),
        cellRender: this.statusTpl,
        filter: {
          type: 'select',
          isMultiple: true,
          showCheckAll: true,
          options: this.dataMapService.toArray('fileHoneypotStatus')
        }
      },
      {
        key: 'period',
        name: this.i18n.get('explore_honeypot_file_update_cycle_label'),
        cellRender: this.periodTpl
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
            items: this.optItems
          }
        }
      }
    ];

    this.tableConfig = {
      table: {
        autoPolling: CommonConsts.TIME_INTERVAL,
        compareWith: 'resourceId',
        columns: cols,
        rows: {
          selectionMode: 'multiple',
          selectionTrigger: 'selector',
          showSelector: true
        },
        colDisplayControl: {
          ignoringColsType: 'hide'
        },
        virtualScroll: true,
        scrollFixed: true,
        scroll: { y: '770px' },
        fetchData: (filter: Filters, args) => {
          this.getData(filter, args);
        },
        selectionChange: selection => {
          this.selectionData = selection;
        },
        trackByFn: (index, item) => {
          return item.uuid;
        }
      }
    };
  }

  search() {
    this.dataTable.filterChange({
      caseSensitive: false,
      filterMode: 'contains',
      key: 'name',
      value: trim(this.name)
    });
  }

  getData(filters?: Filters, args?: { isAutoPolling: any }) {
    const params = {
      pageNum: filters?.paginator.pageIndex + 1,
      pageSize: filters?.paginator.pageSize,
      akLoading:
        !isUndefined(args) && args.isAutoPolling ? !args.isAutoPolling : true
    };

    if (!isEmpty(filters?.conditions)) {
      const conditionsTemp = JSON.parse(filters.conditions);
      if (conditionsTemp.resourceId) {
        assign(params, { fsId: conditionsTemp.resourceId });
      }
      if (conditionsTemp.name) {
        assign(params, { fsName: conditionsTemp.name });
      }
      if (conditionsTemp.tenantName) {
        assign(params, { vstoreName: conditionsTemp.tenantName });
      }
      if (!isEmpty(conditionsTemp.status)) {
        assign(params, { honeypotModes: conditionsTemp.status });
      }
    }

    this.honeypotService
      .ListHoneypotDetail(params)
      .pipe(
        map(res => {
          each(res.records, tmp => {
            assign(tmp, {
              name: tmp.resourceName,
              tenantName: tmp.vstoreName,
              status: tmp.mode,
              period: tmp?.period !== 0 ? tmp?.period : '--',
              extendInfo: {
                tenantName: tmp.vstoreName
              }
            });
          });
          return res;
        })
      )
      .subscribe(res => {
        this.tableData = {
          data: res.records,
          total: res.totalCount
        };
        this.cdr.detectChanges();
      });
  }

  startDetection(data) {
    this.drawModalService.create(
      assign({}, MODAL_COMMON.generateDrawerOptions(), {
        lvModalKey: 'start-honeypot-detection',
        lvWidth: MODAL_COMMON.normalWidth + 100,
        lvHeader: includes(
          [
            DataMap.fileHoneypotStatus.enable.value,
            DataMap.fileHoneypotStatus.deploy.value
          ],
          data[0].status
        )
          ? this.i18n.get('explore_honeypot_setting_label')
          : this.i18n.get('explore_honeypot_start_detection_label'),
        lvContent: HoneypotSettingComponent,
        lvOkDisabled: true,
        lvAfterOpen: modal => {
          const content = modal.getContentComponent() as HoneypotSettingComponent;
          const modalIns = modal.getInstance();
          content.formGroup.statusChanges.subscribe(res => {
            modalIns.lvOkDisabled = res !== 'VALID';
          });
        },
        lvComponentParams: {
          data: data
        },
        lvOk: modal => {
          return new Promise(resolve => {
            const content = modal.getContentComponent() as HoneypotSettingComponent;
            content.onOK().subscribe({
              next: res => {
                resolve(true);
                const fileInfo = JSON.parse(res['records'].rspInfo);
                let fileData = [];
                each(fileInfo, item => {
                  const tmpInfo = JSON.parse(item);
                  fileData.push({
                    fsName: tmpInfo.fsName,
                    vstoreName: tmpInfo.vstoreName,
                    desc: tmpInfo.honeypotOpRet,
                    status: DataMap.Batch_Result_Status.fail.value
                  });
                });
                if (!fileData.length) {
                  this.messageService.success(
                    this.i18n.get('common_operate_success_label'),
                    {
                      lvMessageKey: 'successKey',
                      lvShowCloseButton: true
                    }
                  );
                } else {
                  this.drawModalService.create({
                    lvModalKey: 'batch',
                    lvWidth: MODAL_COMMON.normalWidth + 100,
                    lvHeight: 500,
                    lvContent: HoneypotFileWarningComponent,
                    lvFooter: [
                      {
                        id: 'close',
                        label: this.i18n.get('common_close_label'),
                        onClick: (modal, button) => modal.close()
                      }
                    ],
                    lvComponentParams: {
                      data: fileData
                    }
                  });
                }
                this.selectionData = [];
                this.dataTable?.setSelections([]);
                this.dataTable.fetchData();
                this.getNum();
              },
              error: () => {
                resolve(false);
              }
            });
          });
        }
      })
    );
  }

  stopDetection(data) {
    const body = [];
    each(data, item => {
      body.push({
        vstoreName: item.extendInfo.tenantName,
        fsName: item.name
      });
    });
    const params: any = {
      honeypotRequests: body
    };
    this.honeypotService
      .CloseHoneypot({
        honeypotRequests: params
      })
      .subscribe(res => {
        this.selectionData = [];
        this.dataTable?.setSelections([]);
        this.dataTable.fetchData();
        this.getNum();
      });
  }

  getDetail(data) {
    this.drawModalService.openDetailModal({
      ...MODAL_COMMON.generateDrawerOptions(),
      lvHeader: this.i18n.get('explore_honeypot_file_detail_label'),
      lvModalKey: 'honeypot_detection_file_detail',
      lvWidth: MODAL_COMMON.normalWidth + 100,
      lvContent: FileDetailComponent,
      lvComponentParams: {
        data: data
      },
      lvFooter: [
        {
          id: 'close',
          label: this.i18n.get('common_close_label'),
          onClick: modal => {
            modal.close();
          }
        }
      ]
    });
  }
}
