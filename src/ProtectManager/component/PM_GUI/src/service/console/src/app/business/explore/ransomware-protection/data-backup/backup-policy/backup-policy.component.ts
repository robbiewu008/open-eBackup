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
import {
  AfterViewInit,
  Component,
  EventEmitter,
  OnInit,
  Output,
  TemplateRef,
  ViewChild
} from '@angular/core';
import {
  CommonConsts,
  getPermissionMenuItem,
  getTableOptsItems,
  I18NService,
  OperateItems,
  SlaApiService,
  WarningMessageService
} from 'app/shared';
import {
  Filters,
  ProTableComponent,
  TableCols,
  TableConfig,
  TableData
} from 'app/shared/components/pro-table';
import { AppUtilsService } from 'app/shared/services/app-utils.service';
import { BatchOperateService } from 'app/shared/services/batch-operate.service';
import { SlaService } from 'app/shared/services/sla.service';
import { VirtualScrollService } from 'app/shared/services/virtual-scroll.service';
import {
  assign,
  cloneDeep,
  each,
  find,
  first,
  includes,
  isEmpty,
  isUndefined,
  size
} from 'lodash';

@Component({
  selector: 'aui-ransomware-backup-policy',
  templateUrl: './backup-policy.component.html',
  styleUrls: ['./backup-policy.component.less'],
  providers: [DatePipe]
})
export class BackupPolicyComponent implements OnInit, AfterViewInit {
  tableConfig: TableConfig;
  tableData: TableData;
  selectionData = [];
  optItems: any[];

  @Output() refreshPolicy = new EventEmitter();

  @ViewChild('dataTable', { static: false }) dataTable: ProTableComponent;
  @ViewChild('slaTpl', { static: true }) slaTpl: TemplateRef<any>;
  @ViewChild('countTpl', { static: true }) countTpl: TemplateRef<any>;
  @ViewChild('polic1yTpl', { static: true }) polic1yTpl: TemplateRef<any>;
  @ViewChild('polic2yTpl', { static: true }) polic2yTpl: TemplateRef<any>;
  @ViewChild('polic3yTpl', { static: true }) polic3yTpl: TemplateRef<any>;
  @ViewChild('polic4yTpl', { static: true }) polic4yTpl: TemplateRef<any>;

  constructor(
    private i18n: I18NService,
    private datePipe: DatePipe,
    private slaService: SlaService,
    public slaApiService: SlaApiService,
    private appUtilsService: AppUtilsService,
    private virtualScroll: VirtualScrollService,
    private batchOperateService: BatchOperateService,
    private warningMessageService: WarningMessageService
  ) {}

  ngAfterViewInit(): void {
    // 由资源页面跳转过来
    const policyName = this.appUtilsService.getCacheValue(
      'copyDetectionPolicyName',
      false
    );
    if (policyName) {
      this.dataTable.setFilterMap(
        assign(this.dataTable.filterMap, {
          filters: [
            {
              caseSensitive: false,
              key: 'name',
              value: policyName
            }
          ]
        })
      );
    }
    this.dataTable.fetchData();
  }

  ngOnInit(): void {
    this.initConfig();
  }

