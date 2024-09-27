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
import { Injectable } from '@angular/core';
import { includes, map } from 'lodash';
import { Observable, Observer, finalize } from 'rxjs';
import { RoleAuthApiService } from '../api/services';
import { RoleOperationAuth } from '../consts/permission.const';
import { I18NService } from '../services/i18n.service';
import { DataMap } from '../consts/data-map.config';

@Injectable({
  providedIn: 'root'
})
export class PermissionService {
  constructor(
    private i18n: I18NService,
    private roleAuthApiService: RoleAuthApiService
  ) {}

  getUserPermission() {
    return new Observable<void>((observer: Observer<void>) => {
      if (
        includes(
          [
            DataMap.Deploy_Type.cyberengine.value,
            DataMap.Deploy_Type.hyperdetect.value,
            DataMap.Deploy_Type.cloudbackup.value,
            DataMap.Deploy_Type.cloudbackup2.value
          ],
          this.i18n.get('deploy_type')
        )
      ) {
        observer.next();
        observer.complete();
        return;
      }
      this.roleAuthApiService
        .queryRoleAuthSetByCurrentUser({
          akDoException: false
        })
        .pipe(
          finalize(() => {
            observer.next();
            observer.complete();
          })
        )
        .subscribe(res => {
          // 写入当前用户操作权限
          RoleOperationAuth.push(...map(res, 'authOperation'));
        });
    });
  }
}
