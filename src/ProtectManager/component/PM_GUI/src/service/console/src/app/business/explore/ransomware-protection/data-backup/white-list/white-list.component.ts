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
  BackupCopyWhitelistManagementService,
  CommonConsts,
  DataMapService,
  I18NService,
  MODAL_COMMON,
  OperateItems,
  WarningMessageService,
  getPermissionMenuItem
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
  includes,
  isEmpty,
  isUndefined,
  map,
  reject,
  size,
  values
} from 'lodash';
import { CreateWhiteListComponent } from './create-white-list/create-white-list.component';
import { AssociateFsComponent } from './associate-policy/associate-fs.component';

@Component({
  selector: 'aui-data-backup-white-list',
  templateUrl: './white-list.component.html',
  styleUrls: ['./white-list.component.less']
})
export class WhiteListComponent implements OnInit, AfterViewInit {
  tableConfig: TableConfig;
  tableData: TableData;
  optsConfig;
  selectionData = [];
  optItems;

  @ViewChild('dataTable', { static: false }) dataTable: ProTableComponent;
  @ViewChild('policyTpl', { static: true })
  policyTpl: TemplateRef<any>;

  constructor(
    private i18n: I18NService,
    private dataMapService: DataMapService,
    private drawModalService: DrawModalService,
    private warningMessageService: WarningMessageService,
    private backupCopyWhitelistService: BackupCopyWhitelistManagementService
  ) {}

  ngAfterViewInit(): void {
    this.dataTable?.fetchData();
  }
  ngOnInit(): void {
    this.initConfig();
  }

