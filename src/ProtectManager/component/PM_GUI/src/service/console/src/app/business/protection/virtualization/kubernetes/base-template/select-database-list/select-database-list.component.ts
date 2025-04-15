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
  OnInit,
  TemplateRef,
  ViewChild
} from '@angular/core';
import {
  CommonConsts,
  DataMap,
  extendSlaInfo,
  I18NService,
  MODAL_COMMON,
  ProtectedResourceApiService
} from 'app/shared';
import {
  Filters,
  ProTableComponent,
  TableCols,
  TableConfig,
  TableData
} from 'app/shared/components/pro-table';
import { DrawModalService } from 'app/shared/services/draw-modal.service';
import {
  assign,
  defer,
  each,
  every,
  first,
  get,
  isEmpty,
  map,
  reject,
  size
} from 'lodash';
import { Observable, Subject } from 'rxjs';
import { AddVolumeComponent } from './add-volume/add-volume.component';

@Component({
  selector: 'aui-select-database-list',
  templateUrl: './select-database-list.component.html',
  styleUrls: ['./select-database-list.component.less']
})
export class SelectDatabaseListComponent implements OnInit, AfterViewInit {
  activeIndex = 'selected';
  tableConfig: TableConfig;
  datasTableConfig: TableConfig;
  tableData: TableData;
  selectionData: any[];
  selectData: TableData;
  total = CommonConsts.PAGE_TOTAL;
  selectedDataNum;
  title: string = this.i18n.get('common_selected_label');
  modifyFlag: boolean = false;
  resourceSubType: string;
  dataMap = DataMap;
  source: any;
  valid$ = new Subject<boolean>();

  singleSelectFlag: boolean = false;
  enableSelectAll: boolean = false;

  @ViewChild('dataTable', { static: false }) dataTable: ProTableComponent;
  @ViewChild('selectTable', { static: false }) selectTable: ProTableComponent;
  @ViewChild('slaTpl', { static: true }) slaTpl: TemplateRef<any>;
  @ViewChild('selectVolTpl', { static: true }) selectVolTpl: TemplateRef<any>;
  @ViewChild('slaComplianceExtraTpl', { static: true })
  slaComplianceExtraTpl: TemplateRef<any>;

  constructor(
    private i18n: I18NService,
    private drawModalService: DrawModalService,
    private protectedResourceApiService: ProtectedResourceApiService
  ) {}

  ngAfterViewInit() {
    if (this.dataTable) {
      this.dataTable.fetchData();
    }
    if (this.selectTable) {
      this.selectTable.fetchData();
    }
  }

  ngOnInit(): void {
    this.initTable();
  }

  getResourceType() {
    if (this.resourceSubType === DataMap.Resource_Type.MongoDB.value) {
      return [
        DataMap.Resource_Type.MongodbClusterInstance.value,
        DataMap.Resource_Type.MongodbSingleInstance.value
      ];
    }
    return [this.resourceSubType];
  }

  getData(filters: Filters, args: any) {
    const params = {
      pageNo: filters.paginator.pageIndex,
      pageSize: filters.paginator.pageSize
    };
    const defaultConditions = {
      subType: this.getResourceType()
    };
    if (this.resourceSubType === DataMap.Resource_Type.MongoDB.value) {
      assign(defaultConditions, {
        isTopInstance: [['=='], '1']
      });
    }
    assign(params, { conditions: JSON.stringify(defaultConditions) });

    if (!!size(filters.sort)) {
      assign(params, { orders: filters.orders });
    }
    const queryFunc: Observable<any> =
      this.resourceSubType === DataMap.Resource_Type.vmGroup.value
        ? this.protectedResourceApiService.ListResourceGroups(params)
        : this.protectedResourceApiService.ListResources(params);
    queryFunc.subscribe(res => {
      each(res.records, item => {
        assign(item, {
          sub_type: item.subType,
          cluster: item.environment?.name,
          endpoint: item.environment?.endpoint,
          parentName: item.parentName,
          disabled: !isEmpty(item.protectedObject?.slaId)
        });
        extendSlaInfo(item);
      });
      this.tableData = {
        data: res.records,
        total: res.totalCount
      };
      this.total = res.totalCount;
      this.selectionData = this.selectData.data;
      this.dataTable.setSelections(this.selectData.data);
    });
  }

  initData(data: any, resourceType: string) {
    this.resourceSubType = resourceType;
    this.source = first(data);
    this.modifyFlag = size(data) === 1 && !isEmpty(data[0].sla_id);
    this.singleSelectFlag = size(data) === 1;
    this.selectedDataNum = size(data);
    if (
      this.modifyFlag &&
      this.resourceSubType === DataMap.Resource_Type.KubernetesStatefulset.value
    ) {
      each(data, item => {
        assign(item, {
          enableSelectAll:
            size(item.protectedObject?.extParameters?.volume_names) ===
            size(JSON.parse(get(item, ['extendInfo', 'sts']))?.volumeNames),
          volumes: item.protectedObject?.extParameters?.volume_names
        });
      });
    }
    if (
      !this.modifyFlag &&
      this.resourceSubType === DataMap.Resource_Type.KubernetesStatefulset.value
    ) {
      each(data, item => {
        assign(item, {
          enableSelectAll: true
        });
      });
    }
    this.selectData = {
      data: data,
      total: size(data)
    };
  }

