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
import { Component, OnDestroy, OnInit } from '@angular/core';
import {
  AbstractControl,
  FormBuilder,
  FormControl,
  FormGroup,
  ValidatorFn
} from '@angular/forms';
import { MessageService, OptionItem } from '@iux/live';
import {
  AgentsSubType,
  BaseUtilService,
  ClientManagerApiService,
  CommonConsts,
  DataMap,
  DataMapService,
  GlobalService,
  I18NService,
  PermissonsAttributes,
  ProtectedResourceApiService,
  ProxyHostSelectMode
} from 'app/shared';
import { AppUtilsService } from 'app/shared/services/app-utils.service';
import {
  each,
  filter,
  find,
  includes,
  intersection,
  isArray,
  isEmpty,
  isNumber,
  isUndefined,
  map,
  set,
  size,
  toNumber,
  trim
} from 'lodash';
import { Subject, Subscription } from 'rxjs';

@Component({
  selector: 'aui-advanced-parameter',
  templateUrl: './advanced-parameter.component.html',
  styleUrls: ['./advanced-parameter.component.less']
})
export class AdvancedParameterComponent implements OnInit, OnDestroy {
  includes = includes;
  formGroup: FormGroup;
  resourceData;
  subResourceType;
  dataMap = DataMap;
  permissonsAttributes = PermissonsAttributes;
  proxyHostSelectMode = ProxyHostSelectMode;
  shareModeOptions = this.dataMapService.toArray('Shared_Mode').filter(item => {
    return (item.isLeaf = true);
  });
  proxyHostOptions = [];
  protocolOptions = [];
  poxyOptions = [];
  doradoNasFiles;
  valid$ = new Subject<boolean>();
  isModified = false;
  disableSmallFile = false;
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
  isX3000 = this.i18n.get('deploy_type') === DataMap.Deploy_Type.x3000.value;
  hostOptions = [];
  isAgentExternal = false;
  externalAgentLists = [];
  extParams;
  hasRansomware = false; // 用于判断是否有已创建的防勒索策略
  ransomwareStatus$: Subscription = new Subscription();

  channelsErrorTip = {
    ...this.baseUtilService.rangeErrorTip,
    invalidRang: this.i18n.get('common_valid_rang_label', [1, 40])
  };
  tipsLabel = this.i18n.get('protection_nasshare_channels_tips_label');

  unitOptions: OptionItem[] = [
    {
      label: this.i18n.get('common_minutes_label'),
      value: 60,
      isLeaf: true
    },
    {
      label: this.i18n.get('common_hours_label'),
      value: 60 * 60,
      isLeaf: true
    },
    {
      label: this.i18n.get('common_days_label'),
      value: 24 * 60 * 60,
      isLeaf: true
    },
    {
      label: this.i18n.get('common_months_label'),
      value: 30 * 24 * 60 * 60,
      isLeaf: true
    },
    {
      label: this.i18n.get('common_years_label'),
      value: 365 * 24 * 60 * 60,
      isLeaf: true
    }
  ];

  isDetail = false;
  protectData;
  protectDataHosts: string;
  protectDataPath: string;

  isStorageUnity = false;

  hotDataErrorTip = {
    ...this.baseUtilService.requiredErrorTip,
    ...this.baseUtilService.integerErrorTip,
    invalidMinSize: this.i18n.get('common_valid_minsize_label', [0])
  };
  filterErrorTip = {
    invalidFilter: this.i18n.get('common_incorrect_format_label')
  };

  constructor(
    private fb: FormBuilder,
    private i18n: I18NService,
    private globalService: GlobalService,
    private dataMapService: DataMapService,
    public baseUtilService: BaseUtilService,
    private messageService: MessageService,
    private protectedResourceApiService: ProtectedResourceApiService,
    private clientManagerApiService: ClientManagerApiService,
    public appUtilsService: AppUtilsService
  ) {}

