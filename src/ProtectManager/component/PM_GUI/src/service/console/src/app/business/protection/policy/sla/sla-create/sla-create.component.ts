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
import { Component, OnInit } from '@angular/core';
import { FormBuilder, FormControl, FormGroup } from '@angular/forms';
import { Router } from '@angular/router';
import { MessageService, ModalRef } from '@iux/live';
import {
  ApplicationType,
  ApplicationTypeView,
  BaseUtilService,
  CommonConsts,
  CookieService,
  DataMap,
  DataMapService,
  deepEqualObject,
  I18NService,
  LANGUAGE,
  MODAL_COMMON,
  NasDistributionStoragesApiService,
  PolicyAction,
  PolicyType,
  ProjectedObjectApiService,
  ProtectResourceAction,
  SlaApiService,
  SlaType,
  SLA_BACKUP_NAME,
  WarningMessageService,
  SupportLicense
} from 'app/shared';
import { DrawModalService } from 'app/shared/services/draw-modal.service';
import { SlaParseService } from 'app/shared/services/sla-parse.service';
import {
  assign,
  cloneDeep,
  each,
  filter,
  find,
  findIndex,
  first,
  get,
  has,
  includes,
  isUndefined,
  keys,
  map as _map,
  now,
  omit,
  pick,
  reject,
  size,
  toString,
  union,
  uniq
} from 'lodash';
import { combineLatest, Observable, Observer, Subject } from 'rxjs';
import { SelectApplicationComponent } from './select-application/select-application.component';
import { SpecifiedArchivalPolicyComponent } from './specified-archival-policy/specified-archival-policy.component';
import { SpecifiedBackupPolicyComponent } from './specified-backup-policy/specified-backup-policy.component';
import { SpecifiedReplicationPolicyComponent } from './specified-replication-policy/specified-replication-policy.component';
import { AppUtilsService } from 'app/shared/services/app-utils.service';

@Component({
  selector: 'sla-create',
  templateUrl: './sla-create.component.html',
  styleUrls: ['./sla-create.component.less'],
  providers: [DatePipe]
})
export class SlaCreateComponent implements OnInit {
  isDisabled = false;
  sla;
  action;
  backupModes = [];
  valid$ = new Subject<boolean>();
  policy_list = [];
  slaType = SlaType;
  formGroup: FormGroup;
  slaBackupName = cloneDeep(SLA_BACKUP_NAME);
  protectResourceAction = ProtectResourceAction;
  applicationTypeView = ApplicationTypeView;
  applicationType = ApplicationType;
  isHyperdetect =
    this.i18n.get('deploy_type') === DataMap.Deploy_Type.hyperdetect.value;
  isEnglish = this.i18n.language !== 'zh-cn';
  isHcsUser = false;
  dwsGroup = false;
  dwsParallel = false;
  application = {
    label: '',
    value: '',
    checkedUrl: '',
    tooltip: this.i18n.get('protection_sla_application_tooltip_label'),
    viewType: this.cookieService.isCloudBackup
      ? this.applicationTypeView.Specified
      : this.applicationTypeView.General,
    click: () => {
      if (this.action !== this.protectResourceAction.Create || this.sla) {
        return;
      }
      this.selectApplication();
    }
  };
  backupPolicy = {
    newData: '',
    policyList: [],
    tooltip: '',
    checkedUrl: 'assets/img/oceanprotect-disabled.png',
    checked: false
  };
  archival = {
    label: '＋',
    newData: '',
    tooltip: '',
    policyList: [],
    checked: false,
    url: 'assets/img/archival_disabled.gif'
  };
  replication = {
    label: '＋',
    newData: '',
    tooltip: '',
    policyList: [],
    checked: false,
    url: 'assets/img/replication_disabled.gif'
  };
  upCurved = {
    checked: false,
    url: 'assets/img/up_curved_disabled.png'
  };
  downCurved = {
    checked: false,
    url: 'assets/img/up_curved_disabled.png'
  };
  constructor(
    public fb: FormBuilder,
    public modal: ModalRef,
    public i18n: I18NService,
    public datePipe: DatePipe,
    public cookieService: CookieService,
    public slaApiService: SlaApiService,
    public dataMapService: DataMapService,
    public baseUtilService: BaseUtilService,
    public slaParseService: SlaParseService,
    public drawModalService: DrawModalService,
    private messageService: MessageService,
    private router: Router,
    private warningMessageService: WarningMessageService,
    private projectedObjectApiService: ProjectedObjectApiService,
    private nasDistributionStoragesApiService: NasDistributionStoragesApiService,
    public appUtilsService: AppUtilsService
  ) {}

