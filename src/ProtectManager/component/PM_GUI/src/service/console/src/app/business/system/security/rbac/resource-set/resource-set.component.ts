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
  ChangeDetectorRef,
  Component,
  EventEmitter,
  OnInit,
  Output,
  TemplateRef,
  ViewChild
} from '@angular/core';
import {
  CommonConsts,
  CookieService,
  DataMap,
  getPermissionMenuItem,
  I18NService,
  MODAL_COMMON,
  OperateItems,
  ResourceSetApiService,
  RoleType,
  WarningMessageService
} from 'app/shared';
import { ProButton } from 'app/shared/components/pro-button/interface';
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
  cloneDeep,
  find,
  isEmpty,
  isUndefined,
  map,
  set,
  size,
  trim,
  values
} from 'lodash';
import { CreateResourcesetComponent } from './create-resourceset/create-resourceset.component';

@Component({
  selector: 'aui-resource-set',
  templateUrl: './resource-set.component.html',
  styleUrls: ['./resource-set.component.less']
})
export class ResourceSetComponent implements OnInit, AfterViewInit {
  @Output() openPage = new EventEmitter<any>();
  tableConfig: TableConfig;
  tableData: TableData;
  dataMap = DataMap;
  selectionData = [];
  isSysAdmin = this.cookieService.role === RoleType.SysAdmin;

  name;
  optItems = [];

  @ViewChild('dataTable', { static: false }) dataTable: ProTableComponent;
  @ViewChild('nameTpl', { static: true }) nameTpl: TemplateRef<any>;

  constructor(
    public i18n: I18NService,
    private cdr: ChangeDetectorRef,
    private drawModalService: DrawModalService,
    private resourceSetService: ResourceSetApiService,
    public warningMessageService: WarningMessageService,
    private cookieService: CookieService
  ) {}

  ngOnInit(): void {
    this.initConfig();
  }

  ngAfterViewInit() {
    this.dataTable?.fetchData();
  }

  initConfig() {
    const opts: { [key: string]: ProButton } = {
      modify: {
        id: 'modifyResourceSet',
        label: this.i18n.get('common_modify_label'),
        permission: OperateItems.SysadminOnly,
        disableCheck: data => {
          return data[0].isPublic || data[0].isDefault;
        },
        onClick: data => {
          this.create(data);
        }
      },
      delete: {
        id: 'deleteResourceSet',
        label: this.i18n.get('common_delete_label'),
        permission: OperateItems.SysadminOnly,
        disableCheck: data => {
          return data[0].isPublic || data[0].userNum > 0;
        },
        onClick: data => {
          this.delete(data);
        }
      }
    };
    this.optItems = cloneDeep(getPermissionMenuItem(values(opts)));
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
        name: this.i18n.get('common_resource_set_label'),
        cellRender: this.nameTpl,
        filter: {
          type: 'search',
          filterMode: 'contains'
        }
      },
      {
        key: 'userNum',
        name: this.i18n.get('system_role_associatedusers_label'),
        sort: true
      },
      {
        key: 'description',
        name: this.i18n.get('common_desc_label')
      },
      {
        key: 'operation',
        width: 130,
        hidden: !this.isSysAdmin,
        name: this.i18n.get('common_operation_label'),
        cellRender: {
          type: 'operation',
          config: {
            maxDisplayItems: 3,
            items: this.optItems
          }
        }
      }
    ];

    this.tableConfig = {
      table: {
        columns: cols,
        autoPolling: CommonConsts.TIME_INTERVAL_RESOURCE,
        compareWith: 'uuid',
        rows: {
          selectionMode: 'multiple',
          selectionTrigger: 'selector',
          showSelector: this.isSysAdmin
        },
        colDisplayControl: {
          ignoringColsType: 'hide'
        },
        fetchData: (filter: Filters, args) => {
          this.getData(filter, args);
        },
        selectionChange: selection => {
          this.selectionData = selection;
        },
        trackByFn: (index, item) => {
          return item.uuid;
        }
      }
    };
  }

  getData(filters?: Filters, args?) {
    const params = {
      pageNo: filters?.paginator.pageIndex,
      pageSize: filters?.paginator.pageSize,
      akLoading:
        !isUndefined(args) && args.isAutoPolling ? !args.isAutoPolling : true
    };

    if (!isEmpty(filters.conditions_v2)) {
      const conditionsTemp = JSON.parse(filters.conditions_v2);
      assign(params, { conditions: JSON.stringify(conditionsTemp) });
    }

    if (!!size(filters.sort)) {
      assign(params, { orders: filters.orders });
    }

    this.resourceSetService.queryResourceSet(params).subscribe(res => {
      res.records = map(res.records, item => {
        set(item, 'disabled', item.isDefault);
        return item;
      });
      this.tableData = {
        data: res.records,
        total: res.totalCount
      };
      this.cdr.detectChanges();
    });
  }

  create(data?) {
    this.drawModalService.create(
      assign({}, MODAL_COMMON.generateDrawerOptions(), {
        lvModalKey: 'create-resourceSet',
        lvWidth: MODAL_COMMON.xLargeWidth + 100,
        lvHeader: !!data
          ? this.i18n.get('system_modify_resource_set_label')
          : this.i18n.get('system_create_resource_set_label'),
        lvContent: CreateResourcesetComponent,
        lvOkDisabled: true,
        lvComponentParams: {
          data
        },
        lvOk: modal => {
          return new Promise(resolve => {
            const content = modal.getContentComponent() as CreateResourcesetComponent;
            content.onOK().subscribe({
              next: () => {
                resolve(true);
                this.dataTable.fetchData();
              },
              error: () => {
                resolve(false);
              }
            });
          });
        }
      })
    );
  }

  getDetail(data) {
    this.openPage.emit({
      name: 'resourceSetDetail',
      data: data
    });
  }

  validDelete() {
    return !!find(
      this.selectionData,
      item => item.isPublic || item.userNum > 0
    );
  }

  delete(data) {
    const params: any = {
      resourceSetIdList: map(data, item => item.uuid)
    };
    this.warningMessageService.create({
      content: this.i18n.get('system_resourceset_delete_tip_label', [
        map(data, item => item.name).join(',')
      ]),
      onOK: () => {
        this.resourceSetService
          .batchDeleteResourceSetId({
            resourceSetDeleteRequest: params
          })
          .subscribe(res => {
            this.dataTable.setSelections([]);
            this.selectionData = [];
            this.dataTable.fetchData();
          });
      }
    });
  }

  search() {
    assign(this.dataTable?.filterMap, {
      filters: [
        {
          filterMode: 'contains',
          caseSensitive: false,
          key: 'name',
          value: trim(this.name)
        }
      ]
    });
    this.dataTable?.fetchData();
  }
}
