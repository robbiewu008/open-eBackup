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
  CommonConsts
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
  selector: 'aui-postgre-add-host',
  templateUrl: './postgre-add-host.component.html',
  styleUrls: ['./postgre-add-host.component.less']
})
export class PostgreAddHostComponent implements OnInit {
  parentUuid;
  data;
  item;
  children = []; // 表格数据
  isTest = false;
  okLoading = false;
  testLoading = false;
  hostOptions = [];
  formGroup: FormGroup;
  osUsername = '';
  isPgpool;

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
      archive_path: new FormControl(
        isEmpty(this.item) ? '' : this.item.extendInfo?.archiveDir,
        {
          validators: [this.validArchivePath()]
        }
      ),
      pgPath: new FormControl(
        isEmpty(this.item) ? '' : this.item.extendInfo?.pgpoolClientPath,
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
        isEmpty(this.item) ? '5432' : this.item.extendInfo?.instancePort,
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

  validArchivePath(): ValidatorFn {
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

      const regx = /^(\/[^\/]{1,2048})+\/?$|^\/$/;

      if (
        find(paths, path => {
          return !regx.test(path);
        }) ||
        find(paths, path => {
          return path.length > 1024;
        })
      ) {
        return { pathError: { value: control.value } };
      }
    };
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
          const templateLinuxPath = /^(\/[^\/]{1,2048})+\/?$|^\/$/;
          return !templateLinuxPath.test(path);
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
      if (!!size(this.children) && isEmpty(this.item)) {
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

  getArchiveDir() {
    if (isEmpty(this.formGroup.value.archive_path)) {
      return this.formGroup.value.archive_path;
    } else {
      return this.formGroup.value.archive_path.endsWith('/')
        ? this.formGroup.value.archive_path
        : this.formGroup.value.archive_path + '/';
    }
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
        subType: DataMap.Resource_Type.PostgreSQLInstance.value,
        extendInfo: {
          hostId: this.formGroup.value.host,
          instancePort: this.formGroup.value.port,
          clientPath: this.formGroup.value.client.replace(/\/?$/, ''),
          archiveDir: this.getArchiveDir(),
          pgpoolClientPath: this.formGroup.value.pgPath.replace(/\/?$/, ''),
          serviceIp: this.formGroup.value.business_ip,
          osUsername: this.osUsername,
          isTopInstance: InstanceType.NotTopinstance
        },
        dependencies: {
          agents: [{ uuid: this.formGroup.value.host }]
        },
        auth: {
          extendInfo: {}
        },
        port: this.formGroup.value.port,
        hostName: host?.name,
        ip: host?.endpoint,
        client: this.formGroup.value.client.replace(/\/?$/, ''),
        archive_path: this.getArchiveDir(),
        business_ip: this.formGroup.value.business_ip,
        pgpoolClientPath: this.formGroup.value.pgPath.replace(/\/?$/, '')
      };
      observer.next();
      observer.complete();
    });
  }
}