  initTable() {
    const cols: TableCols[] = [
      {
        key: 'name',
        name: this.i18n.get('common_name_label')
      },
      {
        key: 'sla_name',
        name: 'SLA',
        cellRender: this.slaTpl
      },
      {
        key: 'volume',
        width: 200,
        thExtra: this.slaComplianceExtraTpl,
        name: this.i18n.get('common_kubernetes_volume_protect_name_label'),
        hidden:
          this.resourceSubType !==
          DataMap.Resource_Type.KubernetesStatefulset.value,
        cellRender: this.selectVolTpl
      },
      {
        key: 'operation',
        name: this.i18n.get('common_operation_label'),
        width: 110,
        hidden: this.singleSelectFlag,
        cellRender: {
          type: 'operation',
          config: {
            maxDisplayItems: 2,
            items: [
              {
                id: 'remove',
                label: this.i18n.get('common_remove_label'),
                onClick: ([data]) => {
                  this.selectData = {
                    data: reject(this.selectData.data, item => {
                      return item.uuid === data.uuid;
                    }),
                    total: --this.selectData.total
                  };
                  this.selectionData = this.selectData.data;
                  this.dataTable.setSelections(this.selectData.data);
                  this.selectedDataNum = size(this.selectData.data);
                  this.setVaild();
                }
              }
            ]
          }
        }
      }
    ];

    this.datasTableConfig = {
      table: {
        size: 'small',
        columns: [cols[0], cols[1]],
        async: true,
        rows: {
          selectionMode: 'multiple',
          selectionTrigger: 'selector',
          showSelector: true
        },
        compareWith: 'uuid',
        colDisplayControl: false,
        scrollFixed: true,
        fetchData: (filter: Filters, args: {}) => {
          this.getData(filter, args);
        },
        selectionChange: selection => {
          this.selectionData = selection;
          this.selectData = {
            data: selection,
            total: size(selection)
          };
          this.selectedDataNum = size(selection);
          this.setVaild();
        },
        trackByFn: (index, item) => {
          return item.uuid;
        }
      },
      pagination: {
        pageSize: CommonConsts.PAGE_SIZE_SMALL,
        pageSizeOptions: CommonConsts.SIMPLE_PAGE_SIZE_OPTIONS,
        winTablePagination: true,
        mode: 'simple',
        showPageSizeOptions: false
      }
    };
    this.tableConfig = {
      table: {
        size: 'small',
        columns: cols,
        async: false,
        colDisplayControl: false,
        compareWith: 'uuid',
        trackByFn: (index, item) => {
          return item.uuid;
        }
      },
      pagination: {
        pageSize: CommonConsts.PAGE_SIZE_SMALL,
        pageSizeOptions: CommonConsts.SIMPLE_PAGE_SIZE_OPTIONS,
        winTablePagination: true,
        mode: 'simple',
        showPageSizeOptions: false
      }
    };
  }

  switchChange() {
    defer(() => this.setVaild());
  }

  selectPod(data) {
    this.drawModalService.create(
      assign({}, MODAL_COMMON.generateDrawerOptions(), {
        lvModalKey: 'add-KubernetesStatefulset-volume',
        lvWidth: MODAL_COMMON.normalWidth + 100,
        lvHeader: this.i18n.get('common_kubernetes_select_volume_label'),
        lvContent: AddVolumeComponent,
        lvComponentParams: { data },
        lvOkDisabled: isEmpty(data.volumes),
        lvOk: modal => {
          const content = modal.getContentComponent() as AddVolumeComponent;
          assign(data, {
            volumes: map(content.onOK(), 'name')
          });
          this.setVaild();
        }
      })
    );
  }

  setVaild() {
    if (this.singleSelectFlag) {
      if (
        this.resourceSubType !==
        DataMap.Resource_Type.KubernetesStatefulset.value
      ) {
        this.valid$.next(true);
      } else {
        this.valid$.next(
          this.source.enableSelectAll || !isEmpty(this.source.volumes)
        );
      }
    } else {
      if (
        this.resourceSubType !==
        DataMap.Resource_Type.KubernetesStatefulset.value
      ) {
        this.valid$.next(!!size(this.selectionData));
      } else {
        this.valid$.next(
          !!size(this.selectionData) &&
            every(this.selectionData, item => {
              return item.enableSelectAll || !isEmpty(item.volumes);
            })
        );
      }
    }
  }

  onOK() {
    return { selectedList: this.selectionData };
  }
}
