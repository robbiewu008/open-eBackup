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
import { Component, OnInit, TemplateRef, ViewChild } from '@angular/core';
import {
  CommonConsts,
  I18NService,
  IODETECTFILESYSTEMService
} from 'app/shared';
import {
  TableCols,
  TableConfig,
  TableData
} from 'app/shared/components/pro-table';
import { each, isEmpty, isString, keys, last, map, size } from 'lodash';

@Component({
  selector: 'aui-honeypot-detail',
  templateUrl: './honeypot-detail.component.html',
  styleUrls: ['./honeypot-detail.component.less']
})
export class HoneypotDetailComponent implements OnInit {
  rowData;
  tableConfig: TableConfig;
  tableData: TableData;

  @ViewChild('nameTpl', { static: true })
  nameTpl: TemplateRef<any>;

  constructor(
    public i18n: I18NService,
    private ioDetectFilesystemService: IODETECTFILESYSTEMService
  ) {}

  ngOnInit(): void {
    this.initConfig();
    this.getHoneypotInfo();
  }

  initConfig() {
    const cols: TableCols[] = [
      {
        key: 'path',
        name: this.i18n.get('explore_honeypot_path_label')
      },
      {
        key: 'num',
        name: this.i18n.get('explore_honeypot_file_num_label')
      },
      {
        key: 'name',
        name: this.i18n.get('explore_honeypot_file_name_label'),
        width: '40%',
        cellRender: this.nameTpl
      }
    ];

    this.tableConfig = {
      table: {
        async: false,
        columns: cols,
        colDisplayControl: false,
        scrollFixed: true
      },
      pagination: {
        mode: 'simple',
        showPageSizeOptions: false,
        winTablePagination: true,
        pageSize: CommonConsts.PAGE_SIZE_SMALL
      }
    };
  }

  getHoneypotInfo() {
    this.ioDetectFilesystemService
      .getHoneypotInfoByFsId({ fsId: this.rowData?.id })
      .subscribe(res => {
        let result;
        if (res.records?.honeypotFileList) {
          result = JSON.parse(res.records?.honeypotFileList);
        }
        const displayData = [];
        each(result, item => {
          if (!isEmpty(item.filenames)) {
            displayData.push(
              ...this.parseData(item.path, item.filenames.split(','))
            );
          } else {
            displayData.push({
              path: item.path,
              num: 0
            });
          }
        });
        this.tableData = {
          data: displayData,
          total: size(displayData)
        };
      });
  }

  parseData(root, fileList: string[]) {
    const fileObj = {};
    each(fileList, item => {
      if (isString(item)) {
        const name = last(item.split('/'));
        const path = item.replace(name, '') || root;
        if (isEmpty(fileObj[path])) {
          fileObj[path] = [name];
        } else {
          fileObj[path].push(name);
        }
      }
    });
    const paths = keys(fileObj);
    return map(paths, path => {
      return {
        path: path === root ? path : `${root}${path}`,
        name: fileObj[path],
        num: size(fileObj[path])
      };
    });
  }
}
