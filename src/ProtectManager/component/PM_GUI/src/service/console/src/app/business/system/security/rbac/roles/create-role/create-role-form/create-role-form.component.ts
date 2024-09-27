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
import { FormControl, FormGroup } from '@angular/forms';
import {
  BaseUtilService,
  CommonConsts,
  DataMap,
  I18NService,
  PermissionTable,
  RoleAuthApiService
} from 'app/shared';
import { AppUtilsService } from 'app/shared/services/app-utils.service';
import { cloneDeep, each, find, map, uniq } from 'lodash';

@Component({
  selector: 'aui-create-role-form',
  templateUrl: './create-role-form.component.html',
  styleUrls: ['./create-role-form.component.less']
})
export class CreateRoleFormComponent implements OnInit {
  @Input() formGroup: FormGroup;
  @Input() data;
  permissionTable: any = cloneDeep(PermissionTable).filter(
    item =>
      !(this.appUtilsService.isDistributed && item.value === 'DataSecurity')
  );
  nameErrorTip = {
    ...this.baseUtilService.requiredErrorTip,
    invalidName: this.i18n.get('common_storage_pool_name_invalid_label')
  };

  constructor(
    public i18n: I18NService,
    public baseUtilService: BaseUtilService,
    public roleAuthApiService: RoleAuthApiService,
    private appUtilsService: AppUtilsService
  ) {}

  ngOnInit() {
    this.initForm();
    this.dealPermissionTable();
  }

  initForm() {
    this.formGroup.addControl(
      'roleName',
      new FormControl('', [
        this.baseUtilService.VALID.required(),
        this.baseUtilService.VALID.name(CommonConsts.REGEX.storagePoolName)
      ])
    );
    this.formGroup.addControl(
      'roleDescription',
      new FormControl('', this.baseUtilService.VALID.maxLength(255))
    );
    this.formGroup.addControl('authList', new FormControl([]));
    this.formGroup.get('authList').valueChanges.subscribe(res => {
      each(this.permissionTable, item => {
        const tmpPermissionList = item.children.filter(val =>
          find(res, auth => auth === val.value)
        );
        item.allSelected = tmpPermissionList.length === item.children.length;
      });
    });

    if (this.data) {
      this.formGroup.patchValue(this.data);
      this.roleAuthApiService
        .getAuthListByRoleIdUsingGet({
          id: this.data.roleId
        })
        .subscribe(res => {
          this.formGroup.get('authList').setValue(map(res, 'uuid'));
        });
    }
  }

  dealPermissionTable() {
    if (this.i18n.get('deploy_type') === DataMap.Deploy_Type.x3000.value) {
      const tmpSecurity = find(this.permissionTable, { value: 'DataSecurity' });
      tmpSecurity.children.pop();
      tmpSecurity.children[1].label = this.i18n.get('common_worm_policy_label');
    }
  }

  allSelectPermission(e, item) {
    e.stopPropagation();
    const tmpPermissionList = find(this.permissionTable, { value: item.value });
    const tmpAllSelect = !tmpPermissionList?.allSelected;
    if (tmpAllSelect) {
      this.formGroup
        .get('authList')
        .setValue(
          uniq([
            ...this.formGroup.get('authList').value,
            ...tmpPermissionList.children.map(val => val.value)
          ])
        );
    } else {
      this.formGroup
        .get('authList')
        .setValue(
          this.formGroup
            .get('authList')
            .value.filter(
              val => !find(tmpPermissionList.children, { value: val })
            )
        );
    }
    tmpPermissionList.allSelected = tmpAllSelect;
  }
}
