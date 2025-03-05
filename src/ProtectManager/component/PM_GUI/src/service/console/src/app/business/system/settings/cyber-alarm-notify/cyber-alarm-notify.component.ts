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
import { Component, OnInit, ViewChild } from '@angular/core';
import { FormBuilder, FormControl, FormGroup } from '@angular/forms';
import { DomSanitizer } from '@angular/platform-browser';
import { Router } from '@angular/router';
import {
  AlarmEmailNotifyApiService,
  DataMap,
  DataMapService,
  I18NService,
  MODAL_COMMON,
  WarningMessageService
} from 'app/shared';
import { AlarmNotifyRuleApiService } from 'app/shared/api/services/alarm-notify-rule-api.service';
import { ProButton } from 'app/shared/components/pro-button/interface';
import {
  Filters,
  ProTableComponent,
  TableConfig,
  TableData
} from 'app/shared/components/pro-table';
import { BatchOperateService } from 'app/shared/services/batch-operate.service';
import { DrawModalService } from 'app/shared/services/draw-modal.service';
import {
  assign,
  cloneDeep,
  defer,
  isEmpty,
  isString,
  map,
  pick,
  size,
  toLower
} from 'lodash';
import { Observable, Observer, finalize } from 'rxjs';
import { AddEmailComponent } from './add-email/add-email.component';

@Component({
  selector: 'aui-cyber-alarm-notify',
  templateUrl: './cyber-alarm-notify.component.html',
  styleUrls: ['./cyber-alarm-notify.component.less']
})
export class CyberAlarmNotifyComponent implements OnInit {
  formGroup: FormGroup;
  alarmNotify = [];
  severityItems = this.dataMapService
    .toArray('alarmNotifySeverity')
    .map(item => {
      item.isLeaf = true;
      return item;
    });
  languageMethods = this.dataMapService.toArray('alarmNotifyLanguage');
  isModify = false;
  useEnable = false;
  alarmNotifyStatus = false;
  originalAlarmNotify;
  dynamicCodeHelp;
  _toLower = toLower;

  tableConfig: TableConfig;
  tableData: TableData;
  optsConfig: ProButton[];
  selectionData = [];
  isLoaded = false;
  isCyberEngine =
    this.i18n.get('deploy_type') === DataMap.Deploy_Type.cyberengine.value;

  @ViewChild('dataTable', { static: false }) dataTable: ProTableComponent;

  constructor(
    private router: Router,
    private fb: FormBuilder,
    private i18n: I18NService,
    private sanitizer: DomSanitizer,
    private dataMapService: DataMapService,
    private drawModalService: DrawModalService,
    private batchOperateService: BatchOperateService,
    private emailApiService: AlarmEmailNotifyApiService,
    private warningMessageService: WarningMessageService,
    private alarmNotifyRuleApiService: AlarmNotifyRuleApiService
  ) {
    this.dynamicCodeHelp = this.sanitizer.bypassSecurityTrustHtml(
      i18n.get('system_open_email_config_label')
    );
  }

  ngOnInit() {
    this.getEmailNotifyStatus();
    this.initForm();
    this.initConfig();
    this.getAlarmSeverity();
    this.getEmail();
  }

  getEmailNotifyStatus() {
    this.emailApiService
      .queryEmailServerUsingGET({})
      .pipe(
        finalize(() => {
          this.isLoaded = true;
          defer(() => this.openEmailLink());
        })
      )
      .subscribe({
        next: res => {
          this.useEnable = !isEmpty(res.server);
        }
      });
  }

  openEmailLink() {
    const openEmail = document.querySelector('#open-email-settings');
    if (!openEmail) {
      return;
    }
    openEmail.addEventListener('click', () => {
      this.router.navigateByUrl('/system/settings/alarm-notify');
    });
  }

  initForm() {
    this.formGroup = this.fb.group({
      alarmSeveritySet: new FormControl(),
      language: new FormControl()
    });
  }