  ngOnInit() {
    this.initHcsUser();
    this.initIcons();
    this.initForm();
    this.initSla();
  }

  initHcsUser() {
    this.isHcsUser =
      this.cookieService.get('userType') === CommonConsts.HCS_USER_TYPE;
  }

  initIcons() {
    if (!this.sla) {
      return;
    }
    if (!this.sla.uuid) {
      return;
    }
    if (this.sla?.application === DataMap.Resource_Type.Dameng.value) {
      const params = {
        slaId: get(this.sla, 'uuid'),
        pageNo: CommonConsts.PAGE_START,
        pageSize: CommonConsts.PAGE_SIZE,
        subType: [DataMap.Resource_Type.Dameng_cluster.value]
      };
      this.projectedObjectApiService
        .pageQueryV1ProtectedObjectsGet(params as any)
        .subscribe(res => {
          if (!!size(res.items)) {
            this.isDisabled = true;
          }
        });
    } else {
      return;
    }
  }

  initForm() {
    const defaultName = `SLA_${now()}`;

    this.formGroup = this.fb.group({
      uuid: new FormControl(''),
      name: new FormControl(defaultName, {
        validators: [
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.name()
        ],
        updateOn: 'change'
      }),
      type: new FormControl(this.slaType.Backup)
    });
    this.formGroup.statusChanges.subscribe(res => {
      this.valid$.next(res === 'VALID' && !!size(this.policy_list));
    });
  }

  initSla() {
    if (!this.sla) {
      if (
        this.i18n.get('deploy_type') === DataMap.Deploy_Type.hyperdetect.value
      ) {
        if (SupportLicense.isBoth || SupportLicense.isFile) {
          assign(this.application, {
            label: 'common_local_file_system_label',
            tooltip: '',
            value: DataMap.Resource_Type.LocalFileSystem.value,
            viewType: 1
          });
        }

        if (!SupportLicense.isBoth && SupportLicense.isSan) {
          assign(this.application, {
            label: 'protection_local_lun_label',
            tooltip: '',
            value: DataMap.Resource_Type.LocalLun.value,
            viewType: 1
          });
        }
      }
      return;
    }
    // Application
    assign(this.application, this.slaParseService.getApplication(this.sla));

    // Backup
    assign(
      this.backupPolicy,
      this.slaParseService.getBackupPolicy(this.sla, this.policy_list)
    );

    // Archival
    assign(
      this.archival,
      this.slaParseService.getArchival(this.sla, this.policy_list)
    );

    // Replication
    assign(
      this.replication,
      this.slaParseService.getReplication(this.sla, this.policy_list)
    );

    this.formGroup.patchValue(this.sla);
    setTimeout(() => {
      this.modal.getInstance().lvOkDisabled = !(
        this.formGroup.valid && !!size(this.policy_list)
      );
    }, 0);
  }

