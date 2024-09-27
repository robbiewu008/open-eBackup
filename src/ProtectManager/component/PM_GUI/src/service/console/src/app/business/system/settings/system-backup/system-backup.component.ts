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
  ChangeDetectionStrategy,
  ChangeDetectorRef,
  Component,
  OnDestroy,
  OnInit
} from '@angular/core';
import { MessageService } from '@iux/live';
import {
  CAPACITY_UNIT,
  CommonConsts,
  CookieService,
  DataMap,
  DataMapService,
  GROUP_COMMON,
  I18NService,
  MODAL_COMMON,
  OperateItems,
  SysbackupApiService,
  getPermissionMenuItem
} from 'app/shared';
import { SysVersionServiceService } from 'app/shared/api/services/sys-version-service.service';
import { DrawModalService } from 'app/shared/services/draw-modal.service';
import { VirtualScrollService } from 'app/shared/services/virtual-scroll.service';
import { assign, each, includes, isArray, isEmpty, now } from 'lodash';
import { Subject, Subscription, timer } from 'rxjs';
import { finalize, switchMap, takeUntil } from 'rxjs/operators';
import { WarningMessageService } from './../../../../shared/services/warning-message.service';
import { BackupRestoreComponent } from './backup-restore/backup-restore.component';
import { ImportBackupComponent } from './import-backup/import-backup.component';
import { ManuallBackupComponent } from './manuall-backup/manuall-backup.component';
import { PolicyConfigComponent } from './policy-config/policy-config.component';
import { AppUtilsService } from 'app/shared/services/app-utils.service';

@Component({
  selector: 'aui-system-backup',
  templateUrl: './system-backup.component.html',
  changeDetection: ChangeDetectionStrategy.OnPush,
  providers: [DatePipe]
})
export class SystemBackupComponent implements OnInit, OnDestroy {
  orderBy;
  orderType;
  _isEmpty = isEmpty;
  _isArray = isArray;
  _isEn = this.i18n.isEn;
  timeSub$: Subscription;
  filterParams: any = {};
  backupData = [] as any;
  policyData = {} as any;
  destroy$ = new Subject();

  unitconst = CAPACITY_UNIT;
  total = CommonConsts.PAGE_TOTAL;
  startPage = CommonConsts.PAGE_START;
  pageSize = CommonConsts.PAGE_SIZE;
  sizeOptions = CommonConsts.PAGE_SIZE_OPTIONS;
  backupStatus = DataMap.System_Backup_Status;
  groupCommon = GROUP_COMMON;

  manualBackupBtnTip = this.i18n.get('system_manual_backup_tip_label');
  isCyberengine =
    this.i18n.get('deploy_type') === DataMap.Deploy_Type.cyberengine.value;

  columns = [
    {
      key: 'backupTime',
      showSort: true,
      label: this.i18n.get('common_time_label')
    },
    {
      key: 'status',
      label: this.i18n.get('common_status_label'),
      filter: true,
      filterMap: this.dataMapService.toArray('System_Backup_Status')
    },
    {
      key: 'backupSize',
      showSort: true,
      label: this.i18n.get('common_size_label')
    },
    {
      key: 'backupType',
      label: this.i18n.get('common_backup_type_label'),
      filter: true,
      filterMap: this.dataMapService
        .toArray('System_Backup_Type')
        .filter(item => {
          if (!this.isCyberengine) {
            return !includes(
              [DataMap.System_Backup_Type.upgrade.value],
              item.value
            );
          }
          return true;
        })
    },
    {
      key: 'backupDesc',
      label: this.i18n.get('common_desc_label')
    }
  ];

  validConnectDisabled = true;
  version: string;

  constructor(
    private i18n: I18NService,
    public datePipe: DatePipe,
    private message: MessageService,
    private dataMapService: DataMapService,
    private drawModalService: DrawModalService,
    private warningMessageService: WarningMessageService,
    private sysbackupApiService: SysbackupApiService,
    private cookieService: CookieService,
    public virtualScroll: VirtualScrollService,
    private cdr: ChangeDetectorRef,
    private appUtilsService: AppUtilsService,
    private sysVersionService: SysVersionServiceService
  ) {}

  ngOnDestroy() {
    this.destroy$.next(true);
    this.destroy$.complete();
  }

  ngOnInit() {
    this.getSysBackupVersion();
    this.virtualScroll.getScrollParam(350);
  }

  onChange() {
    this.ngOnInit();
  }

  getSysBackupVersion() {
    if (this.isCyberengine) {
      this.sysVersionService
        .GetSysbackupVersion({ akDoException: false })
        .pipe(
          finalize(() => {
            this.getPolicyData();
            this.getBackupDatas();
          })
        )
        .subscribe((res: any) => {
          try {
            this.version = JSON.parse(res)?.pointRelease;
          } catch (error) {
            this.version = res.pointRelease;
          }
        });
    } else {
      this.getPolicyData();
      this.getBackupDatas();
    }
  }

