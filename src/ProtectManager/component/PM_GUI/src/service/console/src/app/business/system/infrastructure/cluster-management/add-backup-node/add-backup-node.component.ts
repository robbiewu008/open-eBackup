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
import { Component, Input, OnInit } from '@angular/core';
import {
  AbstractControl,
  FormBuilder,
  FormControl,
  FormGroup,
  ValidatorFn
} from '@angular/forms';
import {
  BackupClustersNetplaneService,
  BaseUtilService,
  ClustersApiService,
  CommonConsts,
  DataMap,
  DataMapService,
  I18NService
} from 'app/shared';
import { assign, cloneDeep, isEmpty, isUndefined } from 'lodash';
import { Observable, Observer } from 'rxjs';

@Component({
  selector: 'aui-add-backup-node',
  templateUrl: './add-backup-node.component.html',
  styleUrls: ['./add-backup-node.component.less']
})
export class AddBackupNodeComponent implements OnInit {
  formGroup: FormGroup;
  @Input() unitData;

  drawData: any;
  modifyMemberNode;
  modifyNetworkPlane;
  changedName = true;
  backRole = false;
  targetClusterRole = DataMap.Target_Cluster_Role;
  requiredLabel = this.i18n.get('common_required_label');
  nameLabel = this.i18n.get('common_name_label');
  ipLabel = this.i18n.get('common_ip_address_label');
  portLabel = this.i18n.get('common_port_label');
  userNameLabel = this.i18n.get('common_username_label');
  passwordLabel = this.i18n.get('common_password_label');
  requiredErrorTip = {
    required: this.requiredLabel
  };
  clusterNameErrorTip = assign({}, this.baseUtilService.requiredErrorTip, {
    invalidName: this.i18n.get('common_backup_cluster_name_invalid_label')
  });

  portErrorTip = assign({}, this.baseUtilService.requiredErrorTip, {
    invalidInteger: this.i18n.get('common_valid_integer_label'),
    invalidRang: this.i18n.get('common_valid_rang_label', [1, 65535]),
    invalidName: this.i18n.get('system_port_error_info_label')
  });

  passwordErrorTip = assign({}, this.baseUtilService.requiredErrorTip, {
    invalidNameLength: this.i18n.get('common_valid_length_rang_label', [8, 64])
  });

  usernameErrorTip = assign({}, this.baseUtilService.requiredErrorTip, {
    invalidNameLength: this.i18n.get('common_valid_length_rang_label', [5, 64])
  });

  nameErrorTip = {
    ...this.baseUtilService.requiredErrorTip,
    invalidName: this.i18n.get('system_netplane_name_error_info_label')
  };

  constructor(
    public i18n: I18NService,
    public fb: FormBuilder,
    public clusterApiService: ClustersApiService,
    public backupClusterNetplaneService: BackupClustersNetplaneService,
    public baseUtilService: BaseUtilService,
    public dataMapService: DataMapService
  ) {}

  ngOnInit() {
    this.initForm();
  }

  validLength(min, max): ValidatorFn {
    return (control: AbstractControl): { [key: string]: any } | null => {
      if (isUndefined(this.formGroup) || isEmpty(control.value)) {
        return null;
      }
      const value = control.value;
      if (!new RegExp(`^.{${min},${max}}$`).test(value)) {
        return { invalidNameLength: { value: control.value } };
      }
      return null;
    };
  }

  initForm() {
    let ipValue = '';
    if (!isEmpty(this.drawData)) {
      ipValue = this.drawData.ip;
    }
    if (!isEmpty(this.unitData)) {
      ipValue = this.unitData.ip;
    }
    this.formGroup = this.fb.group({
      clusterName: new FormControl(
        !isEmpty(this.drawData) ? this.drawData.clusterName : '',
        {
          validators: [
            this.baseUtilService.VALID.required(),
            this.baseUtilService.VALID.name(CommonConsts.REGEX.backupName)
          ],
          updateOn: 'change'
        }
      ),
      ip: new FormControl(
        { value: ipValue, disabled: !isEmpty(this.unitData) ? true : false },
        {
          validators: [
            this.baseUtilService.VALID.required(),
            this.baseUtilService.VALID.ip()
          ],
          updateOn: 'change'
        }
      ),
      port: new FormControl(
        !isEmpty(this.drawData) ? this.drawData.port || 25081 : 25081,
        {
          validators: [
            this.baseUtilService.VALID.required(),
            this.baseUtilService.VALID.integer(),
            this.baseUtilService.VALID.rangeValue(1, 65535),
            this.validPort()
          ],
          updateOn: 'change'
        }
      ),
      username: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.required(),
          this.validLength(5, 64)
        ],
        updateOn: 'change'
      }),
      password: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.required(),
          this.validLength(8, 64)
        ],
        updateOn: 'change'
      })
    });
    if (this.modifyMemberNode) {
      this.formGroup.get('username').clearValidators();
      this.formGroup.get('password').clearValidators();
      if (this.drawData) {
        this.formGroup.get('ip').setValue(this.drawData.ip);
        this.formGroup.get('port').setValue(this.drawData.port);
        this.formGroup.get('username').setValue(this.drawData.username);
        this.formGroup.get('password').setValue(this.drawData.password);
        this.formGroup.get('clusterName').setValue(this.drawData.clusterName);
      }
    }
  }

  validPort(): ValidatorFn {
    return (control: AbstractControl): { [key: string]: any } | null => {
      if (!control.value) {
        return { required: { value: control.value } };
      }

      const value = control.value;
      if (value === '25080') {
        return { invalidName: { value: control.value } };
      }
    };
  }

  validName(): ValidatorFn {
    return (control: AbstractControl): { [key: string]: any } | null => {
      if (!control.value) {
        return { required: { value: control.value } };
      }

      const value = control.value;
      const reg = /^[\u4e00-\u9fa5_a-zA-Z0-9_\.-]{1}[\u4e00-\u9fa5_a-zA-Z0-9_\.-]{0,254}$/;
      if (!reg.test(value)) {
        return { invalidName: { value: control.value } };
      }
    };
  }

  registerCluster(): Observable<void> {
    return new Observable<void>((observer: Observer<void>) => {
      if (this.formGroup.invalid) {
        return;
      }
      const params = cloneDeep(this.formGroup.value);
      params.role = 5;

      this.clusterApiService
        .createTargetClusterUsingPOST({
          request: params
        })
        .subscribe({
          next: () => {
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

  modifyCluster(): Observable<void> {
    return new Observable<void>((observer: Observer<void>) => {
      if (this.formGroup.invalid) {
        return;
      }

      const params = cloneDeep(this.formGroup.value);
      params.role = this.drawData.role;

      this.clusterApiService
        .modifyTargetClusterUsingPUT({
          clusterId: this.drawData.clusterId,
          request: params
        })
        .subscribe({
          next: () => {
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

  upgrateBackupStorageUnit(): Observable<void> {
    return new Observable<void>((observer: Observer<void>) => {
      if (this.formGroup.invalid) {
        return;
      }
      const params = cloneDeep(this.formGroup.value);
      assign(params, {
        role: 5,
        ip: this.unitData.ip?.split(',')[0],
        backupUnitId: this.unitData.clusterId
      });
      this.clusterApiService
        .upgradeBackupUnitToMemberClusterUsingPOST({ request: params })
        .subscribe({
          next: () => {
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
}
