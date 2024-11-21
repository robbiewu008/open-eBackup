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
import { Component, OnInit, TemplateRef, ViewChild } from '@angular/core';
import { FormBuilder, FormControl, FormGroup } from '@angular/forms';
import { ModalRef } from '@iux/live';
import {
  BaseUtilService,
  CommonConsts,
  DataMap,
  I18NService,
  ProtectedResourceApiService
} from 'app/shared';
import { AppUtilsService } from 'app/shared/services/app-utils.service';
import {
  assign,
  each,
  find,
  first,
  isEmpty,
  map,
  reject,
  uniqueId
} from 'lodash';
import { Observable, Observer } from 'rxjs';

@Component({
  selector: 'aui-register-dataset',
  templateUrl: './register-dataset.component.html',
  styleUrls: ['./register-dataset.component.less']
})
export class RegisterDatasetComponent implements OnInit {
  rowItem;
  formGroup: FormGroup;
  dataMap = DataMap;
  namespaceOptions = [];

  includeLabels = [];
  prefixInKey = 'prefixInKey';
  prefixInValue = 'prefixInValue';
  excludeLabels = [];
  prefixExKey = 'prefixExKey';
  prefixExValue = 'prefixExValue';

  helpUrl: string;

  nameErrorTip = {
    ...this.baseUtilService.nameErrorTip,
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [64])
  };
  tagErrorTip = {
    ...this.baseUtilService.requiredErrorTip
  };
  userNameErrorTip = {
    ...this.baseUtilService.requiredErrorTip
  };
  passwordErrorTip = {
    ...this.baseUtilService.requiredErrorTip
  };
  keyErrorTip = {
    ...this.baseUtilService.requiredErrorTip,
    invalidLabel: this.i18n.get('protection_labels_key_valid_label')
  };
  valueErrorTip = {
    invalidMaxLength: this.i18n.get('protection_labels_value_valid_label'),
    invalidName: this.i18n.get('protection_labels_value_valid_label')
  };

  @ViewChild('headerTpl', { static: true }) headerTpl: TemplateRef<any>;

  constructor(
    private modal: ModalRef,
    private fb: FormBuilder,
    private i18n: I18NService,
    public baseUtilService: BaseUtilService,
    private AppUtilsService: AppUtilsService,
    private protectedResourceApiService: ProtectedResourceApiService
  ) {}

  ngOnInit() {
    this.initModalHeader();
    this.initForm();
    this.getNamespace();
    this.updateForm();
  }

  initModalHeader() {
    this.modal.setProperty({ lvHeader: this.headerTpl });
  }

  openHelp() {
    const lang = this.i18n.isEn ? 'en-us' : 'zh-cn';
    const targetUrl = `/console/assets/help/a8000/${lang}/index.html#kubernetes_CSI_00012.html`;
    if (this.AppUtilsService.isHcsUser) {
      const herf: string = first(window.location.href.split('#'));
      window.open(herf.replace('/console/', targetUrl), '_blank');
    } else {
      window.open(targetUrl, '_blank');
    }
  }

  addIncludeLabels(key?: string, value?: string) {
    const id = uniqueId();
    this.formGroup.addControl(
      this.prefixInKey + id,
      new FormControl(key || '', {
        validators: [
          this.baseUtilService.VALID.required(),
          this.AppUtilsService.validLabel()
        ]
      })
    );
    this.formGroup.addControl(
      this.prefixInValue + id,
      new FormControl(value || '', {
        validators: [
          this.baseUtilService.VALID.name(CommonConsts.REGEX.label, false),
          this.baseUtilService.VALID.maxLength(63)
        ]
      })
    );
    this.includeLabels.push({ id });
    this.formGroup.updateValueAndValidity();
  }

  deleteIncludeLabels(id) {
    this.includeLabels = reject(this.includeLabels, v => v.id === id);
    this.formGroup.removeControl(this.prefixInKey + id);
    this.formGroup.removeControl(this.prefixInValue + id);
    this.formGroup.updateValueAndValidity();
  }

  addExcludeLabels(key?: string, value?: string) {
    const id = uniqueId();
    this.formGroup.addControl(
      this.prefixExKey + id,
      new FormControl(key || '', {
        validators: [
          this.baseUtilService.VALID.required(),
          this.AppUtilsService.validLabel()
        ]
      })
    );
    this.formGroup.addControl(
      this.prefixExValue + id,
      new FormControl(value || '', {
        validators: [
          this.baseUtilService.VALID.name(CommonConsts.REGEX.label, false),
          this.baseUtilService.VALID.maxLength(63)
        ]
      })
    );
    this.excludeLabels.push({ id });
    this.formGroup.updateValueAndValidity();
  }

  deleteExcludeLabels(id) {
    this.excludeLabels = reject(this.excludeLabels, v => v.id === id);
    this.formGroup.removeControl(this.prefixExKey + id);
    this.formGroup.removeControl(this.prefixExValue + id);
    this.formGroup.updateValueAndValidity();
  }

  initForm() {
    this.formGroup = this.fb.group({
      name: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.name(),
          this.baseUtilService.VALID.maxLength(64)
        ]
      }),
      namespace: new FormControl('', {
        validators: [this.baseUtilService.VALID.required()]
      })
    });

    this.listenForm();
  }

  listenForm() {
    this.formGroup.statusChanges.subscribe(() => this.disableOkBtn());
  }

  updateForm() {
    if (isEmpty(this.rowItem)) {
      return;
    }
    const params = {
      name: this.rowItem.name,
      namespace: this.rowItem.parentUuid
    };
    this.formGroup.patchValue(params);
    if (!isEmpty(this.rowItem.extendInfo?.labels)) {
      each(this.rowItem.extendInfo?.labels.split(','), item => {
        this.addIncludeLabels(item.split('=')[0], item.split('=')[1]);
      });
    }
    if (!isEmpty(this.rowItem.extendInfo?.excludeLabels)) {
      each(this.rowItem.extendInfo?.excludeLabels.split(','), item => {
        this.addExcludeLabels(item.split('!=')[0], item.split('!=')[1]);
      });
    }
  }

  disableOkBtn() {
    this.modal.getInstance().lvOkDisabled = this.formGroup.invalid;
  }

  getNamespace() {
    const extParams = {
      conditions: JSON.stringify({
        subType: [DataMap.Resource_Type.kubernetesNamespaceCommon.value]
      })
    };
    this.AppUtilsService.getResourceByRecursion(
      extParams,
      params => this.protectedResourceApiService.ListResources(params),
      resource => {
        this.namespaceOptions = map(resource, item => {
          return assign(item, {
            label: `${item.parentName}/${item.name}`,
            isLeaf: true
          });
        });
      }
    );
  }

  getIncludeLabels() {
    if (isEmpty(this.includeLabels)) {
      return '';
    }
    const arr = [];
    each(this.includeLabels, item => {
      arr.push(
        `${this.formGroup.get(`${this.prefixInKey}${item.id}`)?.value}=${
          this.formGroup.get(`${this.prefixInValue}${item.id}`)?.value
        }`
      );
    });
    return arr.join(',');
  }

  getExcludeLabels() {
    if (isEmpty(this.excludeLabels)) {
      return '';
    }
    const arr = [];
    each(this.excludeLabels, item => {
      arr.push(
        `${this.formGroup.get(`${this.prefixExKey}${item.id}`)?.value}!=${
          this.formGroup.get(`${this.prefixExValue}${item.id}`)?.value
        }`
      );
    });
    return arr.join(',');
  }

  getParams() {
    const params = {
      name: this.formGroup.value.name,
      type: 'KubernetesCommon',
      subType: DataMap.Resource_Type.kubernetesDatasetCommon.value,
      parentUuid: this.formGroup.value.namespace,
      parentName: find(this.namespaceOptions, {
        uuid: this.formGroup.value.namespace
      })?.name,
      extendInfo: {}
    };
    assign(params.extendInfo, {
      labels: this.getIncludeLabels()
    });
    assign(params.extendInfo, {
      excludeLabels: this.getExcludeLabels()
    });
    return params;
  }

  onOK(): Observable<void> {
    return new Observable<void>((observer: Observer<void>) => {
      const params = this.getParams();
      if (!isEmpty(this.rowItem)) {
        this.protectedResourceApiService
          .UpdateResource({
            resourceId: this.rowItem.uuid,
            UpdateResourceRequestBody: params
          })
          .subscribe(
            () => {
              observer.next();
              observer.complete();
            },
            error => {
              observer.error(error);
              observer.complete();
            }
          );
      } else {
        this.protectedResourceApiService
          .CreateResource({
            CreateResourceRequestBody: params
          })
          .subscribe(
            () => {
              observer.next();
              observer.complete();
            },
            error => {
              observer.error(error);
              observer.complete();
            }
          );
      }
    });
  }
}
