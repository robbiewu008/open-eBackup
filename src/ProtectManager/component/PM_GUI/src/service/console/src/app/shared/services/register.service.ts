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
import { DataMap, I18NService, MODAL_COMMON } from 'app/shared';
import { DrawModalService } from 'app/shared/services/draw-modal.service';
import { assign, includes, isEmpty, mapValues } from 'lodash';
import { combineLatest } from 'rxjs';

export interface RegisterParams {
  subType: any;
  component: any;
  refreshData: () => any;
  refreshDetail: (item) => any;
  rowData?: any;
}

@Injectable({
  providedIn: 'root'
})
export class RegisterService {
  constructor(
    private drawModalService: DrawModalService,
    private i18n: I18NService
  ) {}

  register(option: RegisterParams) {
    this.drawModalService.create(
      assign({}, MODAL_COMMON.generateDrawerOptions(), {
        ...this._getDefaultModalConfig(option),
        ...this._getCustomModalConfig(option)
      })
    );
  }

  private _getDefaultModalConfig(option) {
    return {
      lvModalKey: 'reigster-resource',
      lvWidth: MODAL_COMMON.normalWidth + 100,
      lvHeader: isEmpty(option.rowData)
        ? includes(
            [
              DataMap.Resource_Type.commonShare.value,
              DataMap.Resource_Type.volume.value,
              DataMap.Resource_Type.ndmp.value
            ],
            option.subType
          )
          ? this.i18n.get('common_create_label')
          : this.i18n.get('common_register_label')
        : this.i18n.get('common_modify_label'),
      lvContent: option.component,
      lvOkDisabled: true,
      lvComponentParams: {
        rowData: option.rowData,
        option: option
      },
      lvAfterOpen: modal => {
        const content = modal.getContentComponent();
        const modalIns = modal.getInstance();

        content.formGroup.statusChanges.subscribe(res => {
          modalIns.lvOkDisabled = res === 'INVALID';
        });
        content.formGroup.updateValueAndValidity();
      },
      lvOk: modal => {
        return new Promise(resolve => {
          const content = modal.getContentComponent();

          content.onOK().subscribe({
            next: res => {
              resolve(true);
              if (
                !isEmpty(option.rowData) &&
                includes(
                  mapValues(this.drawModalService.modals, 'key'),
                  'detail-modal'
                )
              ) {
                option.refreshDetail(option.rowData);
              } else {
                option.refreshData();
              }
            },
            error: () => resolve(false)
          });
        });
      },
      lvCancel: modal => {
        if (
          !isEmpty(option.rowData) &&
          includes(
            mapValues(this.drawModalService.modals, 'key'),
            'detail-modal'
          )
        ) {
          option.refreshDetail(option.rowData);
        }
      }
    };
  }

  private _getCustomModalConfig(option) {
    switch (option.subType) {
      case DataMap.Resource_Type.dbTwoInstance.value:
      case DataMap.Resource_Type.dbTwoClusterInstance.value:
        return {
          lvComponentParams: {
            item: option.rowData
          }
        };
      case DataMap.Resource_Type.dbTwoTableSet.value:
      case DataMap.Resource_Type.OceanBaseTenant.value:
        return {
          lvWidth: MODAL_COMMON.largeWidth + 50,
          lvHeader: isEmpty(option.rowData)
            ? this.i18n.get('common_create_label')
            : this.i18n.get('common_modify_label'),
          lvComponentParams: {
            data: option.rowData
          }
        };
      case DataMap.Resource_Type.DWS_Schema.value:
      case DataMap.Resource_Type.DWS_Table.value:
      case DataMap.Resource_Type.oraclePDB.value:
        return {
          lvWidth: MODAL_COMMON.largeWidth,
          lvHeader: isEmpty(option.rowData)
            ? this.i18n.get('common_create_label')
            : this.i18n.get('common_modify_label'),
          lvComponentParams: {
            type: option.subType,
            data: option.rowData
          }
        };
      case DataMap.Resource_Type.goldendbInstance.value:
        return {
          lvWidth: MODAL_COMMON.normalWidth + 200,
          lvHeader: isEmpty(option.rowData)
            ? this.i18n.get('common_create_label')
            : this.i18n.get('common_modify_label'),
          lvAfterOpen: modal => {
            const content = modal.getContentComponent();
            const modalIns = modal.getInstance();
            const combined: any = combineLatest(
              content.formGroup.statusChanges,
              content.valid$
            );
            combined.subscribe(latestValues => {
              const [formGroupStatus, validFile] = latestValues;

              if (!!content.rowData) {
                modalIns.lvOkDisabled = formGroupStatus !== 'VALID';
              } else {
                modalIns.lvOkDisabled = !(
                  formGroupStatus === 'VALID' && validFile
                );
              }
            });
            content.valid$.next(false);
            content.formGroup.updateValueAndValidity();
          }
        };
      case DataMap.Resource_Type.tdsqlCluster.value:
      case DataMap.Resource_Type.OceanBaseCluster.value:
        return {
          lvWidth: MODAL_COMMON.largeWidth
        };
      case DataMap.Resource_Type.tdsqlInstance.value:
        return {
          lvWidth: MODAL_COMMON.xLargeWidth + 200
        };
      case DataMap.Resource_Type.tdsqlDistributedInstance.value:
        return {
          lvWidth: MODAL_COMMON.normalWidth
        };
      case DataMap.Resource_Type.tidbTable.value:
        return {
          lvWidth: this.i18n.isEn
            ? MODAL_COMMON.largeWidth + 30
            : MODAL_COMMON.largeWidth
        };
      case DataMap.Resource_Type.lightCloudGaussdbProject.value:
        return {
          lvWidth: MODAL_COMMON.largeWidth + 80,
          lvAfterOpen: modal => {
            const content = modal.getContentComponent();
            const modalIns = modal.getInstance();
            const combined: any = combineLatest(
              content.formGroup.statusChanges,
              content.valid$
            );
            combined.subscribe(latestValues => {
              const [formGroupStatus, fileStatus] = latestValues;
              modalIns.lvOkDisabled = !(
                formGroupStatus === 'VALID' && fileStatus
              );
            });
            content.valid$.next(false);
            content.formGroup.updateValueAndValidity();
          }
        };
      case DataMap.Resource_Type.tidbCluster.value:
        return {
          lvWidth: MODAL_COMMON.largeWidth
        };
      case DataMap.Resource_Type.volume.value:
        return {
          lvWidth: MODAL_COMMON.xLargeWidth,
          lvAfterOpen: modal => {
            const content = modal.getContentComponent();
            const modalIns = modal.getInstance();

            content.formGroup.statusChanges.subscribe(res => {
              modalIns.lvOkDisabled = res === 'INVALID';
            });
            content.formGroup.updateValueAndValidity();
          }
        };
      case DataMap.Resource_Type.ObjectSet.value:
        return {
          lvWidth: MODAL_COMMON.largeWidth + 100
        };
      case DataMap.Resource_Type.commonShare.value:
        return {
          lvWidth: MODAL_COMMON.smallModal
        };
      default:
        return {};
    }
  }
}

@NgModule({
  imports: [CommonModule],
  providers: [RegisterService]
})
export class RegisterModule {}
