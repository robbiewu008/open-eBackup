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
import { Router } from '@angular/router';
import { MessageService, UploadFile } from '@iux/live';
import {
  BaseUtilService,
  CookieService,
  DataMap,
  DataMapService,
  I18NService,
  SystemApiService
} from 'app/shared';
import { VirtualScrollService } from 'app/shared/services/virtual-scroll.service';
import { assign, get, includes, result, set } from 'lodash';
import { Subject } from 'rxjs';
import { ConfigNetworkTableComponent } from '../config-network-table/config-network-table.component';
import { InitConfigProcessComponent } from '../init-config-process/init-config-process.component';

@Component({
  selector: 'aui-distributed-init',
  templateUrl: './distributed-init.component.html',
  styleUrls: ['./distributed-init.component.less']
})
export class DistributedInitComponent implements OnInit {
  selected = 0;
  activeIndex = 0;
  componentData = {};
  isCertify = false;
  formGroup: FormGroup;
  dataMap = DataMap;

  rawData;
  selectFile;
  valid$ = new Subject<boolean>();
  fileReceive = false;
  filters = [];

  cardData = [
    {
      label: this.i18n.get('common_manual_config_label'),
      content: this.i18n.get('common_manual_config_distribute_tip_label'),
      value: 0
    },
    {
      label: this.i18n.get('common_lld_config_label'),
      content: this.i18n.get('common_lld_config_tip_label'),
      value: 1
    }
  ];

  selectedTabIndex = 'backup';
  serviceType = 'backup';
  selectedData: any = {};

  nameErrorTip = assign(this.baseUtilService.nameErrorTip, {
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [32])
  });
  pwdErrorTip = assign(this.baseUtilService.pwdErrorTip, {
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [32])
  });

  @ViewChild(InitConfigProcessComponent, { static: false })
  initConfigProcessComponent: InitConfigProcessComponent;
  @ViewChild(ConfigNetworkTableComponent, { static: false })
  configNetworkTableComponent: ConfigNetworkTableComponent;

  constructor(
    public baseUtilService: BaseUtilService,
    private router: Router,
    private fb: FormBuilder,
    private i18n: I18NService,
    private cookieSerive: CookieService,
    private dataMapService: DataMapService,
    private messageService: MessageService,
    private systemApiService: SystemApiService,
    public virtualScroll: VirtualScrollService
  ) {}

  ngOnInit() {
    this.initFormGroup();
    this.virtualScroll.getScrollParam(475);
  }

  ngAfterViewInit() {
    this.getStatus();
  }

  initFormGroup() {
    this.formGroup = this.fb.group({
      username: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.maxLength(32)
        ]
      }),
      password: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.maxLength(32)
        ]
      })
    });

    this.filters = [
      {
        name: 'suffix',
        filterFn: (files: UploadFile[]) => {
          const supportSuffix = ['xls'];
          let validFiles = files.filter(file => {
            let suffix = file.name.split('.').pop();
            return supportSuffix.includes(suffix);
          });

          if (validFiles.length !== files.length) {
            this.messageService.error(
              this.i18n.get('common_format_error_label', ['xls']),
              {
                lvMessageKey: 'formatErrorKey',
                lvShowCloseButton: true
              }
            );
            this.selectFile = '';
            this.valid$.next(false);
            return;
          }
          if (files[0].size > 1024 * 1024 * 2) {
            this.messageService.error(
              this.i18n.get('common_max_size_file_label', ['2MB']),
              {
                lvMessageKey: 'maxSizeFileErrorKey',
                lvShowCloseButton: true
              }
            );
            this.selectFile = '';
            this.valid$.next(false);
            return;
          }
          this.valid$.next(true);
          return validFiles;
        }
      }
    ];
  }

  upload() {
    this.fileReceive = false;
    this.selectedData = {};
    this.systemApiService
      .queryinitConfigInfoUSingPost({
        lld: this.selectFile
      })
      .subscribe(res => {
        this.rawData = res;
        this.fileReceive = true;
      });
  }

  uploadChange(e) {
    if (e.activeFiles[0].size === 0) {
      this.messageService.error(result(e, 'file.error.message'));
    } else if (e.action === 'select') {
      this.selectFile = e.activeFiles[0].originFile;
    } else if (e.action === 'remove') {
      this.selectFile = '';
    } else {
    }
  }

  getLld() {
    const a = document.createElement('a');
    if (this.i18n.isEn) {
      a.href = 'assets/LLD_Design_E6000_En.xls';
    } else {
      a.href = 'assets/LLD_Design_E6000.xls';
    }
    a.download = 'LLD_Design.xls';
    a.click();
  }

  checkAuth() {
    const ListAllPortUsingPOSTRequestBody = {
      password: this.formGroup.value.password,
      username: this.formGroup.value.username
    };
    this.systemApiService
      .checkAuth({ storageAuth: ListAllPortUsingPOSTRequestBody })
      .subscribe((res: any) => {
        this.isCertify = true;
        this.formGroup.get('username').disable();
        this.formGroup.get('password').disable();
        set(this.componentData, 'storageAuth', ListAllPortUsingPOSTRequestBody);
        this.configNetworkTableComponent?.queryNetworkInfo();
      });
  }

  onResetChange(event) {
    this.systemApiService.getInitConfigUsingGET({}).subscribe(res => {
      if (res.status === DataMap.System_Init_Status.no.value) {
        this.router.navigate(['/home']);
        return;
      }
      this.activeIndex = 0;
    });
  }
  getStatus() {
    if (
      this.cookieSerive.initedStatus ===
      DataMap.System_Init_Status.running.value
    ) {
      this.activeIndex = 3;
      this.initConfigProcessComponent.getStatus();
    } else if (
      this.cookieSerive.initedStatus ===
      DataMap.System_Init_Status.archiveFailed.value
    ) {
      this.activeIndex = 3;
      this.initConfigProcessComponent.status =
        DataMap.System_Init_Status.archiveFailed.value;
      this.initConfigProcessComponent.result = {
        code: DataMap.System_Init_Status.archiveFailed.label,
        params: []
      };
    }
  }

  createInitConfig() {
    this.systemApiService
      .createInitConfigUsingPOST({
        initNetworkBody: {
          storageAuth: get(this.componentData, 'storageAuth'),
          backupNetworkConfig: {
            pacificInitNetWorkInfoList: this.selectedData?.backup
          },
          copyNetworkConfig: {
            pacificInitNetWorkInfoList: this.selectedData?.replication
          },
          archiveNetworkConfig: {
            pacificInitNetWorkInfoList: this.selectedData?.archived
          }
        },
        akOperationTips: false
      })
      .subscribe(res => {
        if (res.status === DataMap.System_Init_Status.ok.value) {
          this.router.navigate(['/home']);
          return;
        }

        if (res.status === DataMap.System_Init_Status.running.value) {
          this.activeIndex = 3;
          this.initConfigProcessComponent.getStatus();
          return;
        }

        if (
          includes(
            [
              DataMap.System_Init_Status.archiveFailed.value,
              DataMap.System_Init_Status.backupFailed.value,
              DataMap.System_Init_Status.authFailed.value,
              DataMap.System_Init_Status.failed.value
            ],
            res.status
          )
        ) {
          this.messageService.error(
            this.i18n.get(
              this.dataMapService.getLabel('System_Init_Status', res.status)
            ),
            {
              lvMessageKey: 'system_Init_Status_key',
              lvShowCloseButton: true
            }
          );
          return;
        }
      });
  }
}
