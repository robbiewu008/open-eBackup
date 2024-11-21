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
  Component,
  EventEmitter,
  Input,
  OnDestroy,
  OnInit,
  Output,
  TemplateRef,
  ViewChild
} from '@angular/core';
import { FormBuilder } from '@angular/forms';
import { Router } from '@angular/router';
import { MessageboxService, MessageService } from '@iux/live';
import { DebugLogComponent } from 'app/business/system/log-management/debug-log/debug-log.component';
import {
  ApiExportFilesApiService as ExportFileApiService,
  BaseUtilService,
  CommonConsts,
  DataMap,
  DataMapService,
  I18NService,
  LogManagerApiService,
  MODAL_COMMON,
  SystemApiService,
  getAppTheme,
  ThemeEnum
} from 'app/shared';
import { ExportFilesService } from 'app/shared/components/export-files/export-files.component';
import { AppUtilsService } from 'app/shared/services/app-utils.service';
import { DrawModalService } from 'app/shared/services/draw-modal.service';
import { InfoMessageService } from 'app/shared/services/info-message.service';
import { map, now } from 'lodash';
import { Subject, Subscription, timer } from 'rxjs';
import { switchMap, takeUntil } from 'rxjs/operators';
import { DownloadLogsComponent } from './download-logs/download-logs.component';

@Component({
  selector: 'aui-init-config-process',
  templateUrl: './init-config-process.component.html',
  styleUrls: ['./init-config-process.component.less']
})
export class InitConfigProcessComponent implements OnInit, OnDestroy {
  timeSub$: Subscription;
  destroy$ = new Subject();
  result = {};
  dataMap = DataMap;
  status = DataMap.System_Init_Status.running.value;
  isDistributed =
    this.i18n.get('deploy_type') === DataMap.Deploy_Type.e6000.value;
  isDecouple = this.appUtilsService.isDecouple;
  init_running_label = 'common_init_running_label';

  @Input() componentData;
  @Input() isModify;
  @Input() memberEsn;
  @Output() onResetChange = new EventEmitter<any>();

  debugLogComponent: DebugLogComponent;

  fileDownloadCompletedLabel = this.i18n.get(
    'common_file_download_completed_label'
  );
  @ViewChild('fileDownloadCompletedTpl', { static: true })
  fileDownloadCompletedTpl: TemplateRef<any>;

  constructor(
    private fb: FormBuilder,
    private i18n: I18NService,
    private router: Router,
    private dataMapService: DataMapService,
    private messageService: MessageService,
    public appUtilsService: AppUtilsService,
    private baseUtilService: BaseUtilService,
    private systemApiService: SystemApiService,
    private infoMessageService: InfoMessageService,
    private logManagerApiService: LogManagerApiService,
    private exportFilesService: ExportFilesService,
    private exportFilesApi: ExportFileApiService,
    private drawModalService: DrawModalService,
    private messageBox: MessageboxService
  ) {}

  ngOnDestroy() {
    this.destroy$.next(true);
    this.destroy$.complete();
  }

  ngOnInit() {
    this.debugLogComponent = new DebugLogComponent(
      this.fb,
      this.i18n,
      this.dataMapService,
      this.baseUtilService,
      this.infoMessageService,
      this.logManagerApiService,
      this.exportFilesService
    );
    this.debugLogComponent.fileDownloadCompletedTpl = this.fileDownloadCompletedTpl;
    this.init_running_label =
      this.isDistributed || this.isDecouple
        ? 'common_distributed_init_running_label'
        : this.isModify
        ? 'system_modify_network_tip_label'
        : 'common_init_running_label';
  }

  getStatus() {
    if (this.timeSub$) {
      this.timeSub$.unsubscribe();
    }
    this.timeSub$ = timer(0, CommonConsts.TIME_INTERVAL)
      .pipe(
        switchMap(index => {
          return this.systemApiService.getInitStatusInfoUsingGET({
            akLoading: !index,
            memberEsn: this.memberEsn || '',
            akDoException: !this.isModify
          });
        }),
        takeUntil(this.destroy$)
      )
      .subscribe(
        res => {
          if (res.status === DataMap.System_Init_Status.ok.value) {
            this.router.navigate(['/home']);
            return;
          }
          if (res.status === DataMap.networkModifyStatus.finish.value) {
            this.timeSub$.unsubscribe();
            this.onResetChange.emit('modifyFinish');
          }
          if (res.status === DataMap.networkModifyStatus.fail.value) {
            this.appUtilsService.setCacheValue(
              'networkModify',
              DataMap.networkModifyingStatus.modify.value
            );
          }
          if (
            [
              DataMap.System_Init_Status.archiveFailed.value,
              DataMap.System_Init_Status.backupFailed.value,
              DataMap.System_Init_Status.authFailed.value,
              DataMap.System_Init_Status.failed.value,
              DataMap.networkModifyStatus.fail.value
            ].includes(res.status)
          ) {
            this.timeSub$.unsubscribe();
          }
          this.result = res;
          this.status = res.status;
        },
        () => {
          if (!this.isModify) {
            // 初始化的时候获取失败视为失败
            this.status = DataMap.System_Init_Status.failed.value;
          } else {
            setTimeout(() => this.getStatus(), CommonConsts.TIME_INTERVAL);
          }
        }
      );
  }

  exportLog() {
    const name = `${now()}`;

    this.logManagerApiService.collectNodeInfo({}).subscribe(res => {
      const request = {
        name,
        type: DataMap.Export_Query_Type.log.value,
        params: {
          nodeName: map(res.data, 'nodeName'),
          componentName: map(
            this.dataMapService
              .toArray('Component_Name')
              .filter(item => item.value !== DataMap.Component_Name.dee.value),
            'value'
          )
        }
      };
      this.exportFilesApi
        .CreateExportFile({
          request,
          akOperationTips: false
        })
        .subscribe(res => {
          this.drawModalService.create({
            lvType: 'modal',
            lvHeader: this.i18n.get('common_export_log_label'),
            lvWidth: MODAL_COMMON.normalWidth,
            lvContent: DownloadLogsComponent,
            lvComponentParams: {
              name: name
            },
            lvCloseButtonDisplay: false,
            lvFooter: [
              {
                id: 'close',
                label: this.i18n.get('common_close_label'),
                disabled: true,
                onClick: (modal, button) => modal.close()
              }
            ]
          });
        });
    });
  }

  reset() {
    this.onResetChange.emit();
  }

  modifyReset() {
    this.onResetChange.emit('modifyFail');
  }

  isLight() {
    return getAppTheme(this.i18n) === ThemeEnum.light;
  }

  getInitLoadingImg(): string {
    return this.isLight()
      ? 'assets/img/init_loading.gif'
      : 'assets/img/init_loading_dark.gif';
  }

  getInitFailedImg(): string {
    return this.isLight()
      ? 'assets/img/init_failed.png'
      : 'assets/img/init_failed_dark.png';
  }
}
