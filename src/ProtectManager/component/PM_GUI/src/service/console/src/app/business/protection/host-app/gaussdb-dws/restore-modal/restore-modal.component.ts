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
import { Component, Input, OnDestroy, OnInit } from '@angular/core';
import { FormBuilder } from '@angular/forms';
import { ModalRef } from '@iux/live';
import {
  BaseUtilService,
  DataMap,
  I18NService,
  ProtectedEnvironmentApiService,
  ProtectedResourceApiService,
  RestoreApiV2Service
} from 'app/shared';
import {
  assign,
  cloneDeep,
  each,
  find,
  first,
  join,
  map,
  slice,
  split,
  trim
} from 'lodash';
import { Observable, Observer, Subject } from 'rxjs';

@Component({
  selector: 'aui-modal-restore',
  templateUrl: './restore-modal.component.html',
  styleUrls: ['./restore-modal.component.less']
})
export class ModalRestoreComponent implements OnInit, OnDestroy {
  @Input() data;
  @Input() path;

  destroy$ = new Subject();
  tableData = [];
  database;

  newNameErrorTip = {
    invalidName: this.i18n.get('protection_dws_new_name_error_tips_label'),
    invalidSpecialName: this.i18n.get(
      'protection_dws_new_special_name_error_tips_label'
    )
  };

  constructor(
    private fb: FormBuilder,
    private modal: ModalRef,
    private i18n: I18NService,
    private baseUtilService: BaseUtilService,
    private restoreV2Service: RestoreApiV2Service,
    private protectedResourceApiService: ProtectedResourceApiService,
    private protectedEnvironmentApiService: ProtectedEnvironmentApiService
  ) {}

  ngOnInit() {
    this.database = split(first(this.path)['rootPath'], '/')[2];
    each(this.path, item => {
      if (!item.isLeaf || item?.isMoreBtn) {
        return;
      }
      this.tableData.push({
        tableName: item.path,
        targetSchema: item.newName || split(item.rootPath, '/')[3],
        tableNewName: '',
        invalid: false,
        errorTips: '',
        oldName: item.rootPath
      });
    });
  }

  ngOnDestroy() {
    this.destroy$.next(true);
    this.destroy$.complete();
  }

  validNewName(item) {
    const value = item.tableNewName;

    if (!trim(value)) {
      item.invalid = false;
      this.disabledOkbtn();
      return;
    }

    const reg = /^[a-zA-Z\_]{1}[a-zA-Z0-9\_\$\#]{0,62}$/;

    if (!reg.test(value)) {
      item.invalid = true;
      item.errorTips = this.newNameErrorTip.invalidName;
    } else {
      item.invalid = false;
    }
    this.disabledOkbtn();
  }

  disabledOkbtn() {
    this.modal.getInstance().lvOkDisabled = !!find(
      this.tableData,
      item => item.invalid
    );
  }

  onOK(): Observable<void> {
    return new Observable<void>((observer: Observer<void>) => {
      const params = this.data?.requestParams || {};

      assign(params, {
        subObjects: map(cloneDeep(this.tableData), item => {
          return JSON.stringify({
            name: `${split(item.oldName, '/')[2]}/${
              item.targetSchema
            }/${item.tableNewName || item.tableName}`,
            type: 'Database',
            subType: DataMap.Resource_Type.DWS_Table.value,
            extendInfo: {
              oldName: join(slice(split(item.oldName, '/'), 2), '/')
            }
          });
        })
      });

      this.restoreV2Service
        .CreateRestoreTask({ CreateRestoreTaskRequestBody: params })
        .subscribe(
          res => {
            observer.next();
            observer.complete();
          },
          err => {
            observer.error(err);
            observer.complete();
          }
        );
    });
  }
}
