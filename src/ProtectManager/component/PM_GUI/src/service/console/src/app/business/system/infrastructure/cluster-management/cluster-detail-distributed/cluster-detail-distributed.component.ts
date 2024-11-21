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
import { Component, OnInit, Output, EventEmitter } from '@angular/core';
import { FormBuilder, FormControl, FormGroup } from '@angular/forms';
import {
  CapacityCalculateLabel,
  CommonConsts,
  CAPACITY_UNIT,
  DataMapService,
  I18NService,
  WarningMessageService,
  LocalStorageApiService,
  LANGUAGE,
  BaseUtilService,
  MODAL_COMMON,
  StoragesApiService,
  CookieService,
  DataMap
} from 'app/shared';
import {
  BackupClustersApiService,
  ClustersApiService
} from 'app/shared/api/services';
import { DrawModalService } from 'app/shared/services/draw-modal.service';
import {
  assign,
  each,
  extend,
  isEmpty,
  includes,
  filter,
  toString,
  isFunction
} from 'lodash';

@Component({
  selector: 'aui-cluster-detail-distributed',
  templateUrl: './cluster-detail-distributed.component.html',
  styleUrls: ['./cluster-detail-distributed.component.less'],
  providers: [CapacityCalculateLabel]
})
export class ClusterDetailDistributedComponent implements OnInit {
  pageIndex = CommonConsts.PAGE_START;
  pageSize = CommonConsts.PAGE_SIZE;
  total = CommonConsts.PAGE_TOTAL;
  sizeOptions = CommonConsts.PAGE_SIZE_OPTIONS;
  drawData;
  timeData;
  isUpdate = false;
  clusterType = this.dataMapService.getConfig('Cluster_Type');
  basicInfoLabel = this.i18n.get('common_basic_info_label');
  ipLabel = this.i18n.get('system_management_ip_label');
  storageInfo = {} as any;
  currentCluster = {} as any;

  unitconst = CAPACITY_UNIT;
  progressBarColor = [[0, '#6C92FA']];
  usedSizeColor = '#b8becc';

  isView = true;
  isAuth = true;
  formGroup: FormGroup;
  authForm: FormGroup;
  disabled = false;
  serviceAble = false;
  managerAble = false;
  isEn = this.i18n.isEn;

