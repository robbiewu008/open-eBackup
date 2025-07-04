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
  CommonConsts,
  ConfigManagementService,
  DataMap,
  DataMapService,
  DetectionConfigField,
  FileExtensionFilterManagementService,
  HoneypotService,
  I18NService,
  MODAL_COMMON,
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
import { BatchOperateService } from 'app/shared/services/batch-operate.service';
import { DrawModalService } from 'app/shared/services/draw-modal.service';
import {
  assign,
  cloneDeep,
  each,
  filter,
  find,
  includes,
  isEmpty,
  isUndefined,
  map,
  size
} from 'lodash';
import { finalize } from 'rxjs';
import { RealTimeConfirmComponent } from './real-time-confirm/real-time-confirm.component';
import { SetFileBlockingComponent } from './set-file-blocking/set-file-blocking.component';

@Component({
  selector: 'aui-detection-setting-list',
  templateUrl: './detection-setting-list.component.html',
  styleUrls: ['./detection-setting-list.component.less']
})
export class DetectionSettingListComponent implements OnInit, AfterViewInit {
  optsConfig;
  selectionData = [];
  tableData: TableData;
  tableConfig: TableConfig;

  @ViewChild('dataTable', { static: false }) dataTable: ProTableComponent;
  @ViewChild('numTpl', { static: true }) numTpl: TemplateRef<any>;
  @ViewChild('realTimeTpl', { static: true }) realTimeTpl: TemplateRef<any>;
  realTimeStatus = this.dataMapService.toArray('File_Extension_Status');
  enableLabel = this.i18n.get('system_tape_enabled_label');
  falseAlarmEnableLabel = this.i18n.isEn
    ? `${this.enableLabel} (${this.i18n.get(
        'explore_enable_false_alarm_label'
      )})`
    : `${this.enableLabel}（${this.i18n.get(
        'explore_enable_false_alarm_label'
      )}）`;
  falseAlarmDisableLabel = this.i18n.isEn
    ? `${this.enableLabel} (${this.i18n.get(
        'explore_disable_false_alarm_label'
      )})`
    : `${this.enableLabel}（${this.i18n.get(
        'explore_disable_false_alarm_label'
      )}）`;

  constructor(
    private i18n: I18NService,
    private cdr: ChangeDetectorRef,
    private messageBox: MessageboxService,
    private dataMapService: DataMapService,
    private honeypotService: HoneypotService,
    private drawModalService: DrawModalService,
    private warningMessageService: WarningMessageService,
    private configManagementService: ConfigManagementService,
    private fileExtensionFilterManagementService: FileExtensionFilterManagementService,
    private batchOperateService: BatchOperateService
  ) {}

  ngAfterViewInit() {
    this.dataTable.fetchData();
  }

  ngOnInit() {
    this.initConfig();
  }