  selectApplication() {
    this.drawModalService.create(
      assign({}, MODAL_COMMON.generateDrawerOptions(), {
        lvHeader: this.i18n.get('common_select_application_label'),
        lvContent: SelectApplicationComponent,
        lvWidth: MODAL_COMMON.largeModal,
        lvComponentParams: {
          applicationObj: this.application
        },
        lvOkDisabled: true,
        lvAfterOpen: modal => {},
        lvAfterClose: () => {
          setTimeout(() => {
            this.modal.setProperty({
              lvWidth: MODAL_COMMON.xLargeModal
            });
          });
        },
        lvOk: modal => {
          const content = modal.getContentComponent() as SelectApplicationComponent;
          content.onOK().subscribe((res: any) => {
            if (this.application.value === res.applicationType) return;
            this.policy_list = [];
            assign(this.application, {
              tooltip: '',
              label: res.label,
              viewType: res.viewType,
              checkedUrl: res.checkedUrl,
              value: res.applicationType
            });
            this.slaBackupName.difference_increment = includes(
              [
                ApplicationType.NASFileSystem,
                ApplicationType.NASShare,
                ApplicationType.HBase,
                ApplicationType.Hive,
                ApplicationType.HDFS,
                ApplicationType.KubernetesStatefulSet,
                ApplicationType.Vmware,
                ApplicationType.HCSCloudHost,
                ApplicationType.FusionCompute,
                ApplicationType.FusionOne,
                ApplicationType.TDSQL,
                ApplicationType.Volume
              ],
              this.application.value
            )
              ? 'common_permanent_backup_label'
              : 'common_incremental_backup_label';
            assign(this.backupPolicy, {
              tooltip: this.i18n.get('protection_sla_backup_tooltip_label'),
              checkedUrl: 'assets/img/oceanprotect-disabled.gif',
              newData: '',
              policyList: []
            });
            assign(this.archival, {
              newData: '',
              tooltip: '',
              policyList: []
            });
            assign(this.replication, {
              newData: '',
              tooltip: '',
              policyList: []
            });
            this.valid$.next(this.formGroup.valid && !!size(this.policy_list));
          });
        }
      })
    );
  }

  selectSpecifiedBackupPolicy(item?) {
    if (!this.isSupportBackupPolicy()) {
      return;
    }

    let activeIndex = 0;
    if (item) {
      const index = findIndex(this.backupPolicy.policyList, {
        action: item.action
      });

      if (index !== -1) {
        activeIndex = index;
      }
    }

    this.drawModalService.create(
      assign({}, MODAL_COMMON.generateDrawerOptions(), {
        lvHeader: this.isHyperdetect
          ? this.i18n.get('common_anti_detection_snapshot_policy_label')
          : this.i18n.get('common_backup_policy_label'),
        lvContent: SpecifiedBackupPolicyComponent,
        lvWidth: MODAL_COMMON.xLargeModal,
        lvComponentParams: {
          sla: this.sla,
          activeIndex,
          action: this.action,
          applicationData: this.application.value,
          backupData: this.backupPolicy.policyList,
          archvieDataList: this.archival.policyList,
          replicationData: this.replication
        },
        lvOkDisabled: true,
        lvAfterOpen: modal => {
          const content = modal.getContentComponent() as SpecifiedBackupPolicyComponent;
          const modalIns = modal.getInstance();
          content.formGroup.statusChanges.subscribe(result => {
            const backupTeams = content.formGroup.get('backupTeams');
            modalIns.lvOkDisabled = result !== 'VALID' || !size(backupTeams);
          });
        },
        lvOk: modal => {
          return new Promise(resolve => {
            const content = modal.getContentComponent() as SpecifiedBackupPolicyComponent;

            content.onOK().subscribe(
              (res: any) => {
                this.policy_list = union(
                  reject(this.policy_list, { type: PolicyType.BACKUP }),
                  res.newData
                );

                this.archival.tooltip = this.i18n.get(
                  this.appUtilsService.isDistributed
                    ? 'protection_sla_archival_tooltip_distributed_label'
                    : 'protection_sla_archival_tooltip_label'
                );
                this.replication.tooltip = this.i18n.get(
                  'protection_sla_replication_tooltip_label'
                );
                assign(this.backupPolicy, {
                  tooltip: '',
                  newData: res.newData,
                  policyList: res.originalData,
                  checkedUrl: 'assets/img/oceanprotect_enable.png'
                });
                this.valid$.next(
                  this.formGroup.valid && !!size(this.policy_list)
                );
                resolve(true);
              },
              error => resolve(error)
            );
          });
        }
      })
    );
  }

  isSupportBackupPolicy() {
    if (!toString(this.application.value)) {
      return false;
    }
    return true;
  }

