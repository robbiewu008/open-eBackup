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
import { CommonConsts, PolicyControllerService } from 'app/shared';

@Component({
  selector: 'aui-related-object',
  templateUrl: './related-object.component.html',
  styleUrls: ['./related-object.component.less']
})
export class RelatedObjectComponent implements OnInit {
  rowItem;
  pageIndex = CommonConsts.PAGE_START;
  pageSize = CommonConsts.PAGE_SIZE;
  total = CommonConsts.PAGE_TOTAL;
  dbData = [];
  constructor(private policyManagerApiService: PolicyControllerService) {}

  initDb() {
    this.policyManagerApiService
      .getPolicyReferencesUsingGET({
        pageSize: this.pageSize,
        pageNo: this.pageIndex,
        policyId: this.rowItem.id
      })
      .subscribe(res => {
        this.dbData = res.items;
        this.total = res.total;
      });
  }

  pageChange(page) {
    this.pageSize = page.pageSize;
    this.pageIndex = page.pageIndex;
    this.initDb();
  }

  ngOnInit() {
    this.initDb();
  }
}
