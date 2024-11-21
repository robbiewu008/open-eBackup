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
import { ModalRef } from '@iux/live';
import {
  FormGroup,
  FormBuilder,
  FormControl,
  ValidatorFn,
  AbstractControl
} from '@angular/forms';
import { Component, OnInit } from '@angular/core';
import { Observable, Observer } from 'rxjs';
import {
  I18NService,
  BaseUtilService,
  DataMap,
  ProtectedResourceApiService,
  ResourceType,
  InstanceType,
  CommonConsts,
  DataMapService
} from 'app/shared';
import {
  each,
  find,
  first,
  isEmpty,
  isUndefined,
  map,
  size,
  trim,
  uniq
} from 'lodash';

@Component({
  selector: 'aui-king-base-add-host',
  templateUrl: './king-base-add-host.component.html',
  styleUrls: ['./king-base-add-host.component.less']
})
export class KingBaseAddHostComponent implements OnInit {
  name;
  item;
  portStatus;
  hostStatus;
  parentUuid;
  data;
  children = []; // 表格数据
  isTest = false;
  okLoading = false;
  testLoading = false;
  hostOptions = [];
  formGroup: FormGroup;
  extendsAuth = {};
  dataMap = DataMap;

  // 认证方式
  authMethodOptions = this.dataMapService
    .toArray('postgre_Auth_Method_Type')
    .filter(v => (v.isLeaf = true));

