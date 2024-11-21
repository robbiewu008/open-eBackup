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
import { DrawModalService } from './draw-modal.service';
import { MODAL_COMMON, SearchResource } from '../consts';
import { AddResourceTagComponent } from '../components/add-resource-tag/add-resource-tag.component';
import { Router } from '@angular/router';
import { combineLatest } from 'rxjs';
import { isFunction } from 'lodash';
import { GlobalService } from './store.service';

export interface SetParams {
  isAdd: boolean; // 判断是添加还是删除标签
  rowDatas: any; // 资源数据
  type?: any; // 类型
  onOk?: () => void; // 回调
}
@Injectable({
  providedIn: 'root'
})
export class SetResourceTagService {
  constructor(
    private drawModalService: DrawModalService,
    public router: Router,
    private globalService: GlobalService
  ) {}

  setTag(params: SetParams) {
    this.drawModalService.create({
      ...MODAL_COMMON.generateDrawerOptions(),
      ...{
        lvWidth: MODAL_COMMON.largeWidth,
        lvOkDisabled: true,
        lvContent: AddResourceTagComponent,
        lvAfterOpen: modal => {
          const content = modal.getContentComponent() as AddResourceTagComponent;
          const modalIns = modal.getInstance();
          const combined: any = combineLatest([
            content.formGroup.statusChanges,
            content.selectValid$
          ]);
          combined.subscribe(latestValues => {
            const [formGroupStatus, valid] = latestValues;
            modalIns.lvOkDisabled = !valid || formGroupStatus !== 'VALID';
          });
          content.formGroup.updateValueAndValidity();
        },
        lvComponentParams: params,
        lvOk: modal => {
          this.dealConfirm(modal, params);
        },
        lvCancel: modal => {
          modal.close();
        }
      }
    });
  }
  dealConfirm(modal, params) {
    return new Promise(resolve => {
      const content = modal.getContentComponent() as AddResourceTagComponent;
      content.onOK().subscribe(
        res => {
          resolve(true);
          this.emitSearchStore();
          if (isFunction(params.onOk)) {
            params.onOk();
          }
        },
        err => {
          resolve(false);
        }
      );
    });
  }

  emitSearchStore() {
    if (this.router.url === '/search') {
      this.globalService.emitStore({
        action: SearchResource.Refresh,
        state: ''
      });
    }
  }
}
