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
  PipeTransform,
  ViewChild
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
  IdentRuleControllerService,
  MODAL_COMMON,
  OperateItems,
  RoleOperationMap
} from 'app/shared';
import { BatchOperateService } from 'app/shared/services/batch-operate.service';
import { DrawModalService } from 'app/shared/services/draw-modal.service';
import { InfoMessageService } from 'app/shared/services/info-message.service';
import { VirtualScrollService } from 'app/shared/services/virtual-scroll.service';
import { assign, each, filter, isEmpty, size } from 'lodash';
import { combineLatest } from 'rxjs';
import { AddIdentifiedRuleComponent } from './add-identified-rule/add-identified-rule.component';
import { RelatedDesensitizationPolicyComponent } from './related-desensitization-policy/related-desensitization-policy.component';

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
  selector: 'aui-identified-rule',
  templateUrl: './identified-rule.component.html',
  styleUrls: ['./identified-rule.component.less'],
  changeDetection: ChangeDetectionStrategy.OnPush,
  providers: [SelectionPipe]
})
export class IdentifiedRuleComponent implements OnInit {
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
      label: this.i18n.get('explore_expression_label'),
      key: 'expression'
    },
    {
      label: this.i18n.get('explore_desensitize_rule_label'),
      key: 'mask_name'
    },
    {
      label: this.i18n.get('explore_desensitization_policy_rel_number_label'),
      key: 'ref_num'
    }
  ];
  groupCommon = GROUP_COMMON;
  roleOperationMap = RoleOperationMap;
  constructor(
    private i18n: I18NService,
    private drawModalService: DrawModalService,
    private policyManagerApiService: IdentRuleControllerService,
    private dataMapService: DataMapService,
    private batchOperateService: BatchOperateService,
    private cookieService: CookieService,
    private messageBox: MessageboxService,
    private infoMessageService: InfoMessageService,
    public virtualScroll: VirtualScrollService,
    private cdr: ChangeDetectorRef
  ) {}

  pageChange(page) {
    this.pageSize = page.pageSize;
    this.pageIndex = page.pageIndex;
    this.getTableData();
  }

  getTableData() {
    this.selection = [];
    const params = {
      pageSize: this.pageSize,
      pageNo: this.pageIndex
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
      .getPageIdentRulesUsingGET(assign(params, this.filterParams))
      .subscribe(res => {
        this.tableData = res.items;
        this.total = res.total;
        this.cdr.detectChanges();
      });
  }

  searchByName(name) {
    this.ruleName = name;
    this.getTableData();
  }

  filterChange(e) {
    if (e.key === 'create_method') {
      assign(this.filterParams, {
        createMethod: e.value
      });
    }
    this.getTableData();
  }

  getRelNum(item) {
    this.drawModalService.create({
      ...MODAL_COMMON.generateDrawerOptions(),
      lvHeader: this.i18n.get('explore_desensitization_policy_rel_label'),
      lvContent: RelatedDesensitizationPolicyComponent,
      lvWidth: MODAL_COMMON.normalWidth,
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

  create() {
    this.drawModalService.create({
      ...MODAL_COMMON.generateDrawerOptions(),
      lvHeader: this.i18n.get('explore_add_identified_rule_label'),
      lvContent: AddIdentifiedRuleComponent,
      lvOkDisabled: true,
      lvWidth: MODAL_COMMON.largeWidth,
      lvAfterOpen: modal => {
        const content = modal.getContentComponent() as AddIdentifiedRuleComponent;
        const combined: any = combineLatest(
          content.formGroup.statusChanges,
          content.valid$
        );
        combined.subscribe(latestValues => {
          const [formGroupStatus, valid] = latestValues;
          modal.lvOkDisabled = !(formGroupStatus === 'VALID' && valid);
        });
      },
      lvOk: modal => {
        return new Promise(resolve => {
          const content = modal.getContentComponent() as AddIdentifiedRuleComponent;
          content.create().subscribe(
            res => {
              resolve(true);
              this.getTableData();
            },
            error => resolve(false)
          );
        });
      }
    });
  }

  delete(datas) {
    this.infoMessageService.create({
      content: this.i18n.get('explore_desensitization_delete_label', [
        this.i18n.get('explore_identified_rule_label'),
        this.i18n.get('explore_identified_rule_label'),
        this.i18n.get('explore_identified_rule_label')
      ]),
      onOK: () => {
        if (size(datas) === 1) {
          this.policyManagerApiService
            .deleteIdentRuleUsingDELETE({
              id: datas[0].id
            })
            .subscribe(res => {
              this.getTableData();
            });
        } else {
          this.batchOperateService.selfGetResults(
            item => {
              return this.policyManagerApiService.deleteIdentRuleUsingDELETE({
                id: item.id,
                akDoException: false,
                akOperationTips: false,
                akLoading: false
              });
            },
            datas,
            () => {
              this.getTableData();
            }
          );
        }
      }
    });
  }

  modify(data, isClone?) {
    this.drawModalService.create({
      ...MODAL_COMMON.generateDrawerOptions(),
      lvHeader: isClone
        ? this.i18n.get('common_clone_label')
        : this.i18n.get('common_modify_label'),
      lvContent: AddIdentifiedRuleComponent,
      lvOkDisabled: false,
      lvWidth: MODAL_COMMON.largeWidth,
      lvComponentParams: {
        rowItem: assign({}, data, { isClone })
      },
      lvAfterOpen: modal => {
        const content = modal.getContentComponent() as AddIdentifiedRuleComponent;
        const combined: any = combineLatest(
          content.formGroup.statusChanges,
          content.valid$
        );
        combined.subscribe(latestValues => {
          const [formGroupStatus, valid] = latestValues;
          modal.lvOkDisabled = !(formGroupStatus === 'VALID' && valid);
        });
        content.formGroup.get('name').markAsTouched();
        content.formGroup.get('name').updateValueAndValidity();
        content.valid$.next(true);
      },
      lvOk: modal => {
        return new Promise(resolve => {
          const content = modal.getContentComponent() as AddIdentifiedRuleComponent;
          if (isClone) {
            content.create().subscribe(
              res => {
                resolve(true);
                this.getTableData();
              },
              error => resolve(false)
            );
          } else {
            content.modify().subscribe(
              res => {
                resolve(true);
                this.getTableData();
              },
              error => resolve(false)
            );
          }
        });
      }
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
        permission: OperateItems.ModifyIdentificationRule,
        onClick: () => this.modify(data)
      },
      {
        id: 'delete',
        hidden:
          data.create_method ===
          DataMap.Senesitization_Create_Method.preset.value,
        disabled: data.ref_num > 0,
        label: this.i18n.get('common_delete_label'),
        permission: OperateItems.DeleteIdentificationRule,
        onClick: () => this.delete([data])
      },
      {
        id: 'clone',
        label: this.i18n.get('common_clone_label'),
        permission: OperateItems.CloneIdentificationRule,
        onClick: () => this.modify(data, true)
      }
    ];
    return getPermissionMenuItem(menus, this.cookieService.role);
  };

  ngOnInit() {
    this.getTableData();
    this.virtualScroll.getScrollParam(270);
  }
}
