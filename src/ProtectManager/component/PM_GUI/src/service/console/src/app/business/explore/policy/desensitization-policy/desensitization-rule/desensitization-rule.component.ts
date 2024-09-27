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
import {
  ChangeDetectionStrategy,
  ChangeDetectorRef,
  Component,
  OnInit,
  Pipe,
  PipeTransform
} from '@angular/core';
import { MessageboxService } from '@iux/live';
import {
  CommonConsts,
  CookieService,
  DataMap,
  DataMapService,
  getPermissionMenuItem,
  GROUP_COMMON,
  I18NService,
  MaskRuleControllerService,
  MODAL_COMMON,
  OperateItems,
  RoleOperationMap
} from 'app/shared';
import { BatchOperateService } from 'app/shared/services/batch-operate.service';
import { DrawModalService } from 'app/shared/services/draw-modal.service';
import { InfoMessageService } from 'app/shared/services/info-message.service';
import { VirtualScrollService } from 'app/shared/services/virtual-scroll.service';
import { assign, each, filter, includes, isEmpty, size } from 'lodash';
import { AddRuleComponent } from './add-rule/add-rule.component';
import { RelatedIdentifiedRuleComponent } from './related-identified-rule/related-identified-rule.component';

@Pipe({ name: 'selectionPipe' })
export class SelectionPipe implements PipeTransform {
  constructor() {}
  transform(value: any[], method: string = 'create_method') {
    return filter(
      value,
      item =>
        item[method] ===
          DataMap.Senesitization_Create_Method.customized.value &&
        item['ref_num'] === 0
    );
  }
}

@Component({
  selector: 'aui-desensitization-rule',
  templateUrl: './desensitization-rule.component.html',
  styleUrls: ['./desensitization-rule.component.less'],
  changeDetection: ChangeDetectionStrategy.OnPush,
  providers: [SelectionPipe]
})
export class DesensitizationRuleComponent implements OnInit {
  tableData = [];
  selection = [];
  pageIndex = CommonConsts.PAGE_START;
  pageSize = CommonConsts.PAGE_SIZE;
  total = CommonConsts.PAGE_TOTAL;
  sizeOptions = CommonConsts.PAGE_SIZE_OPTIONS;
  dataMap = DataMap;
  ruleName;
  filterParams = {};
  columns = [
    {
      label: this.i18n.get('common_name_label'),
      key: 'name'
    },
    {
      label: this.i18n.get('explore_anonymization_rule_mode_label'),
      key: 'create_method',
      filterMap: this.dataMapService.toArray('Senesitization_Create_Method')
    },
    {
      label: this.i18n.get('common_type_label'),
      key: 'type',
      filterMap: this.dataMapService.toArray('Desensitization_Rule_Type')
    },
    {
      label: this.i18n.get('explore_rule_description_label'),
      key: 'type_description'
    },
    {
      label: this.i18n.get('explore_rule_details_label'),
      key: 'example'
    },
    {
      label: this.i18n.get('explore_identification_rel_number_label'),
      key: 'ref_num'
    }
  ];
  groupCommon = GROUP_COMMON;
  roleOperationMap = RoleOperationMap;
  constructor(
    private i18n: I18NService,
    private drawModalService: DrawModalService,
    private policyManagerApiService: MaskRuleControllerService,
    private dataMapService: DataMapService,
    private batchOperateService: BatchOperateService,
    private cookieService: CookieService,
    private infoMessageService: InfoMessageService,
    public virtualScroll: VirtualScrollService,
    private cdr: ChangeDetectorRef
  ) {}

  getRules() {
    this.selection = [];
    const params = {
      pageNo: this.pageIndex,
      pageSize: this.pageSize
    };
    if (this.ruleName) {
      assign(params, {
        name: this.ruleName
      });
    }
    each(this.filterParams, (value, key) => {
      if (isEmpty(value)) {
        delete this.filterParams[key];
      }
    });
    this.policyManagerApiService
      .getPageMaskRulesUsingGET({
        ...params,
        ...this.filterParams
      })
      .subscribe(res => {
        this.tableData = res.items;
        this.total = res.total;
        this.cdr.detectChanges();
      });
  }

  searchByName(name) {
    this.ruleName = name;
    this.getRules();
  }

  filterChange(e) {
    if (e.key === 'create_method') {
      assign(this.filterParams, {
        createMethod: e.value
      });
    }
    if (e.key === 'type') {
      assign(this.filterParams, {
        maskType: e.value
      });
    }
    this.getRules();
  }

  getRelNum(item) {
    this.drawModalService.create({
      ...MODAL_COMMON.generateDrawerOptions(),
      lvHeader: this.i18n.get('explore_identification_rel_label'),
      lvContent: RelatedIdentifiedRuleComponent,
      lvWidth: MODAL_COMMON.normalWidth + 100,
      lvComponentParams: {
        rowItem: item
      },
      lvFooter: [
        {
          label: this.i18n.get('common_close_label'),
          onClick: modal => {
            modal.close();
          }
        }
      ]
    });
  }

