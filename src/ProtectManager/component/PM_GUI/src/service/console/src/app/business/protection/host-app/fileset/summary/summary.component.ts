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
import { Component, OnInit } from '@angular/core';
import {
  CommonConsts,
  CopiesService,
  DataMap,
  DataMapService,
  FileSetFilterType,
  FilterBy,
  FilterMode,
  I18NService
} from 'app/shared';
import {
  cloneDeep,
  each,
  isString,
  isUndefined,
  map,
  pick,
  reduce
} from 'lodash';

@Component({
  selector: 'aui-fileset-summary',
  templateUrl: './summary.component.html',
  styleUrls: ['./summary.component.less'],
  providers: [DatePipe]
})
export class SummaryComponent implements OnInit {
  source;
  dataMap = DataMap;
  filesTree = [];
  filterExcludeArr = [];
  filterIncludeArr = [];
  showProtectedInfos = false;
  pageSize = CommonConsts.PAGE_SIZE_SMALL;
  pageSizeOptions = [CommonConsts.PAGE_SIZE_SMALL];
  selectionPath = [];
  constructor(
    public i18n: I18NService,
    public dataMapService: DataMapService,
    public datePipe: DatePipe,
    public copiesApiService: CopiesService
  ) {}

  initDetailData(data: any) {
    this.source = data;
  }

  getProtectedInfos() {
    this.copiesApiService
      .queryResourcesV1CopiesGet({
        pageSize: CommonConsts.PAGE_SIZE,
        pageNo: CommonConsts.PAGE_START,
        conditions: JSON.stringify({ resource_id: this.source.uuid })
      })
      .subscribe(
        res => {
          if (
            !(
              res.items[0] &&
              res.items[0].generated_by ===
                DataMap.CopyData_generatedType.replicate.value
            )
          ) {
            this.showProtectedInfos = true;
            this.getProtectedFiles();
            this.getFilesFilter();
          }
        },
        err => {
          this.showProtectedInfos = true;
          this.getProtectedFiles();
          this.getFilesFilter();
        }
      );
  }

  getProtectedFiles() {
    this.selectionPath = JSON.parse(this.source.extendInfo.paths);
  }

  getFilesFilter() {
    if (this.source && this.source.type === DataMap.Resource_Type.HDFS.value) {
      this.source['filters'] = map(
        cloneDeep(JSON.parse(this.source.extendInfo.filters)),
        obj => {
          obj['type'] = FilterBy[obj['filterBy']];
          obj['model'] = FilterMode[obj['mode']];
          obj['content'] =
            obj['filterBy'] === 'Format'
              ? {
                  filterTypes: map(obj['values'], v => +v),
                  other: []
                }
              : obj['values'];
          return pick(obj, ['type', 'model', 'content']);
        }
      );
    }

    if (
      this.source &&
      this.source.type === DataMap.Resource_Type.fileset.value &&
      this.source.subType !== DataMap.Resource_Type.volume.value
    ) {
      this.source['filters'] = map(
        cloneDeep(JSON.parse(this.source.extendInfo.filters)),
        obj => {
          obj['type'] =
            obj['type'] === 'File' && obj['filterBy'] === 'Name'
              ? DataMap.Files_Filter_Type.file.value
              : DataMap.Files_Filter_Type.content.value;
          obj['model'] = FilterMode[obj['mode']];
          obj['content'] =
            obj['filterBy'] === 'Format'
              ? {
                  filterTypes: map(obj['values'], v => +v),
                  other: []
                }
              : obj['values'];
          return pick(obj, ['type', 'model', 'content']);
        }
      );
    }

    each(this.source.filters, item => {
      if (isString(item.content)) {
        item.content = JSON.parse(item.content);
      }
      if (isUndefined(item.type)) {
        item.type = item.filterType;
      }
      if (isUndefined(item.model)) {
        item.model = item.filterMode;
      }
      switch (item.type) {
        case this.dataMapService.getConfig('Files_Filter_Type').file.value:
          const fileObj = {
            key: 'file',
            label: this.i18n.get('common_files_label'),
            content: item.content
          };
          if (item.model === FileSetFilterType.Exclude) {
            this.filterExcludeArr.push(fileObj);
          } else {
            this.filterIncludeArr.push(fileObj);
          }
          break;
        case this.dataMapService.getConfig('Files_Filter_Type').content.value:
          const contentObj = {
            key: 'content',
            label: this.i18n.get('common_directory_label'),
            content: item.content
          };
          if (item.model === FileSetFilterType.Exclude) {
            this.filterExcludeArr.push(contentObj);
          } else {
            this.filterIncludeArr.push(contentObj);
          }
          break;
        case this.dataMapService.getConfig('Files_Filter_Type').format.value:
          const formatType = reduce(
            item.content.filterTypes,
            (arr, key) => {
              arr.push(this.dataMapService.getLabel('Filter_Format_Type', key));
              return arr;
            },
            []
          );
          const otherType = reduce(
            item.content.other,
            (arr, key) => {
              arr.push(key);
              return arr;
            },
            []
          );
          const formatObj = {
            key: 'format',
            label: this.i18n.get('common_format_label'),
            content: [...formatType, ...otherType]
          };
          if (item.model === FileSetFilterType.Exclude) {
            this.filterExcludeArr.push(formatObj);
          } else {
            this.filterIncludeArr.push(formatObj);
          }
          break;
        case this.dataMapService.getConfig('Files_Filter_Type').date.value:
          const dateObj = {
            key: 'date',
            label: this.i18n.get('protection_date_type_label'),
            type: this.dataMapService.getLabel(
              'Filter_Date_Type',
              item.content.timeType
            ),
            content:
              this.datePipe.transform(
                +item.content.fromTimestamp,
                'yyyy-MM-dd HH:mm:ss'
              ) +
              ' - ' +
              this.datePipe.transform(
                +item.content.toTimestamp,
                'yyyy-MM-dd HH:mm:ss'
              )
          };
          if (item.model === FileSetFilterType.Exclude) {
            this.filterExcludeArr.push(dateObj);
          } else {
            this.filterIncludeArr.push(dateObj);
          }
          break;
        default:
          break;
      }
    });
  }

  ngOnInit() {
    this.getProtectedInfos();
  }
}
