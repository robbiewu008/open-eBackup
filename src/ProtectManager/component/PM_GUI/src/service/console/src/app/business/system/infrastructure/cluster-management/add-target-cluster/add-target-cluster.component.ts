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
import { Component, OnInit } from '@angular/core';
import {
  AbstractControl,
  FormBuilder,
  FormControl,
  FormGroup,
  ValidatorFn
} from '@angular/forms';
import {
  BaseUtilService,
  ClustersApiService,
  CommonConsts,
  CookieService,
  DataMap,
  DataMapService,
  I18NService
} from 'app/shared';
import { assign, isEmpty, isUndefined } from 'lodash';
import { Observable, Observer } from 'rxjs';

@Component({
  selector: 'aui-add-target-cluster',
  templateUrl: './add-target-cluster.component.html'
})
export class AddTargetClusterComponent implements OnInit {
  formGroup: FormGroup;
  roleOptions = this.dataMapService
    .toArray('Target_Cluster_Role')
    .map(item => {
      item.isLeaf = true;
      return item;
    })
    .filter(item => {
      return item.value !== DataMap.Target_Cluster_Role.primaryNode.value;
    });
  drawData: any;
  changedName = true;
  backRole = false;
  targetClusterRole = DataMap.Target_Cluster_Role;
  isHcsEnvir =
    this.cookieService.get('serviceProduct') === CommonConsts.serviceProduct;
  addType;
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
    invalidName: this.i18n.get('common_target_cluster_name_invalid_label')
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

