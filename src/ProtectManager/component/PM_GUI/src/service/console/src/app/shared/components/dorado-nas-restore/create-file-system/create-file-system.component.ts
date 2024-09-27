import { Component, Input, OnInit } from '@angular/core';
import { FormBuilder, FormControl, FormGroup } from '@angular/forms';
import { BaseUtilService, DataMapService } from 'app/shared';
import { ApiStorageBackupPluginService } from 'app/shared/api/services';
import {
  CommonConsts,
  DataMap,
  NasSecurityStyle,
  PermissonLimitation,
  PortPermisson,
  RootPermisson
} from 'app/shared/consts';
import { I18NService } from 'app/shared/services';
import {
  assign,
  includes,
  map,
  size,
  trim,
  toString as _toString,
  reject
} from 'lodash';
import { Observable, Observer } from 'rxjs';

@Component({
  selector: 'aui-create-file-system',
  templateUrl: './create-file-system.component.html',
  styleUrls: ['./create-file-system.component.less']
})
export class CreateFileSystemComponent implements OnInit {
  @Input() formGroup: FormGroup;
  @Input() createFileSystem;
  @Input() disableFileSystemName;
  @Input() resourceShareMode;
  @Input() memberEsn;
  permissonsLimitation = PermissonLimitation;
  rootPermisson = RootPermisson;
  portPermisson = PortPermisson;
  dataMap = DataMap;
  nameErrorTip = {
    ...this.baseUtilService.requiredErrorTip,
    invalidName: this.i18n.get('protection_nas_filesystem_valid_label')
  };
  cifsNameErrorTip = {
    ...this.baseUtilService.requiredErrorTip,
    invalidName: this.i18n.get('protection_nas_filesystem_valid_label'),
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [255])
  };
  clientErrorTip = {
    ...this.baseUtilService.requiredErrorTip
  };

  clientHostTipLabel = this.i18n.get(
    'protection_file_system_share_client_tip_label'
  );
  clientNetworkGroupTipLabel = this.i18n.get(
    'protection_file_system_host_group_tip_label'
  );
  rootTiplabel = this.i18n.get('protection_root_permission_tip_label');

  clientTypeOptions = this.dataMapService
    .toArray('Nfs_Share_Client_Type')
    .filter(v => (v.isLeaf = true));
  permissionOptions = this.dataMapService
    .toArray('Livemount_Filesystem_Authority_Level')
    .filter(v => (v.isLeaf = true));
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
  namePrefix = 'Restore_';
  unixPermissionOps = this.dataMapService
    .toArray('unixPermission')
    .filter(v => (v.isLeaf = true));
  permissionLevelOps = this.dataMapService
    .toArray('permissionLevel')
    .filter(v => (v.isLeaf = true));

  constructor(
    private fb: FormBuilder,
    private i18n: I18NService,
    private dataMapService: DataMapService,
    public baseUtilService: BaseUtilService,
    private apiStorageBackupPluginService: ApiStorageBackupPluginService
  ) {}

  ngOnInit() {
    this.initForm();
  }

  initForm() {
    this.namePrefix = this.createFileSystem ? 'Restore_' : 'mount_';
    if (this.formGroup) {
      this.formGroup.addControl(
        'name',
        new FormControl('', {
          validators: [
            this.baseUtilService.VALID.required(),
            this.baseUtilService.VALID.name(
              CommonConsts.REGEX.nasFileSystemName
            ),
            this.baseUtilService.VALID.maxLength(255 - size(this.namePrefix))
          ]
        })
      );
      this.formGroup.addControl('nfsEnable', new FormControl(false));
      this.formGroup.addControl('cifsEnable', new FormControl(false));
      this.formGroup.addControl('nfsShareName', new FormControl(''));
      this.formGroup.addControl('client', new FormControl(''));
      this.formGroup.addControl('unixType', new FormControl(''));
      this.formGroup.addControl(
        'rootType',
        new FormControl(DataMap.rootPermission.squash.value)
      );

      this.formGroup.addControl('cifsShareName', new FormControl(''));
      this.formGroup.addControl('userType', new FormControl(''));
      this.formGroup.addControl('userName', new FormControl([]));
      this.formGroup.addControl('permissionType', new FormControl(''));

      this.formGroup.get('nfsEnable').valueChanges.subscribe(res => {
        if (res) {
          this.formGroup
            .get('client')
            .setValidators([this.baseUtilService.VALID.required()]);
          this.formGroup
            .get('unixType')
            .setValidators([this.baseUtilService.VALID.required()]);
        } else {
          this.formGroup.get('client').clearValidators();
          this.formGroup.get('unixType').clearValidators();
        }
        this.formGroup.get('client').updateValueAndValidity();
        this.formGroup.get('unixType').updateValueAndValidity();
      });
      this.formGroup.get('cifsEnable').valueChanges.subscribe(res => {
        if (res) {
          this.formGroup
            .get('cifsShareName')
            .setValidators([
              this.baseUtilService.VALID.required(),
              this.baseUtilService.VALID.name(
                CommonConsts.REGEX.nasFileSystemName
              ),
              this.baseUtilService.VALID.maxLength(255)
            ]);
          this.formGroup
            .get('userType')
            .setValidators([this.baseUtilService.VALID.required()]);
          this.formGroup
            .get('userName')
            .setValidators([this.baseUtilService.VALID.required()]);
          this.formGroup
            .get('permissionType')
            .setValidators([this.baseUtilService.VALID.required()]);
        } else {
          this.formGroup.get('cifsShareName').clearValidators();
          this.formGroup.get('userType').clearValidators();
          this.formGroup.get('userName').clearValidators();
          this.formGroup.get('permissionType').clearValidators();
        }
        this.formGroup.get('cifsShareName').updateValueAndValidity();
        this.formGroup.get('userType').updateValueAndValidity();
        this.formGroup.get('userName').updateValueAndValidity();
        this.formGroup.get('permissionType').updateValueAndValidity();
      });
    } else {
      this.formGroup = this.fb.group({
        name: new FormControl('', {
          validators: [
            this.baseUtilService.VALID.required(),
            this.baseUtilService.VALID.name(
              CommonConsts.REGEX.nasFileSystemName
            ),
            this.baseUtilService.VALID.maxLength(255 - size(this.namePrefix))
          ]
        }),
        nfsEnable: new FormControl(false),
        cifsEnable: new FormControl(false),
        nfsShareName: new FormControl(''),
        client: new FormControl(''),
        unixType: new FormControl(''),
        rootType: new FormControl(DataMap.rootPermission.squash.value),
        cifsShareName: new FormControl(''),
        userType: new FormControl(''),
        userName: new FormControl(''),
        permissionType: new FormControl('')
      });

      this.formGroup.get('nfsEnable').valueChanges.subscribe(res => {
        if (res) {
          this.formGroup
            .get('client')
            .setValidators([this.baseUtilService.VALID.required()]);
          this.formGroup
            .get('unixType')
            .setValidators([this.baseUtilService.VALID.required()]);
        } else {
          this.formGroup.get('client').clearValidators();
          this.formGroup.get('unixType').clearValidators();
        }
        this.formGroup.get('client').updateValueAndValidity();
        this.formGroup.get('unixType').updateValueAndValidity();
      });

      this.formGroup.get('cifsEnable').valueChanges.subscribe(res => {
        if (res) {
          this.formGroup
            .get('cifsShareName')
            .setValidators([
              this.baseUtilService.VALID.required(),
              this.baseUtilService.VALID.name(
                CommonConsts.REGEX.nasFileSystemName
              ),
              this.baseUtilService.VALID.maxLength(255)
            ]);
          this.formGroup
            .get('userType')
            .setValidators([this.baseUtilService.VALID.required()]);
          this.formGroup
            .get('userName')
            .setValidators([this.baseUtilService.VALID.required()]);
          this.formGroup
            .get('permissionType')
            .setValidators([this.baseUtilService.VALID.required()]);
        } else {
          this.formGroup.get('cifsShareName').clearValidators();
          this.formGroup.get('userType').clearValidators();
          this.formGroup.get('userName').clearValidators();
          this.formGroup.get('permissionType').clearValidators();
        }
        this.formGroup.get('cifsShareName').updateValueAndValidity();
        this.formGroup
          .get('userType')
          .updateValueAndValidity({ emitEvent: false });
        this.formGroup.get('userName').updateValueAndValidity();
        this.formGroup.get('permissionType').updateValueAndValidity();
      });
    }

    assign(this.nameErrorTip, {
      invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [
        255 - size(this.namePrefix)
      ])
    });

    this.formGroup.get('nfsShareName').disable();

    this.formGroup.get('name').valueChanges.subscribe(res => {
      this.formGroup.get('nfsShareName').setValue(`${this.namePrefix}${res}`);
      if (!this.formGroup.value.cifsShareName) {
        if (this.createFileSystem) {
          this.formGroup
            .get('cifsShareName')
            .setValue(`${this.namePrefix}${res}${new Date().getTime()}`);
        } else {
          this.formGroup
            .get('cifsShareName')
            .setValue(`${this.namePrefix}${new Date().getTime() + 1000}`);
        }
      }
    });

    this.formGroup.get('userType').valueChanges.subscribe(res => {
      if (res === '') {
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

    if (!this.createFileSystem) {
      this.formGroup.get('name').setValue(`${new Date().getTime()}`);
    }

    // 创建文件系统
    if (this.createFileSystem && this.resourceShareMode) {
      if (this.resourceShareMode === DataMap.Shared_Mode.nfs.value) {
        this.formGroup.get('nfsEnable').setValue(true);
      } else {
        this.formGroup.get('cifsEnable').setValue(true);
      }
    }
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
        if (this.createFileSystem) {
          this.formGroup.get('userName').setValue('');
        } else {
          this.formGroup.get('userName').setValue([]);
        }
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
        if (this.createFileSystem) {
          this.formGroup.get('userName').setValue('');
        } else {
          this.formGroup.get('userName').setValue([]);
        }
      });
  }

  onOK(): Observable<any> {
    return new Observable<any>((observer: Observer<any>) => {
      const params = {
        name: `Restore_${this.formGroup.value.name}`,
        securityStyle:
          this.resourceShareMode === DataMap.Shared_Mode.nfs.value
            ? NasSecurityStyle.UNIX
            : NasSecurityStyle.NTFS
      } as any;
      if (this.formGroup.value.nfsEnable) {
        assign(params, {
          nfsShare: {
            shareName: `Restore_${this.formGroup.value.name}`,
            clientType: _toString(DataMap.Nfs_Share_Client_Type.host.value),
            client: this.formGroup.value.client,
            accessVal: _toString(
              DataMap.Livemount_Filesystem_Authority_Level.readonly.value
            ),
            squash: _toString(PermissonLimitation.Retained),
            rootSquash: _toString(RootPermisson.Enable),
            secure: _toString(PortPermisson.Arbitrary)
          }
        });
      }
      if (this.formGroup.value.cifsEnable) {
        assign(params, {
          cifsShare: {
            shareName: this.formGroup.value.cifsShareName,
            cifsDomaintype: 2,
            name:
              this.formGroup.value.userType ===
              DataMap.Cifs_Domain_Client_Type.everyone.value
                ? '@Everyone'
                : this.formGroup.value.userType ===
                  DataMap.Cifs_Domain_Client_Type.windowsGroup.value
                ? `@${this.formGroup.value.userName}`
                : this.formGroup.value.userName,
            permission: _toString(
              DataMap.Livemount_Filesystem_Authority_Level.readonly.value
            )
          }
        });
      }
      this.apiStorageBackupPluginService
        .CreateFileSystem({
          CreateFileSystemRequestBody: params,
          memberEsn: this.memberEsn
        })
        .subscribe(
          res => {
            observer.next(res);
            observer.complete();
          },
          err => {
            observer.error(err);
            observer.complete();
          }
        );
    });
  }
}
