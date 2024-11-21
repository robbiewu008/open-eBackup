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
import {
  AbstractControl,
  FormArray,
  FormBuilder,
  FormControl,
  FormGroup,
  ValidatorFn
} from '@angular/forms';
import {
  ApiStorageBackupPluginService,
  BaseUtilService,
  ClustersApiService,
  CommonConsts,
  CookieService,
  DataMap,
  DataMapService,
  I18NService,
  PortPermisson,
  ProtectedEnvironmentApiService,
  ProtectedResourceApiService,
  QosService,
  RootPermisson,
  StorageUnitService
} from 'app/shared';
import { cacheGuideResource } from 'app/shared/consts/guide-config';
import {
  assign,
  cloneDeep,
  defer,
  each,
  get,
  includes,
  isEmpty,
  map,
  reject,
  size,
  trim,
  uniq
} from 'lodash';
import { Observable, Observer } from 'rxjs';

@Component({
  selector: 'aui-create-commonshare',
  templateUrl: './create-commonshare.component.html',
  styleUrls: ['./create-commonshare.component.less']
})
export class CreateCommonShareComponent implements OnInit {
  @Input() rowData;
  includes = includes;
  formGroup: FormGroup;
  whitelistForm: FormGroup;
  dataMap = DataMap;
  rootPermisson = RootPermisson;
  portPermisson = PortPermisson;
  ctrls;
  isModify = false;
  nfsEnabled = false;
  cifsEnabled = false;
  shareInfo = [];
  disableFileSystemName = false;
  shareProtocolOps = [
    {
      value: 'nfs',
      label: 'NFS'
    },
    {
      value: 'cifs',
      label: 'CIFS'
    }
  ];
  extendInfo = {
    storageUnitId: '',
    filesystemName: '',
    nfs: null,
    sharePath: '',
    cifs: null,
    shareName: '',
    userType: null,
    userNames: '',
    whitelist: [],
    qos_id: ''
  };
  shareProtocol = [];
  ipRepeat = false;
  repeatTips;
  qosNames = [];
  isHcsUser = this.cookieService.get('userType') === CommonConsts.HCS_USER_TYPE;
  validArr = [];

