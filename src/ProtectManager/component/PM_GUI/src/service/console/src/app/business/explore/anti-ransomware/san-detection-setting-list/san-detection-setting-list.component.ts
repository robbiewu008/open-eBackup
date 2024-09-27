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
  DataMap,
  DataMapService,
  DetectionConfigField,
  HoneypotService,
  I18NService,
  SanConfigManagementService,
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
import { VirtualScrollService } from 'app/shared/services/virtual-scroll.service';
import {
  assign,
  each,
  filter,
  find,
  includes,
  isEmpty,
  isUndefined,
  map,
  size
} from 'lodash';
import { RealTimeConfirmComponent } from '../detection-setting-list/real-time-confirm/real-time-confirm.component';
import { BatchOperateService } from 'app/shared/services/batch-operate.service';

@Component({
  selector: 'aui-san-detection-setting-list',
  templateUrl: './san-detection-setting-list.component.html',
  styleUrls: ['./san-detection-setting-list.component.less']
})
export class SanDetectionSettingListComponent implements OnInit, AfterViewInit {
  optsConfig;
  selectionData = [];
  tableData: TableData;
  tableConfig: TableConfig;

  @ViewChild('dataTable', { static: false }) dataTable: ProTableComponent;
  @ViewChild('realTimeTpl', { static: true }) realTimeTpl: TemplateRef<any>;
  realTimeStatus = this.dataMapService.toArray('antiSwitchStatus');
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
    private dataMapService: DataMapService,
    private virtualScroll: VirtualScrollService,
    private messageBox: MessageboxService,
    private warningMessageService: WarningMessageService,
    private honeypotService: HoneypotService,
    private sanConfigManagementService: SanConfigManagementService,
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
        id: 'enable-snapshot',
        label: this.i18n.get('explore_enable_snapshot_detection_label'),
        disableCheck: data => {
          return (
            !size(data) ||
            find(
              data,
              item =>
                item.isCopyDetectEnabled ===
                DataMap.antiSwitchStatus.enable.value
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
                item.isRealTimeDetectEnabled ===
                DataMap.antiSwitchStatus.enable.value
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
                item.isRealTimeDetectEnabled !==
                DataMap.antiSwitchStatus.enable.value
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
        id: 'disble-snapshot',
        label: this.i18n.get('explore_disable_snapshot_detection_label'),
        disableCheck: data => {
          return (
            !size(data) ||
            find(
              data,
              item =>
                item.isCopyDetectEnabled !==
                DataMap.antiSwitchStatus.enable.value
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
    this.optsConfig = opts;

    const cols: TableCols[] = [
      {
        key: 'lunName',
        name: this.i18n.get('explore_san_lun_name_label'),
        filter: {
          type: 'search',
          filterMode: 'contains'
        }
      },
      {
        key: 'vstoreName',
        name: this.i18n.get('common_tenant_label'),
        filter: {
          type: 'search',
          filterMode: 'contains'
        }
      },
      {
        key: 'isRealTimeDetectEnabled',
        name: this.i18n.get('explore_real_time_detection_label'),
        filter: {
          type: 'select',
          isMultiple: true,
          showCheckAll: true,
          options: this.dataMapService.toArray('antiSwitchStatus')
        },
        cellRender: this.realTimeTpl
      },
      {
        key: 'isCopyDetectEnabled',
        name: this.i18n.get('explore_snapshot_detection_label'),
        filter: {
          type: 'select',
          isMultiple: true,
          showCheckAll: true,
          options: this.dataMapService.toArray('antiSwitchStatus')
        },
        cellRender: {
          type: 'status',
          config: this.dataMapService.toArray('antiSwitchStatus')
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
      table: {
        autoPolling: CommonConsts.TIME_INTERVAL,
        compareWith: 'id',
        columns: cols,
        scrollFixed: true,
        rows: {
          selectionMode: 'multiple',
          selectionTrigger: 'selector',
          showSelector: true
        },
        scroll: {
          ...this.virtualScroll.scrollParam,
          y: '65vh'
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
      if (conditions.lunName) {
        assign(params, { lunName: conditions.lunName });
      }
      if (conditions.vstoreName) {
        assign(params, { vstoreName: conditions.vstoreName });
      }
      if (conditions.isRealTimeDetectEnabled) {
        assign(params, {
          isRealTimeDetectEnabled: conditions.isRealTimeDetectEnabled
        });
      }
      if (conditions.isCopyDetectEnabled) {
        assign(params, {
          isCopyDetectEnabled: conditions.isCopyDetectEnabled
        });
      }
    }

    this.sanConfigManagementService
      .getSanDetectSwitch(params)
      .subscribe(res => {
        this.tableData = {
          data: res.records,
          total: res.totalCount
        };
        this.cdr.detectChanges();
      });
  }

  enable(data, configField) {
    if (configField === DetectionConfigField.IoDetect) {
      this.messageBox.confirm({
        lvOkType: 'primary',
        lvCancelType: 'default',
        lvContent: RealTimeConfirmComponent,
        lvComponentParams: {
          type: 'SAN'
        },
        lvOk: modal => {
          const content = modal.getContentComponent() as RealTimeConfirmComponent;
          const params: any = {
            detectSwitchConfig: {
              lunDetectConfigField: configField,
              isRealTimeDetectEnabled: true,
              ids: map(data, 'id'),
              lunIds: map(data, 'lunId'),
              isRealTimeEnhancedDetectEnabled: content.isChecked
            }
          };

          this.sanConfigManagementService
            .postSanDetectSwitch(params)
            .subscribe(res => {
              this.selectionData = [];
              this.dataTable.setSelections([]);
              this.dataTable.fetchData();
            });
        }
      });
    } else {
      const params: any = {
        detectSwitchConfig: {
          lunDetectConfigField: configField,
          isCopyDetectEnabled: true,
          ids: map(data, 'id'),
          lunIds: map(data, 'lunId')
        }
      };
      this.sanConfigManagementService
        .postSanDetectSwitch(params)
        .subscribe(res => {
          this.selectionData = [];
          this.dataTable.setSelections([]);
          this.dataTable.fetchData();
        });
    }
  }
  disable(data, configField, contentLabel?) {
    const params: any = {
      detectSwitchConfig: {
        lunDetectConfigField: configField,
        isRealTimeDetectEnabled: false,
        ids: map(data, 'id')
      }
    };
    if (configField === DetectionConfigField.IoDetect) {
      this.disableRealTime(data, configField, contentLabel);
    } else {
      this.disabledIntelligent(data, configField, contentLabel);
    }
  }

  // 禁用实时勒索检测
  disableRealTime(data, configField, contentLabel?) {
    const params: any = {
      detectSwitchConfig: {
        lunDetectConfigField: configField,
        isRealTimeDetectEnabled: false,
        ids: map(data, 'id')
      }
    };
    const body = [];
    each(data, item => {
      body.push({
        vstoreName: item.vstoreName,
        fsName: ''
      });
    });
    const closeParams: any = {
      honeypotRequests: body
    };
    this.warningMessageService.create({
      content: this.i18n.get(contentLabel, [map(data, 'vstoreName')]),
      onOK: () => {
        this.sanConfigManagementService
          .postSanDetectSwitch(params)
          .subscribe(res => {
            this.selectionData = [];
            this.dataTable.setSelections([]);
            this.dataTable.fetchData();
          });
      }
    });
  }

  // 禁用智能勒索检测
  disabledIntelligent(data, configField, contentLabel?) {
    each(data, item => (item.name = item?.lunName));

    this.warningMessageService.create({
      content: this.i18n.get(contentLabel, [map(data, 'vstoreName')]),
      onOK: () => {
        this.batchOperateService.selfGetResults(
          item => {
            return this.sanConfigManagementService.postSanDetectSwitch({
              detectSwitchConfig: {
                lunDetectConfigField: configField,
                isCopyDetectEnabled: false,
                ids: [item.id],
                lunIds: [item.lunId]
              },
              akDoException: false,
              akOperationTips: false,
              akLoading: false
            });
          },
          data,
          () => {
            this.selectionData = [];
            this.dataTable.setSelections([]);
            this.dataTable.fetchData();
          }
        );
      }
    });
  }
}
