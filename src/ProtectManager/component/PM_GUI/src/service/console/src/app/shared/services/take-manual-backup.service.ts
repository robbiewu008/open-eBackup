import { CommonModule } from '@angular/common';
import { Injectable, NgModule } from '@angular/core';
import { MessageService } from '@iux/live';
import { CookieService, DataMap, I18NService } from 'app/shared';
import { assign, each, first, includes, isFunction, set } from 'lodash';
import { MODAL_COMMON, SlaApiService } from '..';
import { MySQLBackupComponent } from '../components/mysql-backup.component';
import { TakeManualBackupComponent } from '../components/take-manual-backup/take-manual-backup.component';
import { TakeManualBackupModule } from '../components/take-manual-backup/take-manual-backup.module';
import { PolicyAction } from '../consts';
import { DrawModalService } from './draw-modal.service';

export interface Params {
  sla_id: string;
  proxy_id?: string;
  host_ip?: string;
  resource_id: string;
  resource_type: string;
}

@Injectable({
  providedIn: 'root'
})
export class TakeManualBackupService {
  mysqlBackupComponent = MySQLBackupComponent;

  constructor(
    public slaApiService: SlaApiService,
    public drawModalService: DrawModalService,
    public i18n: I18NService,
    private messageService: MessageService,
    private cookieService: CookieService
  ) {}