  ipTipLabel = this.i18n.get('common_cluster_ip_tip_label');
  domainNameErrorTip = {
    ...this.baseUtilService.requiredErrorTip,
    invalidName: this.i18n.get('common_invalid_input_label'),
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [255])
  };

  constructor(
    public i18n: I18NService,
    public fb: FormBuilder,
    public cookieService: CookieService,
    public clusterApiService: ClustersApiService,
    public baseUtilService: BaseUtilService,
    public dataMapService: DataMapService
  ) {}

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

  initPort() {
    if (!isEmpty(this.drawData)) {
      return this.drawData.port || 25081;
    } else {
      if (this.addType === DataMap.Target_Cluster_Role.managed.value) {
        return 25081;
      } else {
        return '';
      }
    }
  }

  initForm() {
    this.formGroup = this.fb.group({
      clusterName: new FormControl(
        !isEmpty(this.drawData) ? this.drawData.clusterName : '',
        {
          validators: [
            this.baseUtilService.VALID.required(),
            this.baseUtilService.VALID.name(
              CommonConsts.REGEX.targetClusterName
            )
          ],
          updateOn: 'change'
        }
      ),
      ip: new FormControl(
        !isEmpty(this.drawData) ? this.drawData.ip.split(',')[0] : '',
        {
          validators: [
            this.baseUtilService.VALID.required(),
            this.baseUtilService.VALID.ip()
          ],
          updateOn: 'change'
        }
      ),
      port: new FormControl(this.initPort(), {
        validators: [
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.integer(),
          this.baseUtilService.VALID.rangeValue(1, 65535),
          this.validPort()
        ],
        updateOn: 'change'
      }),
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
      }),
      role: new FormControl(
        this.addType === DataMap.Target_Cluster_Role.managed.value
          ? DataMap.Target_Cluster_Role.managed.value
          : DataMap.Target_Cluster_Role.replication.value,
        {
          validators: [this.baseUtilService.VALID.required()],
          updateOn: 'change'
        }
      ),
      replicationClusterType: new FormControl('', {
        validators: [this.baseUtilService.VALID.required()]
      }),
      domain: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.maxLength(255),
          this.baseUtilService.VALID.name(CommonConsts.REGEX.nasshareDomain)
        ]
      })
    });

    if (!this.isHcsEnvir) {
      // 只在op服务化使用
      this.formGroup.get('replicationClusterType').clearValidators();
      this.formGroup.get('replicationClusterType').updateValueAndValidity();
      this.formGroup.get('domain').clearValidators();
      this.formGroup.get('domain').updateValueAndValidity();
    } else {
      this.formGroup
        .get('replicationClusterType')
        .valueChanges.subscribe(res => {
          if (res === 1) {
            // hcs集群默认端口443
            this.formGroup.get('port').setValue('443');
            this.formGroup
              .get('domain')
              .setValidators([
                this.baseUtilService.VALID.required(),
                this.baseUtilService.VALID.maxLength(255),
                this.baseUtilService.VALID.name(
                  CommonConsts.REGEX.nasshareDomain
                )
              ]);
          } else {
            this.formGroup.get('domain').clearValidators();
          }
          this.formGroup.get('domain').updateValueAndValidity();
        });
    }

    !isEmpty(this.drawData) &&
      this.formGroup.get('username').valueChanges.subscribe(res => {
        if (!!this.drawData && res === this.drawData.username) {
          this.formGroup
            .get('password')
            .setValidators([this.validLength(8, 64)]);
          this.changedName = false;
        } else {
          this.formGroup
            .get('password')
            .setValidators([
              this.baseUtilService.VALID.required(),
              this.validLength(8, 64)
            ]);
          this.changedName = true;
        }
        this.formGroup.get('password').updateValueAndValidity();
      });

    if (!!this.drawData) {
      this.formGroup.get('username').setValue(this.drawData.username);
    }

    // 多域集群的创建：角色只能为被管理集群，修改时类型为管理集群或被管理集群；
    // 复制集群的创建和修改：角色均为复制目标集群
    if (
      this.addType === DataMap.Target_Cluster_Role.managed.value &&
      (!this.drawData || !this.drawData?.enableManage)
    ) {
      this.roleOptions = this.roleOptions.filter(item => {
        return item.value === DataMap.Target_Cluster_Role.managed.value;
      });
    } else if (
      this.addType === DataMap.Target_Cluster_Role.managed.value &&
      this.drawData?.enableManage
    ) {
      this.roleOptions = [
        {
          key: 'management',
          value: 2,
          label: this.i18n.get('common_management_label'),
          isLeaf: true
        }
      ];
    } else {
      this.roleOptions = this.roleOptions.filter(item => {
        return item.value === DataMap.Target_Cluster_Role.replication.value;
      });
    }

    if (!!this.drawData) {
      this.formGroup.get('role').setValue(this.drawData?.role);
    }

    if (!!this.drawData && this.isHcsEnvir) {
      this.formGroup
        .get('replicationClusterType')
        .setValue(this.drawData?.replicationClusterType);
      this.formGroup.get('domain').setValue(this.drawData?.domain);
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

  getUserNamePlaceHolder() {
    if (
      this.isHcsEnvir &&
      this.formGroup.get('replicationClusterType')?.value === 1
    ) {
      return this.i18n.get('system_add_hcs_cluster_username_placeholder_label');
    }
    if (
      this.formGroup.value.role === this.targetClusterRole.replication.value
    ) {
      return this.i18n.get('system_add_replication_placeholder_label');
    } else if (
      this.formGroup.value.role === this.targetClusterRole.managed.value
    ) {
      return this.i18n.get('system_add_managed_placeholder_label');
    } else if (
      this.formGroup.value.role === this.targetClusterRole.backupStorage.value
    ) {
      return this.i18n.get('system_add_backup_placeholder_label');
    } else {
      return '';
    }
  }

  registerCluster(): Observable<void> {
    return new Observable<void>((observer: Observer<void>) => {
      if (this.formGroup.invalid) {
        return;
      }

      const params: any = this.formGroup.getRawValue();
      if (!this.isHcsEnvir) {
        delete params.replicationClusterType;
        delete params.domain;
      } else {
        params.replicationClusterType = +params.replicationClusterType;
      }

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

      const params = { ...this.formGroup.value };

      if (!this.isHcsEnvir) {
        delete params.replicationClusterType;
        delete params.domain;
      } else {
        params.replicationClusterType = +params.replicationClusterType;
      }

      if (this.backRole) {
        params.role = 0;
      }
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

  ngOnInit() {
    this.isHcsEnvir = this.isHcsEnvir && this.addType === 0;
    this.initForm();
  }
}
