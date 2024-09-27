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
  Component,
  Input,
  OnInit,
  TemplateRef,
  ViewChild
} from '@angular/core';
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
  CommonConsts,
  CommonShareRestoreApiService,
  CookieService,
  DataMap,
  DataMapService,
  I18NService,
  MODAL_COMMON,
  ProtectedResourceApiService
} from 'app/shared';
import { DrawModalService } from 'app/shared/services/draw-modal.service';
import {
  assign,
  cloneDeep,
  defer,
  each,
  get,
  includes,
  isEmpty,
  isFunction,
  map,
  reject,
  trim
} from 'lodash';
import { Observable, Observer } from 'rxjs';
import { ModalRef } from '@iux/live';
import { LinkComponent } from '../link/link.component';

@Component({
  selector: 'aui-restore-commonshare',
  templateUrl: './restore-commonshare.component.html',
  styleUrls: ['./restore-commonshare.component.less']
})
export class RestoreCommonshareComponent implements OnInit {
  @Input() copyData;
  @Input() isConfig;
  @Input() refreshFunc;
  includes = includes;
  dataMap = DataMap;
  resourceData;
  detailData;
  formGroup: FormGroup;
  whitelistForm: FormGroup;
  activeStep = 1;
  nfsEnabled = false;
  cifsEnabled = false;
  extendInfo = {
    esn: '',
    filesystemName: '',
    nfs: null,
    sharePath: '',
    cifs: null,
    shareName: '',
    userType: null,
    userNames: '',
    whitelist: []
  };
  shareProtocol = [];
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
  ipRepeat = false;
  repeatTips;

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
  isHcsUser = this.cookieService.get('userType') === CommonConsts.HCS_USER_TYPE;
  validArr = [];