  nameErrorTip = {
    ...this.baseUtilService.nameErrorTip,
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [64])
  };
  filesystemNameErrorTip = {
    ...this.baseUtilService.requiredErrorTip,
    invalidName: this.i18n.get('protection_nas_filesystem_valid_label'),
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [255])
  };
  cifsNameErrorTip = {
    ...this.baseUtilService.requiredErrorTip,
    invalidName: this.i18n.get('protection_nas_filesystem_valid_label'),
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [80])
  };
  clientErrorTip = {
    ...this.baseUtilService.requiredErrorTip
  };

  clientHostTipLabel = this.i18n.get('protection_file_system_host_tip_label');
  clientNetworkGroupTipLabel = this.i18n.get(
    'protection_file_system_host_group_tip_label'
  );

  whiteListErrorTip = {
    ...this.baseUtilService.ipErrorTip,
    invalidInput: this.i18n.get('common_invalid_input_label'),
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [255]),
    invalidRepeat: this.i18n.get('common_duplicate_input_label')
  };

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
  userOptions = [];
  backupStorageUnitOps = [];

  constructor(
    private fb: FormBuilder,
    public baseUtilService: BaseUtilService,
    public i18n: I18NService,
    private dataMapService: DataMapService,
    private apiStorageBackupPluginService: ApiStorageBackupPluginService,
    private clusterApiService: ClustersApiService,
    private protectedEnvironmentApiService: ProtectedEnvironmentApiService,
    private qosServiceApi: QosService,
    private cookieService: CookieService,
    private storageUnitService: StorageUnitService
  ) {}

  ngOnInit() {
    this.setIpValid();
    this.getQosNames();
    this.initForm();
    this.getBackupStorageNames();
  }

  setIpValid() {
    if (this.isHcsUser) {
      this.validArr = [this.validWhiteList()];
    } else {
      this.validArr = [
        this.baseUtilService.VALID.required(),
        this.validWhiteList()
      ];
    }
  }

  getQosNames() {
    this.qosServiceApi
      .queryResourcesV1QosGet({
        pageNo: 0,
        pageSize: 100
      })
      .subscribe(res => {
        this.qosNames = map(res.items, (item: any) => {
          item['isLeaf'] = true;
          item['label'] = item.name;
          return item;
        });
      });
  }

  initForm() {
    if (!isEmpty(this.rowData)) {
      this.isModify = true;
      this.extendInfo = cloneDeep(this.rowData?.extendInfo);
      if (this.extendInfo.nfs === 'true') {
        this.nfsEnabled = true;
        this.shareProtocol.push('nfs');
        this.extendInfo.whitelist = JSON.parse(
          get(this.rowData?.extendInfo, 'whitelist', '[]')
        );
      }

      if (this.extendInfo.cifs === 'true') {
        this.cifsEnabled = true;
        this.shareProtocol.push('cifs');
        if (this.extendInfo.userNames) {
          this.extendInfo.userNames = JSON.parse(
            get(this.extendInfo, 'userNames', '[]')
          );
        }
      }
    }
    this.whitelistForm = this.fb.group({
      ip: new FormControl(
        this.extendInfo.nfs === 'true' ? this.extendInfo.whitelist[0] : '',
        {
          validators: this.validArr,
          updateOn: 'change'
        }
      )
    });
    this.formGroup = this.fb.group({
      name: new FormControl(this.isModify ? this.rowData.name : '', {
        validators: [
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.maxLength(64),
          this.baseUtilService.VALID.name()
        ]
      }),
      esn: new FormControl(
        this.isModify ? this.extendInfo?.storageUnitId : '',
        {
          validators: [this.baseUtilService.VALID.required()]
        }
      ),

      filesystemName: new FormControl(
        this.isModify ? this.extendInfo?.filesystemName : '',
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
      shareProtocol: new FormControl(this.isModify ? this.shareProtocol : [], {
        validators: [this.baseUtilService.VALID.required()]
      }),
      nfsSharePath: new FormControl(
        this.isModify && this.nfsEnabled ? this.extendInfo?.sharePath : ''
      ),
      whitelist: this.fb.array([]),
      shareName: new FormControl(
        this.isModify && this.cifsEnabled ? this.extendInfo?.shareName : ''
      ),
      userType: new FormControl(
        this.isModify && this.cifsEnabled
          ? Number(this.extendInfo?.userType)
          : ''
      ),
      userNames: new FormControl(
        this.extendInfo.cifs === 'true' ? this.extendInfo?.userNames : []
      ),
      qos_id: new FormControl(this.isModify ? this.extendInfo.qos_id : '')
    });
    if (this.rowData) {
      if (this.extendInfo.nfs === 'true') {
        const whitelistControl = this.formGroup.controls[
          'whitelist'
        ] as FormArray;
        whitelistControl.removeAt(0);
        const arr = this.extendInfo?.whitelist;
        if (arr.length > 0) {
          for (let item of arr) {
            this.addIp(item);
          }
        }
      }

      if (
        this.formGroup.value.userType ===
        DataMap.Cifs_Domain_Client_Type.windows.value
      ) {
        this.getUsers();
      }
      if (
        this.formGroup.value.userType ===
        DataMap.Cifs_Domain_Client_Type.windowsGroup.value
      ) {
        this.getUserGroups();
      }
      this.formGroup.get('filesystemName').disable();
      this.cifsEnabled && this.formGroup.get('shareName').disable();
    }
    this.listenForm();
  }

  listenForm() {
    this.formGroup.get('shareProtocol').valueChanges.subscribe(res => {
      const whitelistControl = this.formGroup.controls[
        'whitelist'
      ] as FormArray;
      if (includes(res, 'nfs')) {
        if (!whitelistControl.controls.length) {
          whitelistControl.push(this.whitelistForm);
        }

        each(whitelistControl.controls, form => {
          form?.get('ip').setValidators(this.validArr);
          form?.get('ip').updateValueAndValidity();
        });
      }
      if (!includes(res, 'nfs')) {
        each(whitelistControl.controls, form => {
          form?.get('ip').clearValidators();
          form?.get('ip').updateValueAndValidity();
        });
      }
      if (includes(res, 'cifs')) {
        this.formGroup
          .get('shareName')
          .setValidators([
            this.baseUtilService.VALID.required(),
            this.baseUtilService.VALID.name(
              CommonConsts.REGEX.nasFileSystemName
            ),
            this.baseUtilService.VALID.maxLength(80)
          ]);
        this.formGroup
          .get('userType')
          .setValidators([this.baseUtilService.VALID.required()]);
        if (
          !this.formGroup.value.userType ||
          this.formGroup.value.userType ===
            DataMap.Cifs_Domain_Client_Type.everyone.value
        ) {
          this.formGroup.get('userNames').clearValidators();
        } else {
          this.formGroup
            .get('userNames')
            .setValidators([this.baseUtilService.VALID.required()]);
        }
      }
      if (!includes(res, 'cifs')) {
        this.formGroup.get('shareName').clearValidators();
        this.formGroup.get('userType').clearValidators();
        this.formGroup.get('userNames').clearValidators();
      }
      this.formGroup.get('shareName').updateValueAndValidity();
      this.formGroup
        .get('userType')
        .updateValueAndValidity({ emitEvent: false });
      this.formGroup.get('userNames').updateValueAndValidity();
    });
    this.formGroup
      .get('nfsSharePath')
      .setValue(this.extendInfo?.filesystemName);
    this.formGroup.get('nfsSharePath').disable();

    this.formGroup.get('filesystemName').valueChanges.subscribe(res => {
      this.formGroup.get('nfsSharePath').setValue(res);
    });

    this.formGroup.get('whitelist').statusChanges.subscribe(() => {
      defer(() => this.validRepeat());
    });

    this.formGroup.get('userType').valueChanges.subscribe(res => {
      if (res === '') {
        return;
      }
      if (res === DataMap.Cifs_Domain_Client_Type.everyone.value) {
        this.formGroup.get('userNames').clearValidators();
      } else if (res === DataMap.Cifs_Domain_Client_Type.windows.value) {
        this.getUsers();
        this.formGroup
          .get('userNames')
          .setValidators([this.baseUtilService.VALID.required()]);
      } else {
        this.getUserGroups();
        this.formGroup
          .get('userNames')
          .setValidators([this.baseUtilService.VALID.required()]);
      }
      this.formGroup.get('userNames').updateValueAndValidity();
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
      });
  }

  getBackupStorageNames() {
    const params = {
      pageNo: CommonConsts.PAGE_START,
      pageSize: CommonConsts.PAGE_SIZE * 10,
      akOperationTips: false,
      akLoading: false
    };
    this.storageUnitService.queryBackUnitGET(params).subscribe(res => {
      this.backupStorageUnitOps = map(res.records, item => {
        return {
          isLeaf: true,
          label: item.name,
          value: item.id,
          disabled: false,
          ...item
        };
      });
      this.backupStorageUnitOps = [...this.backupStorageUnitOps];
    });
  }

  get ips() {
    return (this.formGroup.get('whitelist') as FormArray).controls;
  }

  validWhiteList(): ValidatorFn {
    return (control: AbstractControl): { [key: string]: any } | null => {
      if (!trim(control.value)) {
        return null;
      }

      if (this.validIp(control.value)) {
        return null;
      }

      return { invalidInput: { value: control.value } };
    };
  }

  validRepeat() {
    const arr = cloneDeep(this.formGroup.value.whitelist);
    const allIps = [];
    const repeatIps = [];
    each(arr, ({ ip }) => {
      if (!trim(ip)) {
        return;
      }
      if (!includes(allIps, ip)) {
        allIps.push(ip);
      } else {
        repeatIps.push(ip);
      }
    });
    if (repeatIps.length) {
      this.ipRepeat = true;
      this.repeatTips = this.i18n.get('common_same_ip_tips_label', [
        repeatIps.join(this.i18n.isEn ? ',' : '，')
      ]);
    } else {
      this.ipRepeat = false;
    }
  }
  validIp(value) {
    if (value === '*') {
      return true;
    }

    const ipv4Re = CommonConsts.REGEX.ipv4;
    const ipv6Re = CommonConsts.REGEX.ipv6;
    if (ipv4Re.test(value) || ipv6Re.test(value)) {
      // 是ip
      return true;
    }

    // 不是ip，校验是否带子网掩码
    if (value.indexOf('/') < 0) {
      // 既不是ip也没有带子网掩码
      return false;
    }
    const strArr = value.split('/');
    if (strArr.length > 2) {
      // 不是正常格式
      return false;
    }
    const validIpv4 = ipv4Re.test(strArr[0]);
    const validIpv6 = ipv6Re.test(strArr[0]);

    const reg = /^[0-9]*$/;
    // 不全是数字
    if (!strArr[1] || !reg.test(strArr[1])) {
      return false;
    }
    const num = Number(strArr[1]);
    if (validIpv4 && num >= 1 && num <= 32) {
      return true;
    }
    if (validIpv6 && num >= 1 && num <= 128) {
      return true;
    }

    return false;
  }

  getIpsFormGroup(item?) {
    return this.fb.group({
      ip: new FormControl(item || '', {
        validators: this.validArr,
        updateOn: 'change'
      })
    });
  }

  addIp(item?) {
    (this.formGroup.get('whitelist') as FormArray).push(
      this.getIpsFormGroup(item)
    );
  }

  deleteIp(i) {
    (this.formGroup.get('whitelist') as FormArray).removeAt(i);
  }
  getParams() {
    const params = {
      parentUuid: null,
      name: this.formGroup.value.name,
      subType: 'CommonShare',
      type: 'Agentless',
      extendInfo: {
        filesystemName: this.isModify
          ? this.extendInfo?.filesystemName
          : this.formGroup.value.filesystemName,
        storageUnitId: this.isModify
          ? this.extendInfo?.storageUnitId
          : this.formGroup.value.esn,
        nfs: String(includes(this.formGroup.value.shareProtocol, 'nfs')),
        cifs: String(includes(this.formGroup.value.shareProtocol, 'cifs')),
        qos_id: this.formGroup.value.qos_id || ''
      }
    };
    let whitelist = [];
    whitelist = map(this.formGroup.value.whitelist, item => {
      return item.ip;
    }).filter(v => v !== '');
    if (includes(this.formGroup.value.shareProtocol, 'nfs')) {
      assign(params.extendInfo, {
        sharePath: this.isModify
          ? this.extendInfo?.filesystemName
          : this.formGroup.value.filesystemName,
        whitelist: JSON.stringify(whitelist),
        ...params.extendInfo
      });
    }

    if (includes(this.formGroup.value.shareProtocol, 'cifs')) {
      assign(params.extendInfo, {
        shareName: this.cifsEnabled
          ? this.extendInfo?.shareName
          : this.formGroup.value.shareName,
        userType: this.formGroup.value.userType,
        ...params.extendInfo
      });
      if (
        this.formGroup.value.userType &&
        this.formGroup.value.userType !==
          DataMap.Cifs_Domain_Client_Type.everyone.value
      ) {
        assign(params.extendInfo, {
          userNames: JSON.stringify(this.formGroup.value.userNames),
          ...params.extendInfo
        });
      }
    }
    return params;
  }
  onOK(): Observable<void> {
    return new Observable<void>((observer: Observer<void>) => {
      if (this.formGroup.invalid) {
        return;
      }
      const params = this.getParams();

      // 修改接口
      if (this.rowData) {
        this.protectedEnvironmentApiService
          .UpdateProtectedEnvironment({
            envId: this.rowData.uuid,
            UpdateProtectedEnvironmentRequestBody: params
          })
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
      } else {
        // 注册接口
        this.protectedEnvironmentApiService
          .RegisterProtectedEnviroment({
            RegisterProtectedEnviromentRequestBody: params
          })
          .subscribe({
            next: res => {
              cacheGuideResource(res);
              observer.next();
              observer.complete();
            },
            error: err => {
              observer.error(err);
              observer.complete();
            }
          });
      }
    });
  }
}