  initConfig() {
    this.optsConfig = [
      {
        label: this.i18n.get('common_add_label'),
        type: 'primary',
        onClick: () => this.create()
      },
      {
        label: this.i18n.get('common_delete_label'),
        disableCheck: data => {
          return isEmpty(data);
        },
        onClick: data => this.delete(data)
      },
      {
        label: this.i18n.get('common_test_label'),
        disableCheck: data => {
          return isEmpty(data);
        },
        onClick: data => this.testEmail(data)
      }
    ];
    this.tableConfig = {
      table: {
        compareWith: 'emailAddress',
        async: false,
        columns: [
          {
            key: 'emailAddress',
            name: this.i18n.get('system_recipient_email_label')
          },
          {
            key: 'desc',
            name: this.i18n.get('common_desc_label')
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
                    onClick: ([data]) => this.create(data)
                  },
                  {
                    label: this.i18n.get('common_delete_label'),
                    onClick: data => this.delete(data)
                  }
                ]
              }
            }
          }
        ],
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
          return item.emailAddress;
        }
      },
      pagination: {
        mode: 'simple',
        showPageSizeOptions: false,
        winTablePagination: true
      }
    };
  }

  alarmNotifyChange() {
    if (this.alarmNotifyStatus) {
      if (this.originalAlarmNotify?.useEnable) {
        this.alarmNotifyRuleApiService
          .modifyNotifyBaseInfoUsingPUT({
            notifyRuleRequest: {
              alarmSeveritySet: this.originalAlarmNotify?.alarmSeveritySet,
              language: this.originalAlarmNotify?.language,
              useEnable: false
            }
          })
          .subscribe(() => {
            this.isModify = false;
            this.alarmNotifyStatus = false;
            this.getAlarmSeverity();
          });
      } else {
        this.isModify = false;
        this.alarmNotifyStatus = false;
      }
    } else {
      if (
        !this.originalAlarmNotify?.useEnable &&
        this.originalAlarmNotify?.language
      ) {
        this.alarmNotifyRuleApiService
          .modifyNotifyBaseInfoUsingPUT({
            notifyRuleRequest: {
              alarmSeveritySet: this.originalAlarmNotify?.alarmSeveritySet,
              language: this.originalAlarmNotify?.language,
              useEnable: true
            }
          })
          .subscribe(() => {
            this.isModify = false;
            this.alarmNotifyStatus = true;
            this.getAlarmSeverity();
          });
      } else {
        this.isModify = false;
        this.alarmNotifyStatus = true;
      }
    }
  }

  getAlarmSeverity() {
    this.alarmNotifyRuleApiService.getRemoteNotifyRuleUsingGET({}).subscribe(
      res => {
        this.originalAlarmNotify = cloneDeep(res);
        this.alarmNotifyStatus = res.useEnable;
        const params = {
          alarmSeveritySet: res.alarmSeveritySet,
          language: res.language
        };
        this.alarmNotify = [
          {
            label: this.i18n.get('system_alarm_severity_label'),
            key: 'severity',
            value: res.alarmSeveritySet
          },
          {
            label: this.i18n.get('system_language_type_label'),
            key: 'language',
            value: res.language
          }
        ];
        this.formGroup.patchValue(params);
      },
      () => {
        const params = {
          alarmSeveritySet: [],
          language: ''
        };
        this.alarmNotify = [
          {
            label: this.i18n.get('system_alarm_severity_label'),
            key: 'severity',
            value: []
          },
          {
            label: this.i18n.get('system_language_type_label'),
            key: 'language',
            value: ''
          }
        ];
        this.formGroup.patchValue(params);
      }
    );
  }

  getEmail() {
    this.alarmNotifyRuleApiService
      .getEmailsUsingGET({})
      .subscribe((res: any) => {
        if (isString(res)) {
          res = JSON.parse(res);
        }
        this.tableData = {
          data: res,
          total: size(res)
        };
      });
  }

  create(rowData?) {
    this.drawModalService.create(
      assign({}, MODAL_COMMON.generateDrawerOptions(), {
        lvModalKey: 'add-email-modal',
        lvWidth: MODAL_COMMON.normalWidth,
        lvHeader: rowData
          ? this.i18n.get('common_modify_label')
          : this.i18n.get('common_add_label'),
        lvContent: AddEmailComponent,
        lvOkDisabled: true,
        lvComponentParams: {
          rowData
        },
        lvAfterOpen: modal => {
          const content = modal.getContentComponent() as AddEmailComponent;
          const modalIns = modal.getInstance();
          content.formGroup.statusChanges.subscribe(res => {
            modalIns.lvOkDisabled = res !== 'VALID';
          });
          content.formGroup.updateValueAndValidity();
        },
        lvOk: modal => {
          const content = modal.getContentComponent() as AddEmailComponent;
          return new Observable((observer: Observer<void>) => {
            content.onOk().subscribe(
              () => {
                this.getEmail();
                observer.next();
                observer.complete();
              },
              error => {
                observer.error(error);
                observer.complete();
              }
            );
          });
        }
      })
    );
  }

  testEmail(data) {
    this.batchOperateService.selfGetResults(
      item => {
        return this.alarmNotifyRuleApiService.testEmailRecipientUsingPUT({
          emailRequest: {
            emailAddress: item.emailAddress,
            desc: item.desc
          },
          akDoException: false,
          akOperationTips: false,
          akLoading: false
        });
      },
      map(cloneDeep(data), item => {
        return assign(item, {
          name: item.emailAddress
        });
      }),
      () => {}
    );
  }

  delete(datas) {
    this.warningMessageService.create({
      content: this.i18n.get('system_alarm_email_delete_label'),
      onOK: () => {
        this.alarmNotifyRuleApiService
          .deleteEmailUsingDELETE({
            emailRequests: map(datas, item =>
              pick(item, ['emailAddress', 'desc'])
            )
          })
          .subscribe(() => {
            this.getEmail();
            this.selectionData = [];
            this.dataTable.setSelections([]);
          });
      }
    });
  }

  modify() {
    if (!this.alarmNotifyStatus) {
      return;
    }
    this.isModify = true;
    if (!this.originalAlarmNotify?.useEnable) {
      return;
    }
    this.getAlarmSeverity();
  }

  cancel() {
    this.isModify = false;
    if (!this.originalAlarmNotify?.useEnable) {
      return;
    }
    this.getAlarmSeverity();
  }

  save() {
    this.alarmNotifyRuleApiService
      .modifyNotifyBaseInfoUsingPUT({
        notifyRuleRequest: {
          alarmSeveritySet: this.formGroup.value.alarmSeveritySet,
          language: this.formGroup.value.language,
          useEnable: true
        }
      })
      .subscribe(() => {
        this.getAlarmSeverity();
        this.isModify = false;
      });
  }
}
