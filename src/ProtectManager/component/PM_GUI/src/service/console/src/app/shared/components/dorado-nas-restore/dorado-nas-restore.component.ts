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
import { Component, Input, OnInit } from '@angular/core';
import { FormBuilder, FormControl, FormGroup } from '@angular/forms';
import { RegisterNasShareComponent } from 'app/business/protection/storage/nas-shared/register-nas-share/register-nas-share.component';
import { KerberosComponent } from 'app/business/system/security/kerberos/kerberos.component';
import {
  ApiStorageBackupPluginService,
  BackupClustersApiService,
  BaseUtilService,
  DataMapService,
  KerberosAPIService,
  ProtectedResourceApiService
} from 'app/shared';
import {
  ClientManagerApiService,
  RestoreApiV2Service
} from 'app/shared/api/services';
import {
  AgentsSubType,
  CommonConsts,
  DataMap,
  MODAL_COMMON,
  NasFileReplaceStrategy,
  NasSecurityStyle,
  ProxyHostSelectMode,
  RestoreV2LocationType,
  RestoreV2Type
} from 'app/shared/consts';
import { I18NService } from 'app/shared/services';
import { AppUtilsService } from 'app/shared/services/app-utils.service';
import { DrawModalService } from 'app/shared/services/draw-modal.service';
import {
  assign,
  cloneDeep,
  each,
  filter,
  find,
  includes,
  isEmpty,
  isFunction,
  isNumber,
  map,
  omit,
  pick,
  reject,
  size,
  toString,
  trim,
  uniqBy
} from 'lodash';
import { Observable, Observer } from 'rxjs';
import { CreateFileSystemComponent } from './create-file-system/create-file-system.component';
@Component({
  selector: 'aui-dorado-nas-restore',
  templateUrl: './dorado-nas-restore.component.html',
  styleUrls: ['./dorado-nas-restore.component.less']
})
export class DoradoNasRestoreComponent implements OnInit {
  @Input() rowCopy;
  @Input() restoreType;
  @Input() targetParams;

  includes = includes;
  find = find;
  formGroup: FormGroup;
  restoreLocationType = RestoreV2LocationType;
  fileReplaceStrategy = NasFileReplaceStrategy;
  restoreTypeEnum = RestoreV2Type;
  proxyHostSelectMode = ProxyHostSelectMode;
  dataMap = DataMap;
  originalLocation = '';
  equipmentOptions = [];
  isDoradoEquipment = false;
  fileSystemOptions = [];
  nativeFileSystemOptions = [];
  shareOptions = [];
  fileShareMode;
  authModeOptions = [];
  kerberosOptions = [];
  proxyHostOptions = [];
  onlineNodeOps = [];
  allNodeOps = [];
  defaultNode = {};
  abNormalFlag = false;
  objStorageArchival = false;
  resourceObj;
  isNasFileSystemFileRestore = false;
  isCloudArchiveCopy = false;
  restoreToNewLocationOnly = false;
  resourceShareMode;
  isOceanProtect = !includes(
    [
      DataMap.Deploy_Type.cloudbackup2.value,
      DataMap.Deploy_Type.cloudbackup.value,
      DataMap.Deploy_Type.hyperdetect.value,
      DataMap.Deploy_Type.cyberengine.value
    ],
    this.i18n.get('deploy_type')
  );
  isNdmp = false;
  isSupportReg = false;
  isX3000 = this.i18n.get('deploy_type') === DataMap.Deploy_Type.x3000.value;
  exterAgent = includes(
    [
      DataMap.Deploy_Type.x3000.value,
      DataMap.Deploy_Type.x8000.value,
      DataMap.Deploy_Type.x6000.value,
      DataMap.Deploy_Type.x9000.value,
      DataMap.Deploy_Type.e6000.value,
      DataMap.Deploy_Type.decouple.value,
      DataMap.Deploy_Type.openOem.value,
      DataMap.Deploy_Type.openServer.value
    ],
    this.i18n.get('deploy_type')
  );
  channelsErrorTip = {
    ...this.baseUtilService.rangeErrorTip,
    invalidRang: this.i18n.get('common_valid_rang_label', [1, 40])
  };
  tipsLabel = this.i18n.get('protection_nasshare_channels_tips_label');
  hostOptions = [];
  cacheOptions = [];
  poxyOptions = [];
  isShowChannels = false;
  tapeCopy = false;