  getPolicyData() {
    this.sysbackupApiService.getPolicyUsingGET1({}).subscribe(res => {
      this.policyData = assign(res, {
        backupTime: res.backupTime
          ? new Date(`2020/10/10 ${res.backupTime}:00`)
          : ''
      });
      this.cdr.detectChanges();
      this.validConnectDisabled = !res.backupTime;
    });
  }

  pageChange = event => {
    this.pageSize = event.pageSize;
    this.startPage = event.pageIndex;
    this.getBackupDatas();
  };

  sortChange(source) {
    this.orderBy = source.key;
    this.orderType = source.direction;
    this.getBackupDatas();
  }

  filterChange(e) {
    assign(this.filterParams, {
      [e.key]: e.value
    });
    this.getBackupDatas();
  }

  optsCallback = data => {
    const menus = [
      {
        id: 'restore',
        disabled:
          includes(
            [
              DataMap.System_Backup_Status.creating.value,
              DataMap.System_Backup_Status.restoring.value,
              DataMap.System_Backup_Status.invalid.value,
              DataMap.System_Backup_Status.backupFailed.value
            ],
            data.status
          ) ||
          (this.isCyberengine && this.version !== data.backupVersion),
        label: this.i18n.get('common_restore_label'),
        permission: OperateItems.RevertingBackup,
        onClick: (d: any) => this.restore(data)
      },
      {
        id: 'delete',
        disabled: includes(
          [
            DataMap.System_Backup_Status.creating.value,
            DataMap.System_Backup_Status.restoring.value
          ],
          data.status
        ),
        label: this.i18n.get('common_delete_label'),
        permission: OperateItems.DeletingBackup,
        onClick: (d: any) => {
          this.deleteBackup(data);
        }
      },
      {
        id: 'export',
        disabled:
          !includes(
            [
              DataMap.System_Backup_Status.available.value,
              DataMap.System_Backup_Status.restoreFailed.value
            ],
            data.status
          ) ||
          (this.isCyberengine &&
            data.backupType === DataMap.System_Backup_Type.upgrade.value),
        label: this.i18n.get('common_export_label'),
        permission: OperateItems.ExportingBackup,
        onClick: (d: any) => this.exportBackup(data)
      }
    ];
    return getPermissionMenuItem(menus, this.cookieService.role);
  };

  getBackupDatas() {
    if (this.timeSub$) {
      this.timeSub$.unsubscribe();
    }
    this.timeSub$ = timer(0, CommonConsts.TIME_INTERVAL)
      .pipe(
        switchMap(index => {
          const params = this.getParams();
          return this.sysbackupApiService.listUsingGET({
            ...params,
            akLoading: !index
          });
        }),
        takeUntil(this.destroy$)
      )
      .subscribe(res => {
        this.total = res.totalCount;
        this.backupData = res.records;
        this.cdr.detectChanges();
      });
    this.getPolicyData();
  }

  getParams() {
    const params = {
      startPage: this.startPage,
      pageSize: this.pageSize
    };

    each(this.filterParams, (value, key) => {
      if (isEmpty(value)) {
        delete this.filterParams[key];
      }
    });

    if (!isEmpty(this.filterParams)) {
      assign(params, {
        ...this.filterParams
      });
    }

    if (this.orderBy && this.orderType) {
      assign(params, {
        orderBy: this.orderBy,
        orderType: this.orderType
      });
    }

    return params;
  }

  manualBackup() {
    this.drawModalService.create(
      assign({}, MODAL_COMMON.drawerOptions, {
        lvModalKey: 'manualBackupModalKey',
        lvHeader: this.i18n.get('common_manual_backup_label'),
        lvContent: ManuallBackupComponent,
        lvWidth: MODAL_COMMON.normalWidth,
        lvOkDisabled: false,
        lvAfterOpen: modal => {
          const content = modal.getContentComponent() as ManuallBackupComponent;
          const modalIns = modal.getInstance();
          content.formGroup.statusChanges.subscribe(res => {
            modalIns.lvOkDisabled = res !== 'VALID';
          });
        },
        lvOk: modal => {
          return new Promise(resolve => {
            const content = modal.getContentComponent() as ManuallBackupComponent;
            content.onOk().subscribe(
              () => {
                resolve(true);
                this.getBackupDatas();
              },
              () => resolve(false)
            );
          });
        }
      })
    );
  }

