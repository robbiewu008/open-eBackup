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
import { Component, Input, OnInit } from '@angular/core';
import { FormBuilder } from '@angular/forms';
import {
  RestoreType,
  MODAL_COMMON,
  CopiesService,
  getPermissionMenuItem,
  OperateItems,
  DataMap
} from 'app/shared';
import {
  BaseUtilService,
  I18NService,
  CookieService
} from 'app/shared/services';
import { RestoreService } from 'app/shared/services/restore.service';
import { DrawModalService } from 'app/shared/services/draw-modal.service';
import { assign, size, first, each, includes } from 'lodash';
import { CopyDataScnSelectComponent } from '../copy-data-scn-select/copy-data-scn-select.component';
import { CopyControllerService, CopyService } from 'app/shared/api/services';

@Component({
  selector: 'aui-copy-data-scn',
  templateUrl: './copy-data-scn.component.html',
  styleUrls: ['./copy-data-scn.component.less']
})
export class CopyDataScnComponent implements OnInit {
  @Input() rowData;
  @Input() resType;
  restoreType = RestoreType;
  dataMap = DataMap;
  scnErrorTip = {
    ...this.baseUtilService.requiredErrorTip,
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [15]),
    invalidName: this.i18n.get('common_valid_pure_number_label')
  };
  formGroup;
  operation;

  constructor(
    private i18n: I18NService,
    private fb: FormBuilder,
    private baseUtilService: BaseUtilService,
    private restoreService: RestoreService,
    private drawModalService: DrawModalService,
    private copiesApiService: CopiesService,
    private cookieService: CookieService,
    private copyService: CopyService
  ) {}

  ngOnInit() {
    this.getOperation();
    this.initForm();
  }

  hideOracleOpt() {
    return (
      includes(
        [
          DataMap.Resource_Type.oracle.value,
          DataMap.Resource_Type.oracleCluster.value
        ],
        this.rowData.sub_type
      ) && this.rowData.environment?.osType === DataMap.Os_Type.windows.value
    );
  }

  getOperation() {
    this.operation = getPermissionMenuItem(
      [
        {
          id: 'restore',
          label: this.i18n.get('common_restore_label'),
          permission: OperateItems.RestoreCopy,
          disabled: true,
          onClick: () => {
            this.scnRestore(RestoreType.CommonRestore);
          }
        },
        {
          id: 'instantRestore',
          permission: OperateItems.InstanceRecovery,
          label: this.i18n.get('common_live_restore_job_label'),
          disabled: true,
          hidden:
            this.rowData.version?.substring(0, 2) === '11' ||
            this.hideOracleOpt(),
          onClick: () => {
            this.scnRestore(RestoreType.InstanceRestore);
          }
        }
      ],
      this.cookieService.role
    );
  }

  operationFn = () => {
    return this.operation;
  };

  initForm() {
    this.formGroup = this.fb.group({
      scn: [
        '',
        [
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.maxLength(15),
          this.baseUtilService.VALID.name(/^[1-9]\d*$/, false)
        ]
      ]
    });
    this.formGroup.statusChanges.subscribe(res => {
      if ('VALID' === res) {
        this.operation.map(item => {
          item.disabled = false;
          return item;
        });
      } else {
        this.operation.map(item => {
          item.disabled = true;
          return item;
        });
      }
      this.operationFn = () => {
        return this.operation;
      };
    });
  }

  scnRestore(resType) {
    this.copyService
      .ListOracleCopiesByScn({
        resourceId: this.rowData.uuid,
        filterValue: this.formGroup.value.scn
      })
      .subscribe((res: any) => {
        each(res, item => {
          assign(item, {
            timestamp: item.timestamp * 1000
          });
        });
        if (res.length === 1) {
          const copy = <any>first(res);
          this.copiesApiService
            .queryResourcesV1CopiesGet({
              pageNo: 0,
              pageSize: 1,
              conditions: JSON.stringify({ uuid: copy.id })
            })
            .subscribe(res => {
              this.restoreService.restore({
                childResType: this.rowData.sub_type,
                copyData: {
                  ...this.rowData,
                  uuid: copy.id,
                  display_timestamp: copy.timestamp,
                  dbUuid: this.rowData.uuid,
                  resource_type: this.rowData.sub_type,
                  resource_sub_type: this.rowData.sub_type,
                  environment_os_type: this.rowData.environment_os_type,
                  environment_uuid: this.rowData.environment_uuid,
                  parent_uuid: this.rowData.parent_uuid,
                  path: this.rowData.path,
                  name: this.rowData.name,
                  resource_id: this.rowData.uuid,
                  resource_properties: JSON.stringify(this.rowData),
                  properties: JSON.stringify({
                    oracle_metadata: {
                      ORACLEPARAM: {
                        db_name: this.rowData.name,
                        version: this.rowData.version
                      }
                    }
                  }),
                  scn: this.formGroup.value.scn,
                  rowCoyByScn: first(res.items)
                },
                restoreType: resType
              });
            });
        } else {
          this.drawModalService.create(
            assign({}, MODAL_COMMON.generateDrawerOptions(), {
              lvHeader: this.i18n.get('common_select_copydata_label'),
              lvWidth: MODAL_COMMON.normalWidth,
              lvOkDisabled: true,
              lvContent: CopyDataScnSelectComponent,
              lvComponentParams: {
                rowData: this.rowData,
                restoreType: resType,
                tableData: res,
                scn: this.formGroup.value.scn
              },
              lvAfterOpen: modal => {
                const content = modal.getContentComponent() as CopyDataScnSelectComponent;
                content.selection$.subscribe(selection => {
                  modal.lvOkDisabled = !size(selection);
                });
              },
              lvOk: modal => {
                const content = modal.getContentComponent() as CopyDataScnSelectComponent;
                content.restore();
                return false;
              }
            })
          );
        }
      });
  }
}
