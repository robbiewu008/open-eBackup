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
import { Component, OnInit, ViewChild } from '@angular/core';
import { Subject, Observable, Observer } from 'rxjs';
import { DatatableComponent } from '@iux/live';
import { find, assign, trim, each } from 'lodash';
import { DrawModalService } from 'app/shared/services/draw-modal.service';
import {
  I18NService,
  MODAL_COMMON,
  PolicyControllerService,
  CommonConsts,
  DataMap,
  DESENSITIZATION_POLICY_DESC_MAP
} from 'app/shared';
import { CreateDesensitizationPolicyComponent } from '../../policy/desensitization-policy/desensitization-policy-list/create-desensitization-policy/create-desensitization-policy.component';

@Component({
  selector: 'aui-choose-densensitization-policy',
  templateUrl: './choose-densensitization-policy.component.html',
  styleUrls: ['./choose-densensitization-policy.component.less']
})
export class ChooseDensensitizationPolicyComponent implements OnInit {
  rowItem;
  pageIndex = CommonConsts.PAGE_START;
  pageSize = CommonConsts.PAGE_SIZE_SMALL;
  total = CommonConsts.PAGE_TOTAL;
  sizeOptions = [10];
  selectedPolicyView = 0;
  selectedPolicy;
  policyData = [];
  searchName;
  valid$ = new Subject<boolean>();
  @ViewChild(DatatableComponent, { static: false }) lvTable: DatatableComponent;
  constructor(
    private drawModalService: DrawModalService,
    private i18n: I18NService,
    private policyManagerApiService: PolicyControllerService
  ) {}

  viewChange(e) {
    if (!!e) {
      setTimeout(() => {
        if (find(this.policyData, { id: this.selectedPolicy })) {
          this.selectionRow(find(this.policyData, { id: this.selectedPolicy }));
        }
      });
    }
  }

  create() {
    this.drawModalService.create({
      ...MODAL_COMMON.generateDrawerOptions(),
      lvHeader: this.i18n.get('explore_sensitive_policy_create_label'),
      lvContent: CreateDesensitizationPolicyComponent,
      lvOkDisabled: true,
      lvWidth: MODAL_COMMON.largeWidth,
      lvAfterOpen: modal => {
        const content = modal.getContentComponent() as CreateDesensitizationPolicyComponent;
        content.formGroup.statusChanges.subscribe(res => {
          modal.lvOkDisabled = res !== 'VALID';
        });
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

  getPolicyData(name?) {
    const params = {
      pageSize: this.pageSize,
      pageNo: this.pageIndex
    };
    if (name) {
      assign(params, {
        name
      });
    }
    this.policyManagerApiService
      .getPagePoliciesUsingGET(params)
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
        if (
          this.lvTable &&
          this.selectedPolicy &&
          find(res.items, { id: this.selectedPolicy })
        ) {
          this.selectionRow(find(res.items, { id: this.selectedPolicy }));
        }
      });
  }

  pageChange(page) {
    this.pageSize = page.pageSize;
    this.pageIndex = page.pageIndex;
    this.getPolicyData();
  }

  searchByName(name) {
    this.getPolicyData(trim(name));
  }

  searchPolicy(name) {
    this.getPolicyData(trim(name));
  }

  policyChange() {
    this.valid$.next(true);
  }

  selectionRow(source) {
    this.lvTable.toggleSelection(source);
    this.selectedPolicy = source.id;
    this.valid$.next(true);
  }

  onOK(): Observable<void> {
    return new Observable<void>((observer: Observer<void>) => {});
  }

  ngOnInit() {
    if (this.rowItem && this.rowItem.desesitization_policy_id) {
      this.selectedPolicy = this.rowItem.desesitization_policy_id;
    }
    this.getPolicyData();
  }
}
