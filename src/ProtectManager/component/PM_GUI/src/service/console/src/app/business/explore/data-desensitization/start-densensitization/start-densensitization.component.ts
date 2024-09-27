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
import { Component, OnInit } from '@angular/core';
import {
  AnonyControllerService,
  DataMap,
  DesensitizationSourceType,
  DESENSITIZATION_POLICY_DESC_MAP,
  I18NService,
  MODAL_COMMON,
  PolicyControllerService,
  WarningMessageService
} from 'app/shared';
import { DrawModalService } from 'app/shared/services/draw-modal.service';
import {
  assign,
  cloneDeep,
  each,
  filter,
  find,
  first,
  isString,
  now,
  size
} from 'lodash';
import { AnonymizationVerificateComponent } from './anonymization-verificate/anonymization-verificate.component';
import { ModifyIdentificationResultComponent } from './modify-identification-result/modify-identification-result.component';
import { VerificateResultComponent } from './verificate-result/verificate-result.component';
import { AppUtilsService } from 'app/shared/services/app-utils.service';

@Component({
  selector: 'aui-start-densensitization',
  templateUrl: './start-densensitization.component.html',
  styleUrls: ['./start-densensitization.component.less']
})
export class StartDensensitizationComponent implements OnInit {
  rowItem;
  lvShowCheckbox = false;
  tableNamespace;
  tableName;
  resultTree = [];
  selection = [];
  selectedMode = '0';
  validateDisabled = true;
  policyDescription;
  validateTip = this.i18n.get('explore_validation_tip_label');
  columns = [
    {
      label: this.i18n.get('explore_database_table_space_label'),
      key: 'table_namespace'
    },
    {
      label: this.i18n.get('common_table_label'),
      key: 'table_name'
    },
    {
      label: this.i18n.get('explore_sensitive_data_label'),
      key: 'sensitive_count'
    }
  ];
  constructor(
    private i18n: I18NService,
    private appUtilsService: AppUtilsService,
    private drawModalService: DrawModalService,
    private statisticsApiService: AnonyControllerService,
    private warningMessageService: WarningMessageService,
    private policyManagerApiService: PolicyControllerService
  ) {}

  getTreeData() {
    this.lvShowCheckbox = false;
    this.statisticsApiService
      .getIdentResultUsingGET({
        dbId: this.rowItem.uuid,
        dbName: this.rowItem.name,
        dbType: DesensitizationSourceType[this.rowItem.sub_type]
      })
      .subscribe(res => {
        const spaces = [];
        each(res.items, item => {
          spaces.push({
            label: item,
            type: 'space'
          });
        });
        this.resultTree = [
          {
            label: this.rowItem.name,
            icon: 'aui-icon-database',
            type: 'root',
            expanded: true,
            children: spaces
          }
        ];
      });
  }

  refresh() {
    this.getTreeData();
  }

  getPolicyDetail() {
    this.policyManagerApiService
      .getPolicyDetailsUsingGET({
        policyId: this.rowItem.desesitization_policy_id,
        akDoException: false
      })
      .subscribe(res => {
        this.policyDescription =
          res.create_method ===
          DataMap.Senesitization_Create_Method.preset.value
            ? this.i18n.get(DESENSITIZATION_POLICY_DESC_MAP[res.name])
            : res.description;
      });
  }

  downloadData() {
    this.statisticsApiService
      .downloadAnonyReportUsingPOST({
        reportRequest: {
          db_id: this.rowItem.uuid,
          db_type: DesensitizationSourceType[this.rowItem.sub_type]
        }
      })
      .subscribe(blob => {
        const bf = new Blob([blob], {
          type: 'text/csv'
        });
        this.appUtilsService.downloadFile(`report_${now()}.csv`, bf);
      });
  }

  expandedChange(node) {
    if (!node.expanded || !!size(node.children)) {
      return;
    }
    if (node.type === 'space') {
      this.statisticsApiService
        .getIdentResultUsingGET({
          dbId: this.rowItem.uuid,
          dbName: this.rowItem.name,
          dbType: DesensitizationSourceType[this.rowItem.sub_type],
          tableSpace: node.label
        })
        .subscribe(res => {
          const tables = [];
          each(res.items, item => {
            tables.push({
              label: item,
              tableSpace: node.label,
              type: 'table'
            });
          });
          node.children = tables;
          this.resultTree = [...this.resultTree];
        });
    } else {
      this.statisticsApiService
        .getIdentResultUsingGET({
          dbId: this.rowItem.uuid,
          dbName: this.rowItem.name,
          dbType: DesensitizationSourceType[this.rowItem.sub_type],
          tableSpace: node.tableSpace,
          tableName: node.label
        })
        .subscribe((res: any) => {
          this.lvShowCheckbox = true;
          const columns = [];
          each(res.items, item => {
            columns.push({
              label: item.column_name,
              type: 'column',
              pii: first(isString(item.pii) ? JSON.parse(item.pii) : item.pii),
              mask_rule_id: item.mask_rule_id,
              is_mask_type_match_column_type:
                item.is_mask_type_match_column_type,
              isLeaf: true
            });
          });
          node.children = columns;
          this.resultTree = [...this.resultTree];
        });
    }
  }

