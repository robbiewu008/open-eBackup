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
import { Injectable, NgModule } from '@angular/core';
import { CreateBackupPolicyComponent } from 'app/business/explore/ransomware-protection/data-backup/backup-policy/create-backup-policy/create-backup-policy.component';
import { CreateBackupPolicyModule } from 'app/business/explore/ransomware-protection/data-backup/backup-policy/create-backup-policy/create-backup-policy.module';
import { PolicyDetailComponent } from 'app/business/explore/ransomware-protection/data-backup/backup-policy/policy-detail/policy-detail.component';
import { PolicyDetailModule } from 'app/business/explore/ransomware-protection/data-backup/backup-policy/policy-detail/policy-detail.module';
import { SlaCreateComponent } from 'app/business/protection/policy/sla/sla-create/sla-create.component';
import { SlaCreateModule } from 'app/business/protection/policy/sla/sla-create/sla-create.module';
import { SlaDetailComponent } from 'app/business/protection/policy/sla/sla-detail/sla-detail.component';
import { SlaDetailModule } from 'app/business/protection/policy/sla/sla-detail/sla-detail.module';
import { assign, each, includes, isEmpty, isFunction, map, set } from 'lodash';
import {
  ApplicationType,
  MODAL_COMMON,
  ProtectResourceAction,
  SlaApiService,
  WarningMessageService,
  isJson
} from '..';
import { SlaInfoModule } from '../components';
import { PolicyAction } from '../consts';
import { DrawModalService } from './draw-modal.service';
import { I18NService } from './i18n.service';
import { CreateRealPolicyModule } from 'app/business/explore/ransomware-protection/real-time-detection/detection-policy/create-real-policy/create-real-policy.module';
import { CreateRealPolicyComponent } from 'app/business/explore/ransomware-protection/real-time-detection/detection-policy/create-real-policy/create-real-policy.component';
import { PolicyDetailComponent as RealPolicyDetailComponent } from 'app/business/explore/ransomware-protection/real-time-detection/detection-policy/policy-detail/policy-detail.component';
import { USER_GUIDE_CACHE_DATA } from '../consts/guide-config';

export interface Sla {
  uuid: string;
  name: string;
}

@Injectable({
  providedIn: 'root'
})
export class SlaService {
  constructor(
    private i18n: I18NService,
    private slaApiService: SlaApiService,
    private drawModalService: DrawModalService,
    private warningMessageService: WarningMessageService
  ) {}

  create(callback: (res: any) => void, sla?) {
    this.drawModalService.create(
      assign({}, MODAL_COMMON.generateDrawerOptions(), {
        lvOkLoadingText: this.i18n.get('common_loading_label'),
        lvHeader: this.i18n.get('protection_create_sla_label'),
        lvContent: SlaCreateComponent,
        lvWidth:
          sla && sla.application === ApplicationType.Replica
            ? MODAL_COMMON.largeModal
            : MODAL_COMMON.xLargeModal,
        lvComponentParams: {
          sla,
          action:
            sla &&
            includes(
              [ApplicationType.SQLServer, ApplicationType.HyperV],
              sla.application
            )
              ? ProtectResourceAction.Clone
              : ProtectResourceAction.Create
        },
        lvOkDisabled: true,
        lvAfterOpen: modal => {
          const content = modal.getContentComponent() as SlaCreateComponent;
          const modalIns = modal.getInstance();
          content.valid$.subscribe(result => {
            modalIns.lvOkDisabled = !result;
          });
        },
        lvOk: modal => {
          return new Promise(resolve => {
            const content = modal.getContentComponent() as SlaCreateComponent;
            content.onCreate().subscribe({
              next: res => {
                resolve(true);
                callback(res);
                if (isJson(res)) {
                  USER_GUIDE_CACHE_DATA.slas.push(JSON.parse(res)?.uuid);
                }
              },
              error: () => resolve(false)
            });
          });
        }
      })
    );
  }

  delete(sla: Sla, callback: () => void) {
    this.warningMessageService.create({
      content: this.i18n.get(
        'protection_sla_delete_label',
        [sla.name],
        false,
        true
      ),
      onOK: () =>
        this.slaApiService
          .deleteSLAUsingDELETE({ slaId: sla.uuid })
          .subscribe(res => callback())
    });
  }

  batchDelete(selection: any[], callback: () => void) {
    this.warningMessageService.create({
      content: this.i18n.get('protection_batch_delete_sla_warn_label'),
      onOK: () =>
        this.slaApiService
          .batchDeleteSLAUsingDELETE({
            slaBatchRequest: { slaIds: map(selection, 'uuid') }
          })
          .subscribe(() => callback())
    });
  }

