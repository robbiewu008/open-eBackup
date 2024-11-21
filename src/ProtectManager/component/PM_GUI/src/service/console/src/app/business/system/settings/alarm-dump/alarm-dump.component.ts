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
import { Component, OnInit, OnDestroy } from '@angular/core';
import { FormBuilder, FormControl, FormGroup } from '@angular/forms';
import {
  AlarmDumpApiService,
  CommonConsts,
  FILE_FORMAT,
  getPermissionMenuItem,
  LANGUAGR_TYPE,
  OperateItems,
  DataMap
} from 'app/shared';
import {
  BaseUtilService,
  CookieService,
  GlobalService,
  I18NService
} from 'app/shared/services';
import { forEach, includes, now } from 'lodash';
import { WarningMessageService } from './../../../../shared/services/warning-message.service';
import { LoadingService } from '@iux/live';
import { Subscription, timer, Subject } from 'rxjs';
import { switchMap, takeUntil } from 'rxjs/operators';
import { AppUtilsService } from 'app/shared/services/app-utils.service';

@Component({
  selector: 'aui-alarm-dump',
  templateUrl: './alarm-dump.component.html',
  providers: [DatePipe],
  styles: [
    `
      .configform-constraint {
        margin-left: 10px;
        vertical-align: middle;
      }
      .modify-form {
        width: 300px;
      }
      .alarm-dump-dashed-line,
      .alarm-dump-operation {
        margin-bottom: 20px !important;
      }
      .between-page {
        display: flex;
        justify-content: space-between;
      }

      .between-page div.aui-block {
        width: 50%;
      }

      .between-page div:first-child {
        margin-right: 16px;
      }
    `
  ]
})
export class AlarmDumpComponent implements OnInit, OnDestroy {
  alarmSftpServer = {} as any;
  isModify = false;

  formItms;
  dumpForm: FormGroup;
  timeSub1$: Subscription;
  timeSub2$: Subscription;

  destroy1$ = new Subject();
  destroy2$ = new Subject();

  baseTime;

  fileFormatMethods;
  languageMethods;

  historyData;
  startPage = CommonConsts.PAGE_START;
  pageSize = CommonConsts.PAGE_SIZE;
  totalCount = 0;
  sizeOptions = CommonConsts.PAGE_SIZE_OPTIONS;

  optLabel = this.i18n.get('common_operation_label');
  editLabel = this.i18n.get('common_modify_label');
  saveLabel = this.i18n.get('common_save_label');
  cancelLabel = this.i18n.get('common_cancel_label');
  alarmDumpLabel = this.i18n.get('system_event_dump_label');
  dumpParamsLabel = this.i18n.get('system_dump_params_label');
  paramsTipLabel = includes(
    [
      DataMap.Deploy_Type.cloudbackup.value,
      DataMap.Deploy_Type.cloudbackup2.value,
      DataMap.Deploy_Type.hyperdetect.value,
      DataMap.Deploy_Type.decouple.value
    ],
    this.i18n.get('deploy_type')
  )
    ? this.i18n.get('system_dump_params_hyperdetect_tip_label')
    : this.i18n.get('system_dump_params_tip_label');
  dumpHistoryLabel = this.i18n.get('system_dump_history_label');
  dumpedTimeLabel = this.i18n.get('system_dumped_time_label');
  dumpPeriodLabel = this.i18n.get('system_dump_period_label');
  dataSavedLabel = this.i18n.get('system_dump_savedb_records_label');
  nextDumpTimeLabel = this.i18n.get('system_next_dump_time_label');
  fileDumpTimeLabel = this.i18n.get('system_file_dump_time_label');
  fileFormatLabel = this.i18n.get('system_file_format_label');
  languageLabel = this.i18n.get('system_language_type_label');
  resultLabel = this.i18n.get('common_status_label');
  nodeLabel = this.i18n.get('system_alarm_dump_node_label');
  fileLabel = this.i18n.get('system_download_file_label');
  startTimeLabel = this.i18n.get('common_start_time_label');
  endTimeLabel = this.i18n.get('common_end_time_label');
  deleteLabel = this.i18n.get('common_delete_label');
  rangeErrorTip = {
    ...this.baseUtilService.rangeErrorTip,
    invalidRang: this.i18n.get('common_valid_rang_label', [7, 120])
  };

