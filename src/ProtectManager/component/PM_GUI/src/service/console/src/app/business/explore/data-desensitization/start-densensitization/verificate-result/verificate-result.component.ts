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
import { each, isEmpty, isArray, assign, range } from 'lodash';
import { I18NService } from 'app/shared';

@Component({
  selector: 'aui-verificate-result',
  templateUrl: './verificate-result.component.html',
  styleUrls: ['./verificate-result.component.less']
})
export class VerificateResultComponent implements OnInit {
  rowItem;
  tableData = [];
  columns = [
    {
      key: 'column_name',
      label: this.i18n.get('explore_column_name_label')
    },
    {
      key: 'line_content',
      label: this.i18n.get('explore_line_content_label')
    }
  ];
  constructor(private i18n: I18NService) {}

  getTableData() {
    if (isArray(this.rowItem.result) && !isEmpty(this.rowItem.result)) {
      const arr = [];
      const keys = this.rowItem.result.shift();
      each(range(keys.length), index => {
        each(this.rowItem.result, item => {
          const obj = {};
          assign(obj, {
            column_name: keys[index],
            line_content: item[index]
          });
          arr.push(obj);
        });
      });
      this.tableData = arr;
    } else {
      this.tableData = [];
    }
  }

  ngOnInit() {
    this.getTableData();
  }
}