  private setBackupInfo(params, callback) {
    set(params.params, 'resource_type', first(params.params)['resource_type']);

    this.drawModalService.create(
      assign({}, MODAL_COMMON.generateDrawerOptions(), {
        lvWidth:
          params.resource_type === DataMap.Resource_Type.oracle.value &&
          this.i18n.language === 'en-us'
            ? MODAL_COMMON.normalWidth + 110
            : MODAL_COMMON.normalWidth,
        lvOkDisabled: true,
        lvHeader: this.i18n.get('common_manual_backup_label'),
        lvContent: TakeManualBackupComponent,
        lvOk: modal => {
          return new Promise(resolve => {
            const content = modal.getContentComponent() as TakeManualBackupComponent;
            if (
              includes(
                [
                  DataMap.Resource_Type.MySQLInstance.value,
                  DataMap.Resource_Type.MySQLClusterInstance.value,
                  DataMap.Resource_Type.MySQLDatabase.value
                ],
                params?.params?.resource_type
              ) &&
              content.formGroup.value.action !== PolicyAction.LOG
            ) {
              this.drawModalService.create({
                ...MODAL_COMMON.generateDrawerOptions(),
                lvModalKey: 'backupMessage',
                ...{
                  lvType: 'dialog',
                  lvDialogIcon: 'lv-icon-popup-danger-48',
                  lvHeader: this.i18n.get('common_alarms_info_label'),
                  lvContent: this.mysqlBackupComponent,
                  lvWidth: MODAL_COMMON.normalWidth,
                  lvComponentParams: {
                    askManualBackup: false,
                    manualBackup: true
                  },
                  lvOkType: 'primary',
                  lvCancelType: 'default',
                  lvOkDisabled: true,
                  lvFocusButtonId: 'cancel',
                  lvCloseButtonDisplay: true,
                  lvAfterOpen: modal => {
                    const content = modal.getContentComponent() as MySQLBackupComponent;
                    content.valid$.subscribe(res => {
                      modal.lvOkDisabled = !res;
                    });
                  },
                  lvCancel: modal => resolve(false),
                  lvOk: () => {
                    callback(modal);
                  },
                  lvAfterClose: resolve(false)
                }
              });
            } else {
              return callback(modal);
            }
          });
        },
        lvComponentParams: {
          ...params
        },
        lvAfterOpen: modal => {
          const content = modal.getContentComponent() as TakeManualBackupComponent;
          const modalIns = modal.getInstance();
          content.formGroup.statusChanges.subscribe(res => {
            modalIns.lvOkDisabled = res !== 'VALID';
          });
          content.formGroup.updateValueAndValidity();
        }
      })
    );
  }
  execute(params: Params, callback: () => void) {
    this.slaApiService
      .querySLAUsingGET({ slaId: params.sla_id })
      .subscribe(sla => {
        this.drawModalService.create(
          assign({}, MODAL_COMMON.generateDrawerOptions(), {
            lvWidth:
              params.resource_type === DataMap.Resource_Type.oracle.value &&
              this.i18n.language === 'en-us'
                ? MODAL_COMMON.normalWidth + 110
                : MODAL_COMMON.normalWidth,
            lvOkDisabled: true,
            lvHeader: this.i18n.get('common_manual_backup_label'),
            lvContent: TakeManualBackupComponent,
            lvOk: modal => {
              return new Promise(resolve => {
                const component = modal.getContentComponent() as TakeManualBackupComponent;

                if (
                  includes(
                    [
                      DataMap.Resource_Type.MySQLInstance.value,
                      DataMap.Resource_Type.MySQLClusterInstance.value,
                      DataMap.Resource_Type.MySQLDatabase.value
                    ],
                    params.resource_type
                  ) &&
                  component.formGroup.value.action !== PolicyAction.LOG
                ) {
                  this.drawModalService.create({
                    ...MODAL_COMMON.generateDrawerOptions(),
                    lvModalKey: 'backupMessage',
                    ...{
                      lvType: 'dialog',
                      lvDialogIcon: 'lv-icon-popup-danger-48',
                      lvHeader: this.i18n.get('common_alarms_info_label'),
                      lvContent: this.mysqlBackupComponent,
                      lvWidth: MODAL_COMMON.normalWidth,
                      lvComponentParams: {
                        askManualBackup: false,
                        manualBackup: true
                      },
                      lvOkType: 'primary',
                      lvCancelType: 'default',
                      lvOkDisabled: true,
                      lvFocusButtonId: 'cancel',
                      lvCloseButtonDisplay: true,
                      lvAfterOpen: modal => {
                        const content = modal.getContentComponent() as MySQLBackupComponent;
                        content.valid$.subscribe(res => {
                          modal.lvOkDisabled = !res;
                        });
                      },
                      lvCancel: modal => resolve(false),
                      lvOk: () => {
                        component.onOK().subscribe({
                          next: () => {
                            if (
                              !this.cookieService.isCloudBackup &&
                              isFunction(callback)
                            ) {
                              callback();
                            }
                            modal.close();
                            resolve(true);
                          },
                          error: () => resolve(false)
                        });
                      },
                      lvAfterClose: resolve(false)
                    }
                  });
                } else {
                  component.onOK().subscribe({
                    next: () => {
                      if (
                        !this.cookieService.isCloudBackup &&
                        isFunction(callback)
                      ) {
                        callback();
                      }
                      resolve(true);
                    },
                    error: () => resolve(false)
                  });
                }
              });
            },
            lvComponentParams: {
              params: assign({}, sla, params)
            },
            lvAfterOpen: modal => {
              const content = modal.getContentComponent() as TakeManualBackupComponent;
              const modalIns = modal.getInstance();
              content.formGroup.statusChanges.subscribe(res => {
                modalIns.lvOkDisabled = res !== 'VALID';
              });
              content.formGroup.updateValueAndValidity();
            }
          })
        );
      });
  }

  batchExecute(params: Params[], callback: () => void) {
    if (params.length > 100) {
      this.messageService.error(
        this.i18n.get('protection_max_take_manual_error_label'),
        {
          lvShowCloseButton: true,
          lvMessageKey: 'filesetProtectMessageKey'
        }
      );
    } else {
      each(params, item => {
        assign(item, { isAsyn: true });
      });
      this.setBackupInfo({ isBatched: true, params }, modal => {
        return new Promise(resolve => {
          const component = modal.getContentComponent() as TakeManualBackupComponent;
          component.onOK().subscribe({
            next: () => {
              const isCloudBackup = includes(
                [
                  DataMap.Deploy_Type.cloudbackup2.value,
                  DataMap.Deploy_Type.cloudbackup.value
                ],
                this.i18n.get('deploy_type')
              );
              if (!isCloudBackup && isFunction(callback)) {
                callback();
              }
              modal.close();
              resolve(true);
            },
            error: () => resolve(false)
          });
        });
      });
    }
  }
}

@NgModule({
  imports: [CommonModule, TakeManualBackupModule],
  providers: [TakeManualBackupService]
})
export class TakeManualBackupServiceModule {}
