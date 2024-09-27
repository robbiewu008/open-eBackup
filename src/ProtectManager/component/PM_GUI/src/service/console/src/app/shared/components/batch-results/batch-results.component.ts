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
  ChangeDetectionStrategy,
  ChangeDetectorRef,
  Component,
  NgZone,
  OnInit,
  TemplateRef,
  ViewChild
} from '@angular/core';
import { DomSanitizer, SafeHtml } from '@angular/platform-browser';
import { Router } from '@angular/router';
import { ModalRef } from '@iux/live';
import {
  AnonyControllerService,
  CommonConsts,
  DataMap,
  IODETECTFILESYSTEMService
} from 'app/shared';
import {
  ApplicationType,
  FCVmInNormalStatus,
  HCSHostInNormalStatus
} from 'app/shared/consts';
import {
  CookieService,
  DataMapService,
  I18NService
} from 'app/shared/services';
import {
  assign,
  cloneDeep,
  each,
  find,
  first,
  get,
  includes,
  isEmpty,
  isEqual,
  isFunction,
  isString,
  map,
  set,
  size,
  uniqWith
} from 'lodash';

@Component({
  selector: 'batch-results',
  templateUrl: './batch-results.component.html',
  styleUrls: ['./batch-results.component.less'],
  changeDetection: ChangeDetectionStrategy.OnPush
})
export class BatchResultsComponent implements OnInit {
  desc: SafeHtml;
  isSynExecute;
  syncNumber;
  doAction;
  sourceData;
  customLabel;
  tableData;
  pageSize = CommonConsts.PAGE_SIZE_SMALL;
  columns = [];
  extendCols;
  resultDataMap;
  isBatchReport = false;
  total = 0;
  successful = 0;
  failed = 0;
  running = 0;
  isHcsUser = this.cookieService.get('userType') === CommonConsts.HCS_USER_TYPE;
  resultLabel;
  needGetDetection;

  isCyberEngine =
    this.i18n.get('deploy_type') === DataMap.Deploy_Type.cyberengine.value;
  showDetectionTip = false;
  realDetectionTip;

  @ViewChild('headerTpl', { static: true }) headerTpl: TemplateRef<any>;
  @ViewChild('detectionTpl', { static: true }) detectionTpl: TemplateRef<any>;

  constructor(
    private router: Router,
    private ngZone: NgZone,
    public i18n: I18NService,
    private modal: ModalRef,
    private cdr: ChangeDetectorRef,
    private sanitizer: DomSanitizer,
    private cookieService: CookieService,
    private dataMapService: DataMapService,
    private sqlVerificateApiService: AnonyControllerService,
    private detectFilesystemService: IODETECTFILESYSTEMService
  ) {
    this.desc = this.sanitizer.bypassSecurityTrustHtml(
      i18n.get('common_command_successfully_label')
    );
  }

  ngOnInit() {
    if (
      get(first(this.sourceData), 'subType') === 'report' &&
      size(this.extendCols)
    ) {
      this.resultDataMap = 'batchSendEmailStatus';
      this.isBatchReport = true;
    } else {
      this.resultDataMap = 'Batch_Result_Status';
      this.isBatchReport = false;
    }
    this.getColumns();
    this.executeSelfGetResults();
    this.getModalHeader();
  }

  executeSelfGetResults() {
    if (this.isSynExecute) {
      this.asySelfGetResults();
    } else {
      if (!this.syncNumber) {
        this.selfGetResults();
      } else {
        this.syncNumberSelfGetResults();
      }
    }
  }

  getColumns() {
    this.columns = [
      {
        label: this.customLabel || this.i18n.get('common_object_label'),
        key: 'name'
      },
      ...this.extendCols,
      {
        label: this.i18n.get('common_status_label'),
        key: 'status',
        filter: false,
        filterMap: this.dataMapService.toArray('Batch_Result_Status')
      },
      {
        label: this.i18n.get('common_desc_label'),
        key: 'desc'
      }
    ];
  }

  filterChange = e => {};

  jumpTask(event) {
    if (
      event &&
      event.target &&
      includes(event.target.className, 'task-link-btn')
    ) {
      this.ngZone.run(() => {
        if (this.isHcsUser && window.parent) {
          const parentUrl = window.parent.location.href;
          window.parent.location.href = `${first(
            parentUrl.split('#')
          )}#/insight/jobs`;
        } else {
          this.router.navigate(['insight/jobs']);
        }
      });
    }
  }