  selectionChange() {
    let isCross = false;
    let column = this.getSelection();
    let table: any = filter(this.selection, item => {
      return item.type === 'table';
    });
    if (!!size(column)) {
      let tmp = column[0].parent.label;
      if (find(column, item => item.parent.label !== tmp)) {
        isCross = true;
      }
    }

    if (!!size(table) && size(table) > 1) {
      isCross = true;
    }

    if (!!size(table) && size(table) === 1 && !!size(column)) {
      if (!find(column, item => item.parent.label === table.label)) {
        isCross = true;
      }
    }

    this.validateDisabled =
      !size(this.getSelection()) || size(this.getSelection()) > 3 || isCross;
  }

  getSelection() {
    return filter(this.selection, item => {
      return item.isLeaf;
    });
  }

  validate() {
    this.drawModalService.create({
      ...MODAL_COMMON.generateDrawerOptions(),
      lvHeader: this.i18n.get('common_validation_label'),
      lvContent: AnonymizationVerificateComponent,
      lvOkDisabled: true,
      lvWidth: MODAL_COMMON.normalWidth + 100,
      lvAfterOpen: modal => {
        const content = modal.getContentComponent() as AnonymizationVerificateComponent;
        content.formGroup.statusChanges.subscribe(res => {
          modal.lvOkDisabled = res !== 'VALID';
        });
      },
      lvComponentParams: {
        rowItem: assign({}, this.rowItem, {
          columns: this.getSelection(),
          isVerificationPost: true
        })
      },
      lvOk: modal => {
        return new Promise(resolve => {
          const content = modal.getContentComponent() as AnonymizationVerificateComponent;
          content.verificate().subscribe(
            res => {
              resolve(true);
              this.showVerificateData(res);
            },
            err => {
              resolve(false);
            }
          );
        });
      }
    });
  }

  showVerificateData(res) {
    this.drawModalService.create({
      ...MODAL_COMMON.generateDrawerOptions(),
      lvHeader: this.i18n.get('explore_verificate_result_label'),
      lvContent: VerificateResultComponent,
      lvOkDisabled: true,
      lvWidth: MODAL_COMMON.normalWidth + 100,
      lvComponentParams: {
        rowItem: res
      },
      lvFooter: [
        {
          label: this.i18n.get('common_close_label'),
          onClick: modal => modal.close()
        }
      ]
    });
  }

  modify(item) {
    this.drawModalService.create({
      ...MODAL_COMMON.generateDrawerOptions(),
      lvHeader: this.i18n.get('common_modify_label'),
      lvContent: ModifyIdentificationResultComponent,
      lvOkDisabled: true,
      lvWidth: MODAL_COMMON.normalWidth + 100,
      lvAfterOpen: modal => {
        const content = modal.getContentComponent() as ModifyIdentificationResultComponent;
        content.formGroup.statusChanges.subscribe(res => {
          modal.lvOkDisabled = res !== 'VALID';
        });
      },
      lvComponentParams: {
        rowItem: assign({}, this.rowItem, item)
      },
      lvOk: modal => {
        return new Promise(resolve => {
          const content = modal.getContentComponent() as ModifyIdentificationResultComponent;
          content.modify().subscribe(
            res => {
              resolve(true);
              this.refresh();
            },
            err => {
              resolve(false);
            }
          );
        });
      }
    });
  }

  desensitizate(lastModal, callback: () => void) {
    this.drawModalService.create({
      ...MODAL_COMMON.generateDrawerOptions(),
      lvHeader: this.i18n.get('common_auth_label'),
      lvContent: AnonymizationVerificateComponent,
      lvOkDisabled: true,
      lvWidth: MODAL_COMMON.normalWidth + 100,
      lvAfterOpen: modal => {
        const content = modal.getContentComponent() as AnonymizationVerificateComponent;
        content.formGroup.statusChanges.subscribe(res => {
          modal.lvOkDisabled = res !== 'VALID';
        });
      },
      lvComponentParams: {
        rowItem: cloneDeep(this.rowItem)
      },
      lvOk: modal => {
        return new Promise(resolve => {
          const content = modal.getContentComponent() as AnonymizationVerificateComponent;
          this.warningMessageService.create({
            content: this.i18n.get('explore_start_desensitize_warn_label', [
              content.rowItem ? content.rowItem.name : ''
            ]),
            onOK: () => {
              content.desensitizate().subscribe(
                res => {
                  resolve(true);
                  if (lastModal.close) {
                    lastModal.close();
                  }
                  callback();
                },
                err => {
                  resolve(false);
                }
              );
            },
            onCancel: () => resolve(false),
            lvAfterClose: result => {
              if (result && result.trigger === 'close') {
                resolve(false);
              }
            }
          });
        });
      }
    });
  }

  ngOnInit() {
    this.getPolicyDetail();
    this.getTreeData();
  }
}
