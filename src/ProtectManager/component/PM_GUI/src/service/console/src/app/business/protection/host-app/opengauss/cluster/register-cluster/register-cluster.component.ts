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
import { ChangeDetectorRef, Component, OnInit } from '@angular/core';
import {
  AbstractControl,
  FormBuilder,
  FormControl,
  FormGroup,
  ValidatorFn
} from '@angular/forms';
import {
  BaseUtilService,
  CommonConsts,
  DataMap,
  DataMapService,
  getMultiHostOps,
  I18NService,
  MultiCluster,
  ProtectedEnvironmentApiService,
  ProtectedResourceApiService,
  ResourceType
} from 'app/shared';
import { cacheGuideResource } from 'app/shared/consts/guide-config';
import { AppUtilsService } from 'app/shared/services/app-utils.service';
import { DrawModalService } from 'app/shared/services/draw-modal.service';
import {
  assign,
  defer,
  each,
  filter,
  includes,
  isEmpty,
  map,
  trim
} from 'lodash';
import { Observable, Observer } from 'rxjs';

@Component({
  selector: 'aui-register-cluster',
  templateUrl: './register-cluster.component.html',
  styleUrls: ['./register-cluster.component.less']
})
export class RegisterClusterComponent implements OnInit {
  data;
  formGroup: FormGroup;
  dataMap = DataMap;
  deployTypeOptions = this.dataMapService
    .toArray('Deployment_Type')
    .map(item => {
      item.isLeaf = true;
      return item;
    });
  proxyOptions = [];
  proxyMultiple = 'multiple';