  constructor(
    public i18n: I18NService,
    public fb: FormBuilder,
    public datePipe: DatePipe,
    public globalService: GlobalService,
    public baseUtilService: BaseUtilService,
    public alarmDumpApiService: AlarmDumpApiService,
    public warningMessageService: WarningMessageService,
    private cookieService: CookieService,
    private loadingService: LoadingService,
    private appUtilsService: AppUtilsService
  ) {}

  ngOnDestroy(): void {
    this.destroy1$.next(true);
    this.destroy1$.complete();
    this.destroy2$.next(true);
    this.destroy2$.complete();
  }

  ngOnInit() {
    this.initDumpData();
  }

  setSysTime() {
    this.appUtilsService.setTimePickerCurrent(
      this.dumpForm.get('transferStartTime')
    );
  }

  onChange() {
    this.ngOnInit();
  }

  getAlarmSftpServer(data) {
    this.alarmSftpServer = data;
    this.initHistoryData();
  }

  getDumpData(callback) {
    if (this.timeSub1$) {
      this.timeSub1$.unsubscribe();
    }

    this.timeSub1$ = timer(0, CommonConsts.TIME_INTERVAL)
      .pipe(
        switchMap(index => {
          return this.alarmDumpApiService.queryAlarmTransferStorageCfgUsingGET({
            akLoading: !index
          });
        }),
        takeUntil(this.destroy1$)
      )
      .subscribe(res => callback(res));
  }

  addUnit(val) {
    return val ? val + this.i18n.get('common_day_label') : '--';
  }

  initDumpData() {
    this.getDumpData(res => {
      this.baseTime = res.baseTimeStr;
      this.formItms = [
        {
          label: this.dumpedTimeLabel,
          content: res.transferStartTime
        },
        {
          label: this.dumpPeriodLabel,
          content: this.addUnit(res.period)
        },
        {
          label: this.dataSavedLabel,
          content: this.addUnit(res.dataSavedTime)
        },
        {
          label: this.nextDumpTimeLabel,
          content: this.baseTime
        },
        {
          label: this.fileFormatLabel,
          content: FILE_FORMAT[res.fileType]
        },
        {
          label: this.languageLabel,
          content: this.i18n.get(
            `common_${LANGUAGR_TYPE[res.languageType]}_label`
          )
        }
      ];
    });
  }

  initDumpForm() {
    this.getDumpData(res => {
      this.timeSub1$?.unsubscribe();
      const date = res.transferStartTime.split(':');
      this.dumpForm = this.fb.group({
        transferStartTime: new FormControl(
          new Date(1, 2, 3, +date[0], +date[1], +date[2]),
          {
            validators: [this.baseUtilService.VALID.required()],
            updateOn: 'change'
          }
        ),
        period: new FormControl(res.period, {
          validators: [
            this.baseUtilService.VALID.required(),
            this.baseUtilService.VALID.integer(),
            this.baseUtilService.VALID.rangeValue(7, 120)
          ],
          updateOn: 'change'
        }),
        dataSavedTime: new FormControl(res.dataSavedTime, {
          validators: [
            this.baseUtilService.VALID.required(),
            this.baseUtilService.VALID.integer(),
            this.baseUtilService.VALID.rangeValue(7, 120)
          ],
          updateOn: 'change'
        }),
        fileType: new FormControl(res.fileType),
        languageType: new FormControl(res.languageType)
      });
      this.isModify = !this.isModify;
    });
  }

  modifyDump() {
    this.initSelectMethods();
    this.initDumpForm();
  }