  selfGetResults() {
    if (isFunction(this.doAction) && !isEmpty(this.sourceData)) {
      this.tableData = [];
      const totalNum = size(this.sourceData);
      this.total = size(this.sourceData);
      this.successful = 0;
      this.failed = 0;
      this.running = totalNum;
      each(this.sourceData, item => {
        const resData = {
          name: item.name,
          status: DataMap.Batch_Result_Status.running.value,
          desc: '--'
        };
        this.addExtendInfo(resData, item);
        this.tableData = this.tableData.concat(resData);
        this.cdr.detectChanges();
        let doAction = this.doAction;
        if (
          item.subType === ApplicationType.HCSCloudHost &&
          HCSHostInNormalStatus.includes(item.status)
        ) {
          this.failed++;
          this.running--;
          assign(resData, {
            status: DataMap.Batch_Result_Status.fail.value,
            desc: this.i18n.get('protect_hcs_host_innormal_status_label')
          });
          this.cdr.detectChanges();
          if (!this.running) {
            this.setModalOptions();
          }

          return;
        }
        if (
          item.subType === ApplicationType.FusionCompute &&
          FCVmInNormalStatus.includes(item.status)
        ) {
          this.failed++;
          this.running--;
          assign(resData, {
            status: DataMap.Batch_Result_Status.fail.value,
            desc: this.i18n.get('protect_fc_vm_innormal_status_label')
          });
          this.cdr.detectChanges();
          if (!this.running) {
            this.setModalOptions();
          }

          return;
        }
        if (
          item.type &&
          item.jobId &&
          includes(
            [
              DataMap.Job_type.db_desesitization.value,
              DataMap.Job_type.db_identification.value
            ],
            item.type
          )
        ) {
          doAction = val => {
            return this.sqlVerificateApiService.stopTaskUsingPOST({
              request: {
                request_id: val.jobId
              },
              akDoException: false,
              akOperationTips: false,
              akLoading: false
            });
          };
        }
        doAction(item).subscribe(
          res => {
            this.successful++;
            this.running--;
            assign(resData, {
              status: DataMap.Batch_Result_Status.successful.value,
              desc: item.isAsyn === true ? this.desc : '--'
            });
            if (
              res &&
              res.failCount &&
              res.results[0] &&
              res.results[0].errorCode
            ) {
              this.successful--;
              this.failed++;
              assign(resData, {
                status: DataMap.Batch_Result_Status.fail.value,
                desc: this.i18n.get(res.results[0].errorCode)
              });
            }
            this.cdr.detectChanges();
            if (!this.running) {
              this.setModalOptions();
            }
          },
          ex => {
            this.failed++;
            this.running--;
            let response: any = ex.error;
            if (response && isString(response)) {
              try {
                response = JSON.parse(response);
              } catch (error) {
                response = {
                  errorCode: 'common_unknown_exception_label',
                  errorMessage: 'common_unknown_exception_label'
                };
              }
            }
            const errorCode = response.errorCode;
            const errorMessage = response.errorMessage;
            let parameters = response.parameters || [];

            if (isString(response.detailParam)) {
              parameters = [parameters];
            }

            let errorMsg = this.i18n.get(errorCode, parameters);
            if (errorMessage && (errorCode === errorMsg || !errorMsg)) {
              errorMsg = errorMessage;
              if (!!size(parameters)) {
                errorMsg = this.i18n.get(errorMessage, parameters);
              }
            }
            assign(resData, {
              status: DataMap.Batch_Result_Status.fail.value,
              desc: errorMsg
            });
            this.cdr.detectChanges();
            if (!this.running) {
              this.setModalOptions();
            }
          }
        );
      });
    }
  }

  syncNumberSelfGetResults() {
    if (isFunction(this.doAction) && !isEmpty(this.sourceData)) {
      this.tableData = [];
      const totalNum = size(this.sourceData);
      this.total = totalNum;
      this.running = totalNum;
      this.successful = 0;
      this.failed = 0;
      const cloneData = cloneDeep(this.sourceData);
      each(this.sourceData, item => {
        const resData = {
          uuid: item.uuid,
          name: item.name,
          status: DataMap.Batch_Result_Status.running.value,
          desc: '--'
        };
        this.addExtendInfo(resData, item);
        this.tableData = this.tableData.concat(resData);
      });
      this.recursionAction(cloneData);
    }
  }

