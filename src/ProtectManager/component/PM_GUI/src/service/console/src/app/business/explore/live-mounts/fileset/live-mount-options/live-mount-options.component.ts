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
import { Component, EventEmitter, Input, OnInit, Output } from '@angular/core';
import {
  AbstractControl,
  FormBuilder,
  FormControl,
  FormGroup,
  ValidatorFn
} from '@angular/forms';
import {
  ApiStorageBackupPluginService,
  BaseUtilService,
  CommonConsts,
  CookieService,
  DatabasesService,
  DataMap,
  DataMapService,
  GlobalService,
  I18NService,
  isRBACDPAdmin,
  LiveMountAction,
  MountTargetLocation,
  ProtectedEnvironmentApiService,
  ProtectedResourceApiService,
  RestoreFileType,
  RoleType
} from 'app/shared';
import {
  assign,
  cloneDeep,
  defer,
  each,
  filter,
  find,
  first,
  get,
  includes,
  isArray,
  isEmpty,
  isNumber,
  isUndefined,
  last,
  map,
  omit,
  reject,
  replace,
  size,
  split,
  startsWith,
  toString,
  trim
} from 'lodash';
import { pairwise } from 'rxjs/operators';
@Component({
  selector: 'aui-live-mount-fileset-options',
  templateUrl: './live-mount-options.component.html',
  styleUrls: ['./live-mount-options.component.less']
})
export class LiveMountOptionsComponent implements OnInit {
  posOptions;
  filePathData;
  offlineWarnTip;
  dataMap = DataMap;
  restoreFileType = RestoreFileType;
  formGroup: FormGroup;
  targetHostOptions = [];
  userOptions = [];
  isWindows = false;
  nameErrorTip = {
    ...this.baseUtilService.requiredErrorTip,
    invalidName: this.i18n.get('protection_nas_filesystem_valid_label')
  };
  userErrorTip = {
    ...this.baseUtilService.requiredErrorTip,
    nameFormat: this.i18n.get('protection_fileset_user_valid_label'),
    invalidMinLength: this.i18n.get('common_valid_length_rang_label', [3, 20]),
    invalidMaxLength: this.i18n.get('common_valid_length_rang_label', [3, 20])
  };
  pwdErrorTip = {
    ...this.baseUtilService.requiredErrorTip,
    invalidMinLength: this.i18n.get('common_valid_length_rang_label', [8, 32]),
    invalidMaxLength: this.i18n.get('common_valid_length_rang_label', [8, 32]),
    invalidWord: this.i18n.get('protection_fileset_password_word_valid_label'),
    invalidSameWord: this.i18n.get(
      'protection_fileset_password_same_word_valid_label'
    ),
    invalidSameUser: this.i18n.get(
      'protection_fileset_password_same_user_valid_label'
    )
  };
  namePrefix = 'mount_';
  disableFileSystemName = false;
  userTypeOptions = this.dataMapService
    .toArray('Cifs_Domain_Client_Type')
    .filter(v => {
      v.isLeaf = true;
      return includes(
        [
          DataMap.Cifs_Domain_Client_Type.everyone.value,
          DataMap.Cifs_Domain_Client_Type.windows.value,
          DataMap.Cifs_Domain_Client_Type.windowsGroup.value
        ],
        v.value
      );
    });
  hostOptions = [];
  hostOptionsCache = [];
  latencyOptions = this.dataMapService
    .toArray('LiveMount_Latency')
    .filter(v => (v.isLeaf = true));
  disabledTips = this.i18n.get('protection_fileset_sys_dir_tips_label');
  createFileSystem = false;
  authHosts = [];
  isDataProtectionAdmin = isRBACDPAdmin(this.cookieService.role);
  isFileset;
  volumePos = '/mnt/databackup';
  volumeTips = this.i18n.get('protection_fileset_livemount_dir_tips_label');
  // OP服务化环境
  isHcsUser = this.cookieService.get('userType') === CommonConsts.HCS_USER_TYPE;