  selectSpecifiedArchivalPolicy(item?) {
    if (!this.isSupportArchive()) {
      return;
    }

    let activeIndex = 0;
    if (item) {
      const index = findIndex(this.archival.policyList, {
        name: item.name
      });

      if (index !== -1) {
        activeIndex = index;
      }
    }

    this.drawModalService.create(
      assign({}, MODAL_COMMON.generateDrawerOptions(), {
        lvHeader: this.i18n.get('protection_archival_policy_label'),
        lvContent: SpecifiedArchivalPolicyComponent,
        lvWidth: MODAL_COMMON.smallModal,
        lvComponentParams: {
          activeIndex,
          sla: this.sla,
          action: this.action,
          archivalData: this.archival.policyList,
          applicationData: this.application.value
        },
        lvOkDisabled: true,
        lvAfterOpen: modal => {
          const content = modal.getContentComponent() as SpecifiedArchivalPolicyComponent;
          const modalIns = modal.getInstance();
          content.formGroup.statusChanges.subscribe(result => {
            modalIns.lvOkDisabled = result !== 'VALID';
          });
        },
        lvOk: modal => {
          return new Promise(resolve => {
            const content = modal.getContentComponent() as SpecifiedArchivalPolicyComponent;
            content.onOK().subscribe(
              res => {
                this.archival.tooltip = '';
                this.archival.newData = res.newData;
                this.archival.policyList = res.originalData;
                this.policy_list = union(
                  reject(this.policy_list, { type: PolicyType.ARCHIVING }),
                  res.newData
                );
                if (
                  includes(
                    [
                      DataMap.Resource_Type.Replica.value,
                      DataMap.Resource_Type.ImportCopy.value
                    ],
                    this.application.value
                  )
                ) {
                  this.valid$.next(
                    this.formGroup.valid && !!size(this.policy_list)
                  );
                }
                resolve(true);
              },
              error => resolve(error)
            );
          });
        }
      })
    );
  }

  isSupportArchive() {
    if (
      !toString(this.backupPolicy.newData) &&
      !includes(
        [this.applicationType.Replica, this.applicationType.ImportCopy],
        this.application.value
      )
    ) {
      return false;
    }
    if (this.isDisabled || this.isHcsUser) {
      return false;
    }

    return true;
  }

  selectSpecifiedReplicationPolicy(item?) {
    if (!this.isSupportReplication()) {
      return;
    }

    let activeIndex = 0;
    if (item) {
      const index = findIndex(this.replication.policyList, {
        name: item.name
      });

      if (index !== -1) {
        activeIndex = index;
      }
    }

    this.drawModalService.create(
      assign({}, MODAL_COMMON.generateDrawerOptions(), {
        lvHeader: this.i18n.get('common_replication_policy_label'),
        lvContent: SpecifiedReplicationPolicyComponent,
        lvWidth: this.i18n.isEn ? 900 : 750,
        lvComponentParams: {
          sla: this.sla,
          activeIndex,
          action: this.action,
          replicationData: this.replication.policyList,
          applicationData: this.application.value,
          backupData: this.backupPolicy.policyList,
          application: this.application.value
        },
        lvOkDisabled: true,
        lvAfterOpen: modal => {
          const content = modal.getContentComponent() as SpecifiedReplicationPolicyComponent;
          const modalIns = modal.getInstance();
          content.formGroup.statusChanges.subscribe(result => {
            modalIns.lvOkDisabled = result !== 'VALID';
          });
        },
        lvOk: modal => {
          return new Promise(resolve => {
            const content = modal.getContentComponent() as SpecifiedReplicationPolicyComponent;
            content.onOK().subscribe(
              res => {
                if (this.application.value === ApplicationType.GaussDBDWS) {
                  this.dwsGroup = content.ReplicationPolicyComponent?.dwsGroup;
                  this.dwsParallel =
                    content.ReplicationPolicyComponent?.dwsParallel;
                }
                this.replication.tooltip = '';
                this.replication.newData = res.newData;
                this.replication.policyList = res.originalData;
                this.policy_list = union(
                  reject(this.policy_list, { type: PolicyType.REPLICATION }),
                  res.newData
                );
                if (
                  includes(
                    [
                      DataMap.Resource_Type.ImportCopy.value,
                      DataMap.Resource_Type.Replica.value
                    ],
                    this.application.value
                  )
                ) {
                  this.valid$.next(
                    this.formGroup.valid && !!size(this.policy_list)
                  );
                }
                resolve(true);
              },
              error => resolve(error)
            );
          });
        }
      })
    );
  }