  ngOnInit() {
    if (this.isX3000) {
      this.tipsLabel =
        this.tipsLabel + this.i18n.get('protection_x3000_channels_tips_label');
    }
    this.initForm();
    this.getProtocol();
    this.getProxyOptions();
    this.getRansomwareStatus();
  }

  ngOnDestroy() {
    this.ransomwareStatus$.unsubscribe();
  }

  initDetailData(data) {
    if (data.protectionStatus === DataMap.Protection_Status.protected.value) {
      this.isDetail = true;
      this.protectData = data.protectedObject?.extParameters;
      this.resourceData = data;
      this.subResourceType = data.subType;
      this.protectDataPath = this.protectData.filters.join(',');
      this.isStorageUnity =
        data.environment?.extendInfo?.ndmpType === '3' &&
        data.extendInfo?.isFs === '0';
    }
  }

  getProtocol() {
    if (this.isDetail) {
      return;
    }
    const extParameters = this.resourceData.protectedObject?.extParameters;
    if (
      this.subResourceType === DataMap.Resource_Type.ndmp.value &&
      isArray(extParameters?.filters)
    ) {
      this.formGroup
        .get('filterFile')
        ?.setValue(extParameters.filters.join(','));
    }
    if (
      !includes(
        [DataMap.Resource_Type.NASFileSystem.value],
        this.subResourceType
      )
    ) {
      return;
    }
    const allProtocol = [];
    each(this.doradoNasFiles, item => {
      if (item.protocol === DataMap.NasFileSystem_Protocol.nfs_cifs.value) {
        allProtocol.push([
          DataMap.Shared_Mode.nfs.value,
          DataMap.Shared_Mode.cifs.value
        ]);
      } else if (item.protocol === DataMap.NasFileSystem_Protocol.ndmp.value) {
        allProtocol.push([DataMap.NasFileSystem_Protocol.ndmp.value]);
      } else {
        allProtocol.push([trim(item.protocol)]);
      }
    });
    const intersectionProtocol = intersection.apply(null, allProtocol);
    this.protocolOptions = this.dataMapService
      .toArray('NasFileSystem_Protocol')
      .filter(item => {
        if (includes(intersectionProtocol, item.value)) {
          return (item.isLeaf = true);
        }
      });
    if (extParameters) {
      this.formGroup.get('protocol').setValue(extParameters.protocol + '');
    }
  }

