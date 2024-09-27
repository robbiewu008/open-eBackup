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
import { CommonModule } from '@angular/common';
import { Injectable, NgModule } from '@angular/core';
import { assign, isFunction } from 'lodash';
import { I18NService } from '.';
import { MODAL_COMMON, SlaApiService } from '..';
import { TakeManualArchiveComponent } from '../components/take-manual-archive/take-manual-archive.component';
import { TakeManualArchiveModule } from '../components/take-manual-archive/take-manual-archive.module';
import { DrawModalService } from './draw-modal.service';

@Injectable({
  providedIn: 'root'
})
export class TakeManualArchiveService {
  constructor(
    public slaApiService: SlaApiService,
    public drawModalService: DrawModalService,
    public i18n: I18NService
  ) {}

  manualArchive(data, callback: () => void) {
    this.drawModalService.create(
      assign({}, MODAL_COMMON.generateDrawerOptions(), {
        lvWidth: MODAL_COMMON.normalWidth,
        lvOkDisabled: true,
        lvHeader: this.i18n.get('common_manual_archive_label'),
        lvContent: TakeManualArchiveComponent,
        lvComponentParams: {
          data
        },
        lvAfterOpen: modal => {
          const content = modal.getContentComponent() as TakeManualArchiveComponent;
          const modalIns = modal.getInstance();
          content.formGroup.statusChanges.subscribe(res => {
            modalIns.lvOkDisabled = res !== 'VALID';
          });
          content.formGroup.updateValueAndValidity();
        },
        lvOk: modal => {
          return new Promise(resolve => {
            const content = modal.getContentComponent() as TakeManualArchiveComponent;
            content.onOK().subscribe({
              next: () => {
                resolve(true);
                if (isFunction(callback)) {
                  callback();
                }
              },
              error: () => resolve(false)
            });
          });
        }
      })
    );
  }
}

@NgModule({
  imports: [CommonModule, TakeManualArchiveModule],
  providers: [TakeManualArchiveService]
})
export class TakeManualArchiveServiceModule {}