  isSupportReplication() {
    if (
      !toString(this.backupPolicy.newData) &&
      !includes(
        [this.applicationType.ImportCopy, this.applicationType.Replica],
        this.application.value
      )
    ) {
      return false;
    }
    return true;
  }

  onCreate(): Observable<any> {
    return new Observable<any>((observer: Observer<any>) => {
      if (this.formGroup.invalid) {
        return;
      }

      if (!this.dwsStorageVaild()) {
        observer.error(null);
        return;
      }

      const body = assign(this.formGroup.value, {
        application: this.application.value,
        policy_list: this.policy_list
      });

      // 升级场景复制策略，replication_target_type为空，默认复制所有。
      each(body.policy_list, item => {
        if (
          this.action === this.protectResourceAction.Clone &&
          item.action === PolicyType.REPLICATION &&
          isUndefined(item.ext_parameters?.replication_target_type)
        ) {
          assign(item.ext_parameters, {
            replication_target_type: DataMap.slaReplicationRule.all.value
          });
        }
      });

      if (
        includes(
          [
            ApplicationType.Fileset,
            ApplicationType.NASShare,
            ApplicationType.Volume,
            ApplicationType.ObjectStorage
          ],
          this.application.value
        )
      ) {
        each(body.policy_list, item => {
          if (has(item, 'permanentBackup')) {
            item.action = get(item, 'permanentBackup')
              ? PolicyAction.PERMANENT
              : PolicyAction.INCREMENT;
          }
        });
      }

      if (this.application.value === ApplicationType.GaussDBDWS) {
        const repPolicy = filter(
          this.policy_list,
          item => item.type === PolicyType.REPLICATION
        );

        const backupPolicy = find(
          this.policy_list,
          item => item.type === PolicyType.BACKUP
        );

        if (
          !!size(repPolicy) &&
          !!get(backupPolicy, 'ext_parameters.storage_id')
        ) {
          const storageDetail = [
            this.nasDistributionStoragesApiService.NasDistributionStorageInfo({
              id: get(backupPolicy, 'ext_parameters.storage_id'),
              akDoException: false
            })
          ];

          each(repPolicy, item => {
            if (get(item, 'ext_parameters.external_storage_id')) {
              storageDetail.push(
                this.nasDistributionStoragesApiService.NasDistributionStorageInfo(
                  {
                    id: get(item, 'ext_parameters.external_storage_id'),
                    clustersId: get(item, 'ext_parameters.external_system_id'),
                    clustersType: '2',
                    akDoException: false
                  }
                )
              );
            }
          });

          combineLatest(storageDetail).subscribe(
            resArr => {
              const backupClusters = _map(first(resArr).unitList, item => {
                return get(item, '');
              });

              resArr.shift();
              const repeatCluster = find(resArr, res => {
                const repClusters = _map(res.unitList, item => {
                  return get(item, 'deviceId');
                });

                const clusters = [...repClusters, ...backupClusters];

                if (size(clusters) === size(uniq(clusters))) {
                  return false;
                } else {
                  return true;
                }
              });

              if (!repeatCluster) {
                this.slaApiService
                  .createSLAUsingPOST({
                    slaDto: body,
                    akOperationTipsContent: this.getOperationTipsContent()
                  })
                  .subscribe(
                    res => {
                      observer.next(res);
                      observer.complete();
                    },
                    error => {
                      observer.error(error);
                      observer.complete();
                    }
                  );
              } else {
                this.messageService.error(
                  this.i18n.get(
                    'protection_dws_sla_repeat_cluster_error_label',
                    [repeatCluster.name]
                  ),
                  {
                    lvMessageKey: 'lvMsg_dws_sla_valid_error',
                    lvShowCloseButton: true
                  }
                );
                observer.error(null);
                observer.complete();
              }
            },
            error => {
              this.slaApiService
                .createSLAUsingPOST({
                  slaDto: body,
                  akOperationTipsContent: this.getOperationTipsContent()
                })
                .subscribe(
                  res => {
                    observer.next(res);
                    observer.complete();
                  },
                  error => {
                    observer.error(error);
                    observer.complete();
                  }
                );
            }
          );
        } else {
          this.slaApiService
            .createSLAUsingPOST({
              slaDto: body,
              akOperationTipsContent: this.getOperationTipsContent()
            })
            .subscribe(
              res => {
                observer.next(res);
                observer.complete();
              },
              error => {
                observer.error(error);
                observer.complete();
              }
            );
        }
      } else {
        this.slaApiService
          .createSLAUsingPOST({
            slaDto: body,
            akOperationTipsContent: this.getOperationTipsContent()
          })
          .subscribe(
            res => {
              observer.next(res);
              observer.complete();
            },
            error => {
              observer.error(error);
              observer.complete();
            }
          );
      }
    });
  }