  policyConfig() {
    this.sysbackupApiService.getPolicyUsingGET1({}).subscribe(data => {
      this.drawModalService.create(
        assign({}, MODAL_COMMON.generateDrawerOptions(), {
          lvModalKey: 'policyConfigModalKey',
          lvHeader: this.i18n.get('common_backup_policy_label'),
          lvContent: PolicyConfigComponent,
          lvWidth: MODAL_COMMON.normalWidth,
          lvOkDisabled: true,
          lvAfterOpen: modal => {
            const content = modal.getContentComponent() as PolicyConfigComponent;
            const modalIns = modal.getInstance();
            content.formGroup.statusChanges.subscribe(res => {
              modalIns.lvOkDisabled = res !== 'VALID';
            });
          },
          lvComponentParams: {
            data,
            callBack: () => {
              this.getPolicyData();
              this.getBackupDatas();
            }
          },
          lvOk: modal => {
            return new Promise(resolve => {
              const content = modal.getContentComponent() as PolicyConfigComponent;
              content.onOk().subscribe(
                () => {
                  resolve(true);
                  this.getPolicyData();
                  this.getBackupDatas();
                },
                () => resolve(false)
              );
            });
          }
        })
      );
    });
  }

  importBackup() {
    this.drawModalService.create(
      assign({}, MODAL_COMMON.generateDrawerOptions(), {
        lvModalKey: 'importPolicyModalKey',
        lvHeader: this.i18n.get('system_backup_import_label'),
        lvContent: ImportBackupComponent,
        lvWidth: this.i18n.isEn
          ? MODAL_COMMON.normalWidth + 80
          : MODAL_COMMON.normalWidth,
        lvOkDisabled: true,
        lvAfterOpen: modal => {
          const content = modal.getContentComponent() as ImportBackupComponent;
          const modalIns = modal.getInstance();
          content.valid$.subscribe(res => {
            modalIns.lvOkDisabled = !res;
          });
        },
        lvComponentParams: {},
        lvOk: modal => {
          return new Promise(resolve => {
            const content = modal.getContentComponent() as ImportBackupComponent;
            content.onOk().subscribe(
              () => {
                resolve(true);
                this.getBackupDatas();
              },
              () => resolve(false)
            );
          });
        }
      })
    );
  }

  connectTest() {
    this.sysbackupApiService
      .testSftpConnection({ request: {} })
      .subscribe(() => {});
  }

  restore(item) {
    if (
      this.isCyberengine &&
      item.backupType === DataMap.System_Backup_Type.upgrade.value
    ) {
      this.warningMessageService.create({
        content: this.i18n.get('system_cyber_backup_restore_warn_label', [
          this.datePipe.transform(item.backupTime, 'yyyy-MM-dd HH:mm:ss')
        ]),
        onOK: () => {
          const params = {
            rsq: {
              password: ''
            },
            imagesId: item.id
          };
          this.sysbackupApiService
            .recoveryUsingPOST(params)
            .subscribe(() => this.getBackupDatas());
        }
      });
      return;
    }
    this.drawModalService.create(
      assign({}, MODAL_COMMON.generateDrawerOptions(), {
        lvModalKey: 'backupRestoreModalKey',
        lvHeader: this.i18n.get('common_restore_label'),
        lvContent: BackupRestoreComponent,
        lvWidth: MODAL_COMMON.normalWidth,
        lvOkDisabled: true,
        lvAfterOpen: modal => {
          const content = modal.getContentComponent() as BackupRestoreComponent;
          const modalIns = modal.getInstance();
          content.formGroup.statusChanges.subscribe(res => {
            modalIns.lvOkDisabled = res !== 'VALID';
          });
        },
        lvComponentParams: {
          data: item
        },
        lvOk: modal => {
          return new Promise(resolve => {
            const content = modal.getContentComponent() as BackupRestoreComponent;
            content.onOk().subscribe(
              () => {
                resolve(true);
                this.getBackupDatas();
              },
              () => resolve(false)
            );
          });
        }
      })
    );
  }

  deleteBackup(item) {
    this.warningMessageService.create({
      content: this.i18n.get('system_delete_backup_label', [
        this.datePipe.transform(item.backupTime, 'yyyy-MM-dd HH:mm:ss')
      ]),
      onOK: () => {
        this.sysbackupApiService
          .deleteBackupUsingDELETE({ imagesId: item.id })
          .subscribe(() => this.getBackupDatas());
      }
    });
  }

  exportBackup(item) {
    this.sysbackupApiService
      .downloadBackupUsingGET({ imagesId: item.id })
      .subscribe(blob => {
        const bf = new Blob([blob], {
          type: 'application/zip'
        });
        this.appUtilsService.downloadFile(`backup_${now()}.zip`, bf);
      });
  }

  trackById = (index, item) => {
    return item.id;
  };
}