  recursionAction(sourceData) {
    if (!size(sourceData)) {
      return;
    }
    const tempData = sourceData.splice(0, this.syncNumber);
    let tempExcuteNum = size(tempData);
    each(tempData, item => {
      const resData = find(this.tableData, { uuid: item.uuid });
      this.doAction(item).subscribe(
        res => {
          tempExcuteNum--;
          this.successful++;
          this.running--;
          assign(resData, {
            status: DataMap.Batch_Result_Status.successful.value,
            desc: item.isAsyn === true ? this.desc : '--'
          });
          if (
            res &&
            res.failCount &&
            res.results[0] &&
            res.results[0].errorCode
          ) {
            assign(resData, {
              status: DataMap.Batch_Result_Status.fail.value,
              desc: this.i18n.get(res.results[0].errorCode)
            });
          }
          this.cdr.detectChanges();
          if (!this.running) {
            this.setModalOptions();
          }
          if (!tempExcuteNum) {
            this.recursionAction(sourceData);
          }
        },
        ex => {
          tempExcuteNum--;
          this.failed++;
          this.running--;
          let response: any = ex.error;
          if (response && isString(response)) {
            try {
              response = JSON.parse(response);
            } catch (error) {
              response = {
                errorCode: 'common_timeout_retry_label',
                errorMessage: 'common_timeout_retry_label'
              };
            }
          }
          const errorCode = response.errorCode;
          const errorMessage = response.errorMessage;
          let parameters = response.parameters || [];

          if (isString(response.detailParam)) {
            parameters = [parameters];
          }

          let errorMsg = this.i18n.get(errorCode, parameters);
          if (errorMessage && (errorCode === errorMsg || !errorMsg)) {
            errorMsg = errorMessage;
            if (!!size(parameters)) {
              errorMsg = this.i18n.get(errorMessage, parameters);
            }
          }
          if (isEmpty(errorMsg)) {
            errorMsg = this.i18n.get('common_timeout_retry_label');
          }
          assign(resData, {
            status: DataMap.Batch_Result_Status.fail.value,
            desc: errorMsg
          });
          this.cdr.detectChanges();
          if (!this.running) {
            this.setModalOptions();
          }
          if (!tempExcuteNum) {
            this.recursionAction(sourceData);
          }
        }
      );
    });
  }