  nameErrorTip = {
    ...this.baseUtilService.requiredErrorTip,
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [32])
  };
  pwdErrorTip = {
    ...this.baseUtilService.requiredErrorTip,
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [32])
  };

  @Output() openDeviceChange = new EventEmitter<any>();

  constructor(
    public i18n: I18NService,
    public drawmodalservice: DrawModalService,
    public warningMessageService: WarningMessageService,
    public dataMapService: DataMapService,
    private fb: FormBuilder,
    private localStorageApiService: LocalStorageApiService,
    private cookieService: CookieService,
    public clusterApiService: ClustersApiService,
    public baseUtilService: BaseUtilService,
    public backupClustersApiService: BackupClustersApiService,
    private capacityCalculateLabel: CapacityCalculateLabel
  ) {}

  ngOnInit() {
    this.getCurrentCluster();
    this.initForm();
    this.getData();
  }

  initForm() {
    this.formGroup = this.fb.group({
      status: new FormControl(),
      userName: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.maxLength(32)
        ],
        updateOn: 'change'
      }),
      password: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.maxLength(32)
        ],
        updateOn: 'change'
      })
    });
    this.authForm = this.fb.group({
      status: new FormControl(),
      userName: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.maxLength(32)
        ],
        updateOn: 'change'
      }),
      password: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.maxLength(32)
        ],
        updateOn: 'change'
      })
    });
    this.timeData = {
      memberEsn: this.drawData.esn
    };
  }

  getCurrentCluster() {
    this.currentCluster = JSON.parse(
      decodeURIComponent(this.cookieService.get('currentCluster'))
    ) || {
      clusterId: DataMap.Cluster_Type.local.value,
      clusterName: this.i18n.get('common_all_clusters_label'),
      clusterType: DataMap.Cluster_Type.local.value
    };
  }

  getData(callback?: () => void) {
    this.backupClustersApiService
      .getBackupClusterLocalDetail({
        memberEsn: this.drawData.esn
      })
      .subscribe(
        res => {
          this.storageInfo = {
            ...res
          };

          assign(this.storageInfo, {
            sizePercent: this.getSizePercent(this.storageInfo),
            alarmThresholdPer: `${(
              this.storageInfo.alarmThreshold * 100
            ).toFixed()}%`
          });

          if (
            this.storageInfo.sizePercent / 100 >=
            this.storageInfo.alarmThreashold
          ) {
            this.progressBarColor = [[0, '#FA8E5A']];
            this.usedSizeColor = '#FA8E5A';
          } else {
            this.progressBarColor = [[0, '#6C92FA']];
            this.usedSizeColor = '#6C92FA';
          }

          this.managerAble = res.managerAuth.modifiable === '1';
          this.formGroup.patchValue({ ...res.managerAuth, password: '' });
          this.formGroup.get('status').value.toUpperCase();
          this.serviceAble = res.serviceAuth.modifiable === '1';
          this.authForm.patchValue({ ...res.serviceAuth, password: '' });
          this.authForm.get('status').value.toUpperCase();
          if (
            this.authForm.value.status === 'NORMAL' &&
            this.authForm.value.userName === 'dataprotect_admin' &&
            !this.serviceAble
          ) {
            this.isUpdate = true;
          } else {
            this.isUpdate = false;
          }
          isFunction(callback) && callback();
        },
        () => {
          isFunction(callback) && callback();
        }
      );
  }

  modify() {
    this.getData(() => {
      this.isView = !this.isView;
    });
  }

  cancel() {
    this.getData(() => {
      this.isView = true;
      this.isAuth = true;
    });
  }

  authService() {
    this.getData(() => {
      this.isAuth = !this.isAuth;
    });
  }

  save() {
    let params = {
      password: '',
      userName: '',
      authType: ''
    };
    if (this.formGroup.valid) {
      params = {
        password: this.formGroup.value.password,
        userName: this.formGroup.value.userName,
        authType: 'managerAuth'
      };
    }

    if (this.authForm.valid) {
      params = {
        password: this.authForm.value.password,
        userName: this.authForm.value.userName,
        authType: 'serviceAuth'
      };
    }

    this.localStorageApiService
      .updateLocalStorageAuthUsingPUT({
        request: params,
        memberEsn: this.drawData.esn
      })
      .subscribe(() => this.cancel());
  }

  updateStoragePwd() {
    this.warningMessageService.create({
      content: this.i18n.get('system_password_update_warning_label'),
      onOK: () => {
        this.localStorageApiService
          .updateLocalStorageUserUsingPUT({
            memberEsn: this.drawData.esn
          })
          .subscribe(res => {});
      }
    });
  }

  openDeviceManage() {
    this.localStorageApiService
      .getStorageTokenUsingGET({
        memberEsn: this.drawData.esn
      })
      .subscribe(
        res => {
          this.openDeviceChange.emit();
          const language = this.isEn ? 'en' : 'zh';
          const url = `https://${encodeURI(res.ip)}:${encodeURI(
            toString(res.port)
          )}/#/initialize?ticket=${encodeURIComponent(
            res.token
          )}&language=${encodeURIComponent(language)}`;
          window.open(url, '_blank');
        },
        err => {
          this.openDeviceChange.emit();
        }
      );
  }

  getSizePercent(source): string {
    const sizePercent = parseFloat(
      (source.usedCapacity / source.totalCapacity) * 100 + ''
    );
    return this.capacityCalculateLabel.formatDecimalPoint(sizePercent, 3);
  }
}
