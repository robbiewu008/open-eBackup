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
import { FormBuilder, FormGroup } from '@angular/forms';
import { AddServerNodeComponent } from 'app/business/system/settings/cyber-alarm-notify/sys-log-notify/add-server-node/add-server-node.component';
import {
  BaseUtilService,
  CommonConsts,
  DataMap,
  DataMapService,
  I18NService,
  MODAL_COMMON,
  SyslogApiService,
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
import { assign, cloneDeep, isEmpty, isUndefined, map } from 'lodash';
import { Observable, Observer } from 'rxjs';
import { finalize } from 'rxjs/operators';

@Component({
  selector: 'aui-sys-log-notify',
  templateUrl: './sys-log-notify.component.html',
  styleUrls: ['./sys-log-notify.component.less']
})
export class SysLogNotifyComponent implements OnInit, AfterViewInit {
  formGroup: FormGroup;
  sysLogNotifyStatus = false;
  optsConfig: ProButton[];
  tableConfig: TableConfig;
  tableData: TableData = {
    data: [],
    total: 0
  };
  selectionData: any[];
  isModify = false;
  originalConfigInfo; // 保存环境上的config信息
  severityTypes = this.dataMapService.toArray('AlarmSyslogSeverityType');
  languageMethods = this.dataMapService.toArray('SysLogLanguage');

  sendAlarmType = DataMap.SendAlarmType;
  alarmLevelDisplayLabel = ''; // 告警级别展示词条
  alarmTypeDisplayLabel = ''; // 通知类型展示词条
  sendDeviceNameDisplayLabel = ''; // 发送设备名称展示词条
  deviceNameErrorTip = {
    ...this.baseUtilService.requiredErrorTip,
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [32])
  };
  alarmErrorTip = {
    invalidMinLength: this.i18n.get('system_syslog_alarm_type_tips_label')
  };
  @ViewChild('dataTable', { static: true }) dataTable: ProTableComponent;

  constructor(
    private fb: FormBuilder,
    private i18n: I18NService,
    private dataMapService: DataMapService,
    public baseUtilService: BaseUtilService,
    private syslogApiService: SyslogApiService,
    private drawModalService: DrawModalService,
    private batchOperateService: BatchOperateService,
    private warningMessageService: WarningMessageService
  ) {}

  ngOnInit() {
    this.initConfig();
    this.initForm();
    this.getSyslogConfig();
  }

  ngAfterViewInit() {
    if (this.dataTable) {
      this.dataTable.fetchData();
    }
  }

  initConfig() {
    this.optsConfig = [
      {
        label: this.i18n.get('common_delete_label'),
        disableCheck: data => {
          return isEmpty(data);
        },
        onClick: data => this.deleteNode(this.dataTable.getSelections())
      },
      {
        label: this.i18n.get('common_test_label'),
        disableCheck: data => {
          return isEmpty(data);
        },
        onClick: data => this.testNode(this.dataTable.getSelections())
      }
    ];
    this.tableConfig = {
      table: {
        async: true,
        compareWith: 'id',
        autoPolling: CommonConsts.TIME_INTERVAL,
        columns: this.initTableCols(),
        rows: {
          selectionMode: 'multiple',
          selectionTrigger: 'selector',
          showSelector: true
        },
        colDisplayControl: false,
        selectionChange: selection => {
          this.selectionData = selection;
        },
        trackByFn: (_, item) => {
          return item.id;
        },
        fetchData: (filter: Filters, args) => {
          this.getData(filter, args);
        }
      },
      pagination: {
        mode: 'simple',
        showPageSizeOptions: false,
        winTablePagination: true
      }
    };
  }

  initTableCols(): TableCols[] {
    return [
      {
        key: 'ip',
        name: this.i18n.get('common_server_address_label')
      },
      {
        key: 'port',
        name: this.i18n.get('common_port_label')
      },
      {
        key: 'protocol',
        name: this.i18n.get('common_protocol_label'),
        cellRender: {
          type: 'status',
          config: this.dataMapService.toArray('AlarmProtocolType')
        }
      },
      {
        key: 'cert_name',
        name: this.i18n.get('insight_certificate_label')
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
            items: [
              {
                label: this.i18n.get('common_modify_label'),
                onClick: ([data]) => this.addServerNode(data)
              },
              {
                label: this.i18n.get('common_delete_label'),
                onClick: data => this.deleteNode(data)
              }
            ]
          }
        }
      }
    ];
  }

  initForm() {
    this.formGroup = this.fb.group({
      severity: DataMap.AlarmSyslogSeverityType.info.value,
      alarmType: [
        DataMap.SendAlarmType.alarm.value,
        [this.baseUtilService.VALID.minLength(1)]
      ],
      isSendDeviceName: true,
      deviceName: ['', [this.baseUtilService.VALID.maxLength(32)]],
      serverNodes: this.fb.array([]),
      language: DataMap.SysLogLanguage.english.value
    });
    this.listenForm();
  }

  listenForm() {
    this.formGroup.get('isSendDeviceName').valueChanges.subscribe(res => {
      const formItem = this.formGroup.get('deviceName');
      if (res) {
        formItem.setValidators([
          this.baseUtilService.VALID.maxLength(32),
          this.baseUtilService.VALID.required()
        ]);
      } else {
        formItem.clearValidators();
      }
      formItem.updateValueAndValidity();
    });
  }

  addServerNode(rowData?) {
    this.drawModalService.create(
      assign({}, MODAL_COMMON.generateDrawerOptions(), {
        lvModalKey: 'add-server-modal',
        lvWidth: MODAL_COMMON.normalWidth,
        lvHeader: rowData
          ? this.i18n.get('common_modify_label')
          : this.i18n.get('common_add_label'),
        lvContent: AddServerNodeComponent,
        lvOkDisabled: true,
        lvComponentParams: {
          rowData,
          ipSet: new Set(map(this.tableData.data, 'ip')) // 用于判断ip是否重复
        },
        lvAfterOpen: modal => {
          const content = modal.getContentComponent() as AddServerNodeComponent;
          const modalIns = modal.getInstance();
          content.formGroup.statusChanges.subscribe(res => {
            modalIns.lvOkDisabled = res !== 'VALID';
          });
          content.formGroup.updateValueAndValidity();
        },
        lvOk: modal => {
          const content = modal.getContentComponent() as AddServerNodeComponent;
          return new Observable((observer: Observer<void>) => {
            content.onOk().subscribe({
              next: () => {
                this.dataTable.fetchData();
                this.selectionData = [];
                this.dataTable.setSelections([]);
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
      })
    );
  }

  deleteNode(rowData) {
    this.warningMessageService.create({
      content: this.i18n.get('system_syslog_delete_ip_tips_label'),
      onOK: () => {
        this.batchOperateService.selfGetResults(
          item => {
            return this.syslogApiService.deleteSyslogIpUsingDELETE({
              id: item.id
            });
          },
          map(cloneDeep(rowData), item => {
            return assign(item, {
              name: item.ip
            });
          }),
          () => {
            this.dataTable.fetchData();
            this.selectionData = [];
            this.dataTable.setSelections([]);
          }
        );
      }
    });
  }

  testNode(rowData) {
    this.batchOperateService.selfGetResults(
      item => {
        return this.syslogApiService.addSyslogIpUsingPost({
          syslogIpParam: {
            ip: item.ip,
            port: item.port,
            protocol: item.protocol,
            component_id: item?.cert_id,
            is_test_server: true
          },
          akDoException: false,
          akOperationTips: false,
          akLoading: false
        });
      },
      map(cloneDeep(rowData), item => {
        return assign(item, {
          name: item.ip
        });
      }),
      () => {}
    );
  }

  modify() {
    this.isModify = !this.isModify;
  }

  sysLogNotifyChange() {
    this.isModify = false;
    // sysLogNotifyStatus代表的是switch点击时的状态，判断开启关闭要取反
    const enableStatus = !this.sysLogNotifyStatus;
    const params = {
      language: DataMap.SysLogLanguage.english.value,
      enable_syslog_cfg: enableStatus,
      alarm_syslog_severity: DataMap.AlarmSyslogSeverityType.info.value,
      enable_send_device_name: false,
      send_alarm_type: [DataMap.SendAlarmType.alarm.value]
    };
    // originalConfigInfo为空说明是首次开启syslog
    if (!isEmpty(this.originalConfigInfo)) {
      // 将保存的信息重新发送
      assign(params, {
        ...this.originalConfigInfo,
        enable_syslog_cfg: enableStatus
      });
    }
    this.syslogApiService
      .modifySyslogCfgUsingPut({
        syslogConfigParam: params
      })
      .pipe(
        finalize(() => {
          this.getSyslogConfig();
        })
      )
      .subscribe(() => {
        this.sysLogNotifyStatus = !this.sysLogNotifyStatus;
      });
  }

  save() {
    const formGroupValue = this.formGroup.value;
    const params = {
      language: formGroupValue.language,
      alarm_syslog_severity: formGroupValue.severity,
      enable_send_device_name: formGroupValue.isSendDeviceName,
      enable_syslog_cfg: true,
      send_alarm_type: formGroupValue.alarmType,
      user_def_device_name: formGroupValue.deviceName
    };
    if (!formGroupValue.isSendDeviceName) {
      // 如果不启用发送设备名称，则不发送名称
      delete params.user_def_device_name;
    }
    this.syslogApiService
      .modifySyslogCfgUsingPut({
        syslogConfigParam: params
      })
      .subscribe(res => {
        this.isModify = false;
        this.getSyslogConfig();
      });
  }

  cancel() {
    this.isModify = false;
  }

  getData(filters, args) {
    const params = {
      akLoading:
        !isUndefined(args) && args.isAutoPolling ? !args.isAutoPolling : true
    };
    this.getSyslogIp(params);
  }

  getSyslogConfig() {
    this.syslogApiService.getSyslogCfgUsingGET({}).subscribe({
      next: res => {
        this.originalConfigInfo = cloneDeep(res);
        if (res.enable_syslog_cfg) {
          this.formGroup.patchValue({
            severity: res.alarm_syslog_severity,
            alarmType: res.send_alarm_type,
            isSendDeviceName: res.enable_send_device_name,
            deviceName: res?.user_def_device_name || '',
            language: res.language
          });
        }
        this.sysLogNotifyStatus = res.enable_syslog_cfg;
        this.alarmTypeDisplayLabel = map(res.send_alarm_type, item =>
          this.dataMapService.getLabel('SendAlarmType', item)
        ).join('，');
        this.alarmLevelDisplayLabel = [
          this.i18n.get('common_send_label'),
          this.dataMapService.getLabel(
            'AlarmSyslogSeverityType',
            res.alarm_syslog_severity
          ),
          this.i18n.get('system_syslog_alarm_level_label')
        ].join(this.i18n.isEn ? ' ' : '');
        this.sendDeviceNameDisplayLabel = res.enable_send_device_name
          ? this.i18n.get(`system_syslog_show_device_name_label`, [
              res.user_def_device_name || '--'
            ])
          : this.i18n.get(`common_not_enable_label`);
      },
      error: err => {}
    });
  }

  getSyslogIp(params = {}) {
    this.syslogApiService.getSyslogIpUsingGet(params).subscribe({
      next: res => {
        const data = map(res, item => {
          const [cert_name, cert_id] = item.component_id.split('/');
          return assign(item, {
            cert_name,
            cert_id
          });
        });
        this.tableData = {
          data,
          total: res.length
        };
      }
    });
  }
}