  getOperationTipsContent() {
    if (this.router.url === '/protection/policy/sla') {
      return this.i18n.get('protection_sla_instruction_label');
    } else {
      return '';
    }
  }

  onModify(): Observable<void> {
    return new Observable<void>((observer: Observer<void>) => {
      if (this.formGroup.invalid) {
        return;
      }

      if (!this.dwsStorageVaild()) {
        observer.error(null);
        return;
      }

      const body = assign(this.formGroup.value, {
        application: this.application.value,
        policy_list: this.policy_list
      });

      // 升级场景复制策略，replication_target_type为空，默认复制所有。
      each(body.policy_list, item => {
        if (
          item.action === PolicyType.REPLICATION &&
          isUndefined(item.ext_parameters?.replication_target_type)
        ) {
          assign(item.ext_parameters, {
            replication_target_type: DataMap.slaReplicationRule.all.value
          });
        }
      });
      if (
        includes(
          [
            ApplicationType.Fileset,
            ApplicationType.NASShare,
            ApplicationType.Volume,
            ApplicationType.ObjectStorage
          ],
          this.application.value
        )
      ) {
        each(body.policy_list, item => {
          if (has(item, 'permanentBackup')) {
            item.action = get(item, 'permanentBackup')
              ? PolicyAction.PERMANENT
              : PolicyAction.INCREMENT;
          }
        });
      }
      if (!deepEqualObject(pick(this.sla, keys(body)), body)) {
        this.warningMessageService.create({
          content: this.i18n.get(
            'protection_modify_sla_warn_label',
            [this.formGroup.value.name],
            false,
            true
          ),
          onOK: () => {
            if (this.application.value === ApplicationType.GaussDBDWS) {
              const repPolicy = filter(
                this.policy_list,
                item => item.type === PolicyType.REPLICATION
              );

              const backupPolicy = find(
                this.policy_list,
                item => item.type === PolicyType.BACKUP
              );

              if (
                !!size(repPolicy) &&
                !!get(backupPolicy, 'ext_parameters.storage_id')
              ) {
                const storageDetail = [
                  this.nasDistributionStoragesApiService.NasDistributionStorageInfo(
                    {
                      id: get(backupPolicy, 'ext_parameters.storage_id'),
                      akDoException: false
                    }
                  )
                ];

                each(repPolicy, item => {
                  if (get(item, 'ext_parameters.external_storage_id')) {
                    storageDetail.push(
                      this.nasDistributionStoragesApiService.NasDistributionStorageInfo(
                        {
                          id: get(item, 'ext_parameters.external_storage_id'),
                          clustersId: get(
                            item,
                            'ext_parameters.external_system_id'
                          ),
                          clustersType: '2',
                          akDoException: false
                        }
                      )
                    );
                  }
                });

                combineLatest(storageDetail).subscribe(
                  resArr => {
                    const backupClusters = _map(
                      first(resArr).unitList,
                      item => {
                        return get(item, 'deviceId');
                      }
                    );

                    resArr.shift();
                    const repeatCluster = find(resArr, res => {
                      const repClusters = _map(res.unitList, item => {
                        return get(item, 'deviceId');
                      });

                      const clusters = [...repClusters, ...backupClusters];

                      if (size(clusters) === size(uniq(clusters))) {
                        return false;
                      } else {
                        return true;
                      }
                    });

                    if (!repeatCluster) {
                      this.slaApiService
                        .modifySLAUsingPUT({
                          slaDto: body,
                          akOperationTipsContent: this.getOperationTipsContent()
                        })
                        .subscribe(
                          res => {
                            observer.next();
                            observer.complete();
                          },
                          error => {
                            observer.error(error);
                            observer.complete();
                          }
                        );
                    } else {
                      this.messageService.error(
                        this.i18n.get(
                          'protection_dws_sla_repeat_cluster_error_label',
                          [repeatCluster.name]
                        ),
                        {
                          lvMessageKey: 'lvMsg_dws_sla_valid_error',
                          lvShowCloseButton: true
                        }
                      );
                      observer.error(null);
                      observer.complete();
                    }
                  },
                  error => {
                    this.slaApiService
                      .modifySLAUsingPUT({
                        slaDto: body,
                        akOperationTipsContent: this.getOperationTipsContent()
                      })
                      .subscribe(
                        res => {
                          observer.next();
                          observer.complete();
                        },
                        error => {
                          observer.error(error);
                          observer.complete();
                        }
                      );
                  }
                );
              } else {
                this.slaApiService
                  .modifySLAUsingPUT({
                    slaDto: body,
                    akOperationTipsContent: this.getOperationTipsContent()
                  })
                  .subscribe(
                    res => {
                      observer.next();
                      observer.complete();
                    },
                    error => {
                      observer.error(error);
                      observer.complete();
                    }
                  );
              }
            } else {
              this.slaApiService
                .modifySLAUsingPUT({
                  slaDto: body,
                  akOperationTipsContent: this.getOperationTipsContent()
                })
                .subscribe(
                  res => {
                    observer.next();
                    observer.complete();
                  },
                  error => {
                    observer.error(error);
                    observer.complete();
                  }
                );
            }
          },
          onCancel: () => {
            observer.error(null);
            observer.complete();
          },
          lvAfterClose: result => {
            if (result && result.trigger === 'close') {
              observer.error(null);
              observer.complete();
            }
          }
        });
      } else {
        this.slaApiService
          .modifySLAUsingPUT({
            slaDto: body,
            akOperationTipsContent: this.getOperationTipsContent()
          })
          .subscribe(
            res => {
              observer.next();
              observer.complete();
            },
            error => {
              observer.error(error);
              observer.complete();
            }
          );
      }
    });
  }

