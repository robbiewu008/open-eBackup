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
  DataMapService,
  I18NService,
  PolicyControllerService
} from 'app/shared';
import { assign, each, find, pick } from 'lodash';

@Component({
  selector: 'aui-desensitization-policy-detail',
  templateUrl: './desensitization-policy-detail.component.html',
  styleUrls: ['./desensitization-policy-detail.component.less']
})
export class DesensitizationPolicyDetailComponent implements OnInit {
  rowItem;
  baseInfo = {} as any;
  identifiedData = [];
  desensitizationData = [];
  constructor(
    private policyManagerApiService: PolicyControllerService,
    private i18n: I18NService,
    private dataMapService: DataMapService
  ) {}

  initDetail() {
    this.policyManagerApiService
      .getPolicyDetailsUsingGET({
        policyId: this.rowItem.id
      })
      .subscribe(res => {
        assign(this.baseInfo, pick(this.rowItem, ['name', 'description']));
        if (res.template_name) {
          this.getTemplateRules(res);
        } else {
          const identRule = [];
          const maskRule = [];
          each(res.addition_rules, item => {
            identRule.push(
              assign(item.IdentRule, {
                mask_name: item.MaskRule.name
              })
            );
            if (!find(maskRule, { id: item.MaskRule.id })) {
              maskRule.push(item.MaskRule);
            }
          });
          this.identifiedData = identRule;
          this.desensitizationData = maskRule;
        }
      });
  }

  getTemplateRules(res) {}

  ngOnInit() {
    this.initDetail();
  }
}