  initSelectMethods() {
    this.languageMethods = [];
    this.fileFormatMethods = [];
    forEach(LANGUAGR_TYPE, item => {
      if (isNaN(item)) {
        this.languageMethods.push({
          label: this.i18n.get(`common_${item}_label`),
          value: LANGUAGR_TYPE[item],
          isLeaf: true
        });
      }
    });
    forEach(FILE_FORMAT, item => {
      if (isNaN(item)) {
        this.fileFormatMethods.push({
          label: item,
          value: FILE_FORMAT[item],
          isLeaf: true
        });
      }
    });
  }

  initHistoryData() {
    if (this.timeSub2$) {
      this.timeSub2$.unsubscribe();
    }

    const params = {
      startIndex: this.startPage,
      pageSize: this.pageSize
    };

    this.timeSub2$ = timer(0, CommonConsts.TIME_INTERVAL)
      .pipe(
        switchMap(index => {
          return this.alarmDumpApiService.queryAlarmDumpHisPageUsingGET({
            ...params,
            akLoading: !index
          });
        }),
        takeUntil(this.destroy2$)
      )
      .subscribe(res => {
        this.historyData = [];
        forEach(res.records as any, val => {
          this.historyData.push({
            startTime: val.startTime,
            endTime: val.endTime,
            result: val.result,
            node: val.clusterName,
            resultLink: val.resultLink,
            resultId: val.resultId,
            deviceEsn: val.deviceEsn
          });
        });
        this.totalCount = res.total;
      });
  }

  trackById = (index, item) => {
    return item.resultId;
  };

  pageChange = e => {
    this.pageSize = e.pageSize;
    this.startPage = e.pageIndex;
    this.initHistoryData();
  };

  cancelDump() {
    this.isModify = !this.isModify;
    this.initDumpData();
  }

  saveDumpData() {
    if (!this.dumpForm.valid) {
      return;
    }
    let time = new Date();
    time = this.dumpForm.value['transferStartTime'];
    const hour = time.getHours(),
      min = time.getMinutes(),
      second = time.getSeconds();
    const params = this.dumpForm.value;
    params.transferStartTime =
      (hour < 10 ? '0' + hour : hour) +
      ':' +
      (min < 10 ? '0' + min : min) +
      ':' +
      (second < 10 ? '0' + second : second);
    this.alarmDumpApiService
      .modfiyAlarmTransferStorageCfgUsingPUT({
        dumpRequestBase: params
      })
      .subscribe(res => this.cancelDump());
  }

  optsCallback = item => {
    const menus = [
      {
        id: 'delete',
        label: this.deleteLabel,
        permission: OperateItems.DeletingDumpFile,
        onClick: (d: any) =>
          this.warningMessageService.create({
            content: this.i18n.get('system_delete_dump_label', [
              item.startTime
            ]),
            onOK: () =>
              this.alarmDumpApiService
                .deleteTaskResultsUsingDELETE({
                  fileId: item.resultId,
                  memberEsn: item.deviceEsn
                })
                .subscribe(() => this.initHistoryData())
          })
      },
      {
        id: 'upload',
        label: this.i18n.get('common_upload_label'),
        disabled:
          !this.alarmSftpServer.useEnable ||
          item.result !== DataMap.Dump_History_Result.success.value,
        permission: OperateItems.UploadDumpFile,
        onClick: (d: any) => {
          this.loadingService.show(
            this.i18n.get('common_file_upload_processing_label')
          );
          this.alarmDumpApiService
            .uploadAlarmTransferStorageFileToSftpUsingPOST({
              fileId: item.resultId,
              akLoading: false
            })
            .subscribe(
              () => {
                this.loadingService.hide();
                this.initHistoryData();
              },
              () => {
                this.loadingService.hide();
              }
            );
        }
      }
    ];
    return getPermissionMenuItem(menus, this.cookieService.role);
  };

  exportFile(result) {
    if (!result.resultLink) {
      return;
    }
    this.alarmDumpApiService
      .exportAlarmTransferStorageUsingPOST({
        memberEsn: result.deviceEsn,
        fileId: result.resultId,
        akOperationTips: false
      })
      .subscribe(blob => {
        const bf = new Blob([blob], {
          type: 'application/zip'
        });
        this.appUtilsService.downloadFile(`AlarmDump_${now()}.zip`, bf);
      });
  }
}
