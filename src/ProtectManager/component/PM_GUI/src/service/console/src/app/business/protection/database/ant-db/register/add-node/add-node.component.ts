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
  ResourceType,
  InstanceType,
  CommonConsts
} from 'app/shared';
import { find, first, isEmpty, map, trim, uniq } from 'lodash';

@Component({
  selector: 'aui-add-node',
  templateUrl: './add-node.component.html',
  styleUrls: ['./add-node.component.less']
})
export class AddNodeComponent implements OnInit {
  parentUuid;
  data;
  item;
  hostOptions = [];
  formGroup: FormGroup;
  parentFormGroupValue; // 父组件传入的值
  portErrorTip = {
    ...this.baseUtilService.rangeErrorTip,
    invalidRang: this.i18n.get('common_valid_rang_label', [1, 65535])
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
    public baseUtilService: BaseUtilService
  ) {}

  ngOnInit() {
    this.initForm();
  }

  initForm() {
    this.formGroup = this.fb.group({
      host: new FormControl(
        isEmpty(this.item)
          ? ''
          : first(map(this.item.dependencies?.agents, 'uuid')),
        {
          validators: [this.baseUtilService.VALID.required()]
        }
      ),
      client: new FormControl(
        isEmpty(this.item) ? '' : this.item.extendInfo?.clientPath,
        {
          validators: [this.baseUtilService.VALID.required(), this.validPath()]
        }
      ),
      pgPath: new FormControl(
        isEmpty(this.item) ? '' : this.item.extendInfo?.adbhamgrPath,
        {
          validators: [this.baseUtilService.VALID.required(), this.validPath()]
        }
      ),
      business_ip: new FormControl(
        isEmpty(this.item) ? '' : this.item.extendInfo?.serviceIp,
        {
          validators: [
            this.baseUtilService.VALID.required(),
            this.baseUtilService.VALID.ip()
          ]
        }
      ),
      port: new FormControl(
        isEmpty(this.item) ? '6655' : this.item.extendInfo?.instancePort,
        {
          validators: [
            this.baseUtilService.VALID.required(),
            this.baseUtilService.VALID.integer(),
            this.baseUtilService.VALID.rangeValue(1, 65535)
          ]
        }
      )
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

  onOK(): Observable<void> {
    return new Observable<void>((observer: Observer<void>) => {
      if (this.formGroup.invalid) {
        return;
      }
      const host = find(this.hostOptions, { uuid: this.formGroup.value.host });
      this.data = {
        parentUuid: '',
        name: null,
        type: ResourceType.DATABASE,
        subType: DataMap.Resource_Type.AntDBInstance.value,
        extendInfo: {
          hostId: this.formGroup.value.host,
          instancePort: this.formGroup.value.port,
          clientPath: this.formGroup.value.client,
          adbhamgrPath: this.formGroup.value.pgPath,
          serviceIp: this.formGroup.value.business_ip,
          osUsername: this.parentFormGroupValue.osUsername,
          isTopInstance: InstanceType.NotTopinstance
        },
        dependencies: {
          agents: [{ uuid: this.formGroup.value.host }]
        },
        auth: {
          authType: DataMap.Postgre_Auth_Method.db.value,
          authKey: this.parentFormGroupValue.database_username,
          authPwd: this.parentFormGroupValue.database_password,
          extendInfo: {
            dbStreamRepUser: this.parentFormGroupValue.databaseStreamUserName,
            dbStreamRepPwd: this.parentFormGroupValue.databaseStreamPassword
          }
        },
        port: this.formGroup.value.port,
        hostName: host?.name,
        ip: host?.endpoint,
        client: this.formGroup.value.client,
        business_ip: this.formGroup.value.business_ip,
        adbhamgrPath: this.formGroup.value.pgPath
      };
      observer.next();
      observer.complete();
    });
  }
}
