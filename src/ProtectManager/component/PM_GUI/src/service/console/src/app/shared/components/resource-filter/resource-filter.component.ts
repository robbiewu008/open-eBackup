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
  Component,
  Input,
  OnChanges,
  OnInit,
  SimpleChanges,
  ViewChild
} from '@angular/core';
import { FormBuilder, FormControl, FormGroup } from '@angular/forms';
import { MessageService } from '@iux/live';
import {
  BaseUtilService,
  CommonConsts,
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
  filter,
  find,
  includes,
  isEmpty,
  isString,
  isUndefined,
  map,
  pick,
  reduce,
  size,
  toString,
  trim
} from 'lodash';

@Component({
  selector: 'aui-resource-filter',
  templateUrl: './resource-filter.component.html',
  styleUrls: ['./resource-filter.component.less']
})
export class ResourceFilterComponent implements OnInit, OnChanges {
  @Input() rowItem;
  @Input() filterParams;
  @Input() osType;
  @Input() subType;
  formGroup: FormGroup;
  resourceType = DataMap.Resource_Type;
  fileSetFilterType = FileSetFilterType;
  formatOptions = this.dataMapService
    .toArray('Filter_Format_Type')
    .map(item => {
      item.key = item.value;
      item.value = item.label;
      return item;
    });
  resourceV2Filter = false;
  filesetFilterDesc;
  contentsFilterDesc;

  @ViewChild('input', { static: false }) input;

  constructor(
    public i18n: I18NService,
    public fb: FormBuilder,
    public baseUtilService: BaseUtilService,
    public dataMapService: DataMapService,
    public message: MessageService
  ) {}

  ngOnChanges(changes: SimpleChanges): void {
    if (changes.osType && this.formGroup) {
      this.formGroup.get('filesetTags').setValue([]);
      this.formGroup.get('contentsTags').setValue([]);
    }
  }

  initForm() {
    this.resourceV2Filter = includes(
      [
        DataMap.Resource_Type.NASShare.value,
        DataMap.Resource_Type.fileset.value,
        DataMap.Resource_Type.volume.value
      ],
      this.subType
    );
    this.formGroup = this.fb.group({
      fileCheck: new FormControl(),
      fileFilterType: new FormControl(FileSetFilterType.Exclude),
      filesetFilterInput: new FormControl(),
      filesetTags: new FormControl([]),
      contentCheck: new FormControl(),
      contentFilterType: new FormControl(FileSetFilterType.Exclude),
      contentsFilterInput: new FormControl(),
      contentsTags: new FormControl([]),
      formatCheck: new FormControl(),
      formatFilterType: new FormControl(FileSetFilterType.Exclude),
      formatType: new FormControl(),
      formatTags: new FormControl([]),
      dateCheck: new FormControl(),
      dateFilterType: new FormControl(FileSetFilterType.Exclude),
      dateType: new FormControl(
        this.dataMapService.getConfig('Filter_Date_Type').start.value
      ),
      timeRange: new FormControl()
    });
  }

  setFormGroupFilter(item, formGroup) {
    switch (item.type) {
      case this.dataMapService.getConfig('Files_Filter_Type').file.value:
        formGroup.get('fileFilterType').setValue(item.model);
        formGroup.get('filesetTags').setValue(
          reduce(
            item.content,
            (arr, item) => {
              arr.push(this.getTag(item));
              return arr;
            },
            []
          )
        );
        break;
      case this.dataMapService.getConfig('Files_Filter_Type').content.value:
        formGroup.get('contentFilterType').setValue(item.model);
        formGroup.get('contentsTags').setValue(
          reduce(
            item.content,
            (arr, item) => {
              arr.push(this.getTag(item));
              return arr;
            },
            []
          )
        );
        break;
      case this.dataMapService.getConfig('Files_Filter_Type').format.value:
        formGroup.get('formatFilterType').setValue(item.model);
        const formatType = reduce(
          item.content.filterTypes,
          (arr, item) => {
            arr.push(
              this.getTag(
                this.dataMapService.getLabel('Filter_Format_Type', item),
                item
              )
            );
            return arr;
          },
          []
        );
        const otherType = reduce(
          item.content.other,
          (arr, item) => {
            arr.push(this.getTag(item));
            return arr;
          },
          []
        );
        formGroup.get('formatTags').setValue([...formatType, ...otherType]);
        break;
      case this.dataMapService.getConfig('Files_Filter_Type').date.value:
        formGroup.get('dateFilterType').setValue(item.model);
        formGroup.get('dateType').setValue(item.content.timeType);
        formGroup
          .get('timeRange')
          .setValue([
            new Date(+item.content.fromTimestamp),
            new Date(+item.content.toTimestamp)
          ]);
        break;
      default:
        break;
    }
  }