  initConfig() {
    const opts = [
      {
        id: 'clone',
        label: this.i18n.get('common_clone_label'),
        permission: OperateItems.CloneSLA,
        onClick: data => {
          this.create(first(data), true);
        }
      },
      {
        id: 'modify',
        label: this.i18n.get('common_modify_label'),
        permission: OperateItems.ModifySLA,
        onClick: data => {
          this.create(first(data));
        }
      },
      {
        id: 'delete',
        label: this.i18n.get('common_delete_label'),
        permission: OperateItems.DeleteSLA,
        onClick: data => {
          this.delete(data);
        }
      }
    ];
    this.optItems = getPermissionMenuItem(cloneDeep(opts));
    const cols: TableCols[] = [
      {
        key: 'name',
        name: this.i18n.get('common_name_label'),
        filter: {
          type: 'search',
          filterMode: 'contains'
        },
        cellRender: this.slaTpl,
        sort: true
      },
      {
        key: 'policy1',
        name: this.i18n.get('common_policy_params_label', ['01']),
        cellRender: this.polic1yTpl
      },
      {
        key: 'policy2',
        name: this.i18n.get('common_policy_params_label', ['02']),
        cellRender: this.polic2yTpl
      },
      {
        key: 'policy3',
        name: this.i18n.get('common_policy_params_label', ['03']),
        cellRender: this.polic3yTpl,
        hidden: true
      },
      {
        key: 'policy4',
        name: this.i18n.get('common_policy_params_label', ['04']),
        cellRender: this.polic4yTpl,
        hidden: true
      },
      {
        key: 'resource_count',
        name: this.i18n.get('common_associated_resource_label'),
        cellRender: this.countTpl
      },
      {
        key: 'operation',
        width: 130,
        hidden: 'ignoring',
        name: this.i18n.get('common_operation_label'),
        cellRender: {
          type: 'operation',
          config: {
            maxDisplayItems: 1,
            items: getPermissionMenuItem(opts)
          }
        }
      }
    ];

    this.tableConfig = {
      table: {
        autoPolling: CommonConsts.TIME_INTERVAL,
        compareWith: 'uuid',
        columns: cols,
        rows: {
          selectionMode: 'multiple',
          selectionTrigger: 'selector',
          showSelector: true
        },
        scrollFixed: true,
        colDisplayControl: {
          ignoringColsType: 'hide'
        },
        fetchData: (filter: Filters, args) => {
          this.getData(filter, args);
        },
        selectionChange: selection => {
          this.selectionData = selection;
        },
        trackByFn: (_, item) => {
          return item.uuid;
        }
      }
    };
  }

  getData(filters: Filters, args) {
    const params = {
      pageNo: filters.paginator.pageIndex,
      pageSize: filters.paginator.pageSize,
      akLoading:
        !isUndefined(args) && args.isAutoPolling ? !args.isAutoPolling : true
    };

    if (!isEmpty(filters.conditions)) {
      const conditionsTemp = JSON.parse(filters.conditions);
      assign(params, conditionsTemp);
    }

    if (filters.sort?.key) {
      assign(params, {
        orderType: filters.sort?.direction,
        orderBy: filters.sort?.key
      });
    }

    const policyName = this.appUtilsService.getCacheValue(
      'copyDetectionPolicyName'
    );

    this.slaApiService.pageQueryUsingGET(params).subscribe(res => {
      each(res.items, item => {
        assign(item, {
          policy1: find(item.policy_list, item => includes(item.name, '01')),
          policy2: find(item.policy_list, item => includes(item.name, '02')),
          policy3: find(item.policy_list, item => includes(item.name, '03')),
          policy4: find(item.policy_list, item => includes(item.name, '04'))
        });
      });
      this.tableData = {
        data: res.items,
        total: res.total
      };
      if (policyName && find(res.items, item => item.name === policyName)) {
        this.getPolicyDetail(find(res.items, item => item.name === policyName));
      }
      this.refreshPolicy.emit();
    });
  }

  getPolicyDetail(item, isResource?) {
    if (isResource) {
      assign(item, { isResource });
    }
    this.slaService.getAntiDetail(
      item,
      getTableOptsItems(cloneDeep(this.optItems), item, this)
    );
  }

  getDetectionPolicy(backupItem): string {
    return this.appUtilsService.getDetectionPolicy(backupItem, this.datePipe);
  }

  create(rowData?, isClone?) {
    this.slaService.createDetectionPolicy(
      () => this.dataTable.fetchData(),
      rowData,
      isClone
    );
  }

  delete(data?: any[]) {
    if (size(data) === 1) {
      this.warningMessageService.create({
        content: this.i18n.get('protection_detection_policy_delete_label', [
          data[0].name
        ]),
        onOK: () =>
          this.slaApiService
            .deleteSLACyberUsingDELETE({ slaId: data[0].uuid })
            .subscribe(() => {
              this.selectionData = [];
              this.dataTable.setSelections([]);
              this.dataTable.fetchData();
            })
      });
    } else {
      this.warningMessageService.create({
        content: this.i18n.get(
          'protection_detection_policy_batch_delete_label'
        ),
        onOK: () =>
          this.batchOperateService.selfGetResults(
            item => {
              return this.slaApiService.deleteSLACyberUsingDELETE({
                slaId: item.uuid,
                akDoException: false,
                akOperationTips: false,
                akLoading: false
              });
            },
            data,
            () => {
              this.selectionData = [];
              this.dataTable.setSelections([]);
              this.dataTable.fetchData();
            }
          )
      });
    }
  }
}
