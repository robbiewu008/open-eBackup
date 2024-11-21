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
import { GlobalService } from './store.service';
import { isString, assign, isEmpty } from 'lodash';

@Injectable({
  providedIn: 'root'
})
export class RememberColumnsService {
  constructor() {}
  currentUser;

  setUser(user) {
    this.currentUser = user;
  }

  setColumnsStatus(tableKey, tableParams) {
    if (window.localStorage) {
      let allParams = localStorage.getItem('TABLE_COLUMN_STATUS') || {};
      if (isString(allParams)) {
        allParams = JSON.parse(allParams);
      }
      const currentUserParams = allParams[this.currentUser] || {};
      assign(currentUserParams, {
        [tableKey]: tableParams
      });
      localStorage.setItem(
        'TABLE_COLUMN_STATUS',
        JSON.stringify(
          assign(allParams, {
            [this.currentUser]: currentUserParams
          })
        )
      );
    }
  }

  getColumnsStatus(tableKey) {
    if (window.localStorage) {
      let columnsStatus = localStorage.getItem('TABLE_COLUMN_STATUS') || {};
      if (isString(columnsStatus)) {
        columnsStatus = JSON.parse(columnsStatus);
      }
      return columnsStatus[this.currentUser]
        ? columnsStatus[this.currentUser][tableKey]
        : {};
    }
    return {};
  }
}
