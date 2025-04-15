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
import { Component } from '@angular/core';
import { FormBuilder, FormControl, FormGroup } from '@angular/forms';
import { OptionItem } from '@iux/live';
import {
  BaseUtilService,
  CommonConsts,
  DataMapService,
  I18NService,
  ProtectedResourceApiService,
  UsersApiService
} from 'app/shared';
import { AppUtilsService } from 'app/shared/services/app-utils.service';
import { isNumber, map } from 'lodash';
import { Observable, Observer } from 'rxjs';

@Component({
  selector: 'aui-modify-owned-user',
  templateUrl: './modify-owned-user.component.html',
  styleUrls: ['./modify-owned-user.component.less']
})
export class ModifyOwnedUserComponent {
  rowItem;
  formGroup: FormGroup;
  userOps: OptionItem[] = [];

  constructor(
    public i18n: I18NService,
    private fb: FormBuilder,
    private baseUtilService: BaseUtilService,
    private usersApiService: UsersApiService,
    private protectedResourceApiService: ProtectedResourceApiService,
    public dataMapService: DataMapService,
    private appUtilsService: AppUtilsService
  ) {}

  ngOnInit(): void {
    this.initForm();
    this.getUserOps();
  }

  initForm() {
    this.formGroup = this.fb.group({
      owned_user: new FormControl('', {
        validators: [this.baseUtilService.VALID.required()]
      })
    });
  }

  getUserOps(recordsTemp?, startPage?) {
    const filterUser = map(
      this.dataMapService.toArray('defaultRoleName'),
      'value'
    ).filter(v => v !== 'Role_DP_Admin');
    filterUser.push('Role_Device_Manager');
    const params = {
      startIndex: startPage || 1,
      pageSize: 500,
      filter: JSON.stringify({
        includeRoleList: filterUser
      })
    };

    this.usersApiService.getAllUserUsingGET(params).subscribe(res => {
      if (!recordsTemp) {
        recordsTemp = [];
      }
      if (!isNumber(startPage)) {
        startPage = 1;
      }
      recordsTemp = [...recordsTemp, ...res.userList];
      if (startPage === Math.ceil(res.total / 500) || res.total === 0) {
        const arr = [];
        recordsTemp.map(item => {
          arr.push({
            ...item,
            key: item.userId,
            value: item.userId,
            label: item.userName,
            isLeaf: true
          });
        });
        this.userOps = arr;
        return;
      }
      startPage++;
      this.getUserOps(recordsTemp, startPage);
    });
  }

  onOK(): Observable<void> {
    return new Observable<void>((observer: Observer<void>) => {
      this.protectedResourceApiService
        .UpdateCopyUser({
          updateCopyUserObjectReq: {
            resourceId: this.rowItem.resourceId,
            userId: this.formGroup.get('owned_user').value
          }
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
}