  whiteListErrorTip = {
    ...this.baseUtilService.ipErrorTip,
    invalidInput: this.i18n.get('common_invalid_input_label'),
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [255])
  };

  @ViewChild('headerTpl', { static: true }) headerTpl: TemplateRef<any>;
  @ViewChild(LinkComponent, { static: false })
  LinkComponent: LinkComponent;
  constructor(
    private modal: ModalRef,
    private fb: FormBuilder,
    public baseUtilService: BaseUtilService,
    public i18n: I18NService,
    private dataMapService: DataMapService,
    private drawModalService: DrawModalService,
    private apiStorageBackupPluginService: ApiStorageBackupPluginService,
    private protectedResourceApiService: ProtectedResourceApiService,
    private commonShareRestoreApiService: CommonShareRestoreApiService,
    private cookieService: CookieService
  ) {}

  ngOnInit() {
    this.setIpValid();
    if (this.copyData?.status === DataMap.copydata_validStatus.sharing.value) {
      this.activeStep = 2;
      this.isConfig = false;
      const properties = JSON.parse(get(this.copyData, 'properties', '{}'));
      this.detailData = JSON.parse(get(properties, 'shareInfo', '{}'));
    }
    this.getModalHeader();
    this.queryData();
    this.initForm();
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

  getModalHeader() {
    if (
      this.copyData?.status === DataMap.copydata_validStatus.sharing.value &&
      this.activeStep === 2
    ) {
      this.modal.setProperty({
        lvHeader: this.i18n.get('explore_commonshare_view_shareinfo_label'),
        lvFooter: [
          {
            label: this.i18n.get('common_close_label'),
            disabled: false,
            onClick: modal => {
              modal.close();
            }
          }
        ]
      });
    }
  }

  queryData() {
    const params = {
      subType: 'CommonShare',
      uuid: [['~~'], this.copyData?.resource_id]
    };
    this.protectedResourceApiService
      .ListResources({
        pageNo: 0,
        pageSize: 1,
        akLoading: true,
        conditions: JSON.stringify(params)
      })
      .subscribe(res => {
        if (res.records.length) {
          this.resourceData = res.records[0];
          this.updateData();
        }
      });
  }

  initForm() {
    this.whitelistForm = this.fb.group({
      ip: new FormControl('', {
        validators: this.validArr,
        updateOn: 'change'
      })
    });
    this.formGroup = this.fb.group({
      shareProtocol: new FormControl([], {
        validators: [this.baseUtilService.VALID.required()]
      }),
      whitelist: this.fb.array([]),
      userType: new FormControl(''),
      userNames: new FormControl([])
    });
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
        this.formGroup.get('userType').clearValidators();
        this.formGroup.get('userNames').clearValidators();
      }
      this.formGroup.get('whitelist').updateValueAndValidity();
      this.formGroup
        .get('userType')
        .updateValueAndValidity({ emitEvent: false });
      this.formGroup.get('userNames').updateValueAndValidity();
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

  updateData() {
    if (!isEmpty(this.resourceData)) {
      this.extendInfo = cloneDeep(this.resourceData?.extendInfo);
      if (this.extendInfo.nfs === 'true') {
        this.nfsEnabled = true;
        this.shareProtocol.push('nfs');
        this.extendInfo.whitelist = JSON.parse(
          get(this.resourceData?.extendInfo, 'whitelist', '[]')
        );
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

      if (this.extendInfo.cifs === 'true') {
        this.cifsEnabled = true;
        this.shareProtocol.push('cifs');
        if (this.extendInfo.userNames) {
          this.extendInfo.userNames = JSON.parse(
            get(this.extendInfo, 'userNames', '[]')
          );
          this.formGroup.get('userNames').setValue(this.extendInfo.userNames);
        }
        this.formGroup
          .get('userType')
          .setValue(Number(this.extendInfo.userType));
      }

      this.formGroup.get('shareProtocol').setValue(this.shareProtocol);

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
    }
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
    const form = this.fb.group({
      ip: new FormControl('', {
        validators: this.validArr
      })
    });
    if (item) {
      defer(() => {
        form.get('ip').setValue(item);
      });
    }
    return form;
  }

  addIp(item?) {
    (this.formGroup.get('whitelist') as FormArray).push(
      this.getIpsFormGroup(item)
    );
  }

  deleteIp(i) {
    (this.formGroup.get('whitelist') as FormArray).removeAt(i);
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
  getParams() {
    let params: any = {};
    let whitelist = [];
    whitelist = map(this.formGroup.value.whitelist, item => {
      return item.ip;
    }).filter(v => v !== '');
    if (includes(this.formGroup.value.shareProtocol, 'nfs')) {
      const nfsInfo = {
        whitelist: whitelist
      };
      assign(params, { nfs: nfsInfo });
    }

    if (includes(this.formGroup.value.shareProtocol, 'cifs')) {
      const cifsInfo = {
        userType: this.formGroup.value.userType
      };
      assign(params, {
        cifs: cifsInfo,
        ...params
      });
      if (
        this.formGroup.value.userType &&
        this.formGroup.value.userType !==
          DataMap.Cifs_Domain_Client_Type.everyone.value
      ) {
        assign(params, {
          cifs: {
            userNames: this.formGroup.value.userNames,
            ...params.cifs
          }
        });
      }
    }
    assign(params, {
      copyId: this.copyData?.uuid,
      ...params
    });
    return params;
  }
  // 点击确定，下发请求，将表单关闭，打开信息展示表单
  onOK(): Observable<void> {
    return new Observable<void>((observer: Observer<void>) => {
      if (this.formGroup.invalid) {
        return;
      }

      const params = this.getParams();

      this.commonShareRestoreApiService
        .CreateRestore({
          CreateRestoreRequestBody: params,
          akLoading: false
        })
        .subscribe(
          res => {
            if (isFunction(this.refreshFunc)) {
              this.refreshFunc();
            }
            observer.next();
            observer.complete();
          },
          () => {
            observer.error(null);
            observer.complete();
          }
        );
    });
  }
}
