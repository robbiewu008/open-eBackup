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
import { CommonConsts, DataMap, MaskRuleControllerService } from 'app/shared';

@Component({
  selector: 'aui-related-identified-rule',
  templateUrl: './related-identified-rule.component.html',
  styleUrls: ['./related-identified-rule.component.less']
})
export class RelatedIdentifiedRuleComponent implements OnInit {
  rowItem;
  dataMap = DataMap;
  ruleData = [];
  pageIndex = CommonConsts.PAGE_START;
  pageSize = CommonConsts.PAGE_SIZE;
  total = CommonConsts.PAGE_TOTAL;
  sizeOptions = CommonConsts.PAGE_SIZE_OPTIONS;
  constructor(private policyManagerApiService: MaskRuleControllerService) {}

  getRuleData() {
    this.ruleData = [];
    this.policyManagerApiService
      .getMaskRuleReferencesUsingGET({
        maskId: this.rowItem.id,
        pageNo: this.pageIndex,
        pageSize: this.pageSize
      })
      .subscribe(res => {
        this.ruleData = res.items;
        this.total = res.total;
      });
  }

  pageChange(page) {
    this.pageSize = page.pageSize;
    this.pageIndex = page.pageIndex;
    this.getRuleData();
  }

  ngOnInit() {
    this.getRuleData();
  }
}