  async asySelfGetResults() {
    if (isFunction(this.doAction) && !isEmpty(this.sourceData)) {
      this.tableData = [];
      const totalNum = size(this.sourceData);
      this.total = totalNum;
      this.successful = 0;
      this.failed = 0;
      this.running = totalNum;
      each(this.sourceData, item => {
        const resData = {
          name: item.name,
          status: DataMap.Batch_Result_Status.running.value,
          desc: '--'
        };
        this.addExtendInfo(resData, item);

        if (item?.body?.type !== DataMap.Resource_Type.vmGroup.value) {
          this.tableData = this.tableData.concat(resData);
        }
        this.cdr.detectChanges();
        let doAction = this.doAction;
        if (
          item.type &&
          item.jobId &&
          includes(
            [
              DataMap.Job_type.db_desesitization.value,
              DataMap.Job_type.db_identification.value
            ],
            item.type
          )
        ) {
          doAction = val => {
            return this.sqlVerificateApiService.stopTaskUsingPOST({
              request: {
                request_id: val.jobId
              },
              akDoException: false,
              akOperationTips: false,
              akLoading: false
            });
          };
        }
      });

      for (let index = 0; index < this.sourceData.length; index++) {
        const item = this.sourceData[index];
        await new Promise((resolve, reject) => {
          if (item.status === 'HCSCloudHostInNormal') {
            resolve(true);
            this.failed++;
            this.running--;
            assign(this.tableData[index], {
              status: DataMap.Batch_Result_Status.fail.value,
              desc: this.i18n.get('protect_hcs_host_innormal_status_label')
            });
            this.cdr.detectChanges();
            if (!this.running) {
              this.setModalOptions();
            }
            return;
          }
          if (item.status === 'FCVmStatusInNormal') {
            resolve(true);
            this.failed++;
            this.running--;
            assign(this.tableData[index], {
              status: DataMap.Batch_Result_Status.fail.value,
              desc: this.i18n.get('protect_fc_vm_innormal_status_label')
            });
            this.cdr.detectChanges();
            if (!this.running) {
              this.setModalOptions();
            }
            return;
          }
          this.doAction(item).subscribe(
            res => {
              resolve(true);
              // 成功的数据
              if (item?.body?.type === DataMap.Resource_Type.vmGroup.value) {
                this.total = this.total + res.length - 1;
                this.running--;
                each(res, item => {
                  if (item?.isSuccess) {
                    this.successful++;
                    this.tableData = this.tableData.concat({
                      name: item.resourceName,
                      status: DataMap.Batch_Result_Status.successful.value,
                      desc: this.desc
                    });
                  } else {
                    this.failed++;
                    this.tableData = this.tableData.concat({
                      name: item.resourceName,
                      status: DataMap.Batch_Result_Status.fail.value,
                      desc: this.i18n.get(item.errorCode)
                    });
                  }
                });
                this.tableData = [...this.tableData];
              } else {
                this.successful++;
                this.running--;
                assign(this.tableData[index], {
                  status: DataMap.Batch_Result_Status.successful.value,
                  desc: item.isAsyn === true ? this.desc : '--'
                });
                if (
                  res &&
                  res.failCount &&
                  res.results[0] &&
                  res.results[0].errorCode
                ) {
                  this.successful--;
                  this.failed++;
                  assign(this.tableData[index], {
                    status: DataMap.Batch_Result_Status.fail.value,
                    desc: this.i18n.get(res.results[0].errorCode)
                  });
                }
              }
              this.cdr.detectChanges();
              if (!this.running) {
                this.setModalOptions();
              }
            },
            ex => {
              resolve(true);
              this.failed++;
              this.running--;
              let response: any = ex.error;
              if (response && isString(response)) {
                try {
                  response = JSON.parse(response);
                } catch (error) {
                  response = {
                    errorCode: 'common_unknown_exception_label',
                    errorMessage: 'common_unknown_exception_label'
                  };
                }
              }
              const errorCode = response.errorCode;
              const errorMessage = response.errorMessage;
              let parameters = response.parameters || [];

              if (isString(response.detailParam)) {
                parameters = [parameters];
              }

              let errorMsg = this.i18n.get(errorCode, parameters);
              if (errorMessage && (errorCode === errorMsg || !errorMsg)) {
                errorMsg = errorMessage;
                if (!!size(parameters)) {
                  errorMsg = this.i18n.get(errorMessage, parameters);
                }
              }
              if (item?.body?.type === DataMap.Resource_Type.vmGroup.value) {
                this.tableData = this.tableData.concat({
                  name: item.name,
                  status: DataMap.Batch_Result_Status.fail.value,
                  desc: errorMsg
                });
              } else {
                assign(this.tableData[index], {
                  status: DataMap.Batch_Result_Status.fail.value,
                  desc: errorMsg
                });
              }

              this.cdr.detectChanges();
              if (!this.running) {
                this.setModalOptions();
              }
            }
          );
        });
      }
    }
  }

  getModalHeader() {
    this.modal.setProperty({ lvHeader: this.headerTpl });
  }

  setModalOptions() {
    this.getVstoreFsIoDetectAllClosed();
    this.modal.setProperty({
      lvCloseButtonDisplay: true,
      lvFooter: [
        {
          id: 'close',
          label: this.i18n.get('common_close_label'),
          onClick: (modal, button) => modal.close()
        }
      ]
    });
  }

  getVstoreFsIoDetectAllClosed() {
    if (this.isCyberEngine && this.needGetDetection) {
      this.detectFilesystemService
        .filterVstoreFsIoDetectAllClosed({
          akOperationTips: false,
          akDoException: false,
          deviceVstoreInfos: uniqWith(
            map(this.sourceData, item => {
              return { deviceId: item.deviceId, vstoreId: item.vstoreId };
            }),
            isEqual
          )
        })
        .subscribe(res => {
          this.showDetectionTip = size(res) > 0;
          if (!this.showDetectionTip) {
            return;
          }
          const tips = [];
          each(res, item => {
            tips.push(
              this.i18n.get('explore_device_vstore_label', [
                item.deviceInfo.deviceName,
                map(item.vstoreInfoList, 'vstoreName').join(',')
              ])
            );
          });
          this.realDetectionTip = this.i18n.get(
            'explore_real_detection_remove_label',
            [tips.join(this.i18n.isEn ? ',' : '、')]
          );
          this.cdr.detectChanges();
        });
    }
  }

  private addExtendInfo(rowData, item) {
    each(this.extendCols, col => {
      set(rowData, col.key, item[col.key]);
    });
  }
}