  getProxyOptions(recordsTemp?, startPage?) {
    if (
      !this.exterAgent ||
      !includes(
        [
          DataMap.Resource_Type.NASFileSystem.value,
          DataMap.Resource_Type.ndmp.value
        ],
        this.subResourceType
      )
    ) {
      return;
    }
    const params = {
      pageNo: startPage || CommonConsts.PAGE_START,
      pageSize: 200,
      conditions: JSON.stringify({
        pluginType:
          this.subResourceType === DataMap.Resource_Type.NASFileSystem.value
            ? AgentsSubType.NasFileSystem
            : AgentsSubType.Ndmp
      })
    };
    this.clientManagerApiService
      .queryAgentListInfoUsingGET(params)
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
          recordsTemp = filter(recordsTemp, item => {
            return (
              item.linkStatus ===
                DataMap.resource_LinkStatus_Special.normal.value ||
              includes(
                this.resourceData.protectedObject?.extParameters?.agents,
                item.rootUuid
              )
            );
          });
          const hostArray = [];
          each(recordsTemp, item => {
            hostArray.push({
              ...item,
              key: item.uuid,
              label: `${item.name}(${item.endpoint})`,
              value: item.rootUuid || item.parentUuid,
              isLeaf: true
            });
          });
          this.hostOptions = hostArray;
          each(hostArray, item => {
            if (
              item.extendInfo.scenario === DataMap.proxyHostType.builtin.value
            ) {
              this.poxyOptions.push(item.rootUuid || item.parentUuid);
            }
          });
          this.formGroup.get('proxyHost').setValue(this.poxyOptions);
          if (
            !isEmpty(this.resourceData.protectedObject) &&
            !isEmpty(this.resourceData.protectedObject.extParameters)
          ) {
            this.formGroup
              .get('proxyHost')
              .setValue(
                this.resourceData.protectedObject.extParameters.agents?.split(
                  ';'
                ) || []
              );
          }
          if (this.isDetail && this.protectData.agents) {
            this.protectDataHosts = map(
              this.protectData.agents.split(';'),
              agent => {
                return find(this.hostOptions, host => host.uuid === agent)
                  ?.endpoint;
              }
            ).join(',');
          }
          return;
        }
        this.getProxyOptions(recordsTemp, startPage);
      });
  }

  getRansomwareStatus() {
    // 开启了防勒索策略的资源是不能开小文件聚合的
    this.ransomwareStatus$ = this.globalService
      .getState('syncRansomwareStatus')
      .subscribe(res => {
        this.hasRansomware = res;
      });
  }

  splitFilter(value: string): string[] {
    const filters = [];
    let startIndex = 0;
    for (let i = 0; i <= value.length; i++) {
      if ((value[i] === ',' && value[i - 1] !== '\\') || i === value.length) {
        filters.push(value.slice(startIndex, i));
        startIndex = i + 1;
      }
    }
    return filters;
  }

  validStar(value: string) {
    for (var i = 0; i < value.length; i++) {
      if (value[i] === '*' && i !== 0 && i !== value.length - 1) {
        return false;
      }
    }
    return true;
  }

  validFilter(): ValidatorFn {
    return (control: AbstractControl): { [key: string]: any } | null => {
      if (isUndefined(this.formGroup)) {
        return null;
      }

      const values = this.splitFilter(control.value).filter(item => {
        return !isEmpty(item);
      });
      // 排除列表不能超过32
      if (size(values) > 32) {
        return {
          invalidFilter: {
            value: control.value
          }
        };
      }
      // 每个条件不能超过255个字符
      for (let i = 0; i < size(values); i++) {
        const value = values[i];
        if (value.length > 255 || includes(value, '/')) {
          return { invalidFilter: { value: control.value } };
        }
        if (!this.validStar(value)) {
          return {
            invalidFilter: {
              value: control.value
            }
          };
        }
      }
      return null;
    };
  }

  initForm() {
    this.formGroup = this.fb.group({
      channels: new FormControl(10, {
        validators: [
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.integer(),
          this.baseUtilService.VALID.rangeValue(1, 40)
        ]
      }),
      aggregation_mode: new FormControl(DataMap.Aggregation_Mode.disable.value),
      permissons_attributes: new FormControl(PermissonsAttributes.FolderOnly),
      protocol: new FormControl(''),
      sparseFileDetect: new FormControl(false),
      smallFile: new FormControl(false),
      fileSize: new FormControl(4096),
      maxFileSize: new FormControl(1024),
      smbHardlinkProtection: new FormControl(true),
      smbAclProtection: new FormControl(true),
      enableHotData: new FormControl(false),
      enableColdData: new FormControl(false),
      hotData: new FormControl(''),
      unit: new FormControl(60),
      coldData: new FormControl(''),
      coldUnit: new FormControl(60)
    });
    // NDMP过滤文件
    if (this.subResourceType === DataMap.Resource_Type.ndmp.value) {
      this.formGroup.addControl(
        'filterFile',
        new FormControl('', { validators: [this.validFilter()] })
      );
    }
    this.formGroup.get('enableHotData').valueChanges.subscribe(res => {
      if (res) {
        this.formGroup
          .get('hotData')
          .setValidators([
            this.baseUtilService.VALID.required(),
            this.baseUtilService.VALID.integer(),
            this.baseUtilService.VALID.minSize(0)
          ]);
      } else {
        this.formGroup.get('hotData').clearValidators();
      }
      this.formGroup.get('hotData').updateValueAndValidity();
    });
    this.formGroup.get('enableColdData').valueChanges.subscribe(res => {
      if (res) {
        this.formGroup
          .get('coldData')
          .setValidators([
            this.baseUtilService.VALID.required(),
            this.baseUtilService.VALID.integer(),
            this.baseUtilService.VALID.minSize(0)
          ]);
      } else {
        this.formGroup.get('coldData').clearValidators();
      }
      this.formGroup.get('coldData').updateValueAndValidity();
    });

    // X3000、X6000、X8000增加代理主机
    if (
      this.exterAgent &&
      includes(
        [
          DataMap.Resource_Type.NASFileSystem.value,
          DataMap.Resource_Type.ndmp.value
        ],
        this.subResourceType
      )
    ) {
      this.formGroup.addControl(
        'proxyHost',
        new FormControl([], {
          validators: [this.baseUtilService.VALID.required()]
        })
      );
      this.formGroup.get('proxyHost').valueChanges.subscribe(res => {
        this.isAgentExternal = false;
        this.externalAgentLists = [];
        each(res, item => {
          const proxyData = find(this.hostOptions, val => val.uuid === item);
          if (
            proxyData.extendInfo.scenario === '0' &&
            !includes([DataMap.Resource_Type.ndmp.value], this.subResourceType)
          ) {
            this.isAgentExternal = true;
            this.externalAgentLists.push(`${proxyData.label}`);
          }
        });
      });
    }
    if (
      includes(
        [DataMap.Resource_Type.NASFileSystem.value],
        this.subResourceType
      )
    ) {
      this.formGroup
        .get('protocol')
        .setValidators([this.baseUtilService.VALID.required()]);
      this.formGroup.get('protocol').updateValueAndValidity();
    }
    this.formGroup.get('aggregation_mode').valueChanges.subscribe(res => {
      if (res === DataMap.Aggregation_Mode.enable.value) {
        this.formGroup
          .get('permissons_attributes')
          .setValue(PermissonsAttributes.FolderOnly);
      }
    });
    this.patchForm();

    this.formGroup.get('fileSize').valueChanges.subscribe(res => {
      if (
        this.formGroup.get('fileSize').value <
        this.formGroup.get('maxFileSize').value
      ) {
        this.messageService.error(
          this.i18n.get('protection_small_file_size_tips_label')
        );
      }
    });

    this.formGroup.get('maxFileSize').valueChanges.subscribe(res => {
      if (
        this.formGroup.get('fileSize').value <
        this.formGroup.get('maxFileSize').value
      ) {
        this.messageService.error(
          this.i18n.get('protection_small_file_size_tips_label')
        );
      }
    });
    this.formGroup.statusChanges.subscribe(res => {
      this.valid$.next(
        this.formGroup.valid &&
          this.formGroup.get('fileSize').value >=
            this.formGroup.get('maxFileSize').value
      );
    });
  }

  patchForm() {
    if (
      this.isDetail ||
      isEmpty(this.resourceData.protectedObject) ||
      isEmpty(this.resourceData.protectedObject.extParameters)
    ) {
      return;
    }
    const extParameters = this.resourceData.protectedObject.extParameters;
    if (this.subResourceType === DataMap.Resource_Type.NASShare.value) {
      this.isModified = true;
      this.formGroup
        .get('aggregation_mode')
        .setValue(extParameters.small_file_aggregation);
      this.formGroup
        .get('permissons_attributes')
        .setValue(extParameters.permissions_and_attributes);

      this.formGroup.patchValue({
        sparseFileDetect: extParameters.sparse_file_detection,
        smallFile:
          extParameters?.small_file_aggregation ===
          DataMap.Aggregation_Mode.enable.value,
        fileSize: extParameters?.aggregation_file_size || 4096,
        maxFileSize: extParameters?.aggregation_file_max_size || 1024,
        smbHardlinkProtection: extParameters?.smb_hardlink_protection ?? true,
        smbAclProtection: extParameters?.smb_acl_protection ?? true,
        channels: extParameters?.channels || 10,
        enableHotData:
          extParameters?.backup_hot_data && extParameters?.unit ? true : false,
        enableColdData:
          extParameters?.backup_cold_data && extParameters?.coldUnit
            ? true
            : false,
        hotData:
          extParameters?.backup_hot_data && extParameters?.unit
            ? extParameters?.backup_hot_data / extParameters?.unit
            : '',
        coldData:
          extParameters?.backup_cold_data && extParameters?.coldUnit
            ? extParameters?.backup_cold_data / extParameters?.coldUnit
            : '',
        unit: extParameters?.unit || 60,
        coldUnit: extParameters?.coldUnit || 60
      });
    }
    // 修改索引
    this.extParams = extParameters;
  }

  initData(data: any, resourceType: string) {
    this.resourceData = isArray(data) ? data[0] : data;
    this.subResourceType = this.resourceData.sub_type;
    this.doradoNasFiles = isArray(data) ? data : [data];
    this.isStorageUnity =
      this.resourceData.environment?.extendInfo?.ndmpType === '3' &&
      this.resourceData.extendInfo?.isFs === '0';
  }

  onOK() {
    const ext_parameters = includes(
      [
        DataMap.Resource_Type.NASFileSystem.value,
        DataMap.Resource_Type.ndmp.value
      ],
      this.subResourceType
    )
      ? {
          proxy_host_mode: ProxyHostSelectMode.Auto,
          protocol:
            this.subResourceType === DataMap.Resource_Type.ndmp.value
              ? Number(DataMap.NasFileSystem_Protocol.ndmp.value)
              : +this.formGroup.value.protocol
        }
      : {
          channels: toNumber(this.formGroup.get('channels').value),
          sparse_file_detection: this.formGroup.get('sparseFileDetect').value,
          small_file_aggregation: this.formGroup.value.smallFile
            ? DataMap.Aggregation_Mode.enable.value
            : DataMap.Aggregation_Mode.disable.value,
          smb_hardlink_protection: this.formGroup.value.smbHardlinkProtection,
          smb_acl_protection: this.formGroup.value.smbAclProtection,
          backup_hot_data:
            this.formGroup.value.enableHotData && this.formGroup.value.hotData
              ? this.formGroup.value.hotData * this.formGroup.value.unit
              : 0,
          unit: this.formGroup.value.unit,
          backup_cold_data:
            this.formGroup.value.enableColdData && this.formGroup.value.coldData
              ? this.formGroup.value.coldData * this.formGroup.value.coldUnit
              : 0,
          coldUnit: this.formGroup.value.coldUnit
        };
    if (
      includes(
        [
          DataMap.Resource_Type.NASFileSystem.value,
          DataMap.Resource_Type.ndmp.value
        ],
        this.subResourceType
      ) &&
      this.exterAgent
    ) {
      set(ext_parameters, 'agents', this.formGroup.value.proxyHost?.join(';'));
      set(ext_parameters, 'proxy_host_mode', ProxyHostSelectMode.Manual);
    }
    if (this.formGroup.get('smallFile').value) {
      set(
        ext_parameters,
        'aggregation_file_size',
        this.formGroup.get('fileSize').value
      );
      set(
        ext_parameters,
        'aggregation_file_max_size',
        this.formGroup.get('maxFileSize').value
      );
    }

    // NDMP过滤文件
    if (this.subResourceType === DataMap.Resource_Type.ndmp.value) {
      set(
        ext_parameters,
        'filters',
        filter(
          this.splitFilter(this.formGroup.get('filterFile').value),
          item => !isEmpty(item)
        )
      );
    }

    // 索引
    if (
      includes(
        [
          DataMap.Resource_Type.NASFileSystem.value,
          DataMap.Resource_Type.NASShare.value,
          DataMap.Resource_Type.ndmp.value
        ],
        this.subResourceType
      )
    ) {
      each(
        [
          'backup_res_auto_index',
          'archive_res_auto_index',
          'tape_archive_auto_index',
          'enable_security_archive'
        ],
        key => {
          if (this.formGroup.get(key)) {
            set(ext_parameters, key, this.formGroup.get(key).value);
          }
        }
      );
    }

    return {
      ext_parameters
    };
  }
}