  initConfig() {
    const opts: ProButton[] = [
      {
        id: 'rescan',
        type: 'primary',
        label: this.i18n.get('common_scan_label'),
        onClick: data => {
          this.scan(data);
        }
      },
      {
        id: 'set-blocking',
        label: this.i18n.get('explore_set_blocking_files_rule_label'),
        divide: true,
        onClick: data => {
          this.setBlockingType(data);
        }
      },
      {
        id: 'enable-blocking',
        label: this.i18n.get('explore_enable_ransom_blocking_files_label'),
        disableCheck: data => {
          return (
            !size(data) ||
            find(
              data,
              item =>
                item.fileExtensionFilterConfigStatus ===
                DataMap.File_Extension_Status.enable.value
            )
          );
        },
        onClick: data => {
          this.enable(data, DetectionConfigField.FileExtensionFilter);
        }
      },
      {
        id: 'enable-snapshot',
        label: this.i18n.get('explore_enable_snapshot_detection_label'),
        disableCheck: data => {
          return (
            !size(data) ||
            find(
              data,
              item =>
                item.copyDetectConfigStatus ===
                DataMap.File_Extension_Status.enable.value
            )
          );
        },
        onClick: data => {
          this.enable(data, DetectionConfigField.CopyDetect);
        }
      },
      {
        id: 'enable-real-time',
        label: this.i18n.get('explore_enable_real_time_detection_label'),
        disableCheck: data => {
          return (
            !size(data) ||
            find(
              data,
              item =>
                item.ioDetectConfigStatus ===
                DataMap.File_Extension_Status.enable.value
            )
          );
        },
        onClick: data => {
          this.enable(data, DetectionConfigField.IoDetect);
        }
      },
      {
        id: 'disble-real-time',
        label: this.i18n.get('explore_disable_real_time_detection_label'),
        divide: true,
        disableCheck: data => {
          return (
            !size(data) ||
            find(
              data,
              item =>
                item.ioDetectConfigStatus !==
                DataMap.File_Extension_Status.enable.value
            )
          );
        },
        onClick: data => {
          this.disable(
            data,
            DetectionConfigField.IoDetect,
            'explore_disable_io_detect_label'
          );
        }
      },
      {
        id: 'disable-blocking',
        label: this.i18n.get('explore_disable_ransom_blocking_files_label'),
        divide: true,
        disableCheck: data => {
          return (
            !size(data) ||
            find(
              data,
              item =>
                item.fileExtensionFilterConfigStatus !==
                DataMap.File_Extension_Status.enable.value
            )
          );
        },
        onClick: data => {
          this.disable(
            data,
            DetectionConfigField.FileExtensionFilter,
            'explore_disable_blocking_file_extension_label'
          );
        }
      },
      {
        id: 'disble-snapshot',
        label: this.i18n.get('explore_disable_snapshot_detection_label'),
        disableCheck: data => {
          return (
            !size(data) ||
            find(
              data,
              item =>
                item.copyDetectConfigStatus !==
                DataMap.File_Extension_Status.enable.value
            )
          );
        },
        onClick: data => {
          this.disable(
            data,
            DetectionConfigField.CopyDetect,
            'explore_disable_copy_detect_label'
          );
        }
      }
    ];
    this.optsConfig = filter(opts, item => {
      return !includes(['set-blocking'], item.id);
    });

    const cols: TableCols[] = [
      {
        key: 'vstoreName',
        name: this.i18n.get('common_tenant_label'),
        filter: {
          type: 'search',
          filterMode: 'contains'
        }
      },
      {
        key: 'enabledSuffixCount',
        name: this.i18n.get('explore_blocking_files_rule_num_label'),
        cellRender: this.numTpl
      },
      {
        key: 'fileExtensionFilterConfigStatus',
        name: this.i18n.get('explore_blocking_files_label'),
        filter: {
          type: 'select',
          isMultiple: true,
          showCheckAll: true,
          options: this.dataMapService.toArray('File_Extension_Status')
        },
        cellRender: {
          type: 'status',
          config: this.dataMapService.toArray('File_Extension_Status')
        }
      },
      {
        key: 'ioDetectConfigStatus',
        name: this.i18n.get('explore_real_time_detection_label'),
        filter: {
          type: 'select',
          isMultiple: true,
          showCheckAll: true,
          options: this.dataMapService.toArray('File_Extension_Status')
        },
        cellRender: this.realTimeTpl
      },
      {
        key: 'copyDetectConfigStatus',
        name: this.i18n.get('explore_snapshot_detection_label'),
        filter: {
          type: 'select',
          isMultiple: true,
          showCheckAll: true,
          options: this.dataMapService.toArray('File_Extension_Status')
        },
        cellRender: {
          type: 'status',
          config: this.dataMapService.toArray('File_Extension_Status')
        }
      },
      {
        key: 'operation',
        name: this.i18n.get('common_operation_label'),
        cellRender: {
          type: 'operation',
          config: {
            maxDisplayItems: 1,
            items: filter(opts, item => {
              return !includes(['rescan'], item.id);
            })
          }
        }
      }
    ];

    this.tableConfig = {
      filterTags: true,
      table: {
        autoPolling: CommonConsts.TIME_INTERVAL,
        compareWith: 'vstoreId',
        columns: cols,
        scrollFixed: true,
        rows: {
          selectionMode: 'multiple',
          selectionTrigger: 'selector',
          showSelector: true
        },
        colDisplayControl: false,
        fetchData: (filter: Filters, args) => {
          this.getData(filter, args);
        },
        selectionChange: selection => {
          this.selectionData = selection;
        }
      }
    };
  }