  setResourceV2FilterFormGroup() {
    if (
      !!size(
        filter(this.rowItem.filters, item => {
          return item.type === 'File' && item.filterBy === 'Name';
        })
      )
    ) {
      this.formGroup.get('fileCheck').setValue(true);
    }
    if (
      !!size(
        filter(this.rowItem.filters, item => {
          return item.type === 'Dir' && item.filterBy === 'Name';
        })
      )
    ) {
      this.formGroup.get('contentCheck').setValue(true);
    }
    if (
      !!size(
        filter(this.rowItem.filters, item => {
          return item.type === 'File' && item.filterBy === 'Format';
        })
      )
    ) {
      this.formGroup.get('formatCheck').setValue(true);
    }
    if (
      !!size(
        filter(this.rowItem.filters, item => {
          return (
            item.type === 'File' &&
            includes(['CreateTime', 'ModifyTime', 'AccessTime'], item.filterBy)
          );
        })
      )
    ) {
      this.formGroup.get('dateCheck').setValue(true);
    }
    each(this.rowItem.filters, item => {
      this.setFormGroupV2Filter(item, this.formGroup);
    });
  }

  setFormGroupV2Filter(item, formGroup) {
    if (item.type === 'File' && item.filterBy === 'Name') {
      formGroup
        .get('fileFilterType')
        .setValue(
          item.mode === 'INCLUDE'
            ? FileSetFilterType.Include
            : FileSetFilterType.Exclude
        );
      formGroup.get('filesetTags').setValue(
        map(item.values, v => {
          return this.getTag(v);
        })
      );
    }
    if (item.type === 'Dir' && item.filterBy === 'Name') {
      formGroup
        .get('contentFilterType')
        .setValue(
          item.mode === 'INCLUDE'
            ? FileSetFilterType.Include
            : FileSetFilterType.Exclude
        );
      formGroup.get('contentsTags').setValue(
        map(item.values, v => {
          return this.getTag(v);
        })
      );
    }
    if (item.type === 'File' && item.filterBy === 'Format') {
      formGroup
        .get('formatFilterType')
        .setValue(
          item.mode === 'INCLUDE'
            ? FileSetFilterType.Include
            : FileSetFilterType.Exclude
        );
      formGroup.get('formatTags').setValue(
        map(item.values, v => {
          return includes(
            this.dataMapService.toArray('Filter_Format_Type').map(i => {
              return i.value;
            }),
            +v
          )
            ? this.getTag(
                this.dataMapService.getLabel('Filter_Format_Type', +v),
                +v
              )
            : this.getTag(v);
        })
      );
    }
    if (
      item.type === 'File' &&
      includes(['CreateTime', 'ModifyTime', 'AccessTime'], item.filterBy)
    ) {
      formGroup
        .get('dateFilterType')
        .setValue(
          item.mode === 'INCLUDE'
            ? FileSetFilterType.Include
            : FileSetFilterType.Exclude
        );
      formGroup
        .get('dateType')
        .setValue(
          item.filterBy === 'CreateTime'
            ? DataMap.Filter_Date_Type.start.value
            : item.filterBy === 'ModifyTime'
            ? DataMap.Filter_Date_Type.modify.value
            : DataMap.Filter_Date_Type.interview.value
        );
      formGroup
        .get('timeRange')
        .setValue([new Date(+item.values[0]), new Date(+item.values[1])]);
    }
  }

