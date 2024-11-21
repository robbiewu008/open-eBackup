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
import { Injectable, NgModule } from '@angular/core';
import { DrawModalService } from './draw-modal.service';
import { cloneDeep, assign, map, isEmpty, toString, isBoolean } from 'lodash';
import { MODAL_COMMON, I18NService, DataMap } from '..';
import { BatchResultsComponent } from '../components/batch-results/batch-results.component';
import { BatchOperationResult } from '../api/models';
import { CommonModule } from '@angular/common';
import { BatchResultsModule } from '../components/batch-results/batch-results.module';

@Injectable({
  providedIn: 'root'
})
export class BatchOperateService {
  constructor(
    private i18n: I18NService,
    private drawModalService: DrawModalService
  ) {}

  create(
    res: BatchOperationResult,
    callback?: () => void,
    customLabel?: string
  ) {
    const tableData = cloneDeep(res.results);
    map(tableData, (object: any) => {
      return assign(object, {
        status:
          toString(object.errorCode) === '0'
            ? DataMap.Batch_Result_Status.successful.value
            : DataMap.Batch_Result_Status.fail.value,
        name: this.i18n.get(
          object.targetName,
          isEmpty(object.targetNameParam) ? [] : [...object.targetNameParam]
        ),
        desc: !!object.errorCode
          ? this.i18n.get(object.errorCode, object.detailParam)
          : '--'
      });
    });
    this.drawModalService.create({
      lvModalKey: 'batch',
      lvWidth: MODAL_COMMON.normalWidth + 100,
      lvContent: BatchResultsComponent,
      lvFooter: [
        {
          id: 'close',
          label: this.i18n.get('common_close_label'),
          onClick: (modal, button) => modal.close()
        }
      ],
      lvComponentParams: {
        tableData,
        customLabel
      },
      lvAfterClose: result => {
        if (callback) {
          callback();
        }
      }
    });
  }

  selfGetResults(
    action: (any) => any,
    source = [],
    callback?: () => any,
    customLabel?: string,
    isSynExecute = false,
    syncNumber?: number,
    extendCols = [],
    needGetDetection = false
  ) {
    this.drawModalService.create({
      lvModalKey: 'batchResultModal',
      lvWidth: MODAL_COMMON.normalWidth,
      lvContent: BatchResultsComponent,
      lvCloseButtonDisplay: false,
      lvFooter: [
        {
          id: 'close',
          label: this.i18n.get('common_close_label'),
          disabled: true,
          onClick: (modal, button) => modal.close()
        }
      ],
      lvComponentParams: {
        doAction: action,
        sourceData: source,
        customLabel,
        isSynExecute,
        syncNumber,
        extendCols,
        needGetDetection
      },
      lvAfterClose: result => callback()
    });
  }
}

@NgModule({
  imports: [CommonModule, BatchResultsModule],
  providers: [BatchOperateService]
})
export class BatchOperateServiceModule {}