  getData(filters: Filters, args) {
    const params = {
      pageNum: filters.paginator.pageIndex + 1,
      pageSize: filters.paginator.pageSize,
      akLoading:
        !isUndefined(args) && args.isAutoPolling ? !args.isAutoPolling : true
    };

    if (!isEmpty(filters.conditions)) {
      const conditions = JSON.parse(filters.conditions);
      if (conditions.vstoreName) {
        assign(params, { vstoreName: conditions.vstoreName });
      }
      if (conditions.ioDetectConfigStatus) {
        assign(params, {
          ioDetectConfigStatus: conditions.ioDetectConfigStatus
        });
      }
      if (conditions.fileExtensionFilterConfigStatus) {
        assign(params, {
          fileExtensionFilterConfigStatus:
            conditions.fileExtensionFilterConfigStatus
        });
      }
      if (conditions.copyDetectConfigStatus) {
        assign(params, {
          copyDetectConfigStatus: conditions.copyDetectConfigStatus
        });
      }
    }

    this.configManagementService
      .getVstoreDetectConfigs(params)
      .subscribe(res => {
        this.tableData = {
          data: res.records,
          total: res.totalCount
        };
        this.cdr.detectChanges();
      });
  }

  scan(data) {
    this.messageBox.confirm({
      lvOkType: 'primary',
      lvCancelType: 'default',
      LvHeader: this.i18n.get('common_scan_label'),
      lvContent: this.i18n.get('explore_rescan_copy_detect_label'),
      lvOk: () => {
        this.fileExtensionFilterManagementService
          .scanFileExtensionFilter({})
          .subscribe(res => {
            this.dataTable.fetchData();
          });
      }
    });
  }

  setBlockingType(data) {
    this.drawModalService.create({
      ...MODAL_COMMON.generateDrawerOptions(),
      lvHeader: this.i18n.get('explore_set_blocking_files_rule_label'),
      lvModalKey: 'associate_vstore',
      lvWidth: MODAL_COMMON.normalWidth,
      lvContent: SetFileBlockingComponent,
      lvComponentParams: {
        vstoreId: data[0].vstoreId
      },
      lvOk: modal => {
        return new Promise(resolve => {
          const content = modal.getContentComponent() as SetFileBlockingComponent;
          content.onOK().subscribe({
            next: () => {
              resolve(true);
              this.selectionData = [];
              this.dataTable.setSelections([]);
              this.dataTable.fetchData();
            },
            error: error => resolve(false)
          });
        });
      }
    });
  }

  getSuffixDetail(data) {
    this.drawModalService.create({
      ...MODAL_COMMON.generateDrawerOptions(),
      lvHeader: this.i18n.get('explore_blocking_files_rule_label'),
      lvModalKey: 'suffix_detail',
      lvWidth: MODAL_COMMON.normalWidth,
      lvContent: SetFileBlockingComponent,
      lvOkDisabled: true,
      lvComponentParams: {
        isDetail: true,
        vstoreId: data.vstoreId
      },
      lvFooter: [
        {
          label: this.i18n.get('common_close_label'),
          onClick: modal => modal.close()
        }
      ]
    });
  }

