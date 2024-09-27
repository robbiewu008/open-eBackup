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
import { DatePipe } from '@angular/common';
import { Component, Input, OnInit, ViewChild } from '@angular/core';
import { CommonConsts, I18NService } from 'app/shared';
import { each, map, uniqueId } from 'lodash';

@Component({
  selector: 'aui-template-detail',
  templateUrl: './template-detail.component.html',
  styleUrls: ['./template-detail.component.less'],
  providers: [DatePipe]
})
export class TemplateDetailComponent implements OnInit {
  @Input() rowItem;
  @Input() activeIndex;
  allPath = [];
  filterExcludeArr = [];
  filterIncludeArr = [];
  sizeOptions: number[] = CommonConsts.SIMPLE_PAGE_SIZE_OPTIONS;
  pageSize: number = CommonConsts.PAGE_START;

  @ViewChild('page', { static: false }) page;

  constructor(private i18n: I18NService) {}

  ngOnInit() {
    this.getFilters();
  }

  getFilters() {
    this.allPath = map(this.rowItem.files, item => {
      return {
        id: uniqueId(),
        path: item
      };
    });
    each(this.rowItem.filters, item => {
      switch (item.type) {
        case 'File':
          const fileObj = {
            key: 'file',
            label: this.i18n.get('common_files_label'),
            content: item.values
          };
          if (item.mode === 'EXCLUDE') {
            this.filterExcludeArr.push(fileObj);
          } else {
            this.filterIncludeArr.push(fileObj);
          }
          break;
        case 'Dir':
          const contentObj = {
            key: 'content',
            label: this.i18n.get('common_directory_label'),
            content: item.values
          };
          if (item.mode === 'EXCLUDE') {
            this.filterExcludeArr.push(contentObj);
          } else {
            this.filterIncludeArr.push(contentObj);
          }
          break;
        default:
          break;
      }
    });
  }
}
