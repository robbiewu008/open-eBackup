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
import { ModalService } from '@iux/live';
import { InfoComponent } from '../components/info.component';
import { MODAL_COMMON } from '../consts/live.const';
import { I18NService } from './i18n.service';

interface Options {
  content: string;
  width?: number;
  onOK: () => void;
  onCancel?: () => void;
  lvAfterClose?: (rseult: any) => void;
  noBreak?: boolean;
  header?: string;
}

@Injectable({
  providedIn: 'root'
})
export class InfoMessageService {
  private infoComponent = InfoComponent;

  constructor(
    private drawModalService: ModalService,
    private i18n: I18NService
  ) {}

  create(options: Options) {
    this.drawModalService.create({
      ...MODAL_COMMON.generateDrawerOptions(),
      lvModalKey: 'infoMessage',
      ...{
        lvType: 'dialog',
        lvDialogIcon: 'lv-icon-popup-info-48',
        lvHeader: options.header || this.i18n.get('common_alarms_info_label'),
        lvContent: this.infoComponent,
        lvComponentParams: {
          content: options.content,
          noBreak: options.noBreak
        },
        lvWidth: options.width || MODAL_COMMON.normalWidth,
        lvOkType: 'primary',
        lvCancelType: 'default',
        lvOkDisabled: false,
        lvFocusButtonId: 'cancel',
        lvCloseButtonDisplay: true,
        lvCancel: modal => {
          if (options.onCancel) {
            options.onCancel();
          }
        },
        lvOk: modal => options.onOK(),
        lvAfterClose: result => {
          if (options.lvAfterClose) {
            options.lvAfterClose(result);
          }
        }
      }
    });
  }
}