  dwsStorageVaild() {
    if (this.application.value === ApplicationType.GaussDBDWS) {
      const repPolicy = find(
        this.policy_list,
        item => item.type === PolicyType.REPLICATION
      );

      const backupPolicy = find(
        this.policy_list,
        item => item.type === PolicyType.BACKUP
      );

      if (!!repPolicy) {
        if (
          !!get(backupPolicy, 'ext_parameters.storage_info.storage_id') &&
          !get(repPolicy, 'ext_parameters.external_storage_id') &&
          this.dwsGroup &&
          !(this.dwsGroup && !this.dwsParallel)
        ) {
          this.messageService.error(
            this.i18n.get('protection_dws_sla_storage_vaild_tips_label'),
            {
              lvShowCloseButton: true,
              lvMessageKey: 'dwsStorageIdErrorMessageKey'
            }
          );
          return false;
        }

        if (
          !get(backupPolicy, 'ext_parameters.storage_info.storage_id') &&
          !!get(repPolicy, 'ext_parameters.external_storage_id')
        ) {
          each(this.policy_list, item => {
            if (item.type === PolicyType.REPLICATION) {
              item.ext_parameters = omit(item.ext_parameters, [
                'external_storage_id'
              ]);
            }
          });
        }
      }
    }
    return true;
  }
}