  @Input() activeIndex;
  @Input() componentData;
  @Output() selectMountOptionChange = new EventEmitter<any>();
  minBandwidthErrorTip = assign({}, this.baseUtilService.rangeErrorTip, {
    invalidRang: this.i18n.get('common_valid_rang_label', [1, 999999999]),
    invalidMin: this.i18n.get('explore_min_max_valid_label')
  });
  maxBandwidthErrorTip = assign({}, this.baseUtilService.rangeErrorTip, {
    invalidRang: this.i18n.get('common_valid_rang_label', [1, 999999999]),
    invalidMax: this.i18n.get('explore_max_min_valid_label')
  });
  burstBandwidthErrorTip = assign({}, this.baseUtilService.rangeErrorTip, {
    invalidRang: this.i18n.get('common_valid_rang_label', [1, 999999999]),
    invalidBurst: this.i18n.get('explore_burst_valid_label')
  });
  minIopsErrorTip = assign({}, this.baseUtilService.rangeErrorTip, {
    invalidRang: this.i18n.get('common_valid_rang_label', [100, 999999999]),
    invalidMin: this.i18n.get('explore_min_max_valid_label')
  });
  maxIopsErrorTip = assign({}, this.baseUtilService.rangeErrorTip, {
    invalidRang: this.i18n.get('common_valid_rang_label', [100, 999999999]),
    invalidMax: this.i18n.get('explore_max_min_valid_label')
  });
  burstIopsErrorTip = assign({}, this.baseUtilService.rangeErrorTip, {
    invalidRang: this.i18n.get('common_valid_rang_label', [100, 999999999]),
    invalidBurst: this.i18n.get('explore_burst_valid_label')
  });
  burstTimeErrorTip = assign({}, this.baseUtilService.rangeErrorTip, {
    invalidRang: this.i18n.get('common_valid_rang_label', [1, 999999999])
  });
  targetHostErrorTip = assign({}, this.baseUtilService.rangeErrorTip, {
    invalidSameDb: this.i18n.get('protection_target_host_oneline_db_label'),
    invalidSameHost: this.i18n.get('explore_target_same_host_label'),
    invalidHaveDb: this.i18n.get('protection_target_host_install_db_label')
  });
  scriptNameErrorTip = assign({}, this.baseUtilService.scriptNameErrorTip, {
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [8192])
  });
  targetIpLabel = this.i18n.get('common_ip_address_label', [], true);
  versionLabel = this.i18n.get('protection_database_version_label', [], true);

  iopsItems = [
    {
      id: 850000,
      header: '16KB'
    },
    {
      id: 680000,
      header: '32KB'
    },
    {
      id: 450000,
      header: '64KB'
    },
    {
      id: 225000,
      header: '128KB'
    },
    {
      id: 112500,
      header: '256KB'
    },
    {
      id: 56250,
      header: '512KB'
    },
    {
      id: 28125,
      header: '1024KB'
    },
    {
      id: 14063,
      header: '2048KB'
    }
  ];
  latencyItems = [
    {
      id: 850000,
      header: '16KB'
    },
    {
      id: 680000,
      header: '32KB'
    },
    {
      id: 450000,
      header: '64KB'
    },
    {
      id: 225000,
      header: '128KB'
    },
    {
      id: 112500,
      header: '256KB'
    },
    {
      id: 56250,
      header: '512KB'
    },
    {
      id: 28125,
      header: '1024KB'
    },
    {
      id: 14063,
      header: '2048KB'
    }
  ];

  constructor(
    private i18n: I18NService,
    private fb: FormBuilder,
    public baseUtilService: BaseUtilService,
    private globalService: GlobalService,
    private dataMapService: DataMapService,
    private protectedResourceApiService: ProtectedResourceApiService,
    private databasesService: DatabasesService,
    private apiStorageBackupPluginService: ApiStorageBackupPluginService,
    private protectedEnvironmentApiService: ProtectedEnvironmentApiService,
    private cookieService: CookieService
  ) {}

  ngOnInit() {
    this.isFileset =
      this.componentData?.selectionResource?.sub_type ===
        DataMap.Resource_Type.fileset.value ||
      (!!this.componentData?.childResourceType &&
        this.componentData?.childResourceType[0] ===
          DataMap.Resource_Type.fileset.value);
    this.initForm();
    this.getHosts();
    if (this.componentData) {
      this.isWindows =
        this.componentData.selectionResource.environment_os_type ===
        DataMap.Os_Type.windows.value;
    }
  }

  updateIopsItems(value, type: 'min' | 'max' | 'burst') {
    each(this.iopsItems, item => {
      const obj = {};
      obj[type] =
        isNaN(+value) || !trim(value)
          ? '--'
          : Math.round((item.id * value) / 1000000);
      assign(item, obj);
    });
  }

  updateLatencyData(value) {
    each(this.latencyItems, item => {
      const max = isNaN(+value)
        ? '--'
        : Math.round((1000000 * value) / item.id) / 1000 + 'ms';
      assign(item, { max });
    });
  }

  // 用户名称校验
  nameFormatValidator(): ValidatorFn {
    return (control: AbstractControl): { [key: string]: any } | null => {
      if (!control.value) {
        return null;
      }
      const rule = /[\"\/\\\]\[\:\;\|\=\,\+\*\?\<\>\@\s\x00-\x1F\x7F-\x9F]+/;
      const value: string = control.value;

      // 1、不能为"/\][:;|=,+*?<>@、空格以及控制字符，可以为中文等字符
      if (rule.test(value)) {
        return { nameFormat: true };
      }

      // 2、不能以.结尾
      if (value.endsWith('.')) {
        return { nameFormat: true };
      }
      return null;
    };
  }

  // 密码复杂度校验
  passwordFormatValidator(): ValidatorFn {
    return (control: AbstractControl): { [key: string]: any } | null => {
      if (!control.value) {
        return null;
      }
      const specialWordReg = /[`|~\!@#\$%\^&\*\(\)\-_=+\\[\{\}\]\;:\'\"\,\<\.\>\/\?\u0020]+/;
      const regUpWord = /[A-Z]+/;
      const regDownWord = /[a-z]+/;
      const regNumber = /[0-9]+/;
      let weight = 0;
      const value: string = control.value;

      // 1、大写字母
      if (regUpWord.test(value)) {
        weight += 1;
      }

      // 2、小写字母
      if (regDownWord.test(value)) {
        weight += 1;
      }

      // 3、数字
      if (regNumber.test(value)) {
        weight += 1;
      }

      // 特殊字符
      if (specialWordReg.test(value)) {
        weight += 1;
      }

      // 字符复杂度不够
      if (weight < 2) {
        return { invalidWord: { value: control.value } };
      }

      // 连续3个相同字符
      if (new RegExp('(.)\\1{3}').test(value)) {
        return { invalidSameWord: { value: control.value } };
      }

      // 密码和用户名相同
      const userName = this.formGroup.value.customUserName || '';
      const _reverseName = userName
        .split('')
        .reverse()
        .join('');
      if (userName && (value === userName || value === _reverseName)) {
        return { invalidSameUser: { value: control.value } };
      }

      return null;
    };
  }

  initForm() {
    this.formGroup = this.fb.group({
      targetPos: new FormControl('', {
        validators: [this.baseUtilService.VALID.required()]
      }),
      metadataPath: new FormControl(this.isFileset ? '' : this.volumePos),
      bindWidthStatus: new FormControl(false),
      iopsStatus: new FormControl(false),
      latencyStatus: new FormControl(false),
      power_on: new FormControl(true),
      min_bandwidth: new FormControl(''),
      max_bandwidth: new FormControl(''),
      burst_bandwidth: new FormControl(''),
      min_iops: new FormControl(''),
      max_iops: new FormControl(''),
      burst_iops: new FormControl(''),
      burst_time: new FormControl(''),
      latency: new FormControl(''),
      name: new FormControl(
        { value: '', disabled: true },
        {
          validators: [
            this.baseUtilService.VALID.required(),
            this.baseUtilService.VALID.name(
              CommonConsts.REGEX.nasFileSystemName
            ),
            this.baseUtilService.VALID.maxLength(255)
          ]
        }
      ),
      userType: new FormControl(
        this.isHcsUser && this.isFileset
          ? DataMap.Cifs_Domain_Client_Type.windows.value
          : DataMap.Cifs_Domain_Client_Type.everyone.value
      ),
      userName: new FormControl([])
    });
    this.formGroup
      .get('name')
      .setValue(`${this.namePrefix}${new Date().getTime()}`);
    this.formGroup.get('bindWidthStatus').valueChanges.subscribe(res => {
      if (!res) {
        this.formGroup.get('min_bandwidth').clearValidators();
        this.formGroup.get('max_bandwidth').clearValidators();
        if (this.formGroup.get('burst_bandwidth')) {
          this.formGroup.get('burst_bandwidth').clearValidators();
        }
        if (
          !(
            (this.formGroup.value.iopsStatus &&
              this.formGroup.value.burst_iops) ||
            (this.formGroup.value.bindWidthStatus &&
              this.formGroup.value.burst_bandwidth)
          )
        ) {
          this.formGroup.get('burst_time').clearValidators();
        }
      } else {
        this.formGroup
          .get('min_bandwidth')
          .setValidators([
            this.baseUtilService.VALID.required(),
            this.baseUtilService.VALID.integer(),
            this.baseUtilService.VALID.rangeValue(1, 999999999),
            this.validMinBandWidth()
          ]);
        this.formGroup
          .get('max_bandwidth')
          .setValidators([
            this.baseUtilService.VALID.required(),
            this.baseUtilService.VALID.integer(),
            this.baseUtilService.VALID.rangeValue(1, 999999999),
            this.validMaxBandWidth()
          ]);
        this.updateMinAndMaxBandwidthValidity('min_bandwidth');
        if (this.formGroup.get('burst_bandwidth')) {
          this.formGroup
            .get('burst_bandwidth')
            .setValidators([
              this.baseUtilService.VALID.integer(),
              this.baseUtilService.VALID.rangeValue(1, 999999999),
              this.validBurstBandWidth()
            ]);
          this.formGroup.get('burst_bandwidth').updateValueAndValidity();
        }

        if (
          (this.formGroup.value.iopsStatus &&
            this.formGroup.value.burst_iops) ||
          (this.formGroup.value.bindWidthStatus &&
            this.formGroup.value.burst_bandwidth)
        ) {
          this.formGroup
            .get('burst_time')
            .setValidators([
              this.baseUtilService.VALID.required(),
              this.baseUtilService.VALID.integer(),
              this.baseUtilService.VALID.rangeValue(1, 999999999)
            ]);
        }
      }
      this.formGroup.get('burst_time').updateValueAndValidity();
      this.formGroup.get('min_bandwidth').updateValueAndValidity();
      this.formGroup.get('max_bandwidth').updateValueAndValidity();
    });
    this.formGroup.get('iopsStatus').valueChanges.subscribe(res => {
      if (!res) {
        this.formGroup.get('min_iops').clearValidators();
        this.formGroup.get('max_iops').clearValidators();
        if (this.formGroup.get('burst_iops')) {
          this.formGroup.get('burst_iops').clearValidators();
        }
        if (
          !(
            (this.formGroup.value.iopsStatus &&
              this.formGroup.value.burst_iops) ||
            (this.formGroup.value.bindWidthStatus &&
              this.formGroup.value.burst_bandwidth)
          )
        ) {
          this.formGroup.get('burst_time').clearValidators();
        }
      } else {
        this.formGroup
          .get('min_iops')
          .setValidators([
            this.baseUtilService.VALID.required(),
            this.baseUtilService.VALID.integer(),
            this.baseUtilService.VALID.rangeValue(100, 999999999),
            this.validMinIops()
          ]);
        this.formGroup
          .get('max_iops')
          .setValidators([
            this.baseUtilService.VALID.required(),
            this.baseUtilService.VALID.integer(),
            this.baseUtilService.VALID.rangeValue(100, 999999999),
            this.validMaxIops()
          ]);
        this.updateMinAndMaxIopsValidity('min_iops');
        if (this.formGroup.get('burst_iops')) {
          this.formGroup
            .get('burst_iops')
            .setValidators([
              this.baseUtilService.VALID.integer(),
              this.baseUtilService.VALID.rangeValue(100, 999999999),
              this.validBurstIops()
            ]);
          this.formGroup.get('burst_iops').updateValueAndValidity();
        }

        if (
          (this.formGroup.value.iopsStatus &&
            this.formGroup.value.burst_iops) ||
          (this.formGroup.value.bindWidthStatus &&
            this.formGroup.value.burst_bandwidth)
        ) {
          this.formGroup.get('burst_time').setValidators([
            //this.baseUtilService.VALID.required(),
            this.baseUtilService.VALID.integer(),
            this.baseUtilService.VALID.rangeValue(1, 999999999)
          ]);
        }
      }
      this.formGroup.get('burst_time').updateValueAndValidity();
      this.formGroup.get('min_iops').updateValueAndValidity();
      this.formGroup.get('max_iops').updateValueAndValidity();
    });
    this.formGroup.get('latencyStatus').valueChanges.subscribe(res => {
      if (!res) {
        this.formGroup.get('latency').clearValidators();
      } else {
        this.formGroup
          .get('latency')
          .setValidators([this.baseUtilService.VALID.required()]);
      }
      this.formGroup.get('latency').updateValueAndValidity();
    });
    // 服务化场景需要用户填写名称、密码
    if (this.isHcsUser && this.isFileset) {
      this.userTypeOptions = filter(this.userTypeOptions, item =>
        includes([DataMap.Cifs_Domain_Client_Type.windows.value], item.value)
      );
      this.formGroup.addControl(
        'customUserName',
        new FormControl('', {
          validators: [
            this.baseUtilService.VALID.required(),
            this.baseUtilService.VALID.minLength(3),
            this.baseUtilService.VALID.maxLength(20),
            this.nameFormatValidator()
          ]
        })
      );
      this.formGroup.addControl(
        'customUserPwd',
        new FormControl('', {
          validators: [
            this.baseUtilService.VALID.required(),
            this.baseUtilService.VALID.minLength(8),
            this.baseUtilService.VALID.maxLength(32),
            this.passwordFormatValidator()
          ]
        })
      );
      this.formGroup
        .get('customUserName')
        .valueChanges.subscribe(() =>
          defer(() =>
            this.formGroup.get('customUserPwd').updateValueAndValidity()
          )
        );
    }
    this.formGroup.get('userType').valueChanges.subscribe(res => {
      if (res === '' || (this.isHcsUser && this.isFileset)) {
        return;
      }
      if (res === DataMap.Cifs_Domain_Client_Type.everyone.value) {
        this.formGroup.get('userName').clearValidators();
      } else if (res === DataMap.Cifs_Domain_Client_Type.windows.value) {
        this.getUsers();
        this.formGroup
          .get('userName')
          .setValidators([this.baseUtilService.VALID.required()]);
      } else {
        this.getUserGroups();
        this.formGroup
          .get('userName')
          .setValidators([this.baseUtilService.VALID.required()]);
      }
      this.formGroup.get('userName').updateValueAndValidity();
    });

    this.formGroup
      .get('min_bandwidth')
      .valueChanges.pipe(pairwise())
      .subscribe(res => {
        if (toString(res[0]) === toString(res[1])) {
          return;
        }
        setTimeout(() => {
          this.updateMinAndMaxBandwidthValidity('min_bandwidth');
          if (
            this.formGroup.value.min_bandwidth &&
            this.formGroup.value.max_bandwidth
          ) {
            this.formGroup.get('max_bandwidth').markAsTouched();
            this.formGroup.get('max_bandwidth').updateValueAndValidity();
            if (this.formGroup.value.burst_bandwidth) {
              this.formGroup.get('burst_bandwidth').markAsTouched();
              this.formGroup.get('burst_bandwidth').updateValueAndValidity();
            }
          }
        }, 0);
      });
    this.formGroup
      .get('max_bandwidth')
      .valueChanges.pipe(pairwise())
      .subscribe(res => {
        if (toString(res[0]) === toString(res[1])) {
          return;
        }
        setTimeout(() => {
          this.updateMinAndMaxBandwidthValidity('max_bandwidth');
          if (
            this.formGroup.value.max_bandwidth &&
            this.formGroup.value.min_bandwidth
          ) {
            this.formGroup.get('min_bandwidth').markAsTouched();
            this.formGroup.get('min_bandwidth').updateValueAndValidity();
            if (this.formGroup.value.burst_bandwidth) {
              this.formGroup.get('burst_bandwidth').markAsTouched();
              this.formGroup.get('burst_bandwidth').updateValueAndValidity();
            }
          }
        }, 0);
      });
    this.formGroup
      .get('burst_bandwidth')
      .valueChanges.pipe(pairwise())
      .subscribe(res => {
        if (toString(res[0]) === toString(res[1])) {
          return;
        }
        setTimeout(() => {
          if (this.formGroup.value.min_bandwidth) {
            this.formGroup.get('min_bandwidth').markAsTouched();
            this.formGroup.get('min_bandwidth').updateValueAndValidity();
          }

          if (this.formGroup.value.max_bandwidth) {
            this.formGroup.get('max_bandwidth').markAsTouched();
            this.formGroup.get('max_bandwidth').updateValueAndValidity();
          }

          if (
            (this.formGroup.value.iopsStatus &&
              this.formGroup.value.burst_iops) ||
            (this.formGroup.value.bindWidthStatus &&
              this.formGroup.value.burst_bandwidth)
          ) {
            this.formGroup
              .get('burst_time')
              .setValidators([
                this.baseUtilService.VALID.required(),
                this.baseUtilService.VALID.integer(),
                this.baseUtilService.VALID.rangeValue(1, 999999999)
              ]);
          } else {
            this.formGroup.get('burst_time').clearValidators();
          }
          this.formGroup.get('burst_time').updateValueAndValidity();
        }, 0);
      });
    this.formGroup
      .get('min_iops')
      .valueChanges.pipe(pairwise())
      .subscribe(res => {
        if (this.formGroup.get('min_iops').invalid) {
          this.updateIopsItems('--', 'min');
        } else {
          this.updateIopsItems(res[1], 'min');
        }
        if (toString(res[0]) === toString(res[1])) {
          return;
        }
        setTimeout(() => {
          this.updateMinAndMaxIopsValidity('min_iops');
          if (this.formGroup.value.min_iops && this.formGroup.value.max_iops) {
            this.formGroup.get('max_iops').markAsTouched();
            this.formGroup.get('max_iops').updateValueAndValidity();
            if (this.formGroup.value.burst_iops) {
              this.formGroup.get('burst_iops').markAsTouched();
              this.formGroup.get('burst_iops').updateValueAndValidity();
            }
          }
        }, 0);
      });
    this.formGroup
      .get('max_iops')
      .valueChanges.pipe(pairwise())
      .subscribe(res => {
        if (this.formGroup.get('max_iops').invalid) {
          this.updateIopsItems('--', 'max');
        } else {
          this.updateIopsItems(res[1], 'max');
        }
        if (toString(res[0]) === toString(res[1])) {
          return;
        }
        setTimeout(() => {
          this.updateMinAndMaxIopsValidity('max_iops');
          if (this.formGroup.value.max_iops && this.formGroup.value.min_iops) {
            this.formGroup.get('min_iops').markAsTouched();
            this.formGroup.get('min_iops').updateValueAndValidity();
            if (this.formGroup.value.burst_iops) {
              this.formGroup.get('burst_iops').markAsTouched();
              this.formGroup.get('burst_iops').updateValueAndValidity();
            }
          }
        }, 0);
      });
    this.formGroup
      .get('burst_iops')
      .valueChanges.pipe(pairwise())
      .subscribe(res => {
        if (this.formGroup.get('burst_iops').invalid) {
          this.updateIopsItems('--', 'burst');
        } else {
          this.updateIopsItems(res[1], 'burst');
        }
        if (toString(res[0]) === toString(res[1])) {
          return;
        }
        setTimeout(() => {
          if (this.formGroup.value.min_iops) {
            this.formGroup.get('min_iops').markAsTouched();
            this.formGroup.get('min_iops').updateValueAndValidity();
          }
          if (this.formGroup.value.max_iops) {
            this.formGroup.get('max_iops').markAsTouched();
            this.formGroup.get('max_iops').updateValueAndValidity();
          }

          if (
            (this.formGroup.value.iopsStatus &&
              this.formGroup.value.burst_iops) ||
            (this.formGroup.value.bindWidthStatus &&
              this.formGroup.value.burst_bandwidth)
          ) {
            this.formGroup
              .get('burst_time')
              .setValidators([
                this.baseUtilService.VALID.required(),
                this.baseUtilService.VALID.integer(),
                this.baseUtilService.VALID.rangeValue(1, 999999999)
              ]);
          } else {
            this.formGroup.get('burst_time').clearValidators();
          }
          this.formGroup.get('burst_time').updateValueAndValidity();
        }, 0);
      });

    this.formGroup.statusChanges.subscribe(res => {
      this.selectMountOptionChange.emit(res === 'VALID');
    });

    this.globalService
      .getState(LiveMountAction.SelectResource)
      .subscribe(res => {
        this.formGroup
          .get('pre_script')
          .setValidators([
            this.baseUtilService.VALID.maxLength(8192),
            this.baseUtilService.VALID.name(
              CommonConsts.REGEX.linuxScript,
              false
            )
          ]);
        this.formGroup
          .get('post_script')
          .setValidators([
            this.baseUtilService.VALID.maxLength(8192),
            this.baseUtilService.VALID.name(
              CommonConsts.REGEX.linuxScript,
              false
            )
          ]);
        this.formGroup
          .get('failed_script')
          .setValidators([
            this.baseUtilService.VALID.maxLength(8192),
            this.baseUtilService.VALID.name(
              CommonConsts.REGEX.linuxScript,
              false
            )
          ]);
        this.formGroup.get('pre_script').updateValueAndValidity();
        this.formGroup.get('post_script').updateValueAndValidity();
        this.formGroup.get('failed_script').updateValueAndValidity();
      });

    this.formGroup
      .get('latency')
      .valueChanges.pipe(pairwise())
      .subscribe(res => {
        if (this.formGroup.get('latency').invalid) {
          this.updateLatencyData('--');
        } else {
          this.updateLatencyData(res[1]);
        }
      });

    this.formGroup.get('targetPos').valueChanges.subscribe(res => {
      if (res === '') {
        return;
      }
      const selectHost = cloneDeep(find(this.hostOptions, { uuid: res }));
      if (selectHost) {
        assign(selectHost, {
          label: selectHost?.environment?.name,
          children: [],
          disabled: true,
          isLeaf: false
        });
        this.filePathData = [selectHost];
      }
    });
  }

  setValidForm() {
    if (!this.isWindows && this.isFileset) {
      this.formGroup
        .get('metadataPath')
        .setValidators([this.baseUtilService.VALID.required()]);
      this.formGroup.get('userType').clearValidators();
      this.formGroup.get('name').clearValidators();
    } else {
      this.formGroup
        .get('userType')
        .setValidators([this.baseUtilService.VALID.required()]);
      this.formGroup
        .get('name')
        .setValidators([this.baseUtilService.VALID.required()]);
      this.formGroup.get('metadataPath').clearValidators();
    }
    this.formGroup.get('metadataPath').updateValueAndValidity();
    this.formGroup.get('userType').updateValueAndValidity();
    this.formGroup.get('name').updateValueAndValidity();
  }

  getHosts(recordsTemp?, startPage?) {
    this.protectedResourceApiService
      .ListResources({
        pageSize: 20,
        pageNo: startPage || 0,
        conditions: JSON.stringify({
          subType: this.isFileset ? ['FilesetPlugin'] : ['VolumePlugin']
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
          startPage === Math.ceil(res.totalCount / CommonConsts.PAGE_SIZE) ||
          res.totalCount === 0
        ) {
          const hostArr = [];
          each(recordsTemp, item => {
            if (
              item.environment?.linkStatus ===
              DataMap.resource_LinkStatus_Special.normal.value
            ) {
              hostArr.push({
                key: item.uuid,
                value: item.uuid,
                label: !isEmpty(item.environment?.endpoint)
                  ? `${item.environment?.name}(${item.environment?.endpoint})`
                  : item.environment?.name,
                os_type: item.environment?.osType,
                parentUuid: item.parentUuid,
                isLeaf: true,
                ...item
              });
            }
          });
          this.hostOptionsCache = hostArr;
          this.hostOptions = this.componentData?.selectionCopy?.uuid
            ? filter(
                hostArr,
                item =>
                  item.os_type ===
                  JSON.parse(
                    this.componentData.selectionCopy.resource_properties
                  )['environment_os_type']
              )
            : hostArr;

          // 当角色为数据保护管理员时需要判断是否授权
          if (this.isDataProtectionAdmin) {
            this.authHosts = map(hostArr, item => {
              return {
                key: item.uuid,
                value:
                  item?.userId ===
                  get(item.environment.extendInfo, 'register_user_id', '')
              };
            });
          }
          return;
        }
        this.getHosts(recordsTemp, startPage);
      });
  }

  expandedChange(node) {
    if (!node.expanded) {
      return;
    }
    node.children = [];
    this.getFileResource(node);
  }

  getFileResource(node, startPage?: number) {
    const sysDir = [
      '/bin',
      '/boot',
      '/dev',
      '/etc',
      '/lib',
      '/lib64',
      '/lost+found',
      '/media',
      '/proc',
      '/root',
      '/sbin',
      '/selinux',
      '/srv',
      '/sys',
      '/usr',
      '/var',
      '/run',
      '/mnt/databackup',
      '/opt/DataBackup'
    ];
    const params = {
      envId: find(this.hostOptions, {
        key: this.formGroup.value.targetPos
      })?.parentUuid,
      pageNo: startPage || CommonConsts.PAGE_START,
      pageSize: 100,
      parentId: node.extendInfo?.path || '',
      resourceType: this.isFileset
        ? DataMap.Resource_Type.fileset.value
        : DataMap.Resource_Type.volume.value
    };
    this.protectedEnvironmentApiService
      .ListEnvironmentResource(params)
      .subscribe(res => {
        const osType = get(
          JSON.parse(
            this.componentData?.selectionCopy?.resource_properties ?? '{}'
          ),
          'environment_os_type'
        );

        if (osType === DataMap.Fileset_Template_Os_Type.windows.value) {
          each(res.records, item => {
            const pathArr = split(item.extendInfo?.path, '\\');
            const pathLabel =
              size(pathArr) === 2 && !last(pathArr)
                ? item.extendInfo?.path
                : replace(last(pathArr), '\\', '');

            assign(item, {
              label: pathLabel,
              isLeaf: item.extendInfo.hasChildren === 'false',
              disabled: item.extendInfo?.type === RestoreFileType.File
            });
          });
        } else {
          const hostId = this.formGroup.getRawValue().targetPos;
          each(res.records, item => {
            let isAuthHost = false;
            if (this.isDataProtectionAdmin) {
              isAuthHost = !find(this.authHosts, { key: hostId })?.value;
            }
            const isNotMntPath = !startsWith(item.extendInfo?.path, '/mnt');
            assign(item, {
              rootPath: node.rootPath
                ? `${node.rootPath}/${item.extendInfo?.path}`
                : `/${item.extendInfo?.path}`,
              label: replace(last(split(item.extendInfo?.path, '/')), '/', ''),
              disabled:
                (isAuthHost && isNotMntPath) ||
                includes(['/mnt', '/opt', '/tmp'], item.extendInfo.path) ||
                !!find(
                  sysDir,
                  str =>
                    startsWith(item.extendInfo.path, `${str}/`) ||
                    item.extendInfo.path === str
                ) ||
                item.extendInfo?.type === RestoreFileType.File,
              isLeaf: item.extendInfo.hasChildren === 'false'
            });
          });
        }
        if (isArray(node.children) && !isEmpty(node.children)) {
          node.children = [
            ...reject(node.children, n => {
              return n.isMoreBtn;
            }),
            ...res.records
          ];
        } else {
          node.children = [...res.records];
        }
        if (res.totalCount > size(node.children)) {
          const moreClickNode = {
            label: `${this.i18n.get('common_more_label')}...`,
            isMoreBtn: true,
            isLeaf: true,
            disabled: true,
            startPage: Math.floor(size(node.children) / 200) + 1
          };
          node.children = [...node.children, moreClickNode];
        }
        this.filePathData = [...this.filePathData];
      });
  }

  updateMinAndMaxBandwidthValidity(focusKey) {
    this.formGroup.get('min_bandwidth').clearValidators();
    this.formGroup.get('max_bandwidth').clearValidators();
    if (
      (focusKey === 'min_bandwidth' &&
        !trim(this.formGroup.value.max_bandwidth)) ||
      (focusKey === 'max_bandwidth' &&
        !trim(this.formGroup.value.max_bandwidth) &&
        trim(this.formGroup.value.min_bandwidth))
    ) {
      this.formGroup
        .get('min_bandwidth')
        .setValidators([
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.integer(),
          this.baseUtilService.VALID.rangeValue(1, 999999999),
          this.validMinBandWidth()
        ]);
      this.formGroup
        .get('max_bandwidth')
        .setValidators([
          this.baseUtilService.VALID.integer(),
          this.baseUtilService.VALID.rangeValue(1, 999999999),
          this.validMaxBandWidth()
        ]);
    } else {
      this.formGroup
        .get('min_bandwidth')
        .setValidators([
          this.baseUtilService.VALID.integer(),
          this.baseUtilService.VALID.rangeValue(1, 999999999),
          this.validMinBandWidth()
        ]);
      this.formGroup
        .get('max_bandwidth')
        .setValidators([
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.integer(),
          this.baseUtilService.VALID.rangeValue(1, 999999999),
          this.validMaxBandWidth()
        ]);
    }
    this.formGroup.get('min_bandwidth').updateValueAndValidity();
    this.formGroup.get('max_bandwidth').updateValueAndValidity();
  }

  updateMinAndMaxIopsValidity(focusKey) {
    this.formGroup.get('min_iops').clearValidators();
    this.formGroup.get('max_iops').clearValidators();
    if (
      (focusKey === 'min_iops' && !trim(this.formGroup.value.max_iops)) ||
      (focusKey === 'max_iops' &&
        !trim(this.formGroup.value.max_iops) &&
        trim(this.formGroup.value.min_iops))
    ) {
      this.formGroup
        .get('min_iops')
        .setValidators([
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.integer(),
          this.baseUtilService.VALID.rangeValue(100, 999999999),
          this.validMinIops()
        ]);
      this.formGroup
        .get('max_iops')
        .setValidators([
          this.baseUtilService.VALID.integer(),
          this.baseUtilService.VALID.rangeValue(100, 999999999),
          this.validMaxIops()
        ]);
    } else {
      this.formGroup
        .get('min_iops')
        .setValidators([
          this.baseUtilService.VALID.integer(),
          this.baseUtilService.VALID.rangeValue(100, 999999999),
          this.validMinIops()
        ]);
      this.formGroup
        .get('max_iops')
        .setValidators([
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.integer(),
          this.baseUtilService.VALID.rangeValue(100, 999999999),
          this.validMaxIops()
        ]);
    }
    this.formGroup.get('min_iops').updateValueAndValidity();
    this.formGroup.get('max_iops').updateValueAndValidity();
  }

  asyncValidSameNameDB() {
    return (
      control: AbstractControl
    ): Promise<{ [key: string]: any } | null> => {
      return new Promise(resolve => {
        this.databasesService
          .queryResourcesV1DatabasesGet({
            pageNo: 0,
            pageSize: 1,
            akLoading: false,
            conditions: JSON.stringify({
              resource_name: this.componentData.selectionResource.resource_name,
              environment_uuid: control.value,
              valid: true
            })
          })
          .subscribe(res => {
            const host = find(this.targetHostOptions, { uuid: control.value });
            const normalDB = find(res.items, item => {
              return (
                item.link_status ===
                DataMap.Database_Resource_LinkStatus.normal.value
              );
            });
            const offlineDB = find(res.items, item => {
              return (
                item.link_status ===
                DataMap.Database_Resource_LinkStatus.offline.value
              );
            });
            if (normalDB) {
              this.targetHostErrorTip = assign({}, this.targetHostErrorTip, {
                invalidSameDb: this.i18n.get(
                  'protection_target_host_oneline_db_label',
                  [host.name, normalDB.name]
                )
              });
              resolve({ invalidSameDb: { value: control.value } });
            }
            this.offlineWarnTip = offlineDB
              ? this.i18n.get('protection_target_host_offline_db_label', [
                  host.name,
                  offlineDB.name
                ])
              : undefined;
            resolve(null);
          });
      });
    };
  }

  validMinBandWidth(): ValidatorFn {
    return (control: AbstractControl): { [key: string]: any } | null => {
      if (!this.formGroup || !trim(control.value)) {
        return null;
      }

      if (
        isEmpty(toString(this.formGroup.value.max_bandwidth)) ||
        !isNumber(+control.value) ||
        !isNumber(+this.formGroup.value.max_bandwidth)
      ) {
        return null;
      }

      return +control.value <= +this.formGroup.value.max_bandwidth
        ? null
        : {
            invalidMin: { value: control.value }
          };
    };
  }

  validMaxBandWidth(): ValidatorFn {
    return (control: AbstractControl): { [key: string]: any } | null => {
      if (!this.formGroup || !trim(control.value)) {
        return null;
      }

      if (
        isEmpty(toString(this.formGroup.value.min_bandwidth)) ||
        !isNumber(+control.value) ||
        !isNumber(+this.formGroup.value.min_bandwidth)
      ) {
        return null;
      }

      return +control.value >= +this.formGroup.value.min_bandwidth
        ? null
        : {
            invalidMax: { value: control.value }
          };
    };
  }

  validBurstBandWidth(): ValidatorFn {
    return (control: AbstractControl): { [key: string]: any } | null => {
      if (!this.formGroup) {
        return null;
      }

      if (!trim(control.value)) {
        return null;
      }

      if (
        !isNumber(+control.value) ||
        isEmpty(toString(this.formGroup.value.max_bandwidth)) ||
        !isNumber(+this.formGroup.value.max_bandwidth)
      ) {
        return null;
      }

      return +control.value > +this.formGroup.value.max_bandwidth
        ? null
        : {
            invalidBurst: { value: control.value }
          };
    };
  }

  validMinIops(): ValidatorFn {
    return (control: AbstractControl): { [key: string]: any } | null => {
      if (!this.formGroup || !trim(control.value)) {
        return null;
      }

      if (
        isEmpty(toString(this.formGroup.value.max_iops)) ||
        !isNumber(+control.value) ||
        !isNumber(+this.formGroup.value.max_iops)
      ) {
        return null;
      }

      return +control.value <= +this.formGroup.value.max_iops
        ? null
        : {
            invalidMin: { value: control.value }
          };
    };
  }

  validMaxIops(): ValidatorFn {
    return (control: AbstractControl): { [key: string]: any } | null => {
      if (!this.formGroup || !trim(control.value)) {
        return null;
      }

      if (
        isEmpty(toString(this.formGroup.value.min_iops)) ||
        !isNumber(+control.value) ||
        !isNumber(+this.formGroup.value.min_iops)
      ) {
        return null;
      }

      return +control.value >= +this.formGroup.value.min_iops
        ? null
        : {
            invalidMax: { value: control.value }
          };
    };
  }

  validBurstIops(): ValidatorFn {
    return (control: AbstractControl): { [key: string]: any } | null => {
      if (!this.formGroup) {
        return null;
      }

      if (!trim(control.value)) {
        return null;
      }

      if (
        !isNumber(+control.value) ||
        isEmpty(toString(this.formGroup.value.max_iops)) ||
        !isNumber(+this.formGroup.value.max_iops)
      ) {
        return null;
      }

      return +control.value > +this.formGroup.value.max_iops
        ? null
        : {
            invalidBurst: { value: control.value }
          };
    };
  }

  getComponentData() {
    const winMountInfo = {};
    if (this.isWindows) {
      assign(this.componentData, {
        share_name: this.formGroup.get('name').value,
        type: this.formGroup.value.userType,
        userName: this.formGroup.value.userName
      });
    }
    const mountTargetHost = {};
    const targetHostList = [];
    const target_resource_uuid_list = [];

    target_resource_uuid_list.push(
      find(
        this.hostOptions,
        item => item.uuid === this.formGroup.value.targetPos
      ).environment.uuid
    );
    if (this.isWindows) {
      assign(this.componentData.requestParams, {
        target_resource_uuid_list,
        target_location: MountTargetLocation.Others,
        file_system_share_info_list: [
          {
            fileSystemName: this.isFileset
              ? `fileset_mount_${Date.now()}`
              : `volume_mount_${Date.now()}`,
            type: 0,
            accessPermission: 1,
            advanceParams: {
              shareName: this.formGroup.get('name').value,
              domainType: this.formGroup.value.userType,
              usernames:
                this.isHcsUser && this.isFileset
                  ? [this.formGroup.value.customUserName]
                  : this.formGroup.value.userType ===
                    DataMap.Cifs_Domain_Client_Type.everyone.value
                  ? ['@EveryOne']
                  : this.formGroup.value.userType ===
                    DataMap.Cifs_Domain_Client_Type.windowsGroup.value
                  ? map(this.formGroup.value.userName, item => {
                      return '@' + item;
                    })
                  : this.formGroup.value.userName
            }
          }
        ]
      });
    } else {
      assign(this.componentData.requestParams, {
        target_resource_uuid_list,
        target_location: MountTargetLocation.Others,
        file_system_share_info_list: [
          {
            fileSystemName: this.isFileset
              ? `fileset_mount_${Date.now()}`
              : `volume_mount_${Date.now()}`,
            type: 1,
            accessPermission: 1,
            advanceParams: {
              clientType: 0,
              clientName: '*',
              squash: 1,
              rootSquash: 1,
              portSecure: 1
            }
          }
        ]
      });
    }

    let parameters = {
      performance: {}
    };
    // HCS服务化环境用户输入
    if (this.isWindows && this.isHcsUser && this.isFileset) {
      assign(parameters, {
        cifsUserHcs: {
          userName: this.formGroup.value.customUserName,
          password: this.formGroup.value.customUserPwd
        }
      });
    }
    let summary = this.formGroup.value;
    const advanceParameters = omit(this.formGroup.value, [
      'targetPos',
      'bindWidthStatus',
      'iopsStatus',
      'latencyStatus',
      'isModify',
      'power_on',
      'name',
      'userType',
      'userName',
      'customUserName',
      'customUserPwd'
    ]);

    each(advanceParameters, (v, k) => {
      if (isEmpty(trim(String(v)))) {
        return;
      }

      if (
        !includes(
          [
            'pre_script',
            'post_script',
            'failed_script',
            'metadataPath',
            'targetPos'
          ],
          k
        )
      ) {
        parameters.performance[k] = +v;
      }
    });

    if (!get(parameters.performance, 'max_bandwidth')) {
      parameters.performance = omit(parameters.performance, 'burst_bandwidth');
      summary['burst_bandwidth'] = '';
    }

    if (!get(parameters.performance, 'max_iops')) {
      parameters.performance = omit(parameters.performance, 'burst_iops');
      summary['burst_iops'] = '';
    }

    if (
      !(
        get(parameters.performance, 'max_bandwidth') &&
        get(parameters.performance, 'max_bandwidth')
      ) &&
      !(
        get(parameters.performance, 'max_iops') &&
        get(parameters.performance, 'burst_iops')
      )
    ) {
      parameters.performance = omit(parameters.performance, 'burst_time');
      summary['burst_time'] = '';
    }
    assign(this.componentData.requestParams, {
      parameters: {
        ...parameters,
        name: `${
          find(
            this.hostOptions,
            item => item.uuid === this.formGroup.value.targetPos
          ).environment.name
        }(${
          find(
            this.hostOptions,
            item => item.uuid === this.formGroup.value.targetPos
          ).environment.endpoint
        })`,
        dstPath: this.isFileset
          ? get(first(advanceParameters.metadataPath), 'extendInfo.path')
          : this.volumePos
      }
    });

    return assign(this.componentData, {
      requestParams: assign(this.componentData.requestParams, {}),
      selectionMount: assign(
        { targetHostList },
        {
          ...summary,
          ip: get(
            find(
              this.hostOptions,
              item => item.uuid === this.formGroup.value.targetPos
            ),
            'environment.endpoint'
          ),
          shareName: this.formGroup.get('name').value,
          type: this.formGroup.value.userType,
          users: this.formGroup.value.userName
        }
      )
    });
  }
  getUsers() {
    this.apiStorageBackupPluginService
      .ListNasUsersInfo({ esn: '0' })
      .subscribe(res => {
        this.userOptions = reject(
          map(res.records, item => {
            return {
              id: item.id,
              label: item.name,
              value: item.name,
              isLeaf: true
            };
          }),
          val => {
            return val.value === 'cifs_backup';
          }
        );

        this.formGroup.get('userName').setValue([]);
      });
  }

  getUserGroups() {
    this.apiStorageBackupPluginService
      .ListNasUserGroupsInfo({
        esn: '0'
      })
      .subscribe(res => {
        this.userOptions = map(res['records'], item => {
          return {
            id: item.id,
            label: item.name,
            value: item.name,
            isLeaf: true
          };
        });

        this.formGroup.get('userName').setValue([]);
      });
  }
}