  clone(sla: Sla, callback: () => void) {
    this.slaApiService.querySLAUsingGET({ slaId: sla.uuid }).subscribe(sla => {
      if (
        includes(
          [ApplicationType.Fileset, ApplicationType.NASShare],
          sla.application as any
        )
      ) {
        each(sla.policy_list, item => {
          if ((item.action as any) === PolicyAction.PERMANENT) {
            set(item, 'action', PolicyAction.INCREMENT);
            set(item, 'permanentBackup', true);
          }
        });
      }
      this.drawModalService.create(
        assign({}, MODAL_COMMON.generateDrawerOptions(), {
          lvOkLoadingText: this.i18n.get('common_loading_label'),
          lvHeader: this.i18n.get('protection_clone_sla_label'),
          lvContent: SlaCreateComponent,
          lvWidth: MODAL_COMMON.xLargeModal,
          lvComponentParams: {
            sla,
            action: ProtectResourceAction.Clone
          },
          lvOkDisabled: true,
          lvAfterOpen: modal => {
            const content = modal.getContentComponent() as SlaCreateComponent;
            const modalIns = modal.getInstance();
            content.valid$.subscribe(result => {
              modalIns.lvOkDisabled = !result;
            });
          },
          lvOk: modal => {
            return new Promise(resolve => {
              const content = modal.getContentComponent() as SlaCreateComponent;
              content.onCreate().subscribe({
                next: () => {
                  resolve(true);
                  callback();
                },
                error: () => resolve(false)
              });
            });
          }
        })
      );
    });
  }

  modify(sla: Sla, callback?: () => void) {
    this.slaApiService
      .querySLAUsingGET({ slaId: sla.uuid })
      .subscribe(resSla => {
        if (
          includes(
            [ApplicationType.Fileset, ApplicationType.NASShare],
            resSla.application as any
          )
        ) {
          each(resSla.policy_list, item => {
            if ((item.action as any) === PolicyAction.PERMANENT) {
              set(item, 'action', PolicyAction.INCREMENT);
              set(item, 'permanentBackup', true);
            }
          });
        }
        resSla['resource_count'] = sla['resource_count'];
        this.drawModalService.create(
          assign({}, MODAL_COMMON.generateDrawerOptions(), {
            lvOkLoadingText: this.i18n.get('common_loading_label'),
            lvHeader: this.i18n.get('protection_modify_sla_label'),
            lvContent: SlaCreateComponent,
            lvWidth: MODAL_COMMON.xLargeModal,
            lvComponentParams: {
              sla: resSla,
              action: ProtectResourceAction.Modify
            },
            lvOkDisabled: true,
            lvAfterOpen: modal => {
              const content = modal.getContentComponent() as SlaCreateComponent;
              const modalIns = modal.getInstance();
              content.valid$.subscribe(result => {
                modalIns.lvOkDisabled = !result;
              });
            },
            lvOk: modal => {
              return new Promise(resolve => {
                const content = modal.getContentComponent() as SlaCreateComponent;
                content.onModify().subscribe({
                  next: () => {
                    resolve(true);
                    callback();
                  },
                  error: () => resolve(false)
                });
              });
            }
          })
        );
      });
  }

  activate(sla: Sla, callback?: () => void) {
    this.slaApiService
      .activeSLAUsingPost({
        slaId: sla.uuid
      })
      .subscribe({
        next: () => {
          callback();
        }
      });
  }

  disable(sla: Sla, callback?: () => void) {
    this.warningMessageService.create({
      content: this.i18n.get('protection_sla_disable_tip_label', [sla.name]),
      onOK: () => {
        this.slaApiService
          .deactiveSLAUsingPost({
            slaId: sla.uuid
          })
          .subscribe({
            next: () => {
              callback();
            }
          });
      }
    });
  }

  getDetail(sla: Sla, optItems?, viewResource?, closeCallback?) {
    this.slaApiService.querySLAUsingGET({ slaId: sla.uuid }).subscribe(sla => {
      this.drawModalService.openDetailModal(
        assign({}, MODAL_COMMON.generateDrawerOptions(), {
          lvContent: SlaDetailComponent,
          lvModalKey: 'slaDetailModalKey',
          lvWidth: 800,
          lvModality: false,
          lvComponentParams: {
            sla,
            optItems,
            viewResource
          },
          lvFooter: [
            {
              label: this.i18n.get('common_close_label'),
              onClick: modal => modal.close()
            }
          ],
          lvAfterClose: () => {
            if (isFunction(closeCallback)) {
              closeCallback();
            }
          }
        })
      );
    });
  }