  optsCallback = data => {
    const menus = [
      {
        id: 'modify',
        hidden:
          data.create_method ===
          DataMap.Senesitization_Create_Method.preset.value,
        disabled: data.ref_num > 0,
        label: this.i18n.get('common_modify_label'),
        permission: OperateItems.ModifyDesensitizationRule,
        onClick: () => this.modify(data)
      },
      {
        id: 'delete',
        hidden:
          data.create_method ===
          DataMap.Senesitization_Create_Method.preset.value,
        disabled: data.ref_num > 0,
        label: this.i18n.get('common_delete_label'),
        permission: OperateItems.DeleteDesensitizationRule,
        onClick: () => this.delete([data])
      },
      {
        id: 'clone',
        label: this.i18n.get('common_clone_label'),
        permission: OperateItems.CloneDesensitizationRule,
        disabled: !includes(
          [
            DataMap.Desensitization_Rule_Type.fullMask.value,
            DataMap.Desensitization_Rule_Type.partialMask.value,
            DataMap.Desensitization_Rule_Type.numberic.value,
            DataMap.Desensitization_Rule_Type.fixedNumber.value
          ],
          data.type
        ),
        onClick: () => this.modify(data, true)
      }
    ];
    return getPermissionMenuItem(menus, this.cookieService.role);
  };

  create() {
    this.drawModalService.create({
      ...MODAL_COMMON.generateDrawerOptions(),
      lvHeader: this.i18n.get('explore_add_desensitize_rule_label'),
      lvContent: AddRuleComponent,
      lvOkDisabled: true,
      lvWidth: MODAL_COMMON.normalWidth + 100,
      lvAfterOpen: modal => {
        const content = modal.getContentComponent() as AddRuleComponent;
        content.formGroup.statusChanges.subscribe(res => {
          modal.lvOkDisabled = res !== 'VALID';
        });
      },
      lvOk: modal => {
        return new Promise(resolve => {
          const content = modal.getContentComponent() as AddRuleComponent;
          content.create().subscribe(
            res => {
              resolve(true);
              this.getRules();
            },
            error => resolve(false)
          );
        });
      }
    });
  }

  modify(item, isClone?) {
    this.drawModalService.create({
      ...MODAL_COMMON.generateDrawerOptions(),
      lvHeader: isClone
        ? this.i18n.get('common_clone_label')
        : this.i18n.get('common_modify_label'),
      lvContent: AddRuleComponent,
      lvOkDisabled: false,
      lvWidth: MODAL_COMMON.normalWidth + 100,
      lvAfterOpen: modal => {
        const content = modal.getContentComponent() as AddRuleComponent;
        content.formGroup.statusChanges.subscribe(res => {
          modal.lvOkDisabled = res !== 'VALID';
        });
        if (isClone) {
          content.formGroup.get('name').markAsTouched();
          content.formGroup.get('name').updateValueAndValidity();
        }
      },
      lvComponentParams: {
        rowItem: assign({}, item, { isClone })
      },
      lvOk: modal => {
        return new Promise(resolve => {
          const content = modal.getContentComponent() as AddRuleComponent;
          if (isClone) {
            content.create().subscribe(
              res => {
                resolve(true);
                this.getRules();
              },
              error => resolve(false)
            );
          } else {
            content.modify().subscribe(
              res => {
                resolve(true);
                this.getRules();
              },
              error => resolve(false)
            );
          }
        });
      }
    });
  }

  delete(datas) {
    this.infoMessageService.create({
      content: this.i18n.get('explore_desensitization_delete_label', [
        this.i18n.get('explore_desensitize_rule_label'),
        this.i18n.get('explore_desensitize_rule_label'),
        this.i18n.get('explore_desensitize_rule_label')
      ]),
      onOK: () => {
        if (size(datas) === 1) {
          this.policyManagerApiService
            .deleteMaskRuleUsingDELETE({
              id: datas[0].id
            })
            .subscribe(res => {
              this.getRules();
            });
        } else {
          this.batchOperateService.selfGetResults(
            item => {
              return this.policyManagerApiService.deleteMaskRuleUsingDELETE({
                id: item.id,
                akDoException: false,
                akOperationTips: false,
                akLoading: false
              });
            },
            datas,
            () => {
              this.getRules();
            }
          );
        }
      }
    });
  }

  pageChange(page) {
    this.pageSize = page.pageSize;
    this.pageIndex = page.pageIndex;
    this.getRules();
  }

  ngOnInit() {
    this.getRules();
    this.virtualScroll.getScrollParam(270);
  }
}