  isNdmpPath = false;
  MAX_PATH_LENGTH = 4096;
  directoryErrorTip = {
    ...this.baseUtilService.requiredErrorTip,
    invalidName: this.i18n.get('common_path_error_label'),
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [
      this.MAX_PATH_LENGTH
    ])
  };

  constructor(
    private fb: FormBuilder,
    private i18n: I18NService,
    public restoreV2Service: RestoreApiV2Service,
    private dataMapService: DataMapService,
    private kerberosApi: KerberosAPIService,
    public baseUtilService: BaseUtilService,
    private drawModalService: DrawModalService,
    private protectedResourceApiService: ProtectedResourceApiService,
    private apiStorageBackupPluginService: ApiStorageBackupPluginService,
    private BackupClustersApiService: BackupClustersApiService,
    private appUtilsService?: AppUtilsService,
    private clientManagerApiService?: ClientManagerApiService
  ) {}

  ngOnInit() {
    // 判断副本是否是磁带归档，且已开启索引
    this.tapeCopy =
      this.rowCopy?.generated_by ===
        DataMap.CopyData_generatedType.tapeArchival.value &&
      this.rowCopy?.indexed === DataMap.CopyData_fileIndex.indexed.value;
    this.isNdmp =
      this.rowCopy.resource_sub_type === DataMap.Resource_Type.ndmp.value;
    this.initForm();
    this.getKerberosOptions();
    this.getEquipmentOptions();
    this.objStorageArchival =
      this.rowCopy.generated_by ===
      DataMap.CopyData_generatedType.cloudArchival.value;
    this.getProxyOptions();
  }

  queryNodes() {
    this.BackupClustersApiService.queryAllMembers({
      akLoading: false
    }).subscribe(res => {
      let arr = [];
      this.allNodeOps = res;
      arr = filter(
        res,
        item => item.status === DataMap.Cluster_Status.online.value
      );
      this.onlineNodeOps = map(arr, v => {
        return {
          disabled: v.status !== DataMap.Cluster_Status.online.value,
          isLeaf: true,
          label: v.clusterName,
          value: v.remoteEsn,
          ...v
        };
      });
      this.defaultNode = find(this.allNodeOps, {
        remoteEsn: this.rowCopy.device_esn
      });
      this.abNormalFlag =
        this.defaultNode['status'] !== DataMap.Node_Status.online.value;
      this.formGroup.get('memberEsn').setValue(this.defaultNode['remoteEsn']);
    });
  }
  getProxyOptions() {
    if (
      !this.exterAgent ||
      !includes(
        [
          DataMap.Resource_Type.NASShare.value,
          DataMap.Resource_Type.NASFileSystem.value,
          DataMap.Resource_Type.ndmp.value
        ],
        this.rowCopy.resource_sub_type
      )
    ) {
      return;
    }

    const conditions = {
      pluginType:
        this.rowCopy.resource_sub_type === DataMap.Resource_Type.NASShare.value
          ? AgentsSubType.NasShare
          : this.rowCopy.resource_sub_type ===
            DataMap.Resource_Type.NASFileSystem.value
          ? AgentsSubType.NasFileSystem
          : AgentsSubType.Ndmp,
      linkStatus: [DataMap.resource_LinkStatus_Special.normal.value]
    };

    const extParams = {
      conditions: JSON.stringify(conditions)
    };
    this.appUtilsService.getResourceByRecursion(
      extParams,
      params => this.clientManagerApiService.queryAgentListInfoUsingGET(params),
      resource => {
        const hostArray = [];
        each(resource, item => {
          hostArray.push({
            ...item,
            key: item.uuid,
            value: item.rootUuid || item.parentUuid,
            label: `${item.name}(${item.endpoint})`,
            isLeaf: true
          });
        });

        this.cacheOptions = cloneDeep(hostArray);
        this.filterHostOps(this.formGroup.value.restoreLocation);
        each(hostArray, item => {
          if (
            item.extendInfo.scenario === DataMap.proxyHostType.builtin.value
          ) {
            this.poxyOptions.push(item.rootUuid || item.parentUuid);
          }
        });

        this.formGroup.get('proxyHost').setValue(this.poxyOptions);
      }
    );
  }

  filterHostOps(restoreLocation) {
    if (
      includes(
        [DataMap.Resource_Type.NASFileSystem.value],
        this.rowCopy.resource_sub_type
      ) &&
      this.restoreType === this.restoreTypeEnum.CommonRestore &&
      restoreLocation === this.restoreLocationType.ORIGIN
    ) {
      this.hostOptions = this.cacheOptions.filter(
        item =>
          item.extendInfo.scenario !== DataMap.proxyHostType.external.value
      );
    } else {
      this.hostOptions = this.cacheOptions;
    }
  }

  getEquipmentOptions(recordsTemp?, startPage?) {
    this.protectedResourceApiService
      .ListResources({
        pageSize: CommonConsts.PAGE_SIZE * 10,
        pageNo: startPage || CommonConsts.PAGE_START,
        conditions: JSON.stringify({
          subType: this.dataMapService
            .toArray('Device_Storage_Type')
            .map(item => {
              return item.value;
            })
        })
      })
      .subscribe(res => {
        if (!recordsTemp) {
          recordsTemp = [];
        }
        if (!isNumber(startPage)) {
          startPage = CommonConsts.PAGE_START;
        }
        startPage++;
        recordsTemp = [...recordsTemp, ...res.records];
        if (
          startPage === Math.ceil(res.totalCount / 200) ||
          res.totalCount === 0
        ) {
          this.equipmentOptions = map(recordsTemp, item => {
            assign(item, {
              label: item.name,
              isLeaf: true
            });
            return item;
          });
          if (
            this.rowCopy.resource_sub_type === DataMap.Resource_Type.ndmp.value
          ) {
            this.filterOptions();
          }
          return;
        }
        this.getEquipmentOptions(recordsTemp, startPage);
      });
  }

  filterOptions() {
    const copyType = JSON.parse(this.rowCopy.resource_properties);
    if (!isEmpty(copyType.extendInfo.ndmpType)) {
      this.equipmentOptions = this.equipmentOptions.filter(
        v =>
          v.subType === DataMap.Device_Storage_Type.ndmp.value &&
          v.extendInfo.ndmpType === copyType.extendInfo.ndmpType
      );
    } else {
      this.equipmentOptions = this.equipmentOptions.filter(
        v => v.subType === DataMap.Device_Storage_Type.ndmp.value
      );
    }
  }

  getFileSystemOptions(evId, fileSystem?, recordsTemp?, startPage?) {
    this.protectedResourceApiService
      .ListResources({
        pageNo: startPage || CommonConsts.PAGE_START,
        pageSize: CommonConsts.PAGE_SIZE * 10,
        conditions: JSON.stringify({
          subType:
            this.rowCopy.resource_sub_type === DataMap.Resource_Type.ndmp.value
              ? [DataMap.Resource_Type.ndmp.value]
              : [DataMap.Resource_Type.NASFileSystem.value],
          parentUuid: evId
        })
      })
      .subscribe(res => {
        if (!recordsTemp) {
          recordsTemp = [];
        }
        if (!isNumber(startPage)) {
          startPage = CommonConsts.PAGE_START;
        }
        startPage++;
        recordsTemp = [...recordsTemp, ...res.records];
        if (
          startPage === Math.ceil(res.totalCount / 200) ||
          res.totalCount === 0
        ) {
          this.fileSystemOptions = map(
            reject(recordsTemp, item => {
              return item.uuid === this.resourceObj.uuid;
            }),
            item => {
              assign(item, {
                label: item.name,
                isLeaf: true
              });
              return item;
            }
          );
          if (this.resourceShareMode === DataMap.Shared_Mode.cifs.value) {
            this.fileSystemOptions = reject(this.fileSystemOptions, item => {
              return includes(
                [
                  DataMap.NasFileSystem_Protocol.none.value,
                  DataMap.NasFileSystem_Protocol.nfs.value
                ],
                item.extendInfo?.protocol
              );
            });
          } else {
            this.fileSystemOptions = reject(this.fileSystemOptions, item => {
              return includes(
                [
                  DataMap.NasFileSystem_Protocol.none.value,
                  DataMap.NasFileSystem_Protocol.cifs.value
                ],
                item.extendInfo?.protocol
              );
            });
          }
          if (fileSystem) {
            this.formGroup
              .get('fileSystem')
              .setValue(fileSystem, { emitEvent: false });
          }
          if (!fileSystem) {
            this.formGroup.get('share').setValue('', { emitEvent: false });
          }
          return;
        }
        this.getFileSystemOptions(evId, fileSystem, recordsTemp, startPage);
      });
  }

  getShareOptions(fileSystemId, share?) {
    this.apiStorageBackupPluginService
      .ShowStorageFileSystemInfo({
        fileSystemId
      })
      .subscribe(res => {
        if (this.resourceShareMode === DataMap.Shared_Mode.cifs.value) {
          this.shareOptions = [
            ...uniqBy(
              map(res.cifsShares, item => {
                assign(item, {
                  label: item.shareName,
                  key: item.shareName,
                  type: DataMap.Shared_Mode.cifs.value,
                  isLeaf: true
                });
                return item;
              }),
              'key'
            )
          ];
        } else {
          this.shareOptions = [
            ...filter(
              uniqBy(
                map(res.nfsShares, item => {
                  assign(item, {
                    label: item.shareName || item.sharePath,
                    key: item.sharePath,
                    type: DataMap.Shared_Mode.nfs.value,
                    isLeaf: true
                  });
                  return item;
                }),
                'key'
              ),
              nfs => {
                return !isEmpty(
                  find(nfs.nfsClients, client => {
                    return +client.rootSquash === 1;
                  })
                );
              }
            )
          ];
        }
        if (share) {
          this.formGroup.get('share').setValue(share, { emitEvent: false });
          const selectedFile = find(this.shareOptions, {
            key: share
          });
          if (
            selectedFile &&
            selectedFile.type === DataMap.Shared_Mode.cifs.value
          ) {
            this.authModeOptions = this.dataMapService
              .toArray('Nas_Share_Auth_Mode')
              .filter(item => {
                if (item.value === DataMap.Nas_Share_Auth_Mode.system.value) {
                  item.label = this.i18n.get('common_everyone_label');
                }
                return (item.isLeaf = true);
              });
          } else {
            this.authModeOptions = this.dataMapService
              .toArray('Nas_Share_Auth_Mode')
              .filter(item => {
                return [
                  DataMap.Nas_Share_Auth_Mode.system.value,
                  DataMap.Nas_Share_Auth_Mode.kerberos.value
                ].includes(item.value);
              })
              .filter(v => {
                return (v.isLeaf = true);
              });
          }
          this.formGroup.get('auth_mode').setValue(this.targetParams.auth_mode);
        } else {
          this.formGroup.get('share').setValue('');
          this.formGroup.get('auth_mode').setValue('');
          this.authModeOptions = [];
        }
      });
  }

  getNasshareOptions(evId, nasshare?, recordsTemp?, startPage?) {
    this.protectedResourceApiService
      .ListResources({
        pageNo: startPage || CommonConsts.PAGE_START,
        pageSize: CommonConsts.PAGE_SIZE * 10,
        conditions: JSON.stringify({
          subType: [DataMap.Resource_Type.NASShare.value],
          parentUuid: evId
        })
      })
      .subscribe(res => {
        if (!recordsTemp) {
          recordsTemp = [];
        }
        if (!isNumber(startPage)) {
          startPage = CommonConsts.PAGE_START;
        }
        startPage++;
        recordsTemp = [...recordsTemp, ...res.records];
        if (
          startPage === Math.ceil(res.totalCount / 200) ||
          res.totalCount === 0
        ) {
          this.shareOptions = reject(
            map(recordsTemp, item => {
              assign(item, {
                label:
                  this.isSupportReg && item.extendInfo?.ip
                    ? `${item.name} (${item.extendInfo?.ip})`
                    : item.name,
                key: item.uuid,
                type: item.extendInfo?.shareMode,
                isLeaf: true
              });
              return item;
            }),
            v => {
              return (
                (v.uuid === this.resourceObj.uuid &&
                  this.rowCopy.resource_sub_type ===
                    DataMap.Resource_Type.NASShare.value) ||
                v.type !== this.resourceShareMode ||
                isEmpty(v.extendInfo?.ip)
              );
            }
          );
          if (nasshare) {
            this.formGroup
              .get('share')
              .setValue(nasshare, { emitEvent: false });
          } else {
            this.formGroup.get('share').setValue('');
          }
          return;
        }
        this.getNasshareOptions(evId, nasshare, recordsTemp, startPage);
      });
  }

  getNativeFileSystemOptions(fileSystem?, recordsTemp?, startPage?, esn?) {
    this.apiStorageBackupPluginService
      .ListFilesystems({
        memberEsn: esn || '',
        pageNo: startPage || CommonConsts.PAGE_START,
        pageSize: CommonConsts.PAGE_SIZE * 10
      })
      .subscribe(res => {
        if (!recordsTemp) {
          recordsTemp = [];
        }
        if (!isNumber(startPage)) {
          startPage = CommonConsts.PAGE_START;
        }
        startPage++;
        recordsTemp = [...recordsTemp, ...res.records];
        if (
          startPage === Math.ceil(res.totalCount / 200) ||
          res.totalCount === 0
        ) {
          this.nativeFileSystemOptions = map(recordsTemp, item => {
            assign(item, {
              label: item.name,
              key: item.id,
              isLeaf: true
            });
            return item;
          });
          if (this.resourceShareMode === DataMap.Shared_Mode.cifs.value) {
            this.nativeFileSystemOptions = reject(
              this.nativeFileSystemOptions,
              item => {
                return (
                  includes(
                    [
                      +DataMap.NasFileSystem_Protocol.nfs.value,
                      +DataMap.NasFileSystem_Protocol.none.value
                    ],
                    item.protocolType
                  ) || item.securityStyle === NasSecurityStyle.UNIX
                );
              }
            );
          } else {
            this.nativeFileSystemOptions = reject(
              this.nativeFileSystemOptions,
              item => {
                return (
                  includes(
                    [
                      +DataMap.NasFileSystem_Protocol.cifs.value,
                      +DataMap.NasFileSystem_Protocol.none.value
                    ],
                    item.protocolType
                  ) || item.securityStyle === NasSecurityStyle.NTFS
                );
              }
            );
          }
          if (fileSystem) {
            this.formGroup.get('fileSystem').setValue(fileSystem);
          } else if (
            this.targetParams?.fileSystem &&
            this.targetParams?.restoreLocation === RestoreV2LocationType.NATIVE
          ) {
            this.formGroup
              .get('fileSystem')
              .setValue(this.targetParams?.fileSystem);
          } else {
            this.formGroup.get('fileSystem').setValue('');
          }
          return;
        }
        this.getNativeFileSystemOptions(
          fileSystem,
          recordsTemp,
          startPage,
          esn
        );
      });
  }

  createNasShare() {
    this.drawModalService.create(
      assign({}, MODAL_COMMON.generateDrawerOptions(), {
        lvModalKey: 'reigster-nas-shared',
        lvWidth: MODAL_COMMON.normalWidth + 100,
        lvHeader: this.i18n.get('common_register_label'),
        lvContent: RegisterNasShareComponent,
        lvOkDisabled: true,
        lvComponentParams: {
          item: {
            sub_type: DataMap.Resource_Type.NASShare.value
          }
        },
        lvAfterOpen: modal => {
          const content = modal.getContentComponent() as RegisterNasShareComponent;
          const modalIns = modal.getInstance();
          content.formGroup.statusChanges.subscribe(res => {
            modalIns.lvOkDisabled = res !== 'VALID';
          });
          content.formGroup.updateValueAndValidity();
        },
        lvOk: modal => {
          return new Promise(resolve => {
            const content = modal.getContentComponent() as RegisterNasShareComponent;
            content.onOK().subscribe({
              next: res => {
                resolve(true);
                this.getNasshareOptions(this.formGroup.value.equipment);
              },
              error: () => resolve(false)
            });
          });
        }
      })
    );
  }

  restoreLocationChange(res) {
    if (res === RestoreV2LocationType.ORIGIN) {
      this.formGroup.get('equipment').clearValidators();
      this.formGroup.get('domain').clearValidators();
      this.formGroup.get('username').clearValidators();
      this.formGroup.get('password').clearValidators();
      this.formGroup.get('kerberos').clearValidators();
      this.formGroup.get('fileSystem').clearValidators();
      this.formGroup.get('fullName')?.clearValidators();
      if (
        (this.isNasFileSystemFileRestore || this.isCloudArchiveCopy) &&
        !this.isNdmp
      ) {
        this.formGroup
          .get('share')
          .setValidators([this.baseUtilService.VALID.required()]);
        this.formGroup
          .get('shareIp')
          .setValidators([
            this.baseUtilService.VALID.required(),
            this.baseUtilService.VALID.ip(CommonConsts.REGEX.nasshareDomain)
          ]);
        this.formGroup
          .get('auth_mode')
          .setValidators([this.baseUtilService.VALID.required()]);
        this.getShareOptions(this.resourceObj.uuid);
      } else {
        this.formGroup.get('share').clearValidators();
        this.formGroup.get('shareIp').clearValidators();
        this.formGroup.get('auth_mode').clearValidators();
      }
    } else if (res === RestoreV2LocationType.NEW) {
      this.formGroup.get('fileSystem').clearValidators();
      this.formGroup
        .get('equipment')
        .setValidators([this.baseUtilService.VALID.required()]);
      if (!this.isNdmp) {
        this.formGroup
          .get('share')
          .setValidators([this.baseUtilService.VALID.required()]);
      }
      if (this.isDoradoEquipment) {
        this.formGroup
          .get('shareIp')
          .setValidators([
            this.baseUtilService.VALID.required(),
            this.baseUtilService.VALID.ip(CommonConsts.REGEX.nasshareDomain)
          ]);
        if (!this.isNdmp) {
          this.formGroup
            .get('auth_mode')
            .setValidators([this.baseUtilService.VALID.required()]);
        }
        this.formGroup
          .get('fileSystem')
          .setValidators([this.baseUtilService.VALID.required()]);
      }
      if (this.isNdmpPath) {
        this.formGroup
          .get('fullName')
          ?.setValidators([
            this.baseUtilService.VALID.name(CommonConsts.REGEX.linuxPath),
            this.baseUtilService.VALID.maxLength(this.MAX_PATH_LENGTH)
          ]);
      }
    } else {
      this.formGroup.get('equipment').clearValidators();
      this.formGroup.get('share').clearValidators();
      this.formGroup.get('shareIp').clearValidators();
      this.formGroup.get('auth_mode').clearValidators();
      this.formGroup.get('domain').clearValidators();
      this.formGroup.get('username').clearValidators();
      this.formGroup.get('password').clearValidators();
      this.formGroup.get('kerberos').clearValidators();
      this.formGroup.get('fullName')?.clearValidators();
      this.formGroup
        .get('fileSystem')
        .setValidators([this.baseUtilService.VALID.required()]);
      if (!size(this.nativeFileSystemOptions) && !this.isOceanProtect) {
        this.getNativeFileSystemOptions();
      }

      if (this.isOceanProtect && !size(this.onlineNodeOps)) {
        this.queryNodes();
      }
    }
  }

  initForm() {
    this.isNasFileSystemFileRestore =
      this.restoreType === RestoreV2Type.FileRestore &&
      includes(
        [
          DataMap.Resource_Type.NASFileSystem.value,
          DataMap.Resource_Type.ndmp.value
        ],
        this.rowCopy.resource_sub_type
      );
    this.isCloudArchiveCopy =
      includes(
        [
          DataMap.Resource_Type.NASFileSystem.value,
          DataMap.Resource_Type.ndmp.value
        ],
        this.rowCopy.resource_sub_type
      ) &&
      includes(
        [
          DataMap.CopyData_generatedType.cloudArchival.value,
          DataMap.CopyData_generatedType.tapeArchival.value,
          DataMap.CopyData_generatedType.Imported.value
        ],
        this.rowCopy.generated_by
      );
    this.restoreToNewLocationOnly =
      includes(
        [
          DataMap.CopyData_generatedType.replicate.value,
          DataMap.CopyData_generatedType.cascadedReplication.value
        ],
        this.rowCopy.generated_by
      ) ||
      this.rowCopy.is_replicated ||
      (includes(
        [DataMap.CopyData_generatedType.reverseReplication.value],
        this.rowCopy.generated_by
      ) &&
        includes(
          [
            DataMap.Resource_Type.NASFileSystem.value,
            DataMap.Resource_Type.ndmp.value
          ],
          this.rowCopy.resource_sub_type
        ));

    this.resourceObj = JSON.parse(this.rowCopy.resource_properties);
    if (
      includes(
        [
          DataMap.Resource_Type.NASFileSystem.value,
          DataMap.Resource_Type.ndmp.value
        ],
        this.rowCopy.resource_sub_type
      )
    ) {
      const properties = JSON.parse(this.rowCopy.properties);
      const repositories = find(properties.repositories, item => {
        return item.type === 1 && includes([5, 6], +item.protocol);
      });
      this.resourceShareMode =
        +repositories?.protocol === 5
          ? DataMap.Shared_Mode.nfs.value
          : DataMap.Shared_Mode.cifs.value;
    } else {
      this.resourceShareMode = this.resourceObj.extendInfo?.shareMode;
    }
    // 是否是NDMP目录恢复
    this.isNdmpPath = this.isNdmp && this.resourceObj.extendInfo?.isFs === '0';
    this.formGroup = this.fb.group({
      isDirectRecovery: new FormControl(this.tapeCopy),
      originalLocation: new FormControl(this.rowCopy.resource_location),
      restoreLocation: new FormControl(RestoreV2LocationType.ORIGIN),
      equipment: new FormControl(''),
      share: new FormControl(''),
      shareIp: new FormControl(''),
      auth_mode: new FormControl(''),
      domain: new FormControl(''),
      username: new FormControl(''),
      password: new FormControl(''),
      kerberos: new FormControl(''),
      memberEsn: new FormControl(''),
      fileSystem: new FormControl(''),
      channels: new FormControl(10),
      proxyMode: new FormControl(ProxyHostSelectMode.Auto),
      proxyHost: new FormControl([]),
      originalType: new FormControl(NasFileReplaceStrategy.Replace),
      restoreOrder: new FormControl(false),
      orderBy: new FormControl(1)
    });
    // NAS文件级或归档到云的副本
    if (
      (this.isNasFileSystemFileRestore || this.isCloudArchiveCopy) &&
      !this.isNdmp
    ) {
      this.formGroup
        .get('share')
        .setValidators([this.baseUtilService.VALID.required()]);
      if (!this.targetParams || this.targetParams.shareIp) {
        this.formGroup
          .get('shareIp')
          .setValidators([
            this.baseUtilService.VALID.required(),
            this.baseUtilService.VALID.ip(CommonConsts.REGEX.nasshareDomain)
          ]);
        this.formGroup
          .get('auth_mode')
          .setValidators([this.baseUtilService.VALID.required()]);
      }

      if (this.rowCopy.resource_status === 'NOT_EXIST') {
        this.formGroup
          .get('restoreLocation')
          .setValue(RestoreV2LocationType.NEW);
      }

      if (
        (!this.targetParams ||
          (this.targetParams.restoreLocation === RestoreV2LocationType.ORIGIN &&
            !this.targetParams.share)) &&
        this.rowCopy.resource_status !== 'NOT_EXIST'
      ) {
        this.getShareOptions(this.resourceObj.uuid);
      }
    }

    if (
      includes(
        [
          DataMap.Resource_Type.NASShare.value,
          DataMap.Resource_Type.NASFileSystem.value,
          DataMap.Resource_Type.ndmp.value
        ],
        this.rowCopy.resource_sub_type
      ) &&
      this.exterAgent
    ) {
      this.formGroup
        .get('proxyHost')
        .setValidators([this.baseUtilService.VALID.required()]);
    }

    if (
      includes(
        [DataMap.Resource_Type.NASShare.value],
        this.rowCopy.resource_sub_type
      )
    ) {
      this.formGroup
        .get('channels')
        .setValidators([
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.integer(),
          this.baseUtilService.VALID.rangeValue(1, 40)
        ]);
    }

    // 新增目录
    if (this.isNdmpPath) {
      this.formGroup.addControl('fullName', new FormControl(''));
    }

    this.formGroup.get('restoreLocation').valueChanges.subscribe(res => {
      this.filterHostOps(res);
      this.formGroup.patchValue({
        equipment: '',
        share: '',
        shareIp: '',
        auth_mode: '',
        domain: '',
        username: '',
        password: '',
        kerberos: '',
        fileSystem: '',
        proxyMode: ProxyHostSelectMode.Auto,
        proxyHost: this.poxyOptions,
        originalType: NasFileReplaceStrategy.Replace
      });

      if (
        this.rowCopy.resource_sub_type ===
        DataMap.Resource_Type.NASFileSystem.value
      ) {
        if (res === RestoreV2LocationType.NEW) {
          this.isShowChannels = true;
          this.formGroup
            .get('channels')
            .setValidators([
              this.baseUtilService.VALID.required(),
              this.baseUtilService.VALID.integer(),
              this.baseUtilService.VALID.rangeValue(1, 40)
            ]);
        } else {
          this.isShowChannels = false;
          this.formGroup.get('channels').clearValidators();
        }
        this.formGroup.get('channels').updateValueAndValidity();
      }

      this.restoreLocationChange(res);

      this.formGroup
        .get('equipment')
        .updateValueAndValidity({ emitEvent: false });
      this.formGroup.get('share').updateValueAndValidity({ emitEvent: false });
      this.formGroup.get('shareIp').updateValueAndValidity();
      this.formGroup.get('auth_mode').updateValueAndValidity();
      this.formGroup.get('domain').updateValueAndValidity();
      this.formGroup.get('username').updateValueAndValidity();
      this.formGroup.get('password').updateValueAndValidity();
      this.formGroup.get('kerberos').updateValueAndValidity();
      this.formGroup.get('fullName')?.updateValueAndValidity();
      this.formGroup
        .get('fileSystem')
        .updateValueAndValidity({ emitEvent: false });

      if (res === RestoreV2LocationType.NATIVE && this.isOceanProtect) {
        this.formGroup
          .get('memberEsn')
          .setValidators([this.baseUtilService.VALID.required()]);
      } else {
        this.formGroup.get('memberEsn').clearValidators();
      }
      this.formGroup.get('memberEsn').updateValueAndValidity();
    });

    this.formGroup.get('memberEsn').valueChanges.subscribe(res => {
      if (!res) return;
      this.getNativeFileSystemOptions(undefined, undefined, undefined, res);
    });

    this.formGroup.get('equipment').valueChanges.subscribe(res => {
      this.formGroup.patchValue({
        share: '',
        shareIp: '',
        fileSystem: ''
      });

      if (res && find(this.equipmentOptions, { uuid: res })) {
        this.isDoradoEquipment =
          includes(
            [
              DataMap.Device_Storage_Type.DoradoV7.value,
              DataMap.Device_Storage_Type.OceanStorDoradoV7.value,
              DataMap.Device_Storage_Type.OceanStorDorado_6_1_3.value,
              DataMap.Device_Storage_Type.OceanStor_6_1_3.value
            ],
            find(this.equipmentOptions, { uuid: res }).subType
          ) ||
          this.rowCopy.resource_sub_type === DataMap.Resource_Type.ndmp.value;
      }
      if (this.isDoradoEquipment) {
        this.formGroup
          .get('shareIp')
          .setValidators([
            this.baseUtilService.VALID.required(),
            this.baseUtilService.VALID.ip(CommonConsts.REGEX.nasshareDomain)
          ]);
        this.formGroup
          .get('fileSystem')
          .setValidators([this.baseUtilService.VALID.required()]);
        if (!this.isNdmp) {
          this.formGroup
            .get('auth_mode')
            .setValidators([this.baseUtilService.VALID.required()]);
        }
        this.getFileSystemOptions(res);
      } else {
        this.getNasshareOptions(res);
        this.formGroup.get('shareIp').clearValidators();
        this.formGroup.get('auth_mode').clearValidators();
        this.formGroup.get('domain').clearValidators();
        this.formGroup.get('username').clearValidators();
        this.formGroup.get('password').clearValidators();
        this.formGroup.get('kerberos').clearValidators();
        this.formGroup.get('fileSystem').clearValidators();
      }
      this.formGroup.get('shareIp').updateValueAndValidity();
      this.formGroup.get('auth_mode').updateValueAndValidity();
      this.formGroup.get('domain').updateValueAndValidity();
      this.formGroup.get('username').updateValueAndValidity();
      this.formGroup.get('password').updateValueAndValidity();
      this.formGroup.get('kerberos').clearValidators();
      this.formGroup
        .get('fileSystem')
        .updateValueAndValidity({ emitEvent: false });

      // nas共享：选择的存储设备类型为other时，才可以注册
      this.isSupportReg =
        find(this.equipmentOptions, { uuid: res })?.subType ===
        DataMap.Device_Storage_Type.Other.value;
    });

    this.formGroup.get('fileSystem').valueChanges.subscribe(res => {
      if (
        res &&
        this.formGroup.value.restoreLocation !== RestoreV2LocationType.NATIVE &&
        !this.isNdmp
      ) {
        this.getShareOptions(res);
      }
    });

    this.formGroup.get('share').valueChanges.subscribe(res => {
      const selectedFile = find(this.shareOptions, { key: res });
      if (!selectedFile) {
        return;
      }
      this.fileShareMode = selectedFile.type;
      if (
        (RestoreV2LocationType.ORIGIN ===
          this.formGroup.value.restoreLocation &&
          !this.isNasFileSystemFileRestore &&
          !this.isCloudArchiveCopy) ||
        (!this.isDoradoEquipment &&
          RestoreV2LocationType.NEW === this.formGroup.value.restoreLocation)
      ) {
        return;
      }
      if (selectedFile.type === DataMap.Shared_Mode.cifs.value) {
        this.authModeOptions = this.dataMapService
          .toArray('Nas_Share_Auth_Mode')
          .filter(item => {
            return [
              DataMap.Nas_Share_Auth_Mode.password.value,
              DataMap.Nas_Share_Auth_Mode.kerberos.value
            ].includes(item.value);
          })
          .filter(item => (item.isLeaf = true));
        this.formGroup.get('auth_mode').setValue('');
      } else {
        this.authModeOptions = this.dataMapService
          .toArray('Nas_Share_Auth_Mode')
          .filter(item => {
            return [
              DataMap.Nas_Share_Auth_Mode.system.value,
              DataMap.Nas_Share_Auth_Mode.kerberos.value
            ].includes(item.value);
          })
          .filter(item => (item.isLeaf = true));
        this.formGroup
          .get('auth_mode')
          .setValue(DataMap.Nas_Share_Auth_Mode.system.value);
      }
    });

    this.formGroup.get('auth_mode').valueChanges.subscribe(res => {
      if (
        !trim(res) ||
        (RestoreV2LocationType.ORIGIN ===
          this.formGroup.value.restoreLocation &&
          !this.isNasFileSystemFileRestore &&
          !this.isCloudArchiveCopy)
      ) {
        return;
      }
      if (res === DataMap.Nas_Share_Auth_Mode.kerberos.value) {
        this.formGroup
          .get('kerberos')
          .setValidators([this.baseUtilService.VALID.required()]);
        this.formGroup.get('username').clearValidators();
        this.formGroup.get('password').clearValidators();
      } else if (res === DataMap.Nas_Share_Auth_Mode.password.value) {
        this.formGroup
          .get('username')
          .setValidators([this.baseUtilService.VALID.required()]);
        this.formGroup
          .get('password')
          .setValidators([this.baseUtilService.VALID.required()]);
        this.formGroup.get('kerberos').clearValidators();
      } else {
        this.formGroup.get('kerberos').clearValidators();
        this.formGroup.get('username').clearValidators();
        this.formGroup.get('password').clearValidators();
      }
      this.formGroup.get('kerberos').updateValueAndValidity();
      this.formGroup.get('username').updateValueAndValidity();
      this.formGroup.get('password').updateValueAndValidity();
    });

    this.formGroup.get('originalLocation').disable();

    if (
      this.restoreToNewLocationOnly ||
      this.rowCopy.resource_status === DataMap.Resource_Status.notExist.value
    ) {
      this.formGroup.get('restoreLocation').setValue(RestoreV2LocationType.NEW);
      this.formGroup
        .get('equipment')
        .setValidators([this.baseUtilService.VALID.required()]);
      if (!this.isNdmp) {
        this.formGroup
          .get('share')
          .setValidators([this.baseUtilService.VALID.required()]);
      }
      this.formGroup.get('equipment').updateValueAndValidity();
      this.formGroup.get('share').updateValueAndValidity();
    }

    this.patchValue();
  }

  patchNewTargetParams() {
    if (this.isDoradoEquipment) {
      // 获取filesystem
      this.getFileSystemOptions(
        this.targetParams.equipment,
        this.targetParams.fileSystem
      );
      // 获取共享
      if (!this.isNdmp) {
        this.getShareOptions(
          this.targetParams.fileSystem,
          this.targetParams.share
        );
      }
    } else {
      this.getNasshareOptions(
        this.targetParams.equipment,
        this.targetParams.share
      );
    }
  }

  patchValue() {
    if (this.targetParams) {
      this.isDoradoEquipment = this.targetParams.isDoradoEquipment || false;
      this.fileShareMode = this.targetParams.fileShareMode;
      this.formGroup.patchValue(omit(this.targetParams, ['resource']), {
        emitEvent: false
      });

      if (
        this.targetParams.restoreLocation === RestoreV2LocationType.NATIVE &&
        this.isNasFileSystemFileRestore
      ) {
        this.formGroup
          .get('restoreLocation')
          .setValue(this.targetParams.restoreLocation);
        this.formGroup.get('fileSystem').setValue(this.targetParams.fileSystem);
        this.formGroup
          .get('originalType')
          .setValue(this.targetParams.originalType);
      }

      if (
        this.targetParams.restoreLocation === RestoreV2LocationType.ORIGIN &&
        this.isNasFileSystemFileRestore &&
        !this.isNdmp
      ) {
        this.getShareOptions(this.resourceObj.uuid, this.targetParams.share);
      }
      if (this.targetParams.restoreLocation === RestoreV2LocationType.NEW) {
        this.patchNewTargetParams();
      }
      if (this.targetParams.restoreLocation === RestoreV2LocationType.NATIVE) {
        this.getNativeFileSystemOptions(this.targetParams.fileSystem);
      }
    }
  }

  getKerberosOptions(callback?, recordsTemp?, startPage?) {
    this.kerberosApi
      .queryAllKerberosUsingGET({
        pageSize: CommonConsts.PAGE_SIZE * 10,
        startPage: startPage || CommonConsts.PAGE_START,
        akDoException: false
      })
      .subscribe(res => {
        if (!recordsTemp) {
          recordsTemp = [];
        }
        if (!isNumber(startPage)) {
          startPage = CommonConsts.PAGE_START;
        }
        startPage++;
        recordsTemp = [...recordsTemp, ...res.items];
        if (startPage === Math.ceil(res.total / 200) || res.total === 0) {
          const kerberos = [];
          each(recordsTemp, item => {
            kerberos.push({
              key: item.kerberosId,
              value: item.kerberosId,
              label: item.name,
              isLeaf: true
            });
          });
          this.kerberosOptions = kerberos;
          if (isFunction(callback)) {
            callback();
          }
          return;
        }
        this.getKerberosOptions(callback, recordsTemp, startPage);
      });
  }

  createKerberos() {
    const kerberosComponent = new KerberosComponent(
      this.i18n,
      this.drawModalService
    );
    kerberosComponent.create(undefined, kerberos_id => {
      this.getKerberosOptions(() => {
        this.formGroup.get('kerberos').setValue(kerberos_id);
      });
    });
  }

  createFileSystem() {
    this.drawModalService.create({
      ...MODAL_COMMON.generateDrawerOptions(),
      lvHeader: this.i18n.get('protection_create_file_system_label'),
      lvContent: CreateFileSystemComponent,
      lvOkDisabled: true,
      lvWidth: MODAL_COMMON.normalWidth + 100,
      lvComponentParams: {
        createFileSystem: true,
        resourceShareMode: this.resourceShareMode,
        memberEsn: this.formGroup.value.memberEsn
      },
      lvAfterOpen: modal => {
        const content = modal.getContentComponent() as CreateFileSystemComponent;
        const modalIns = modal.getInstance();
        content.formGroup.statusChanges.subscribe(res => {
          modalIns.lvOkDisabled =
            res !== 'VALID' ||
            (!content.formGroup.value.nfsEnable &&
              !content.formGroup.value.cifsEnable);
        });
      },
      lvOk: modal => {
        return new Promise(resolve => {
          const content = modal.getContentComponent() as CreateFileSystemComponent;
          content.onOK().subscribe(
            res => {
              resolve(true);
              this.getNativeFileSystemOptions(
                null,
                null,
                null,
                this.formGroup.value?.memberEsn
              );
            },
            err => resolve(false)
          );
        });
      }
    });
  }

  getTargetParams() {
    return this.formGroup.value.restoreLocation === RestoreV2LocationType.NEW
      ? assign({}, this.formGroup.value, {
          isDoradoEquipment: this.isDoradoEquipment,
          fileShareMode: this.fileShareMode,
          resource: this.isDoradoEquipment
            ? find(this.fileSystemOptions, {
                uuid: this.formGroup.value.fileSystem
              })
            : {
                name: find(this.shareOptions, {
                  key: this.formGroup.value.share
                })?.name
              },
          requestParams: this.getParams(),
          mountRequestParams: this.getMountParams()
        })
      : assign({}, this.formGroup.value, {
          resource: find(this.nativeFileSystemOptions, {
            key: this.formGroup.value.fileSystem
          }),
          requestParams: this.getParams(),
          mountRequestParams: this.getMountParams()
        });
  }

  getMountParams() {
    if (this.formGroup.value.restoreLocation === RestoreV2LocationType.ORIGIN) {
      return;
    }
    const params = {
      location:
        this.formGroup.value.restoreLocation === RestoreV2LocationType.NATIVE
          ? 'LOCAL'
          : 'NEW',
      envId:
        this.formGroup.value.restoreLocation === RestoreV2LocationType.NATIVE
          ? ''
          : this.formGroup.value.equipment
    };
    const resource =
      this.formGroup.value.restoreLocation === RestoreV2LocationType.NEW
        ? find(this.fileSystemOptions, {
            uuid: this.formGroup.value.fileSystem
          }) || {
            name: find(this.shareOptions, {
              key: this.formGroup.value.share
            })?.name.replace(/\/[0-9]+\.[0-9]+\.[0-9]+\.[0-9]+/, '')
          }
        : find(this.nativeFileSystemOptions, {
            key: this.formGroup.value.fileSystem
          });
    const mountRequestParams = {
      filesystemName: resource ? resource.name : ''
    };
    if (this.formGroup.value.restoreLocation === RestoreV2LocationType.NEW) {
      if (!this.isDoradoEquipment) {
        assign(params, {
          envId: find(this.shareOptions, {
            key: this.formGroup.value.share
          }).uuid,
          targetType: 'HETEROGENEOUS'
        });
      }
      assign(mountRequestParams, {
        sharedName: this.isDoradoEquipment
          ? this.formGroup.value.share
          : find(this.shareOptions, {
              key: this.formGroup.value.share
            })?.name.replace(/\/[0-9]+\.[0-9]+\.[0-9]+\.[0-9]+/, ''),
        sharedProtocol:
          this.fileShareMode === DataMap.Shared_Mode.nfs.value ? 'NFS' : 'CIFS'
      });
      if (this.fileShareMode === DataMap.Shared_Mode.nfs.value) {
        assign(mountRequestParams, {
          nfsAuth: {
            mode:
              this.formGroup.value.auth_mode ===
              DataMap.Nas_Share_Auth_Mode.kerberos.value
                ? 'AUTH'
                : 'NO_AUTH',
            kerberos:
              this.formGroup.value.auth_mode ===
              DataMap.Nas_Share_Auth_Mode.kerberos.value
                ? this.formGroup.value.kerberos
                : ''
          },
          shareIp: this.formGroup.value.shareIp
            ? this.formGroup.value.shareIp
            : find(this.shareOptions, { key: this.formGroup.value.share })
                ?.extendInfo?.ip
        });
      } else {
        assign(mountRequestParams, {
          shareIp: this.formGroup.value.shareIp,
          cifsAuth: {
            mode: this.isDoradoEquipment
              ? toString(this.formGroup.value.auth_mode)
              : toString(
                  find(this.shareOptions, {
                    key: this.formGroup.value.share
                  })?.auth?.authType
                ),
            domainName:
              this.formGroup.value.auth_mode ===
              DataMap.Nas_Share_Auth_Mode.password.value
                ? this.formGroup.value.domain
                : '',
            username:
              this.formGroup.value.auth_mode ===
              DataMap.Nas_Share_Auth_Mode.password.value
                ? this.formGroup.value.username
                : '',
            password:
              this.formGroup.value.auth_mode ===
              DataMap.Nas_Share_Auth_Mode.password.value
                ? this.formGroup.value.password
                : '',
            kerberos:
              !this.isDoradoEquipment &&
              find(this.shareOptions, {
                key: this.formGroup.value.share
              })?.auth?.authType === DataMap.Nas_Share_Auth_Mode.kerberos.value
                ? find(this.shareOptions, {
                    key: this.formGroup.value.share
                  })?.extendInfo?.kerberosId
                : this.formGroup.value.auth_mode ===
                  DataMap.Nas_Share_Auth_Mode.kerberos.value
                ? this.formGroup.value.kerberos
                : ''
          }
        });
      }
    }
    assign(params, {
      CreateMountRequestBody: mountRequestParams
    });
    assign(params, { memberEsn: this.formGroup.value.memberEsn });

    return params;
  }

  getParams() {
    const params = {
      copyId: this.rowCopy.uuid,
      targetEnv:
        this.formGroup.value.restoreLocation === RestoreV2LocationType.NEW
          ? this.formGroup.value.equipment
          : this.isOceanProtect &&
            this.restoreType === this.restoreTypeEnum.FileRestore &&
            this.formGroup.value.restoreLocation ===
              RestoreV2LocationType.NATIVE &&
            includes(
              [
                DataMap.Resource_Type.NASFileSystem.value,
                DataMap.Resource_Type.NASShare.value
              ],
              this.rowCopy.resource_sub_type
            )
          ? 'local'
          : this.resourceObj.environment_uuid,
      restoreType: this.restoreType,
      targetLocation: this.formGroup.value.restoreLocation,
      targetObject:
        this.formGroup.value.restoreLocation !== RestoreV2LocationType.ORIGIN
          ? this.isDoradoEquipment ||
            this.formGroup.value.restoreLocation ===
              RestoreV2LocationType.NATIVE
            ? this.formGroup.value.fileSystem
            : find(this.shareOptions, { key: this.formGroup.value.share }).uuid
          : this.resourceObj.uuid,
      filters: [],
      agents: this.formGroup.value.proxyHost || []
    };

    if (this.tapeCopy && this.formGroup.get('isDirectRecovery')?.value) {
      assign(params, {
        restoreType: RestoreV2Type.FileRestore,
        subObjects: ['/']
      });
    }

    // 同构恢复新位置
    const isCopyRestoreNew =
      this.isDoradoEquipment &&
      this.formGroup.value.restoreLocation === RestoreV2LocationType.NEW;
    // NAS文件系统文集级恢复原位置
    const isNasFileSystemFileRestoreOrigin =
      (this.isNasFileSystemFileRestore || this.isCloudArchiveCopy) &&
      this.formGroup.value.restoreLocation === RestoreV2LocationType.ORIGIN;
    const extendInfo =
      isCopyRestoreNew || isNasFileSystemFileRestoreOrigin
        ? {
            protocol: this.fileShareMode,
            authType: this.formGroup.value.auth_mode,
            sharePath: this.formGroup.value.share,
            shareIp: this.formGroup.value.shareIp
          }
        : {};

    if (
      this.rowCopy.resource_sub_type === DataMap.Resource_Type.NASShare.value ||
      this.isShowChannels
    ) {
      assign(extendInfo, {
        channels: Number(this.formGroup.get('channels').value) || ''
      });
    }

    if (
      this.rowCopy.resource_sub_type === DataMap.Resource_Type.NASShare.value &&
      this.restoreType === this.restoreTypeEnum.CommonRestore
    ) {
      assign(extendInfo, {
        orderOfRestore: this.formGroup.get('restoreOrder').value
          ? this.formGroup.get('orderBy').value
          : 0
      });
    }

    if (
      this.isNdmpPath &&
      this.formGroup.value.restoreLocation === RestoreV2LocationType.NEW
    ) {
      const fileSystem = find(this.fileSystemOptions, {
        uuid: this.formGroup.value.fileSystem
      });
      assign(extendInfo, {
        fullName:
          `${fileSystem?.extendInfo?.fullName}${
            this.formGroup.get('fullName')?.value
          }` || ''
      });
    }

    if (
      this.formGroup.value.auth_mode ===
        DataMap.Nas_Share_Auth_Mode.password.value &&
      (isCopyRestoreNew || isNasFileSystemFileRestoreOrigin)
    ) {
      assign(extendInfo, {
        domainName: this.formGroup.value.domain,
        authKey: this.formGroup.value.username,
        authPwd: this.formGroup.value.password
      });
    }
    if (
      this.formGroup.value.auth_mode ===
        DataMap.Nas_Share_Auth_Mode.kerberos.value &&
      (isCopyRestoreNew || isNasFileSystemFileRestoreOrigin)
    ) {
      assign(extendInfo, {
        kerberosId: this.formGroup.value.kerberos
      });
    }
    if (
      (!this.isDoradoEquipment &&
        this.formGroup.value.restoreLocation !==
          RestoreV2LocationType.ORIGIN) ||
      this.rowCopy.resource_sub_type === DataMap.Resource_Type.NASShare.value ||
      this.restoreType === RestoreV2Type.FileRestore ||
      this.isCloudArchiveCopy ||
      this.formGroup.value.restoreLocation == RestoreV2LocationType.NEW
    ) {
      assign(extendInfo, {
        fileReplaceStrategy: trim(this.formGroup.value.originalType)
      });
    }
    if (
      this.restoreType === RestoreV2Type.FileRestore &&
      this.formGroup.value.restoreLocation === RestoreV2LocationType.NATIVE
    ) {
      assign(extendInfo, {
        fileSystemId: this.formGroup.value.fileSystem,
        protocol: this.resourceShareMode
      });
      return assign(params, { extendInfo });
    }
    return this.formGroup.value.restoreLocation === RestoreV2LocationType.NEW ||
      isNasFileSystemFileRestoreOrigin
      ? assign(params, { extendInfo })
      : this.rowCopy.resource_sub_type ===
          DataMap.Resource_Type.NASShare.value ||
        this.restoreType === RestoreV2Type.FileRestore
      ? assign(params, {
          extendInfo: pick(extendInfo, [
            'fileReplaceStrategy',
            'channels',
            'orderOfRestore'
          ])
        })
      : assign(params, { extendInfo: pick(extendInfo, ['channels']) });
  }

  restore(): Observable<void> {
    return new Observable<void>((observer: Observer<void>) => {
      const params = this.getParams();
      this.restoreV2Service
        .CreateRestoreTask({ CreateRestoreTaskRequestBody: params })
        .subscribe({
          next: res => {
            observer.next();
            observer.complete();
          },
          error: err => {
            observer.error(err);
            observer.complete();
          }
        });
    });
  }
}
