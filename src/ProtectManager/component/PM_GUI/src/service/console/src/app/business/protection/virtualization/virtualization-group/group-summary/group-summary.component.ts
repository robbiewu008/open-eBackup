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
  CAPACITY_UNIT,
  DataMap,
  DataMapService,
  I18NService,
  ProtectedResourceApiService,
  ResourceType,
  WarningMessageService,
  isJson
} from 'app/shared';
import { ProButton } from 'app/shared/components/pro-button/interface';
import {
  TableCols,
  TableConfig,
  TableData
} from 'app/shared/components/pro-table';
import {
  difference,
  each,
  filter,
  find,
  isEmpty,
  map,
  remove,
  size,
  union
} from 'lodash';

@Component({
  selector: 'aui-group-summary',
  templateUrl: './group-summary.component.html',
  styleUrls: ['./group-summary.component.less']
})
export class GroupSummaryComponent implements OnInit {
  unitconst = CAPACITY_UNIT;
  resourceType = ResourceType;
  dataMap = DataMap;
  source;
  tableData: TableData;
  tableConfig: TableConfig;
  selectionData = [];
  optsConfig;

  isVmwareGroupByRule = false;
  showResouceFilter = false;
  resoureNameFilterString: string;
  showResourceTagFilter = false;
  resourceTagFilterString: string;

  constructor(
    private i18n: I18NService,
    private dataMapService: DataMapService,
    private protectedResourceApiService: ProtectedResourceApiService,
    public warningMessageService: WarningMessageService
  ) {}

  ngOnInit(): void {
    this.initConfig();
    this.showTableData();
  }

  fixValueByRule(item, value) {
    return {
      START_WITH: value + '*',
      END_WITH: '*' + value,
      FUZZY: '*' + value + '*',
      ALL: value
    }[item.rule];
  }

  toFilterString(filters): string[] {
    let unionFilter = [];
    each(filters, item => {
      unionFilter = union(
        unionFilter,
        map(item.values, value => this.fixValueByRule(item, value))
      );
    });
    return unionFilter;
  }

  initDetailData(data: any) {
    this.source = data;
    this.isVmwareGroupByRule =
      data.groupType === DataMap.vmGroupType.rule.value;
    const splitSymbol = this.i18n.isEn ? ',' : '、';
    if (isJson(data.extendStr)) {
      const extendStr = JSON.parse(data.extendStr);
      if (!isEmpty(extendStr.resource_filters)) {
        this.showResouceFilter = true;
        const includeFilter = filter(
          extendStr.resource_filters,
          item => item.mode === DataMap.resourceFilterType.include.value
        );
        const excludeFilter = filter(
          extendStr.resource_filters,
          item => item.mode === DataMap.resourceFilterType.exclude.value
        );
        const includeStr = this.toFilterString(includeFilter).join(splitSymbol);
        const excludeStr = this.toFilterString(excludeFilter).join(splitSymbol);
        if (!isEmpty(includeFilter) && !isEmpty(excludeFilter)) {
          this.resoureNameFilterString = this.i18n.get(
            'protection_rule_name_include_exclude_label',
            [includeStr, excludeStr]
          );
        } else if (!isEmpty(includeFilter)) {
          this.resoureNameFilterString = this.i18n.get(
            'protection_rule_name_include_label',
            [includeStr]
          );
        } else {
          this.resoureNameFilterString = this.i18n.get(
            'protection_rule_name_exclude_label',
            [excludeStr]
          );
        }
      }
      if (!isEmpty(extendStr.resource_tag_filters)) {
        this.showResourceTagFilter = true;
        const includeFilter = filter(
          extendStr.resource_tag_filters,
          item => item.mode === DataMap.resourceFilterType.include.value
        );
        const excludeFilter = filter(
          extendStr.resource_tag_filters,
          item => item.mode === DataMap.resourceFilterType.exclude.value
        );
        const includeStr = this.toFilterString(includeFilter).join(splitSymbol);
        const excludeStr = this.toFilterString(excludeFilter).join(splitSymbol);
        if (!isEmpty(includeFilter) && !isEmpty(excludeFilter)) {
          this.resourceTagFilterString = this.i18n.get(
            'protection_rule_tag_include_exclude_label',
            [includeStr, excludeStr]
          );
        } else if (!isEmpty(includeFilter)) {
          this.resourceTagFilterString = this.i18n.get(
            'protection_rule_tag_include_label',
            [includeStr]
          );
        } else {
          this.resourceTagFilterString = this.i18n.get(
            'protection_rule_tag_exclude_label',
            [excludeStr]
          );
        }
      }
    }
  }

  initConfig() {
    const opts: ProButton[] = [
      {
        id: 'remove',
        type: 'primary',
        label: this.i18n.get('common_remove_label'),
        disableCheck: () => {
          return !size(this.selectionData);
        },
        displayCheck: () => {
          return !this.isVmwareGroupByRule;
        },
        onClick: () => {
          this.remove('multiple');
        }
      }
    ];
    this.optsConfig = opts;
    const cols: TableCols[] = [
      {
        key: 'sourceName',
        name: this.i18n.get('common_name_label'),
        filter: {
          type: 'search',
          filterMode: 'contains'
        }
      },
      {
        key: 'path',
        name: this.i18n.get('common_location_label'),
        filter: {
          type: 'search',
          filterMode: 'contains'
        }
      },
      {
        key: 'operation',
        name: this.i18n.get('common_operation_label'),
        cellRender: {
          type: 'operation',
          config: {
            maxDisplayItems: 1,
            items: [
              {
                id: 'remove',
                label: this.i18n.get('common_remove_label'),
                onClick: data => {
                  this.remove('single', data);
                }
              }
            ]
          }
        }
      }
    ];

    if (this.isVmwareGroupByRule) {
      remove(cols, item => item.key === 'operation');
    }

    this.tableConfig = {
      table: {
        async: false,
        columns: cols,
        rows: this.isVmwareGroupByRule
          ? null
          : {
              selectionMode: 'multiple',
              selectionTrigger: 'selector',
              showSelector: true
            },
        compareWith: this.isVmwareGroupByRule ? 'sourceId' : 'uuid',
        colDisplayControl: false,
        selectionChange: selection => {
          this.selectionData = selection;
        },
        trackByFn: (index, item) => {
          return item.uuid;
        }
      },
      pagination: {
        winTablePagination: true,
        mode: 'simple',
        showPageSizeOptions: false
      }
    };
  }

  remove(type, item?) {
    let retainedData = [];
    if (type === 'single') {
      retainedData = difference(this.tableData.data, item);
    } else {
      retainedData = difference(this.tableData.data, this.selectionData);
    }

    this.warningMessageService.create({
      content: this.i18n.get('protection_group_remove_tip_label', [
        this.source.name
      ]),
      onOK: () => {
        this.protectedResourceApiService
          .UpdateResourceGroup({
            UpdateResourceGroupRequestBody: {
              name: this.source.name,
              resourceIds: retainedData.map(item => item.sourceId)
            },
            resourceGroupId: this.source.uuid
          })
          .subscribe(res => {
            this.selectionData = [];
            this.showTableData();
          });
      }
    });
  }

  showTableData() {
    this.protectedResourceApiService
      .ShowResourceGroup({ resourceGroupId: this.source?.uuid })
      .subscribe(res => {
        const data = map(res.resourceGroupMembers, item => ({ ...item }));
        this.tableData = {
          data: data,
          total: size(data)
        };
      });
  }
}
