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
  ChangeDetectorRef,
  Component,
  Input,
  OnInit,
  TemplateRef,
  ViewChild
} from '@angular/core';
import {
  ColorConsts,
  CommonConsts,
  DataMap,
  DataMapService,
  I18NService,
  StorageUserAuthService
} from 'app/shared';
import {
  Filters,
  ProTableComponent,
  TableCols,
  TableConfig
} from 'app/shared/components/pro-table';
import { assign, each, isEmpty, isUndefined } from 'lodash';

@Component({
  selector: 'aui-user-auth',
  templateUrl: './user-auth.component.html',
  styleUrls: ['./user-auth.component.less']
})
export class UserAuthComponent implements OnInit {
  datamap = DataMap;
  progressBarColor = [[0, ColorConsts.NORMAL]];
  userAuthTableConfig: TableConfig;
  userAuthtableData;
  optsConfig;
  selectionData = [];
  authTypeList = this.dataMapService.toArray('authType');
  @Input() storageId: string;

  // authType: 0:未授权， 1:存储单元授权，2:存储单元组授权
  @Input() authType: number;

  @ViewChild('authStatusTpl', { static: true }) authStatusTpl: TemplateRef<any>;
  @ViewChild('dataTable', { static: false }) dataTable: ProTableComponent;

  constructor(
    private i18n: I18NService,
    private cdr: ChangeDetectorRef,
    private storageUserAuthService: StorageUserAuthService,
    public dataMapService: DataMapService
  ) {}

  ngOnInit() {
    this.initConfig();
  }

  ngAfterViewInit() {
    this.dataTable.fetchData();
  }

  initConfig() {
    // 批量新增、删除按钮config
    const opts = [
      {
        id: 'batchAuth',
        type: 'primary',
        label: this.i18n.get('common_batch_auth_label'),
        displayCheck: () => {
          return true;
        },
        onClick: () => {
          this.batchAuth();
        },
        disableCheck: () => {
          if (isEmpty(this.selectionData)) {
            return true;
          }
          let isDisable = false;
          each(this.selectionData, item => {
            if (item.authStatus) {
              isDisable = true;
            }
          });
          return isDisable;
        }
      },
      {
        id: 'batchCancelAuth',
        label: this.i18n.get('common_batch_cancel_auth_label'),
        displayCheck: () => {
          return true;
        },
        disableCheck: data => {
          if (isEmpty(this.selectionData)) {
            return true;
          }
          let isDisable = false;
          each(this.selectionData, item => {
            if (!item.authStatus) {
              isDisable = true;
            }
            // 从存储单元进入授权界面，已被存储单元组授权的无法批量取消授权
            if (item.authStatus && this.authType !== 2 && item.authType === 2) {
              isDisable = true;
            }
          });
          return isDisable;
        },
        onClick: () => {
          this.batchCancelAuth();
        }
      }
    ];
    this.optsConfig = opts;

    // 数据config
    const cols: TableCols[] = [
      {
        key: 'userId',
        name: 'userId',
        hidden: true
      },
      {
        key: 'userName',
        name: this.i18n.get('common_username_label'),
        filter: {
          type: 'search',
          filterMode: 'contains'
        }
      },
      {
        key: 'authStatus',
        name: this.i18n.get('common_auth_status_label'),
        cellRender: this.authStatusTpl,
        filter: {
          type: 'select',
          isMultiple: true,
          showCheckAll: true,
          options: this.authTypeList
        }
      }
    ];

    this.userAuthTableConfig = {
      table: {
        compareWith: 'userId',
        columns: cols,
        scrollFixed: true,
        autoPolling: CommonConsts.TIME_INTERVAL,
        rows: {
          selectionMode: 'multiple',
          selectionTrigger: 'selector',
          showSelector: true
        },
        colDisplayControl: false,
        fetchData: (filter: Filters, args) => {
          this.getData(filter, args);
        },
        selectionChange: selection => {
          this.selectionData = selection;
        },
        trackByFn: (index, item) => {
          return item?.userId || item.uuid;
        }
      }
    };
  }

  getData(filter: Filters, args) {
    const params: any = {
      pageNo: filter.paginator.pageIndex,
      pageSize: filter.paginator.pageSize,
      akLoading:
        !isUndefined(args) && args.isAutoPolling ? !args.isAutoPolling : true
    };
    if (!isEmpty(filter.conditions)) {
      const conditionsTemp = JSON.parse(filter.conditions);
      if (conditionsTemp.userName) {
        assign(params, {
          userName: conditionsTemp.userName
        });
      }
      if (conditionsTemp.authStatus) {
        assign(params, {
          filteredAuthTypes: conditionsTemp.authStatus
        });
      }
    }

    assign(params, { storageId: this.storageId, authType: this.authType });
    this.storageUserAuthService
      .getStorageUserAuthRelations(params)
      .subscribe(res => {
        this.userAuthtableData = {
          data: res.records,
          total: res.totalCount
        };
        this.cdr.detectChanges();
      });
  }

  batchAuth() {
    const params = { userIds: [], authType: this.authType };
    each(this.selectionData, item => params.userIds.push(item.userId));
    this.storageUserAuthService
      .addStorageUserAuthRelations({
        storageId: this.storageId,
        storageUserAuthRelationRequest: params
      })
      .subscribe(res => {
        this.dataTable.setSelections([]);
        this.selectionData = [];
        this.dataTable.fetchData();
      });
  }

  batchCancelAuth() {
    const params = { userIds: [] };
    each(this.selectionData, item => params.userIds.push(item.userId));
    this.storageUserAuthService
      .deleteStorageUserAuthRelations({
        storageId: this.storageId,
        storageUserAuthRelationRequest: params
      })
      .subscribe(res => {
        this.dataTable.setSelections([]);
        this.selectionData = [];
        this.dataTable.fetchData();
      });
  }

  AuthStatusChange(data) {
    const params: any = {};
    if (data.authStatus) {
      assign(params, { userIds: [data.userId], authType: this.authType });
      this.storageUserAuthService
        .addStorageUserAuthRelations({
          storageId: this.storageId,
          storageUserAuthRelationRequest: params
        })
        .subscribe(res => {
          this.dataTable.fetchData();
        });
    } else {
      assign(params, { userIds: [data.userId] });
      this.storageUserAuthService
        .deleteStorageUserAuthRelations({
          storageId: this.storageId,
          storageUserAuthRelationRequest: params
        })
        .subscribe(res => {
          this.dataTable.fetchData();
        });
    }
  }

  disableUserAuth(item) {
    // 从存储单元进入授权界面，已被存储单元组授权的按钮置灰
    const disableFlag =
      item.authStatus === true && this.authType !== 2 && item.authType === 2;
    assign(item, {
      disabled: disableFlag
    });
    return disableFlag;
  }
}