  portErrorTip = {
    ...this.baseUtilService.rangeErrorTip,
    invalidRang: this.i18n.get('common_valid_rang_label', [1, 65535])
  };
  usernameErrorTip = {
    ...this.baseUtilService.requiredErrorTip,
    ...this.baseUtilService.lengthErrorTip,
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [32])
  };
  // 路径校验
  pathErrorTip = {
    ...this.baseUtilService.filePathErrorTip,
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [1024]),
    invalidSpecailChars: this.i18n.get('common_valid_file_path_label'),
    pathError: this.i18n.get('common_path_error_label')
  };

  constructor(
    private fb: FormBuilder,
    public modal: ModalRef,
    private i18n: I18NService,
    public dataMapService: DataMapService,
    private protectedResourceApiService: ProtectedResourceApiService,
    public baseUtilService: BaseUtilService
  ) {}

  ngOnInit() {
    this.initForm();
    this.getProxyOptions();
  }

  initForm() {
    this.formGroup = this.fb.group({
      host: new FormControl(
        isEmpty(this.item)
          ? ''
          : first(map(this.item.dependencies.agents, 'uuid')),
        {
          validators: [this.baseUtilService.VALID.required()]
        }
      ),
      userName: new FormControl(
        {
          value: isEmpty(this.item) ? '' : this.item.extendInfo?.osUsername,
          disabled: this.item && this.hostStatus
        },
        {
          validators: [
            this.baseUtilService.VALID.required(),
            this.baseUtilService.VALID.maxLength(32)
          ]
        }
      ),
      client: new FormControl(
        {
          value: isEmpty(this.item) ? '' : this.item.extendInfo?.clientPath,
          disabled: this.item && this.hostStatus
        },

        {
          validators: [this.baseUtilService.VALID.required(), this.validPath()]
        }
      ),

      business_ip: new FormControl(
        {
          value: isEmpty(this.item) ? '' : this.item.extendInfo?.serviceIp,
          disabled: this.item && this.hostStatus
        },

        {
          validators: [
            this.baseUtilService.VALID.required(),
            this.baseUtilService.VALID.ip()
          ]
        }
      ),
      port: new FormControl(
        {
          value: isEmpty(this.item)
            ? '54321'
            : this.item.extendInfo?.instancePort,
          disabled: this.portStatus ? !!this.item : false
        },

        {
          validators: [
            this.baseUtilService.VALID.required(),
            this.baseUtilService.VALID.integer(),
            this.baseUtilService.VALID.rangeValue(1, 65535)
          ]
        }
      ),
      auth_method: new FormControl(
        isEmpty(this.item)
          ? this.dataMap.postgre_Auth_Method_Type.database.value
          : this.item.auth?.authType,
        {
          validators: [this.baseUtilService.VALID.required()]
        }
      ),
      database_username: new FormControl(
        isEmpty(this.item) ? [] : this.item.auth?.authKey,
        {
          validators: [this.baseUtilService.VALID.required()]
        }
      ),
      database_password: new FormControl([], {
        validators: [this.baseUtilService.VALID.required()]
      })
    });

    this.watch();
  }

  watch() {
    this.formGroup.get('auth_method').valueChanges.subscribe(res => {
      if (res === this.dataMap.postgre_Auth_Method_Type.database.value) {
        this.formGroup
          .get('database_password')
          .setValidators([this.baseUtilService.VALID.required()]);
        this.formGroup
          .get('database_username')
          .setValidators([this.baseUtilService.VALID.required()]);
      } else {
        this.formGroup.get('database_password').clearValidators();
        this.formGroup.get('database_username').clearValidators();
      }
      this.formGroup.get('database_password').updateValueAndValidity();
      this.formGroup.get('database_username').updateValueAndValidity();
    });
  }

  validPath(): ValidatorFn {
    return (control: AbstractControl): { [key: string]: any } | null => {
      if (!trim(control.value)) {
        return null;
      }
      const paths = control.value.split(',')?.filter(item => {
        return !isEmpty(item);
      });
      if (paths.length !== uniq(paths).length) {
        return { samePathError: { value: control.value } };
      }

      if (
        find(paths, path => {
          return !CommonConsts.REGEX.templatLinuxPath.test(path);
        }) ||
        find(paths, path => {
          return path.length > 1024;
        })
      ) {
        return { pathError: { value: control.value } };
      }
    };
  }

  getProxyOptions() {
    const params = {
      resourceId: this.parentUuid
    };
    this.protectedResourceApiService.ShowResource(params).subscribe(res => {
      const hostArray = [];
      each(res['dependencies']['agents'], item => {
        hostArray.push({
          ...item,
          key: item.uuid,
          value: item.uuid,
          label: item.name + `(${item.endpoint})`,
          isLeaf: true
        });
      });
      if (!isEmpty(this.item)) {
        // 修改的场景
        this.hostOptions = hostArray;
        return;
      }

      if (!!size(this.children)) {
        this.hostOptions = hostArray.filter(item =>
          isUndefined(
            find(
              this.children,
              child => child.extendInfo?.hostId === item.value
            )
          )
        );
        return;
      }
      this.hostOptions = hostArray;
    });
  }

  onOK(): Observable<void> {
    if (
      this.formGroup.value.auth_method ===
      this.dataMap.postgre_Auth_Method_Type.database.value
    ) {
      this.extendsAuth['authType'] = DataMap.Postgre_Auth_Method.db.value;
      this.extendsAuth['authKey'] = this.formGroup.value.database_username;
      this.extendsAuth['authPwd'] = this.formGroup.value.database_password;
    } else {
      this.extendsAuth['authType'] = DataMap.Postgre_Auth_Method.os.value;
      this.extendsAuth['authKey'] = '';
      this.extendsAuth['authPwd'] = '';
    }
    return new Observable<void>((observer: Observer<void>) => {
      if (this.formGroup.invalid) {
        return;
      }
      const host = find(this.hostOptions, { uuid: this.formGroup.value.host });
      this.data = {
        parentUuid: '',
        name: null,
        type: ResourceType.DATABASE,
        subType: DataMap.Resource_Type.KingBaseInstance.value,
        extendInfo: {
          hostId: this.formGroup.value.host,
          instancePort: this.formGroup.get('port').value,
          clientPath: this.formGroup.get('client').value,
          serviceIp: this.formGroup.get('business_ip').value,
          osUsername: this.formGroup.get('userName').value,
          isTopInstance: InstanceType.NotTopinstance
        },
        dependencies: {
          agents: [{ uuid: this.formGroup.value.host }]
        },
        auth: {
          ...this.extendsAuth,
          extendInfo: {}
        },
        port: this.formGroup.get('port').value,
        hostName: host?.name,
        ip: host?.endpoint,
        client: this.formGroup.get('client').value,
        business_ip: this.formGroup.get('business_ip').value,
        userName: this.formGroup.get('userName').value,
        databaseUserName: this.formGroup.value.database_username
      };
      observer.next();
      observer.complete();
    });
  }
}