  initConfig() {
    const opts: { [key: string]: ProButton } = {
      create: {
        id: 'create',
        type: 'primary',
        permission: OperateItems.RegisterApplication,
        label: this.i18n.get('common_create_label'),
        onClick: () => this.create()
      },
      associate: {
        id: 'associate',
        permission: OperateItems.RegisterApplication,
        label: this.i18n.get('explore_associate_file_system_label'),
        disableCheck: data => {
          return !size(data);
        },
        onClick: data => this.associateFs(data, true)
      },
      disassociate: {
        id: 'disassociate',
        permission: OperateItems.RegisterApplication,
        divide: true,
        label: this.i18n.get('explore_disassociate_label'),
        disableCheck: data => {
          return (
            !size(data) ||
            !isEmpty(find(data, item => item.assocFsNumber === 0))
          );
        },
        onClick: data => this.associateFs(data)
      },
      delete: {
        id: 'delete',
        permission: OperateItems.RegisterApplication,
        label: this.i18n.get('common_delete_label'),
        disableCheck: data => {
          return !size(data);
        },
        onClick: data => this.delete(data)
      }
    };

    this.optItems = cloneDeep(
      getPermissionMenuItem(
        reject(values(opts), item => includes(['create'], item.id))
      )
    );

    this.optsConfig = getPermissionMenuItem(values(opts));

    const cols: TableCols[] = [
      {
        key: 'whitelistItemContent',
        name: this.i18n.get('common_name_label'),
        filter: {
          type: 'search',
          filterMode: 'contains'
        }
      },
      {
        key: 'whitelistItemType',
        name: this.i18n.get('common_type_label'),
        filter: {
          type: 'select',
          isMultiple: true,
          showCheckAll: true,
          options: this.dataMapService.toArray('dataBackupWhitelistType')
        },
        cellRender: {
          type: 'status',
          config: this.dataMapService.toArray('dataBackupWhitelistType')
        }
      },
      {
        key: 'assocFsNumber',
        name: this.i18n.get('explore_association_file_system_num_label'),
        cellRender: this.policyTpl
      },
      {
        key: 'createTime',
        name: this.i18n.get('common_create_time_label')
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
            items: this.optItems
          }
        }
      }
    ];

    this.tableConfig = {
      table: {
        autoPolling: CommonConsts.TIME_INTERVAL_RESOURCE,
        compareWith: 'id',
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
        fetchData: (filter: Filters, args) => this.getData(filter, args),
        selectionChange: selection => {
          this.selectionData = selection;
        },
        trackByFn: (_, item) => {
          return item.id;
        }
      }
    };
  }

  getData(filters: Filters, args) {
    const params = {
      pageNum: filters.paginator.pageIndex + 1,
      pageSize: filters.paginator.pageSize,
      akLoading:
        !isUndefined(args) && args.isAutoPolling ? !args.isAutoPolling : true
    };

    if (!isEmpty(filters.conditions)) {
      const conditions = JSON.parse(filters.conditions);
      if (conditions.whitelistItemContent) {
        assign(params, { whitelistItemName: conditions.whitelistItemContent });
        delete conditions.whitelistItemContent;
      }
      assign(params, conditions);
    }

    this.backupCopyWhitelistService.listWhiteListItem(params).subscribe(res => {
      this.tableData = {
        data: res.records,
        total: res.totalCount
      };
    });
  }

  getModalTitle(isDisassociated = false, isDetail = false) {
    if (isDetail) {
      return this.i18n.get('explore_association_file_system_label');
    }
    if (isDisassociated) {
      return this.i18n.get('explore_associate_file_system_label');
    }
    return this.i18n.get('explore_disassociate_label');
  }

  associateFn(modal) {
    return new Promise(resolve => {
      const content = modal.getContentComponent() as AssociateFsComponent;
      content.onOK().subscribe({
        next: () => {
          resolve(true);
          this.dataTable?.fetchData();
          this.selectionData = [];
          this.dataTable?.setSelections([]);
        },
        error: () => resolve(false)
      });
    });
  }

  associateFs(data, isDisassociated = false, isDetail = false) {
    const modalParams = {
      ...MODAL_COMMON.generateDrawerOptions(),
      lvHeader: this.getModalTitle(isDisassociated, isDetail),
      lvModalKey: 'associate-policy-modal',
      lvWidth: MODAL_COMMON.normalWidth + 100,
      lvContent: AssociateFsComponent,
      lvComponentParams: {
        rowData: data,
        isDisassociated,
        isDetail
      }
    };
    if (isDetail) {
      assign(modalParams, {
        lvFooter: [
          {
            label: this.i18n.get('common_close_label'),
            onClick: modal => modal.close()
          }
        ]
      });
    } else {
      assign(modalParams, {
        lvOkDisabled: true,
        lvOk: modal => this.associateFn(modal)
      });
    }
    this.drawModalService.create(modalParams);
  }

  create() {
    this.drawModalService.create({
      ...MODAL_COMMON.generateDrawerOptions(),
      lvHeader: this.i18n.get('common_create_label'),
      lvModalKey: 'create-data-backup-white-list-modal',
      lvWidth: this.i18n.isEn
        ? MODAL_COMMON.normalWidth + 200
        : MODAL_COMMON.normalWidth + 100,
      lvContent: CreateWhiteListComponent,
      lvOkDisabled: true,
      lvAfterOpen: modal => {
        const content = modal.getContentComponent() as CreateWhiteListComponent;
        content.formGroup.statusChanges.subscribe(res => {
          modal.lvOkDisabled = res !== 'VALID';
        });
      },
      lvOk: modal => {
        return new Promise(resolve => {
          const content = modal.getContentComponent() as CreateWhiteListComponent;
          content.onOK().subscribe({
            next: () => {
              resolve(true);
              this.dataTable?.fetchData();
            },
            error: () => resolve(false)
          });
        });
      }
    });
  }

  delete(datas) {
    this.warningMessageService.create({
      content: this.i18n.get(
        'explore_delete_white_list_label',
        [map(datas, 'whitelistItemContent').join(',')],
        false,
        true
      ),
      onOK: () => {
        this.backupCopyWhitelistService
          .deleteWhiteListItem({
            deleteWhiteListRequest: {
              whitelistIds: map(datas, 'id')
            }
          })
          .subscribe(() => {
            this.selectionData = [];
            this.dataTable.setSelections([]);
            this.dataTable.fetchData();
          });
      }
    });
  }
}
