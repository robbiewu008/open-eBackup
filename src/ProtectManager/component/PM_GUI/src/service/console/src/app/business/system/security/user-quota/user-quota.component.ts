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
import { CommonModule } from '@angular/common';
import {
  AfterViewInit,
  ChangeDetectionStrategy,
  ChangeDetectorRef,
  Component,
  NgModule,
  OnInit,
  TemplateRef,
  ViewChild
} from '@angular/core';
import { FormsModule } from '@angular/forms';
import { CheckboxModule } from '@iux/live';
import {
  ApiQuotaService,
  BaseModule,
  CAPACITY_UNIT,
  ColorConsts,
  CookieService,
  DataMap,
  DataMapService,
  FunctionSwitchApiService,
  I18NService,
  MODAL_COMMON,
  OperateItems,
  UsersApiService,
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
import { AppUtilsService } from 'app/shared/services/app-utils.service';
import { DrawModalService } from 'app/shared/services/draw-modal.service';
import { VirtualScrollService } from 'app/shared/services/virtual-scroll.service';
import {
  assign,
  each,
  find,
  first,
  get,
  reject,
  set,
  size,
  toLower,
  toNumber
} from 'lodash';
import { Subject } from 'rxjs';
import { map } from 'rxjs/operators';
import { SetQuotaComponent } from './set-quota/set-quota.component';

@Component({
  selector: 'aui-user-quota',
  templateUrl: './user-quota.component.html',
  styleUrls: ['./user-quota.component.less'],
  changeDetection: ChangeDetectionStrategy.OnPush
})
export class UserQuotaComponent implements OnInit, AfterViewInit {
  readonly INIT_SCROLL_HIGHT = 200;
  tableConfig: TableConfig;
  tableData: TableData;
  optsConfig;
  userFunctionComponent = UserFunctionComponent;
  unitconst = CAPACITY_UNIT;
  progressBarColor = [
    [0, ColorConsts.NORMAL],
    [80, ColorConsts.ABNORMAL]
  ];
  lessThanLabel = this.i18n.get('common_less_than_label');

  @ViewChild('dataTable', { static: false }) dataTable: ProTableComponent;
  @ViewChild('userTypeTpl', { static: true })
  userTypeTpl: TemplateRef<any>;
  @ViewChild('backupCapacity', { static: true })
  backupCapacity: TemplateRef<any>;
  @ViewChild('archiveCapacity', { static: true })
  archiveCapacity: TemplateRef<any>;

  constructor(
    public i18n: I18NService,
    public cdr: ChangeDetectorRef,
    public cookieService: CookieService,
    public dataMapService: DataMapService,
    public usersApiService: UsersApiService,
    public userQuotaService: ApiQuotaService,
    public drawModalService: DrawModalService,
    public virtualScroll: VirtualScrollService,
    public warningMessageService: WarningMessageService,
    public functionSwitchApiService: FunctionSwitchApiService,
    public appUtilsService: AppUtilsService
  ) {}

  ngAfterViewInit() {
    this.dataTable.fetchData();
  }

  ngOnInit() {
    this.initConfig();
    this.virtualScroll.getScrollParam(this.INIT_SCROLL_HIGHT);
  }

  onChange() {
    this.ngOnInit();
    this.ngAfterViewInit();
  }

  initConfig() {
    const opts: ProButton[] = [
      {
        id: 'backupQuota',
        permission: OperateItems.DeleteKerberos,
        label: this.i18n.get('protection_set_backup_quota_label'),
        onClick: data => {
          this.setQuota(data, DataMap.userFunction.backup.value);
        },
        displayCheck: ([data]) => {
          return data.userType !== DataMap.loginUserType.dme.value;
        }
      },
      {
        id: 'archiveQuota',
        permission: OperateItems.DeleteKerberos,
        label: this.i18n.get('protection_set_archive_quota_label'),
        onClick: data => {
          this.setQuota(data, DataMap.userFunction.archive.value);
        },
        displayCheck: ([data]) => {
          return data.userType !== DataMap.loginUserType.dme.value;
        }
      }
    ];

    const cols: TableCols[] = [
      {
        key: 'userName',
        name: this.i18n.get('common_name_label'),
        filter: {
          type: 'search',
          filterMode: 'contains'
        }
      },
      {
        key: 'userType',
        name: this.i18n.get('common_type_label'),
        filter: {
          type: 'select',
          isMultiple: true,
          showCheckAll: true,
          options: this.dataMapService.toArray('loginUserType')
        },
        cellRender: this.userTypeTpl
      },
      {
        key: 'backup_quota',
        name: this.i18n.get('protection_openstack_quota_label'),
        cellRender: this.backupCapacity
      },
      {
        key: 'archive_quota',
        name: this.i18n.get('system_archive_quota_label'),
        cellRender: this.archiveCapacity
      },
      {
        key: 'operation',
        width: 130,
        name: this.i18n.get('common_operation_label'),
        cellRender: {
          type: 'operation',
          config: {
            maxDisplayItems: 1,
            items: opts
          }
        }
      }
    ];
    this.tableConfig = {
      table: {
        compareWith: 'uuid',
        columns:
          this.cookieService.get('role') === '2'
            ? reject(cols, { key: 'operation' })
            : cols,
        scrollFixed: true,
        colDisplayControl: false,
        fetchData: (filter: Filters) => {
          this.getData(filter);
        }
      }
    };
  }

  getData(filters?: Filters) {
    const params = {
      pageNo: filters.paginator.pageIndex,
      pageSize: filters.paginator.pageSize
    };

    if (this.cookieService.get('role') === '2') {
      set(params, 'needQueryCurrentUser', true);
    }

    each(filters.filters, filter => {
      if (filter.value && size(filter.value)) {
        params[filter.key] = filter.value;
      }
    });

    this.userQuotaService
      .listUserQuotaInfoUsingGet(params)
      .pipe(
        map(res => {
          each(res.records, item => {
            const supportFunc = [];

            if (item.canBackup) {
              supportFunc.push(this.i18n.get('common_backup_label'));
            }
            if (item.canReplication) {
              supportFunc.push(this.i18n.get('common_replicate_label'));
            }
            if (item.canArchive) {
              supportFunc.push(this.i18n.get('common_archive_label'));
            }
            if (item.canRestore) {
              supportFunc.push(this.i18n.get('common_recovery_label'));
            }
            assign(item, {
              supportFunc: supportFunc.join('+')
            });

            if (item.backupTotalQuota > 0) {
              assign(item, {
                backupUsedPercent:
                  (100 * item.backupUsedQuota) / item.backupTotalQuota
              });
            } else {
              assign(item, {
                backupUsedPercent: 0
              });
            }

            if (item.cloudArchiveTotalQuota > 0) {
              assign(item, {
                archiveUsedPercent:
                  (100 * item.cloudArchiveUsedQuota) /
                  item.cloudArchiveTotalQuota
              });
            } else {
              assign(item, {
                archiveUsedPercent: 0
              });
            }
          });

          return res;
        })
      )
      .subscribe(res => {
        this.tableData = {
          data: res.records,
          total: res.totalCount
        };
        this.cdr.detectChanges();
      });
  }

  setQuota(data, type) {
    this.drawModalService.create(
      assign({}, MODAL_COMMON.generateDrawerOptions(), {
        lvModalKey: 'set-quota',
        lvWidth: MODAL_COMMON.normalWidth,
        lvHeader: this.i18n.get('common_setting_label'),
        lvContent: SetQuotaComponent,
        lvOkDisabled: false,
        lvComponentParams: {
          item: first(data),
          setType: type
        },
        lvAfterOpen: modal => {
          const content = modal.getContentComponent() as SetQuotaComponent;
          const modalIns = modal.getInstance();

          modalIns.lvOkDisabled = content.formGroup.status === 'INVALID';

          content.formGroup.statusChanges.subscribe(res => {
            modalIns.lvOkDisabled = res === 'INVALID';
          });
        },
        lvOk: modal => {
          return new Promise(resolve => {
            const content = modal.getContentComponent() as SetQuotaComponent;
            const item = first(data);
            const backupTotalQuota = content.formGroup.value.backupQuota
              ? content.formGroup.value.backupCapacity *
                toNumber(
                  get(
                    DataMap.Capacity_Unit,
                    `${toLower(
                      content.formGroup.value.backupCapacityUnit
                    )}.convertByte`
                  )
                )
              : -1;
            const cloudArchiveTotalQuota = content.formGroup.value.archiveQuota
              ? content.formGroup.value.archiveCapacity *
                toNumber(
                  get(
                    DataMap.Capacity_Unit,
                    `${toLower(
                      content.formGroup.value.archiveCapacityUnit
                    )}.convertByte`
                  )
                )
              : -1;
            const params = {
              userId: get(item, 'userId'),
              shouldCheckTotalQuota:
                get(item, 'userType') === DataMap.loginUserType.saml.value
                  ? false
                  : true
            };

            if (type === DataMap.userFunction.backup.value) {
              set(params, 'backupTotalQuota', backupTotalQuota);
            } else {
              set(params, 'cloudArchiveTotalQuota', cloudArchiveTotalQuota);
            }

            this.userQuotaService
              .setUserQuotaUsingPost({ userQuota: params as any })
              .subscribe(
                res => {
                  this.dataTable.fetchData();
                  resolve(true);
                },
                error => resolve(false)
              );
          });
        }
      })
    );
  }

  setFunc(item) {
    this.drawModalService.create(
      assign({}, MODAL_COMMON.generateDrawerOptions(), {
        lvModalKey: 'set-func',
        lvWidth: MODAL_COMMON.smallModal,
        lvHeader: this.i18n.get('common_setting_label'),
        lvContent: this.userFunctionComponent,
        lvOkDisabled: false,
        lvComponentParams: {
          item: first(item)
        },
        lvOk: modal => {
          return new Promise(resolve => {
            const content = modal.getContentComponent() as UserFunctionComponent;
            const functionSwitchPo = {
              userId: get(first(item), 'userId'),
              canBackup: !!find(
                content.selected,
                val => val === DataMap.userFunction.backup.value
              ),
              canReplication: !!find(
                content.selected,
                val => val === DataMap.userFunction.replicate.value
              ),
              canArchive: !!find(
                content.selected,
                val => val === DataMap.userFunction.archive.value
              ),
              canRestore: !!find(
                content.selected,
                val => val === DataMap.userFunction.restore.value
              )
            };
            this.functionSwitchApiService
              .setFunctionOfUserUsingPut({ functionSwitchPo: functionSwitchPo })
              .subscribe({
                next: res => {
                  this.dataTable.fetchData();
                  resolve(true);
                },
                error: error => resolve(false)
              });
          });
        }
      })
    );
  }
}

@Component({
  selector: 'aui-warning',
  template: `
    <h3>{{ 'system_function_label' | i18n }}</h3>
    <div class="aui-gutter-column-xs"></div>
    <lv-checkbox-group
      [(ngModel)]="selected"
      (ngModelChange)="selectionChange($event)"
    >
      <lv-group [lvGutter]="'16px'">
        <lv-checkbox *ngFor="let item of userFunction" [lvValue]="item.value">
          {{ item.label | i18n }}
        </lv-checkbox>
      </lv-group>
    </lv-checkbox-group>
  `,
  styles: [
    `
      .descrip-info {
        line-height: 12px;
        font-size: 12px;
        margin-bottom: 12px;
        transform: translateY(-4px);
      }
    `
  ]
})
export class UserFunctionComponent implements OnInit {
  selected = [];
  userFunction = this.dataMapService.toArray('userFunction');
  isChecked$ = new Subject<boolean>();
  item;

  constructor(
    public i18n: I18NService,
    public dataMapService: DataMapService,
    public appUtilsService: AppUtilsService
  ) {}

  ngOnInit() {
    if (this.item.canBackup) {
      this.selected.push(DataMap.userFunction.backup.value);
    }
    if (this.item.canReplication) {
      this.selected.push(DataMap.userFunction.replicate.value);
    }
    if (this.item.canArchive) {
      this.selected.push(DataMap.userFunction.archive.value);
    }
    if (this.item.canRestore) {
      this.selected.push(DataMap.userFunction.restore.value);
    }
  }

  selectionChange(e) {
    this.isChecked$.next(!size(e));
  }
}

@NgModule({
  imports: [CommonModule, BaseModule, FormsModule, CheckboxModule],
  declarations: [UserFunctionComponent],

  exports: [UserFunctionComponent]
})
export class UserFunctionModule {}
