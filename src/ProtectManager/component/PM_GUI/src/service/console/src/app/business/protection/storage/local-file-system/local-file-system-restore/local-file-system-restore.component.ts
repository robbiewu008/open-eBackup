import { Component, Input, OnInit } from '@angular/core';
import { FormBuilder, FormControl, FormGroup } from '@angular/forms';
import {
  ApiStorageBackupPluginService,
  BaseUtilService,
  CommonConsts,
  DataMap,
  OverWriteOption,
  ProtectedResourceApiService,
  RestoreLocationType,
  RestoreManagerService as RestoreService,
  CookieService,
  WarningMessageService,
  I18NService,
  RestoreV2Type,
  RestoreApiV2Service,
  RestoreV2LocationType,
  NasFileReplaceStrategy
} from 'app/shared';
import {
  assign,
  cloneDeep,
  find,
  first,
  isNumber,
  last,
  map,
  pick,
  reject,
  size,
  uniqBy
} from 'lodash';
import { Observable, Observer } from 'rxjs';

@Component({
  selector: 'aui-local-file-system-restore',
  templateUrl: './local-file-system-restore.component.html',
  styleUrls: ['./local-file-system-restore.component.less']
})
export class LocalFileSystemRestoreComponent implements OnInit {
  @Input() rowCopy;
  @Input() childResType;
  @Input() restoreType;
  @Input() targetParams;

  envIdOptions = [];
  shareNameOptions = [];

  formGroup: FormGroup;
  restoreLocationType = RestoreLocationType;
  fileReplaceStrategy = NasFileReplaceStrategy;
  overWriteOption = OverWriteOption;

  dataMap = DataMap;
  resourceObj;

  disableOriginlocation = false;
  isHyperdetect =
    this.i18n.get('deploy_type') === DataMap.Deploy_Type.hyperdetect.value;

  constructor(
    private fb: FormBuilder,
    private i18n: I18NService,
    private cookieService: CookieService,
    private restoreService: RestoreService,
    public baseUtilService: BaseUtilService,
    private restoreV2Service: RestoreApiV2Service,
    private warningMessageService: WarningMessageService,
    private protectedResourceApiService: ProtectedResourceApiService,
    private apiStorageBackupPluginService: ApiStorageBackupPluginService
  ) {}

  ngOnInit() {
    this.initForm();
    this.getResourceObj();
  }

  getResourceObj() {
    this.protectedResourceApiService
      .ListResources({
        akDoException: false,
        pageNo: CommonConsts.PAGE_START,
        pageSize: CommonConsts.PAGE_SIZE,
        conditions: JSON.stringify({
          subType: [this.rowCopy.resource_sub_type],
          uuid: this.rowCopy.resource_id
        })
      })
      .subscribe(
        res => {
          if (!!size(res.records)) {
            this.resourceObj = first(res.records);
          } else {
            this.resourceObj = JSON.parse(this.rowCopy.resource_properties);
            this.disableOriginlocation = true;
            this.formGroup
              .get('restore_location')
              .setValue(this.restoreLocationType.NEW);
          }
        },
        err => {
          this.resourceObj = JSON.parse(this.rowCopy.resource_properties);
        }
      );
  }

