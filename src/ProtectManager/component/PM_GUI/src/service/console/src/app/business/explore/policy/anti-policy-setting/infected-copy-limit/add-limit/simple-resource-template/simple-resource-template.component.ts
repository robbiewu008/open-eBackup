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
  AfterViewInit,
  Component,
  EventEmitter,
  Input,
  OnDestroy,
  OnInit,
  Output,
  ViewChild
} from '@angular/core';
import {
  AntiRansomwareInfectConfigApiService,
  CommonConsts,
  CopyControllerService,
  DataMap,
  GenConditionsService,
  GlobalService,
  I18NService,
  ProtectedResourceApiService,
  VirtualResourceService
} from 'app/shared';
import {
  Filters,
  ProTableComponent,
  TableCols,
  TableConfig,
  TableData
} from 'app/shared/components/pro-table';
import {
  assign,
  cloneDeep,
  differenceBy,
  each,
  find,
  isEmpty,
  isUndefined,
  set
} from 'lodash';
import { Subscription } from 'rxjs';

@Component({
  selector: 'aui-simple-resource-template',
  templateUrl: './simple-resource-template.component.html',
  styleUrls: ['./simple-resource-template.component.less']
})
export class SimpleResourceTemplateComponent
  implements OnInit, AfterViewInit, OnDestroy {
  @Input() copyType;
  @Input() type;
  @Input() subType;
  @Input() subName;
  @Input() allSelectionMap;
  @Output() SelectChange = new EventEmitter<any>();

  tableData: TableData;
  tableConfig: TableConfig;
  selectionData = [];

  dataFetch$: Subscription = new Subscription();

  @ViewChild('dataTable', { static: false }) dataTable: ProTableComponent;

  constructor(
    public globalService: GlobalService,
    private i18n: I18NService,
    private genConditionsServie: GenConditionsService,
    private copyControllerService: CopyControllerService,
    private virtualResourceService: VirtualResourceService,
    private protectedResourceApiService: ProtectedResourceApiService,
    private antiRansomwareInfectedCopyService: AntiRansomwareInfectConfigApiService
  ) {}

  ngOnInit() {
    this.initConfig();
  }

  ngAfterViewInit() {
    this.getFetchState();
  }

  ngOnDestroy() {
    this.dataFetch$.unsubscribe();
  }

  getFetchState() {
    this.dataFetch$ = this.globalService
      .getState(this.subName + 'antiPolicy' + this.copyType)
      .subscribe(res => {
        this.dataTable.fetchData();
      });
  }

  initConfig() {
    const cols: TableCols[] = [
      {
        key: 'uuid',
        name: this.i18n.get('protection_resource_id_label'),
        hidden: true,
        filter: {
          type: 'search',
          filterMode: 'contains'
        }
      },
      {
        key: 'name',
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
      }
    ];

    this.tableConfig = {
      table: {
        columns: cols,
        compareWith: 'uuid',
        rows: {
          selectionMode: 'multiple',
          selectionTrigger: 'selector',
          showSelector: true
        },
        scrollFixed: true,
        scroll: { y: '30vh' },
        colDisplayControl: {
          ignoringColsType: 'hide'
        },
        fetchData: (filter: Filters, args) => {
          this.getDataMiddle(filter, args);
        },
        selectionChange: selection => {
          if (this.selectionData.length < selection.length) {
            // 新增选中的处理
            this.selectionData = selection;
            if (isEmpty(this.allSelectionMap[this.type])) {
              set(
                this.allSelectionMap,
                this.type,
                cloneDeep(this.selectionData)
              );
            } else {
              this.parseSelected();
            }
          } else {
            // 减少选中时的处理
            const diffArray = differenceBy(
              this.selectionData,
              selection,
              'uuid'
            );
            this.allSelectionMap[this.type] = this.allSelectionMap[
              this.type
            ].filter(item => {
              return !find(diffArray, { uuid: item.uuid });
            });
            this.selectionData = selection;
          }
          this.SelectChange.emit();
        },
        trackByFn: (index, item) => {
          return item.uuid;
        }
      },
      pagination: {
        mode: 'simple',
        showPageSizeOptions: false,
        winTablePagination: true,
        pageSize: CommonConsts.PAGE_SIZE_SMALL
      }
    };
  }

  parseSelected() {
    each(this.selectionData, item => {
      if (!find(this.allSelectionMap[this.type], { uuid: item.uuid })) {
        this.allSelectionMap[this.type].push(item);
      }
    });
  }

  getDataMiddle(filters: Filters, args) {
    if (this.copyType === 'Backup') {
      this.getData(filters, args);
    } else {
      this.getReplicaData(filters, args);
    }
  }

  getData(filters: Filters, args) {
    const params = {
      pageNo: filters.paginator.pageIndex,
      pageSize: filters.paginator.pageSize,
      akLoading:
        !isUndefined(args) && args.isAutoPolling ? !args.isAutoPolling : true
    };

    const conditions = this.genConditionsServie.getConditions(this.subType);
    if (!isEmpty(filters.conditions_v2)) {
      const filterConditions = JSON.parse(filters.conditions_v2);
      if (filterConditions.resourceName) {
        assign(conditions, {
          name: filterConditions.resourceName
        });
      }
      if (filterConditions.resourceLocation) {
        assign(conditions, {
          path: filterConditions.resourceLocation
        });
      }
      assign(conditions, filterConditions);
    }

    assign(params, { conditions: JSON.stringify(conditions) });

    this.protectedResourceApiService.ListResources(params).subscribe(res => {
      this.antiRansomwareInfectedCopyService
        .antiRansomwareInfectedCopyConfigGet({
          pageNo: 0,
          pageSize: filters.paginator.pageSize,
          resourceIds: res.records.map(item => item.uuid)
        })
        .subscribe(result => {
          this.tableData = {
            data: res.records.map(item => {
              const resource = find(result.records, { resourceId: item.uuid });
              assign(item, {
                disabled: !isUndefined(resource),
                copyType: this.copyType
              });
              return item;
            }),
            total: res.totalCount
          };
        });
    });
  }

  getReplicaData(filters: Filters, args) {
    const params = {
      pageNo: filters.paginator.pageIndex,
      pageSize: filters.paginator.pageSize
    };
    const conditions = {
      resourceSubType: [this.subType],
      generated_by_array: [
        DataMap.CopyData_generatedType.replicate.value,
        DataMap.CopyData_generatedType.reverseReplication.value,
        DataMap.CopyData_generatedType.cascadedReplication.value
      ]
    };
    if (!isEmpty(filters.conditions)) {
      const filterConditions = JSON.parse(filters.conditions);
      if (filterConditions.name) {
        assign(conditions, {
          resourceName: filterConditions.name
        });
      }
      if (filterConditions.path) {
        assign(conditions, {
          resourceLocation: filterConditions.path
        });
      }
    }
    assign(params, { conditions: JSON.stringify(conditions) });
    this.copyControllerService
      .queryCopySummaryResourceV2(params)
      .subscribe(res => {
        this.antiRansomwareInfectedCopyService
          .antiRansomwareInfectedCopyConfigGet({
            pageNo: 0,
            pageSize: filters.paginator.pageSize,
            resourceIds: res.records.map(item => item.resourceId)
          })
          .subscribe(result => {
            this.tableData = {
              data: res.records.map(item => {
                const resource = find(result.records, {
                  resourceId: item.resourceId
                });
                assign(item, {
                  disabled: !isUndefined(resource),
                  copyType: this.copyType,
                  name: item.resourceName,
                  uuid: item.resourceId,
                  path: item.resourceLocation,
                  subType: item.resourceSubType
                });
                return item;
              }),
              total: res.totalCount
            };
          });
      });
  }
}
