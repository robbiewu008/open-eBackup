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
  CommonConsts,
  DataMap,
  DESENSITIZATION_POLICY_DESC_MAP,
  I18NService,
  IdentRuleControllerService
} from 'app/shared';
import { each, isEmpty } from 'lodash';

@Component({
  selector: 'aui-related-desensitization-policy',
  templateUrl: './related-desensitization-policy.component.html',
  styleUrls: ['./related-desensitization-policy.component.less']
})
export class RelatedDesensitizationPolicyComponent implements OnInit {
  rowItem;
  dataMap = DataMap;
  policyData = [];
  pageIndex = CommonConsts.PAGE_START;
  pageSize = CommonConsts.PAGE_SIZE;
  total = CommonConsts.PAGE_TOTAL;
  sizeOptions = CommonConsts.PAGE_SIZE_OPTIONS;
  constructor(
    private policyManagerApiService: IdentRuleControllerService,
    private i18n: I18NService
  ) {}

  getPolicyData() {
    this.policyManagerApiService
      .getIdentRuleReferencesUsingGET({
        identificationId: this.rowItem.id,
        pageSize: this.pageSize,
        pageNo: this.pageIndex
      })
      .subscribe(res => {
        each(res.items, item => {
          item.description = !isEmpty(
            DESENSITIZATION_POLICY_DESC_MAP[item.name]
          )
            ? this.i18n.get(DESENSITIZATION_POLICY_DESC_MAP[item.name])
            : item.description;
        });
        this.policyData = res.items;
        this.total = res.total;
      });
  }

  pageChange(page) {
    this.pageSize = page.pageSize;
    this.pageIndex = page.pageIndex;
    this.getPolicyData();
  }

  ngOnInit() {
    this.getPolicyData();
  }
}
