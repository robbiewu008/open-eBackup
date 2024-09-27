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
import { CommonModule, DatePipe } from '@angular/common';
import { Component, Injectable, NgModule } from '@angular/core';
import {
  ActivatedRouteSnapshot,
  CanDeactivate,
  RouterStateSnapshot
} from '@angular/router';
import { MenuComponent, MessageboxService } from '@iux/live';
import { Observable } from 'rxjs';
import { I18NService, MODAL_COMMON, WarningMessageService } from '..';
import { DataMap } from '../consts';
import { AppUtilsService } from '../services/app-utils.service';

@Injectable({
  providedIn: 'root'
})
export class CanDeactivateGuard implements CanDeactivate<unknown> {
  constructor(
    public appUtilsService: AppUtilsService,
    private i18n: I18NService,
    private messageBox: MessageboxService,
    private warningMessageService: WarningMessageService
  ) {}

  canDeactivate(
    component: MenuComponent,
    currentRoute: ActivatedRouteSnapshot,
    currentState: RouterStateSnapshot,
    futureState: RouterStateSnapshot
  ): boolean | Observable<boolean> | Promise<boolean> {
    // 调用弹窗服务来获取用户的选择
    return new Promise(resolve => {
      const networkModifyFlag = this.appUtilsService.getCacheValue(
        'networkModify',
        false
      );
      if (
        !!networkModifyFlag &&
        networkModifyFlag !== DataMap.networkModifyingStatus.normal.value &&
        futureState.url !== '/login'
      ) {
        if (networkModifyFlag === DataMap.networkModifyingStatus.modify.value) {
          // 网络修改时切换路由
          this.warningMessageService.create({
            content: this.i18n.get('common_leave_network_config_tip_label'),
            onOK: () => {
              resolve(true);
            },
            onCancel: () => {
              resolve(false);
            },
            lvAfterClose: () => {
              resolve(false);
            }
          });
        } else {
          // 网络修改中切换路由
          this.messageBox.info({
            lvWidth: MODAL_COMMON.smallWidth,
            lvHeader: this.i18n.get('common_alarms_info_label'),
            lvDialogIcon: 'lv-icon-popup-info-48',
            lvContent: TipsComponent,
            lvOk: () => {
              resolve(false);
            }
          });
        }
      } else {
        resolve(true);
      }
    });
  }
}

@Component({
  selector: 'aui-tips',
  template: `
    <span [innerHTML]="tips"></span>
  `,
  styles: [],
  providers: [DatePipe]
})
export class TipsComponent {
  networkModifyFlag = this.appUtilsService.getCacheValue(
    'networkModify',
    false
  );
  tips = this.i18n.get('common_leave_network_config_tip_label');
  constructor(
    public i18n: I18NService,
    public appUtilsService: AppUtilsService
  ) {}
  ngOnInit() {
    if (
      this.networkModifyFlag === DataMap.networkModifyingStatus.modifying.value
    ) {
      this.tips = this.i18n.get(
        'common_leave_network_config_modifying_tip_label'
      );
    }
  }
}
@NgModule({
  imports: [CommonModule],
  declarations: [TipsComponent]
})
export class TipsModule {}
