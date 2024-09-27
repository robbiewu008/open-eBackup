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
  DESENSITIZATION_POLICY_DESC_MAP,
  getPermissionMenuItem,
  GROUP_COMMON,
  I18NService,
  MODAL_COMMON,
  OperateItems,
  PolicyControllerService,
  RoleOperationMap
} from 'app/shared';
import { BatchOperateService } from 'app/shared/services/batch-operate.service';
import { DrawModalService } from 'app/shared/services/draw-modal.service';
import { InfoMessageService } from 'app/shared/services/info-message.service';
import { VirtualScrollService } from 'app/shared/services/virtual-scroll.service';
import { assign, each, filter, isEmpty, size, trim } from 'lodash';
import { combineLatest } from 'rxjs';
import { RelatedObjectComponent } from '../related-object/related-object.component';
import { CreateDesensitizationPolicyComponent } from './create-desensitization-policy/create-desensitization-policy.component';
import { DesensitizationPolicyDetailComponent } from './desensitization-policy-detail/desensitization-policy-detail.component';

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
  selector: 'aui-desensitization-policy-list',
  templateUrl: './desensitization-policy-list.component.html',
  styleUrls: ['./desensitization-policy-list.component.less'],
  changeDetection: ChangeDetectionStrategy.OnPush,
  providers: [SelectionPipe]
})
export class DesensitizationPolicyListComponent implements OnInit {
  selectedPolicyView = 0;
  policyData = [];
  selection = [];
  pageIndex = CommonConsts.PAGE_START;
  pageSize = CommonConsts.PAGE_SIZE;
  total = CommonConsts.PAGE_TOTAL;
  sizeOptions = CommonConsts.PAGE_SIZE_OPTIONS;
  dataMap = DataMap;
  searchName;
  filterParams = {};
  filterMap = this.dataMapService.toArray('Senesitization_Create_Method');
  groupCommon = GROUP_COMMON;
  roleOperationMap = RoleOperationMap;
  constructor(
    private i18n: I18NService,
    private drawModalService: DrawModalService,
    private policyManagerApiService: PolicyControllerService,
    private dataMapService: DataMapService,
    private batchOperateService: BatchOperateService,
    private cookieService: CookieService,
    private messageBox: MessageboxService,
    private infoMessageService: InfoMessageService,
    public virtualScroll: VirtualScrollService,
    private cdr: ChangeDetectorRef
  ) {}

  optsCallback = data => {
    const menus = [
      {
        id: 'modify',
        hidden:
          data.create_method ===
          DataMap.Senesitization_Create_Method.preset.value,
        disabled: data.ref_num > 0,
        label: this.i18n.get('common_modify_label'),
        permission: OperateItems.ModifyDesensitizationPolicy,
        onClick: () => this.create(data)
      },
      {
        id: 'delete',
        hidden:
          data.create_method ===
          DataMap.Senesitization_Create_Method.preset.value,
        disabled: data.ref_num > 0,
        label: this.i18n.get('common_delete_label'),
        permission: OperateItems.DeleteDesensitizationPolicy,
        onClick: () => this.delete([data])
      },
      {
        id: 'clone',
        label: this.i18n.get('common_clone_label'),
        permission: OperateItems.CloneDesensitizationPolicy,
        onClick: () => this.create(data, true)
      }
    ];
    return getPermissionMenuItem(menus, this.cookieService.role);
  };

  create(data?, isClone?) {
    this.drawModalService.create({
      ...MODAL_COMMON.generateDrawerOptions(),
      lvHeader: isEmpty(data)
        ? this.i18n.get('explore_sensitive_policy_create_label')
        : isClone
        ? this.i18n.get('common_clone_label')
        : this.i18n.get('common_modify_label'),
      lvContent: CreateDesensitizationPolicyComponent,
      lvOkDisabled: isEmpty(data),
      lvWidth: MODAL_COMMON.largeWidth,
      lvAfterOpen: modal => {
        const content = modal.getContentComponent() as CreateDesensitizationPolicyComponent;
        const combined: any = combineLatest(
          content.formGroup.statusChanges,
          content.valid$
        );
        combined.subscribe(latestValues => {
          const [formGroupStatus, valid$] = latestValues;
          modal.lvOkDisabled = !(formGroupStatus === 'VALID' && valid$);
        });
        if (data) {
          content.formGroup.get('name').markAsTouched();
          content.formGroup.get('name').updateValueAndValidity();
          content.formGroup.get('description').updateValueAndValidity();
          content.valid$.next(true);
        }
      },
      lvComponentParams: {
        rowItem: assign({}, data, { isClone })
      },
      lvOk: modal => {
        return new Promise(resolve => {
          const content = modal.getContentComponent() as CreateDesensitizationPolicyComponent;
          content.create().subscribe(
            res => {
              resolve(true);
              this.getPolicyData();
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
        this.i18n.get('common_desensitization_policy_label'),
        this.i18n.get('common_desensitization_policy_label'),
        this.i18n.get('common_desensitization_policy_label')
      ]),
      onOK: () => {
        if (size(datas) === 1) {
          this.policyManagerApiService
            .deletePolicyUsingDELETE({
              id: datas[0].id
            })
            .subscribe(res => {
              this.getPolicyData();
            });
        } else {
          this.batchOperateService.selfGetResults(
            item => {
              return this.policyManagerApiService.deletePolicyUsingDELETE({
                id: item.id,
                akDoException: false,
                akOperationTips: false,
                akLoading: false
              });
            },
            datas,
            () => {
              this.getPolicyData();
            }
          );
        }
      }
    });
  }

  getRelNum(item) {
    this.drawModalService.create({
      ...MODAL_COMMON.generateDrawerOptions(),
      lvHeader: this.i18n.get('explore_associate_database_label'),
      lvContent: RelatedObjectComponent,
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

  searchByName(name) {
    this.searchName = name;
    this.getPolicyData();
  }

  filterChange(e) {
    assign(this.filterParams, {
      createMethod: e.value
    });
    this.getPolicyData();
  }

  searchPolicy(name) {
    this.searchName = name;
    this.getPolicyData();
  }

  getDetail(item) {
    this.drawModalService.create({
      ...MODAL_COMMON.generateDrawerOptions(),
      lvHeader: item.name,
      lvContent: DesensitizationPolicyDetailComponent,
      lvWidth: MODAL_COMMON.largeWidth,
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

  viewChange(e) {
    this.searchName = '';
    this.getPolicyData();
  }

  getPolicyData() {
    this.selection = [];
    const params = {
      pageSize: this.pageSize,
      pageNo: this.pageIndex
    };
    if (trim(this.searchName)) {
      assign(params, {
        name: trim(this.searchName)
      });
    }
    each(this.filterParams, (value, key) => {
      if (isEmpty(value)) {
        delete this.filterParams[key];
      }
    });
    this.policyManagerApiService
      .getPagePoliciesUsingGET(assign(params, this.filterParams))
      .subscribe(res => {
        each(res.items, item => {
          if (
            item.create_method ===
            DataMap.Senesitization_Create_Method.preset.value
          ) {
            item.description = this.i18n.get(
              DESENSITIZATION_POLICY_DESC_MAP[item.name]
            );
          }
        });
        this.policyData = res.items;
        this.total = res.total;
        this.cdr.detectChanges();
      });
  }

  pageChange(page) {
    this.pageSize = page.pageSize;
    this.pageIndex = page.pageIndex;
    this.getPolicyData();
  }

  onCardChange(e) {
    this.getPolicyData();
  }

  ngOnInit() {
    this.getPolicyData();
    this.virtualScroll.getScrollParam(270);
  }
}