  createDetectionPolicy(callFn?, rowData?, isClone?, isFromSelectSla?) {
    this.drawModalService.create({
      ...MODAL_COMMON.generateDrawerOptions(),
      lvHeader: isClone
        ? this.i18n.get('common_clone_label')
        : rowData
        ? this.i18n.get('common_modify_label')
        : this.i18n.get('common_create_label'),
      lvModalKey: 'create-backup-policy',
      lvWidth: this.i18n.isEn
        ? MODAL_COMMON.largeWidth + 200
        : MODAL_COMMON.largeWidth + 100,
      lvContent: CreateBackupPolicyComponent,
      lvOkDisabled: isEmpty(rowData),
      lvComponentParams: {
        rowData,
        isClone,
        isFromSelectSla
      },
      lvAfterOpen: modal => {
        const content = modal.getContentComponent() as CreateBackupPolicyComponent;
        content.formGroup.statusChanges.subscribe(res => {
          modal.lvOkDisabled = res !== 'VALID';
        });
      },
      lvOk: modal => {
        return new Promise(resolve => {
          const content = modal.getContentComponent() as CreateBackupPolicyComponent;
          content.onOK().subscribe(
            res => {
              resolve(true);
              if (isFunction(callFn)) {
                callFn(res);
              }
            },
            () => resolve(false)
          );
        });
      }
    });
  }

  getAntiDetail(item, optItems?) {
    this.slaApiService
      .querySLAUsingGET({ slaId: item.uuid })
      .subscribe(policy => {
        each(policy.policy_list, item => {
          assign(item, {
            name: this.i18n.get('common_policy_params_label', [
              item.name?.replace(/[^0-9]/g, '')
            ])
          });
        });
        this.drawModalService.openDetailModal(
          assign({}, MODAL_COMMON.generateDrawerOptions(), {
            lvContent: PolicyDetailComponent,
            lvModalKey: 'antiSlaDetailModalKey',
            lvWidth: this.i18n.isEn ? 840 : 800,
            lvModality: false,
            lvComponentParams: {
              policy: assign(policy, { isResource: item.isResource }),
              optItems
            },
            lvFooter: [
              {
                label: this.i18n.get('common_close_label'),
                onClick: modal => modal.close()
              }
            ]
          })
        );
      });
  }

  createRealDetectionPolicy(callFn?, rowData?, isClone?: boolean) {
    this.drawModalService.create({
      ...MODAL_COMMON.generateDrawerOptions(),
      lvHeader: isClone
        ? this.i18n.get('common_clone_label')
        : rowData
        ? this.i18n.get('common_modify_label')
        : this.i18n.get('common_create_label'),
      lvModalKey: 'create-backup-policy',
      lvWidth: this.i18n.isEn
        ? MODAL_COMMON.normalWidth + 200
        : MODAL_COMMON.normalWidth + 100,
      lvContent: CreateRealPolicyComponent,
      lvOkDisabled: isEmpty(rowData),
      lvComponentParams: {
        rowData,
        isClone
      },
      lvAfterOpen: modal => {
        const content = modal.getContentComponent() as CreateRealPolicyComponent;
        content.formGroup.statusChanges.subscribe(res => {
          modal.lvOkDisabled = res !== 'VALID';
        });
      },
      lvOk: modal => {
        return new Promise(resolve => {
          const content = modal.getContentComponent() as CreateRealPolicyComponent;
          content.onOK().subscribe(
            res => {
              resolve(true);
              if (isFunction(callFn)) {
                callFn(res);
              }
            },
            () => resolve(false)
          );
        });
      }
    });
  }

  getRealDetectionPolicyDetail(rowData) {
    this.drawModalService.create(
      assign({}, MODAL_COMMON.generateDrawerOptions(), {
        lvHeader: rowData.name,
        lvContent: RealPolicyDetailComponent,
        lvWidth: MODAL_COMMON.normalWidth + 100,
        lvComponentParams: { rowData },
        lvFooter: [
          {
            label: this.i18n.get('common_close_label'),
            onClick: modal => modal.close()
          }
        ]
      })
    );
  }
}

@NgModule({
  imports: [
    CommonModule,
    SlaDetailModule,
    SlaCreateModule,
    SlaInfoModule,
    PolicyDetailModule,
    CreateBackupPolicyModule,
    CreateRealPolicyModule
  ],
  providers: [SlaService]
})
export class SlaServiceModule {}