  nameErrorTip = {
    ...this.baseUtilService.nameErrorTip,
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [64])
  };
  usernameErrorTip = {
    ...this.baseUtilService.requiredErrorTip,
    ...this.baseUtilService.lengthErrorTip,
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [32])
  };
  pathErrorTip = {
    ...this.baseUtilService.requiredErrorTip,
    pathError: this.i18n.get('common_path_error_label'),
    samePathError: this.i18n.get('protection_same_path_error_label'),
    unsupportPathError: this.i18n.get(
      'protection_unsupport_fileset_linux_path_label'
    )
  };
  portErrorTip = assign({}, this.baseUtilService.rangeErrorTip, {
    invalidRang: this.i18n.get('common_valid_rang_label', [1, 65535])
  });
  dcsUserErrorTip = {
    ...this.baseUtilService.requiredErrorTip,
    ...this.baseUtilService.lengthErrorTip,
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [64])
  };

  constructor(
    private appUtilsService: AppUtilsService,
    private fb: FormBuilder,
    private i18n: I18NService,
    private cdr: ChangeDetectorRef,
    private dataMapService: DataMapService,
    public baseUtilService: BaseUtilService,
    private drawModalService: DrawModalService,
    private protectedEnvironmentApiService: ProtectedEnvironmentApiService,
    private protectedResourceApiService: ProtectedResourceApiService
  ) {}

  ngOnInit(): void {
    this.initForm();
    this.updateData();
    this.getProxyOptions();
  }

  initForm(): void {
    this.formGroup = this.fb.group({
      name: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.name(),
          this.baseUtilService.VALID.maxLength(64)
        ]
      }),
      userName: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.maxLength(32)
        ],
        updateOn: 'change'
      }),
      deployType: new FormControl('', {
        validators: [this.baseUtilService.VALID.required()]
      }),
      envPath: new FormControl('', {
        validators: [this.validPath(), this.validLinuxPath()]
      }),
      agents: new FormControl([], {
        validators: [
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.minLength(1)
        ]
      }),
      dcsAddress: new FormControl(''),
      dcsPort: new FormControl(''),
      dcsUser: new FormControl(''),
      dcsPassword: new FormControl('')
    });

    if (!this.data) {
      this.listenForm();
    }
  }

  listenForm() {
    this.formGroup.get('deployType').valueChanges.subscribe(res => {
      if (!res) {
        return;
      }

      this.proxyMultiple =
        res === DataMap.Deployment_Type.single.value ? 'single' : 'multiple';
      this.formGroup.get('agents').setValue([]);

      if (res === DataMap.Deployment_Type.cmdb.value) {
        this.formGroup
          .get('dcsAddress')
          .setValidators([
            this.baseUtilService.VALID.required(),
            this.baseUtilService.VALID.ipv4()
          ]);
        this.formGroup
          .get('dcsPort')
          .setValidators([
            this.baseUtilService.VALID.required(),
            this.baseUtilService.VALID.integer(),
            this.baseUtilService.VALID.rangeValue(1, 65535)
          ]);
        this.formGroup
          .get('dcsUser')
          .setValidators([
            this.baseUtilService.VALID.required(),
            this.baseUtilService.VALID.maxLength(64)
          ]);
        this.formGroup
          .get('dcsPassword')
          .setValidators([
            this.baseUtilService.VALID.required(),
            this.baseUtilService.VALID.maxLength(64)
          ]);
      } else {
        this.formGroup.get('dcsAddress').clearValidators();
        this.formGroup.get('dcsPort').clearValidators();
        this.formGroup.get('dcsUser').clearValidators();
        this.formGroup.get('dcsPassword').clearValidators();
      }
      this.formGroup.get('dcsAddress').updateValueAndValidity();
      this.formGroup.get('dcsPort').updateValueAndValidity();
      this.formGroup.get('dcsUser').updateValueAndValidity();
      this.formGroup.get('dcsPassword').updateValueAndValidity();
    });
  }

  updateData() {
    if (!this.data) {
      return;
    }
    this.protectedResourceApiService
      .ShowResource({
        resourceId: this.data.uuid
      })
      .subscribe((res: any) => {
        const data = {
          name: res.name,
          userName: res.auth.authKey,
          deployType: res.extendInfo.deployType,
          envPath: res.extendInfo.envPath,
          agents:
            res.extendInfo.deployType === DataMap.Deployment_Type.single.value
              ? map(res['dependencies']['agents'], 'uuid')[0]
              : map(res['dependencies']['agents'], 'uuid')
        };
        if (res.extendInfo.deployType === DataMap.Deployment_Type.cmdb.value) {
          assign(data, {
            dcsAddress: res.extendInfo.dcsAddress,
            dcsPort: res.extendInfo.dcsPort,
            dcsUser: res.extendInfo.dcsUser
          });
        }
        this.proxyMultiple =
          res.extendInfo.deployType === DataMap.Deployment_Type.single.value
            ? 'single'
            : 'multiple';
        // 需要等select组件切换之后再塞值
        defer(() => {
          this.listenForm();
          this.formGroup.patchValue(data);
        });
      });
  }
  getParams() {
    const params = {
      name: this.formGroup.value.name,
      extendInfo: {
        deployType: this.formGroup.value.deployType,
        envPath: this.formGroup.value.envPath
      },
      type: ResourceType.DATABASE,
      subType: DataMap.Resource_Type.OpenGauss.value,
      auth: {
        authType: DataMap.Database_Auth_Method.db.value,
        authKey: this.formGroup.value.userName,
        authPwd: this.formGroup.value.password
      },
      dependencies: {
        agents: map(
          this.formGroup.get('deployType').value ===
            DataMap.Deployment_Type.single.value
            ? [this.formGroup.value.agents]
            : this.formGroup.value.agents,
          item => {
            return { uuid: item };
          }
        )
      }
    };
    if (
      this.formGroup.value.deployType === DataMap.Deployment_Type.cmdb.value
    ) {
      assign(params.auth, {
        extendInfo: {
          dcsPassword: this.formGroup.get('dcsPassword').value
        }
      });
      assign(params.extendInfo, {
        dcsAddress: this.formGroup.get('dcsAddress').value,
        dcsPort: this.formGroup.get('dcsPort').value,
        dcsUser: this.formGroup.get('dcsUser').value
      });
    }
    return params;
  }

  validPath(): ValidatorFn {
    return (control: AbstractControl): { [key: string]: any } | null => {
      if (!trim(control.value)) {
        return null;
      }
      const paths = control.value;

      if (!CommonConsts.REGEX.unixPath.test(paths) || paths.length > 1024) {
        return { pathError: { value: control.value } };
      }
      return null;
    };
  }
  validLinuxPath(): ValidatorFn {
    return (control: AbstractControl): { [key: string]: any } | null => {
      if (!trim(control.value)) {
        return null;
      }
      const paths = control.value;
      if (includes(['proc', 'dev', 'run'], paths.split('/')[1])) {
        return { unsupportPathError: { value: control.value } };
      }
      return null;
    };
  }

  onOK(): Observable<void> {
    return new Observable<void>((observer: Observer<void>) => {
      if (this.formGroup.invalid) {
        return;
      }
      const params = this.getParams();
      if (this.data) {
        this.protectedEnvironmentApiService
          .UpdateProtectedEnvironment({
            UpdateProtectedEnvironmentRequestBody: params as any,
            envId: this.data.uuid
          })
          .subscribe({
            next: res => {
              observer.next();
              observer.complete();
            },
            error: err => {
              observer.error(err);
              observer.complete();
            }
          });
      } else {
        this.protectedEnvironmentApiService
          .RegisterProtectedEnviroment({
            RegisterProtectedEnviromentRequestBody: params as any
          })
          .subscribe({
            next: res => {
              cacheGuideResource(res);
              observer.next();
              observer.complete();
            },
            error: err => {
              observer.error(err);
              observer.complete();
            }
          });
      }
    });
  }
  getProxyOptions() {
    const extParams = {
      conditions: JSON.stringify({
        type: 'Plugin',
        subType: [`${DataMap.Resource_Type.OpenGauss_instance.value}Plugin`]
      })
    };
    this.appUtilsService.getResourceByRecursion(
      extParams,
      params => this.protectedResourceApiService.ListResources(params),
      resource => {
        resource = filter(resource, item => !isEmpty(item.environment));
        const hostArray = [];
        resource = filter(
          resource,
          item =>
            item.environment.extendInfo.scenario ===
            DataMap.proxyHostType.external.value
        );
        if (MultiCluster.isMulti && isEmpty(this.data)) {
          resource = getMultiHostOps(resource);
        }
        each(resource, item => {
          const tmp = item.environment;
          hostArray.push({
            ...tmp,
            key: tmp.uuid,
            value: tmp.uuid,
            label: `${tmp.name}(${tmp.endpoint})`,
            isLeaf: true
          });
        });
        this.proxyOptions = hostArray;
      }
    );
  }
}