  enable(data, configField) {
    if (configField === DetectionConfigField.IoDetect) {
      this.messageBox.confirm({
        lvOkType: 'primary',
        lvCancelType: 'default',
        lvHeader: this.i18n.get('explore_enable_real_time_detection_label'),
        lvWidth: this.i18n.isEn ? 480 : 400,
        lvContent: RealTimeConfirmComponent,
        lvComponentParams: {
          type: 'NAS'
        },
        lvOk: modal => {
          const content = modal.getContentComponent() as RealTimeConfirmComponent;
          const params = {
            vstoreCopyDetectConfig: {
              configField,
              isEnabled: true,
              vstoreIds: map(data, 'vstoreId'),
              isEnhancedDetect: content.isChecked
            }
          };
          this.configManagementService
            .updateVstoreDetectConfigs(params)
            .subscribe(() => {
              this.selectionData = [];
              this.dataTable.setSelections([]);
              this.dataTable.fetchData();
            });
        }
      });
    } else if (configField === DetectionConfigField.FileExtensionFilter) {
      this.messageBox.confirm({
        lvHeader: this.i18n.get('explore_enable_ransom_blocking_files_label'),
        lvWidth: this.i18n.isEn ? 480 : 400,
        lvContent: this.i18n.get('explore_blocking_files_confirm_label'),
        lvOk: () => {
          const params = {
            vstoreCopyDetectConfig: {
              configField,
              isEnabled: true,
              vstoreIds: map(data, 'vstoreId'),
              isEnhancedDetect: false
            }
          };
          this.configManagementService
            .updateVstoreDetectConfigs(params)
            .subscribe(res => {
              this.selectionData = [];
              this.dataTable.setSelections([]);
              this.dataTable.fetchData();
            });
        }
      });
    } else {
      const params = {
        vstoreCopyDetectConfig: {
          configField,
          isEnabled: true,
          vstoreIds: map(data, 'vstoreId'),
          isEnhancedDetect: false
        }
      };
      this.configManagementService
        .updateVstoreDetectConfigs(params)
        .subscribe(res => {
          this.selectionData = [];
          this.dataTable.setSelections([]);
          this.dataTable.fetchData();
        });
    }
  }

  disable(data, configField, contentLabel?) {
    each(data, item => (item.name = item?.vstoreName));
    if (configField === DetectionConfigField.IoDetect) {
      this.disableRealTime(data, configField, contentLabel);
    } else {
      this.disabledIntelligent(data, configField, contentLabel);
    }
  }

  // 禁用实时勒索检测
  disableRealTime(data, configField, contentLabel?) {
    this.warningMessageService.create({
      content: this.i18n.get(contentLabel, [map(data, 'vstoreName')]),
      onOK: () => {
        this.batchOperateService.selfGetResults(
          item => {
            return this.honeypotService
              .CloseHoneypot({
                honeypotRequests: {
                  honeypotRequests: [
                    {
                      vstoreName: item.vstoreName,
                      fsName: ''
                    }
                  ]
                } as any,
                akDoException: false,
                akOperationTips: false,
                akLoading: false
              })
              .pipe(
                finalize(() => {
                  this.configManagementService
                    .updateVstoreDetectConfigs({
                      vstoreCopyDetectConfig: {
                        configField,
                        isEnabled: false,
                        isEnhancedDetect: false,
                        vstoreIds: [item.vstoreId]
                      } as any,
                      akOperationTips: false,
                      akLoading: false
                    })
                    .subscribe();
                })
              );
          },
          map(cloneDeep(data), item => {
            assign(item, { uuid: item.vstoreId });
            return item;
          }),
          () => {
            this.selectionData = [];
            this.dataTable.setSelections([]);
            this.dataTable.fetchData();
          },
          '',
          true
        );
      }
    });
  }

  // 禁用智能勒索检测
  disabledIntelligent(data, configField, contentLabel?) {
    this.warningMessageService.create({
      content: this.i18n.get(contentLabel, [map(data, 'vstoreName')]),
      onOK: () => {
        this.batchOperateService.selfGetResults(
          item => {
            return this.configManagementService.updateVstoreDetectConfigs({
              vstoreCopyDetectConfig: {
                configField,
                isEnabled: false,
                isEnhancedDetect: false,
                vstoreIds: [item.vstoreId]
              } as any,
              akDoException: false,
              akOperationTips: false,
              akLoading: false
            });
          },
          map(cloneDeep(data), item => {
            assign(item, { uuid: item.vstoreId });
            return item;
          }),
          () => {
            this.selectionData = [];
            this.dataTable.setSelections([]);
            this.dataTable.fetchData();
          },
          '',
          true
        );
      }
    });
  }
}