  getEnvIdOptions(recordsTemp?, startPage?) {
    this.protectedResourceApiService
      .ListResources({
        pageNo: startPage || CommonConsts.PAGE_START,
        pageSize: CommonConsts.PAGE_SIZE * 10,
        conditions: JSON.stringify({
          subType: [DataMap.Resource_Type.LocalFileSystem.value]
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
          this.envIdOptions = reject(
            map(recordsTemp, item => {
              assign(item, {
                label: item.name,
                isLeaf: true
              });
              return item;
            }),
            item => {
              return item.uuid === this.rowCopy.resource_id;
            }
          );
          return;
        }
        this.getEnvIdOptions(recordsTemp, startPage);
      });
  }

  getShareNameOptions(env_id, share?) {
    this.apiStorageBackupPluginService
      .ShowStorageFileSystemInfo({
        fileSystemId: env_id
      })
      .subscribe(res => {
        this.shareNameOptions = uniqBy(
          map(res.nfsShares, item => {
            assign(item, {
              label: item.sharePath,
              value: item.sharePath,
              isLeaf: true
            });
            return item;
          }),
          'value'
        );
        if (share) {
          this.formGroup.get('share_name').setValue(share);
        } else {
          this.formGroup.get('share_name').setValue('');
        }
      });
  }

  initForm() {
    this.formGroup = this.fb.group({
      restore_location: new FormControl(this.restoreLocationType.ORIGIN),
      location: new FormControl(this.rowCopy.resource_location),
      env_id: new FormControl(''),
      share_name: new FormControl(''),
      over_write_option: new FormControl(
        this.isHyperdetect
          ? this.fileReplaceStrategy.Replace
          : this.overWriteOption.Skip
      )
    });
    this.disableOriginlocation =
      this.rowCopy.generated_by ===
        DataMap.CopyData_generatedType.replicate.value ||
      this.rowCopy.is_replicated;
    if (this.disableOriginlocation) {
      setTimeout(() => {
        this.formGroup.get('restoreTo').setValue(this.restoreLocationType.NEW);
      }, 100);
    }

    this.patchValue();

    this.listenFormGroup();
  }

  patchValue() {
    if (this.targetParams) {
      this.formGroup.patchValue(
        pick(this.targetParams, [
          'restore_location',
          'env_id',
          'share_name',
          'over_write_option'
        ]),
        {
          emitEvent: false
        }
      );
      if (this.targetParams.restore_location === this.restoreLocationType.NEW) {
        this.getEnvIdOptions();
        this.getShareNameOptions(
          this.targetParams.env_id,
          this.targetParams.share_name
        );
      }
    }
  }

  listenFormGroup() {
    this.formGroup.get('restore_location').valueChanges.subscribe(res => {
      if (res === this.restoreLocationType.ORIGIN) {
        this.formGroup.get('env_id').clearValidators();
        this.formGroup.get('share_name').clearValidators();
      } else if (res === this.restoreLocationType.NEW) {
        if (!size(this.envIdOptions)) {
          this.getEnvIdOptions();
        }
        this.formGroup
          .get('env_id')
          .setValidators([this.baseUtilService.VALID.required()]);
        this.formGroup
          .get('share_name')
          .setValidators([this.baseUtilService.VALID.required()]);
      }
      this.formGroup.get('env_id').updateValueAndValidity({ emitEvent: false });
      this.formGroup.get('share_name').updateValueAndValidity();
    });

    this.formGroup.get('env_id').valueChanges.subscribe(res => {
      if (
        !res ||
        this.restoreLocationType.ORIGIN ===
          this.formGroup.value.restore_location
      ) {
        return;
      }
      this.getShareNameOptions(res);
    });

    this.formGroup.get('location').disable();
  }

  getTargetParams() {
    const resource = cloneDeep(
      find(this.shareNameOptions, {
        value: this.formGroup.value.share_name
      })
    );
    const filesystemName = find(this.envIdOptions, {
      uuid: this.formGroup.value.env_id
    })?.['name'];
    if (resource) {
      assign(resource, {
        name: last(
          reject(resource.shareName?.split('/'), item => {
            return item === '';
          })
        ),
        path: resource.shareName.replace(`/${filesystemName}`, '')
      });
    }
    return assign({}, this.formGroup.value, {
      resource,
      requestParams: this.getParams(),
      mountRequestParams: this.getMountParams()
    });
  }

  getMountParams() {
    if (this.formGroup.value.restore_location === RestoreLocationType.ORIGIN) {
      return;
    }
    return {
      location: 'LOCAL',
      envId: '',
      CreateMountRequestBody: {
        filesystemName: find(this.envIdOptions, {
          uuid: this.formGroup.value.env_id
        })['name']
      }
    };
  }

  getParams() {
    return {
      copy_id: this.rowCopy.uuid,
      restore_type: this.restoreType,
      object_type: this.rowCopy.resource_sub_type,
      restore_location: this.formGroup.value.restore_location,
      target: {
        details: [],
        env_id:
          this.formGroup.value.restore_location === RestoreLocationType.ORIGIN
            ? this.resourceObj.name
            : find(this.envIdOptions, { uuid: this.formGroup.value.env_id })
                .name,
        env_type: ''
      },
      source: {
        source_location: '',
        source_name: ''
      },
      filters: [],
      restore_objects: [],
      ext_parameters: {
        new_file_system: false,
        over_write_option: this.formGroup.value.over_write_option,
        share_mode: 3,
        share_name:
          this.formGroup.value.restore_location === RestoreLocationType.ORIGIN
            ? ''
            : this.formGroup.value.share_name,
        storage_file_system_id:
          this.formGroup.value.restore_location === RestoreLocationType.ORIGIN
            ? this.resourceObj.extendInfo?.fileSystemId
            : find(this.envIdOptions, { uuid: this.formGroup.value.env_id })
                .extendInfo?.fileSystemId
      }
    };
  }

  getHyperdetectParams() {
    return {
      restoreType: this.restoreType,
      copyId: this.rowCopy.uuid,
      targetEnv: this.resourceObj.environment?.uuid,
      targetLocation: RestoreV2LocationType.ORIGIN,
      targetObject: this.rowCopy.resource_id,
      filters: [],
      agents: [],
      extendInfo: {
        fileReplaceStrategy: this.formGroup.value.over_write_option,
        fileSystemId: this.resourceObj.extendInfo?.fileSystemId
      }
    };
  }

  restore(): Observable<void> {
    return new Observable<void>((observer: Observer<void>) => {
      if (this.formGroup.invalid) {
        return;
      }

      if (this.cookieService.isCloudBackup) {
        this.warningMessageService.create({
          content:
            this.childResType === DataMap.Resource_Type.LocalLun.value
              ? this.i18n.get('protection_lun_restore_warn_label')
              : this.i18n.get('protection_file_level_restore_warn_label'),
          onOK: () => this.createRestore(observer),
          onCancel: () => {
            observer.error(null);
            observer.complete();
          }
        });
      } else {
        this.createRestore(observer);
      }
    });
  }

  createRestore(observer: Observer<void>) {
    if (this.isHyperdetect) {
      const params = this.getHyperdetectParams();
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
    } else {
      const params = this.getParams();
      this.restoreService
        .createRestoreV1RestoresPost({ body: params })
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
    }
  }

  getTargetPath() {
    return this.resourceObj.path;
  }
}
