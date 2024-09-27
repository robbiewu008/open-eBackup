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
import { MessageService, ModalRef } from '@iux/live';
import {
  BaseUtilService,
  DataMap,
  DataMapService,
  GlobalService,
  I18NService,
  ProtectedEnvironmentApiService,
  ProtectedResourceApiService,
  ResourceType
} from 'app/shared';

import { assign, isEmpty, set } from 'lodash';

@Component({
  selector: 'aui-add-user',
  templateUrl: './add-user.component.html',
  styleUrls: ['./add-user.component.less']
})
export class AddUserComponent implements OnInit {
  DataMap = DataMap;
  data;
  formGroup: FormGroup;
  isTest = false;
  treeSelection;
  tenant;
  domainId;
  resourceType = ResourceType;

  nameErrorTip = assign(this.baseUtilService.nameErrorTip, {
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [32])
  });
  pwdErrorTip = assign(this.baseUtilService.pwdErrorTip, {
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [32])
  });

  @ViewChild('footerTpl', { static: true }) footerTpl: TemplateRef<any>;

  constructor(
    private modal: ModalRef,
    private fb: FormBuilder,
    private i18n: I18NService,
    public baseUtilService: BaseUtilService,
    public dataMapService: DataMapService,
    private globalService: GlobalService,
    private messageService: MessageService,
    private protectedResourceApiService: ProtectedResourceApiService,
    private protectedEnvironmentApiService: ProtectedEnvironmentApiService
  ) {}

  ngOnInit() {
    this.initForm();
    this.getFooter();
  }

  initForm() {
    this.formGroup = this.fb.group({
      username: new FormControl(
        {
          value: isEmpty(this.data) ? '' : this.data?.name,
          disabled: !!this.data
        },
        {
          validators: [
            this.baseUtilService.VALID.required(),
            this.baseUtilService.VALID.maxLength(32)
          ]
        }
      ),
      password: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.maxLength(32)
        ]
      })
    });
    this.formGroup.valueChanges.subscribe(res => {
      this.isTest = false;
    });
  }

  getFooter() {
    this.modal.setProperty({ lvFooter: this.footerTpl });
  }

  getParams(data) {
    const params = {
      name: data.name,
      uuid: data.uuid,
      type: data.type,
      subType: data.subType,
      endpoint: data.endpoint,
      auth: {
        authType: data.auth.authType + '',
        authKey: this.formGroup.get('username').value,
        authPwd: this.formGroup.value.password
      },
      dependencies: {
        agents: data.dependencies.agents.map(item => {
          return { uuid: item.uuid };
        })
      },
      extendInfo: {
        domain: this.tenant,
        isVdc: 'true',
        domainId: this.domainId
      }
    };
    return params;
  }
  test() {
    this.protectedResourceApiService
      .ShowResource({
        resourceId: this.treeSelection.uuid
      })
      .subscribe(res => {
        const params = this.getParams(res);
        this.protectedEnvironmentApiService
          .CheckEnvironment({
            checkEnvironmentRequestBody: params as any,
            akOperationTips: false
          })
          .subscribe((response: any) => {
            const returnRes = JSON.parse(response);
            const idx = returnRes.findIndex(item => item.code !== 0);
            if (idx !== -1) {
              this.messageService.error(this.i18n.get(returnRes[idx].code), {
                lvMessageKey: 'errorKey',
                lvShowCloseButton: true
              });
            } else {
              this.messageService.success(
                this.i18n.get('common_test_success_label'),
                {
                  lvMessageKey: 'successKey',
                  lvShowCloseButton: true
                }
              );
              this.isTest = true;
            }
          });
      });
  }

  ok() {
    const newItem = {
      name: this.formGroup.get('username').value,
      passwd: this.formGroup.value.password
    };
    this.globalService.emitStore({
      action: 'add-tenant-user',
      state: newItem
    });
    this.modal.close();
  }
}