  setformGroup() {
    if (this.subType === DataMap.Resource_Type.HDFS.value && this.rowItem) {
      this.rowItem['filters'] = map(
        cloneDeep(JSON.parse(this.rowItem.extendInfo.filters)),
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
    if (!this.rowItem || isEmpty(this.rowItem.filters)) {
      return;
    }
    if (this.resourceV2Filter) {
      this.setResourceV2FilterFormGroup();
      return;
    }
    if (
      !!size(
        filter(this.rowItem.filters, item => {
          return (
            item.type ===
            this.dataMapService.getConfig('Files_Filter_Type').file.value
          );
        })
      )
    ) {
      this.formGroup.get('fileCheck').setValue(true);
    }
    if (
      !!size(
        filter(this.rowItem.filters, item => {
          return (
            item.type ===
            this.dataMapService.getConfig('Files_Filter_Type').content.value
          );
        })
      )
    ) {
      this.formGroup.get('contentCheck').setValue(true);
    }
    if (
      !!size(
        filter(this.rowItem.filters, item => {
          return (
            item.type ===
            this.dataMapService.getConfig('Files_Filter_Type').format.value
          );
        })
      )
    ) {
      this.formGroup.get('formatCheck').setValue(true);
    }
    if (
      !!size(
        filter(this.rowItem.filters, item => {
          return (
            item.type ===
            this.dataMapService.getConfig('Files_Filter_Type').date.value
          );
        })
      )
    ) {
      this.formGroup.get('dateCheck').setValue(true);
    }
    each(this.rowItem.filters, item => {
      if (isString(item.content)) {
        item.content = JSON.parse(item.content);
      }
      this.setFormGroupFilter(item, this.formGroup);
    });
  }

  getTag(label, key?) {
    return {
      label,
      key: key || label,
      removeable: true,
      disabled: false
    };
  }

  iconClick() {
    this.input.nativeElement.focus();
  }

  clearIcon(filterFormGroup, name) {
    filterFormGroup.get(name).setValue('');
  }

  validPath(path) {
    if (
      this.osType === this.dataMapService.getConfig('Os_Type').windows.value &&
      CommonConsts.REGEX.windowsFullPath.test(path)
    ) {
      return true;
    }
    if (
      this.osType !== this.dataMapService.getConfig('Os_Type').windows.value &&
      CommonConsts.REGEX.linuxPath.test(path) &&
      !path.endsWith('/')
    ) {
      return true;
    }
    return false;
  }

  validPathLength(path) {
    if (this.osType === DataMap.Os_Type.windows.value && path.length > 259) {
      this.message.error(this.i18n.get('common_valid_maxlength_label', [259]), {
        lvMessageKey: 'pathLengthError',
        lvShowCloseButton: true
      });
      return false;
    }
    if (this.osType !== DataMap.Os_Type.windows.value && path.length > 4069) {
      this.message.error(
        this.i18n.get('common_valid_maxlength_label', [4069]),
        {
          lvMessageKey: 'pathLengthError',
          lvShowCloseButton: true
        }
      );
      return false;
    }
    return true;
  }

  addFilesFilter(filterFormGroup) {
    if (
      size(this.formGroup.value.filesetTags) +
        size(this.formGroup.value.contentsTags) >=
      32
    ) {
      this.message.error(this.i18n.get('protection_rule_filter_vaild_label'), {
        lvMessageKey: 'filesetError',
        lvShowCloseButton: true
      });
      return;
    }
    if (!this.osType && this.subType !== DataMap.Resource_Type.HDFS.value) {
      this.message.error(
        this.i18n.get('protection_select_host_placeholder_label'),
        {
          lvMessageKey: 'filesetError',
          lvShowCloseButton: true
        }
      );
      return;
    }
    if (
      this.resourceV2Filter &&
      !this.validPathLength(filterFormGroup.get('filesetFilterInput').value)
    ) {
      return;
    }
    if (!this.validPath(filterFormGroup.get('filesetFilterInput').value)) {
      this.message.error(this.i18n.get('common_path_error_label'), {
        lvMessageKey: 'filesetError',
        lvShowCloseButton: true
      });
      return;
    }
    if (
      filterFormGroup.get('filesetFilterInput').value &&
      find(filterFormGroup.value.filesetTags, {
        key: filterFormGroup.get('filesetFilterInput').value
      })
    ) {
      this.message.error(this.i18n.get('common_same_filter_error_label'), {
        lvMessageKey: 'filesetError',
        lvShowCloseButton: true
      });
      return;
    }
    if (
      filterFormGroup.get('filesetFilterInput').value &&
      !find(filterFormGroup.value.filesetTags, {
        key: filterFormGroup.get('filesetFilterInput').value
      })
    ) {
      filterFormGroup
        .get('filesetTags')
        .setValue([
          ...(filterFormGroup.get('filesetTags').value || []),
          this.getTag(filterFormGroup.get('filesetFilterInput').value)
        ]);
      filterFormGroup.get('filesetFilterInput').setValue('');
    }
  }

  addContentsFilter(filterFormGroup) {
    if (
      size(this.formGroup.value.filesetTags) +
        size(this.formGroup.value.contentsTags) >=
      32
    ) {
      this.message.error(this.i18n.get('protection_rule_filter_vaild_label'), {
        lvMessageKey: 'filesetError',
        lvShowCloseButton: true
      });
      return;
    }
    if (!this.osType && this.subType !== DataMap.Resource_Type.HDFS.value) {
      this.message.error(
        this.i18n.get('protection_select_host_placeholder_label'),
        {
          lvMessageKey: 'filesetError',
          lvShowCloseButton: true
        }
      );
      return;
    }
    if (
      this.resourceV2Filter &&
      !this.validPathLength(filterFormGroup.get('contentsFilterInput').value)
    ) {
      return;
    }
    if (!this.validPath(filterFormGroup.get('contentsFilterInput').value)) {
      this.message.error(this.i18n.get('common_path_error_label'), {
        lvMessageKey: 'contentError',
        lvShowCloseButton: true
      });
      return;
    }
    if (
      filterFormGroup.get('contentsFilterInput').value &&
      find(filterFormGroup.value.contentsTags, {
        key: filterFormGroup.get('contentsFilterInput').value
      })
    ) {
      this.message.error(this.i18n.get('common_same_filter_error_label'), {
        lvMessageKey: 'contentError',
        lvShowCloseButton: true
      });
      return;
    }
    if (
      filterFormGroup.get('contentsFilterInput').value &&
      !find(filterFormGroup.value.contentsTags, {
        key: filterFormGroup.get('contentsFilterInput').value
      })
    ) {
      filterFormGroup
        .get('contentsTags')
        .setValue([
          ...(filterFormGroup.get('contentsTags').value || []),
          this.getTag(filterFormGroup.get('contentsFilterInput').value)
        ]);
      filterFormGroup.get('contentsFilterInput').setValue('');
    }
  }

  addFormatFilter(filterFormGroup) {
    if (
      filterFormGroup.get('formatType').value &&
      find(filterFormGroup.value.formatTags, {
        label: filterFormGroup.get('formatType').value
      })
    ) {
      this.message.error(this.i18n.get('common_same_filter_error_label'), {
        lvMessageKey: 'formatError',
        lvShowCloseButton: true
      });
      return;
    }
    if (
      filterFormGroup.get('formatType').value &&
      !find(filterFormGroup.value.formatTags, {
        label: filterFormGroup.get('formatType').value
      })
    ) {
      filterFormGroup.get('formatTags').setValue([
        ...(filterFormGroup.get('formatTags').value || []),
        this.getTag(
          filterFormGroup.get('formatType').value,
          find(this.formatOptions, {
            value: filterFormGroup.get('formatType').value
          })
            ? find(this.formatOptions, {
                value: filterFormGroup.get('formatType').value
              }).key
            : filterFormGroup.get('formatType').value
        )
      ]);
    }
  }

  getFliterContent(filters, options) {
    if (this.formGroup.value.fileCheck && !isEmpty(options.filesetTags)) {
      if (this.resourceV2Filter) {
        filters.push({
          filterBy: 'Name',
          type: 'File',
          rule: find(
            map(options.filesetTags, item => {
              return item.key;
            }),
            value => {
              return value.indexOf('*') !== -1;
            }
          )
            ? 'Fuzzy'
            : 'Exact',
          mode:
            options.fileFilterType === FileSetFilterType.Include
              ? 'INCLUDE'
              : 'EXCLUDE',
          values: map(options.filesetTags, item => {
            return item.key;
          })
        });
      } else {
        filters.push({
          type: this.dataMapService.getConfig('Files_Filter_Type').file.value,
          model: options.fileFilterType,
          content: JSON.stringify(
            map(options.filesetTags, item => {
              return item.key;
            })
          )
        });
      }
    }
    if (this.formGroup.value.contentCheck && !isEmpty(options.contentsTags)) {
      if (this.resourceV2Filter) {
        filters.push({
          filterBy: 'Name',
          type: 'Dir',
          rule: find(
            map(options.contentsTags, item => {
              return item.key;
            }),
            value => {
              return value.indexOf('*') !== -1;
            }
          )
            ? 'Fuzzy'
            : 'Exact',
          mode:
            options.contentFilterType === FileSetFilterType.Include
              ? 'INCLUDE'
              : 'EXCLUDE',
          values: map(options.contentsTags, item => {
            return item.key;
          })
        });
      } else {
        filters.push({
          type: this.dataMapService.getConfig('Files_Filter_Type').content
            .value,
          model: options.contentFilterType,
          content: JSON.stringify(
            map(options.contentsTags, item => {
              return item.key;
            })
          )
        });
      }
    }
    if (this.formGroup.value.formatCheck && !isEmpty(options.formatTags)) {
      if (this.resourceV2Filter) {
        filters.push({
          filterBy: 'Format',
          type: 'File',
          rule: 'Exact',
          mode:
            options.formatFilterType === FileSetFilterType.Include
              ? 'INCLUDE'
              : 'EXCLUDE',
          values: map(options.formatTags, item => {
            return trim(item.key);
          })
        });
      } else {
        filters.push({
          type: this.dataMapService.getConfig('Files_Filter_Type').format.value,
          model: options.formatFilterType,
          content: JSON.stringify({
            filterTypes: map(
              filter(options.formatTags, val => {
                return !isString(val.key);
              }),
              item => {
                return item.key;
              }
            ),
            other: map(
              filter(options.formatTags, val => {
                return isString(val.key);
              }),
              item => {
                return item.key;
              }
            )
          })
        });
      }
    }
    if (
      this.formGroup.value.dateCheck &&
      !isUndefined(options.timeRange[0]) &&
      !isUndefined(options.timeRange[1])
    ) {
      if (this.resourceV2Filter) {
        filters.push({
          filterBy:
            options.dateType === DataMap.Filter_Date_Type.start.value
              ? 'CreateTime'
              : options.dateType === DataMap.Filter_Date_Type.modify.value
              ? 'ModifyTime'
              : 'AccessTime',
          type: 'File',
          rule: 'Exact',
          mode:
            options.dateFilterType === FileSetFilterType.Include
              ? 'INCLUDE'
              : 'EXCLUDE',
          values: [
            toString(new Date(options.timeRange[0]).getTime()),
            toString(new Date(options.timeRange[1]).getTime())
          ]
        });
      } else {
        filters.push({
          type: this.dataMapService.getConfig('Files_Filter_Type').date.value,
          model: options.dateFilterType,
          content: JSON.stringify({
            fromTimestamp: toString(new Date(options.timeRange[0]).getTime()),
            toTimestamp: toString(new Date(options.timeRange[1]).getTime()),
            timeType: options.dateType
          })
        });
      }
    }
  }

  collectParams() {
    this.filterParams.splice(0, this.filterParams.length);
    this.getFliterContent(this.filterParams, this.formGroup.value);
  }

  ngOnInit() {
    this.initForm();
    this.setformGroup();
    this.filesetFilterDesc =
      this.subType === DataMap.Resource_Type.fileset.value
        ? this.i18n.get('protection_file_path_backup_label')
        : this.i18n.get('protection_linux_file_path_label');
    this.contentsFilterDesc =
      this.subType === DataMap.Resource_Type.fileset.value
        ? this.i18n.get('protection_content_path_backup_label')
        : this.i18n.get('protection_linux_path_backup_label');
  }
}
