import {
  ChangeDetectorRef,
  Component,
  EventEmitter,
  Input,
  OnInit,
  Output
} from '@angular/core';
import {
  AbstractControl,
  FormBuilder,
  FormControl,
  FormGroup,
  ValidatorFn
} from '@angular/forms';
import { ModalRef } from '@iux/live';
import {
  BaseUtilService,
  CommonConsts,
  DataMap,
  I18NService,
  NasFileReplaceStrategy,
  ProtectedEnvironmentApiService,
  ProtectedResourceApiService,
  RestoreApiV2Service,
  RestoreType,
  RestoreV2LocationType,
  RestoreV2Type
} from 'app/shared';
import { AppUtilsService } from 'app/shared/services/app-utils.service';
import { assign, each, find, isString, set } from 'lodash';
import { Observable, Observer } from 'rxjs';

@Component({
  selector: 'aui-object-restore',
  templateUrl: './object-restore.component.html',
  styleUrls: ['./object-restore.component.less']
})
export class ObjectRestoreComponent implements OnInit {
  resourceData;
  restoreLocationType = RestoreV2LocationType;
  fileReplaceStrategy = NasFileReplaceStrategy;
  RestoreType = RestoreType;
  dataMap = DataMap;
  pageIndex = CommonConsts.PAGE_START;
  pageSize = CommonConsts.PAGE_SIZE;
  formGroup: FormGroup;
  storageOptions = [];
  bucketOptions = [];
  bucketEnable = false;

  @Input() rowCopy;
  @Input() childResType;
  @Input() restoreType;
  @Output() onStatusChange = new EventEmitter<any>();

  prefixTip = {
    invalidName: this.i18n.get(
      'protection_object_bucket_prefix_letter_tip_label'
    ),
    invalidHead: this.i18n.get(
      'protection_object_bucket_prefix_head_tip_label'
    ),
    invalidNear: this.i18n.get(
      'protection_object_bucket_prefix_near_tip_label'
    ),
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [256])
  };

  constructor(
    private i18n: I18NService,
    private fb: FormBuilder,
    private modal: ModalRef,
    private cdr: ChangeDetectorRef,
    private appUtilsService: AppUtilsService,
    private baseUtilService: BaseUtilService,
    private restoreV2Service: RestoreApiV2Service,
    private protectedResourceApiService: ProtectedResourceApiService,
    private protectedEnvironmentApiService: ProtectedEnvironmentApiService
  ) {}

  ngOnInit(): void {
    this.resourceData = isString(this.rowCopy.resource_properties)
      ? JSON.parse(this.rowCopy.resource_properties)
      : {};
    this.initForm();
    this.getStorageOptions();
  }

  initForm() {
    this.formGroup = this.fb.group({
      restoreLocation: new FormControl(RestoreV2LocationType.ORIGIN),
      originLocation: new FormControl({
        value:
          this.resourceData?.environment_name ||
          this.rowCopy?.resource_environment_name ||
          this.rowCopy?.name,
        disabled: true
      }),
      target: new FormControl(
        { value: '', disabled: true },
        {
          validators: this.baseUtilService.VALID.required()
        }
      ),
      bucketEnable: new FormControl(false),
      bucketType: new FormControl('0'),
      targetBucket: new FormControl(''),
      newBucket: new FormControl(''),
      prefix: new FormControl('', {
        validators: [
          this.validPrefix(),
          this.baseUtilService.VALID.maxLength(256)
        ]
      }),
      overwriteType: new FormControl(NasFileReplaceStrategy.Replace, {
        validators: this.baseUtilService.VALID.required()
      })
    });
    this.modal.getInstance().lvOkDisabled = false;

    this.formGroup.get('restoreLocation').valueChanges.subscribe(res => {
      if (res === RestoreV2LocationType.ORIGIN) {
        this.formGroup.get('target').disable();
      } else {
        this.formGroup.get('target').enable();
        this.getStorageOptions();
      }
    });

    if (
      this.rowCopy?.resource_status === DataMap.Resource_Status.notExist.value
    ) {
      this.formGroup.get('restoreLocation').setValue(RestoreV2LocationType.NEW);
      this.modal.getInstance().lvOkDisabled = true;
    }

    this.formGroup.statusChanges.subscribe(res => this.disableOkBtn());

    this.formGroup.get('target').valueChanges.subscribe(res => {
      if (!res) {
        return;
      }
      this.getBucketOptions(res);
    });
  }

  validPrefix(): ValidatorFn {
    return (control: AbstractControl): { [key: string]: any } | null => {
      if (!control.value) {
        return;
      }

      const reg1 = /[|:*?<>"\\]+/;
      if (reg1.test(control.value)) {
        return { invalidName: { value: control.value } };
      }
      if (control.value.startsWith('/')) {
        return { invalidHead: { value: control.value } };
      }
      if (control.value.indexOf('//') !== -1) {
        return { invalidNear: { value: control.value } };
      }

      return null;
    };
  }

  getStorageOptions() {
    const extParams = {
      conditions: JSON.stringify({
        subType: DataMap.Resource_Type.ObjectStorage.value,
        storageType: ['in', Number(this.resourceData.extendInfo.storageType)]
      })
    };
    this.appUtilsService.getResourceByRecursion(
      extParams,
      params => this.protectedResourceApiService.ListResources(params),
      resource => {
        const hostArray = [];
        each(resource, item => {
          hostArray.push({
            ...item,
            key: item.uuid,
            value: item.uuid,
            label: `${item.name}`,
            isLeaf: true
          });
        });
        this.storageOptions = hostArray;
      }
    );
  }

  getBucketOptions(res) {
    const params: any = {
      pageNo: this.pageIndex,
      pageSize: this.pageSize * 10,
      envId: res
    };

    this.protectedEnvironmentApiService
      .ListEnvironmentResource(params)
      .subscribe(res => {
        const bucketArray = [];
        each(res.records, item => {
          bucketArray.push({
            value: item.name,
            label: item.name,
            key: item.name,
            isLeaf: true
          });
        });
        this.bucketOptions = bucketArray;
        this.cdr.detectChanges();
      });
  }

  getParams() {
    let target;
    if (this.formGroup.value.restoreLocation === RestoreV2LocationType.NEW) {
      target = find(this.storageOptions, item => {
        return item.value === this.formGroup.value.target;
      });
    }
    const params = {
      copyId: this.rowCopy.uuid,
      targetEnv:
        this.formGroup.value.restoreLocation === RestoreV2LocationType.ORIGIN
          ? this.resourceData.environment_uuid ||
            this.resourceData.environment?.uuid
          : this.formGroup.value.target,
      restoreType:
        this.restoreType === RestoreType.CommonRestore
          ? RestoreV2Type.CommonRestore
          : RestoreV2Type.FileRestore,
      targetLocation: this.formGroup.value.restoreLocation,
      targetObject:
        this.formGroup.value.restoreLocation === RestoreV2LocationType.ORIGIN
          ? this.resourceData.uuid
          : target.uuid,
      extendInfo: {
        fileReplaceStrategy: String(this.formGroup.value.overwriteType)
      }
    };
    if (this.formGroup.value.bucketEnable) {
      set(
        params,
        'extendInfo.bucketName',
        this.formGroup.value.bucketType === '0'
          ? this.formGroup.value.targetBucket
          : this.formGroup.value.newBucket
      );
      set(
        params,
        'extendInfo.isNewCreateBucket',
        this.formGroup.value.bucketType === '0' ? 'false' : 'true'
      );
    }
    if (
      !!this.formGroup.value?.prefix &&
      this.formGroup.value.restoreLocation === RestoreV2LocationType.NEW
    ) {
      set(params, 'extendInfo.prefix', this.formGroup.value.prefix);
    }
    return params;
  }

  getTargetPath() {
    return this.formGroup.value.restoreLocation === RestoreV2LocationType.ORIGIN
      ? this.resourceData?.name
      : `${
          find(this.storageOptions, {
            value: this.formGroup.value.target
          })['label']
        }`;
  }

  getTargetParams() {
    return {
      ...this.formGroup.value,
      resource:
        this.formGroup.value.restoreLocation === RestoreV2LocationType.ORIGIN
          ? {
              name: this.rowCopy.resource_environment_name,
              value: this.resourceData.environment_uuid
            }
          : assign(
              {},
              find(this.storageOptions, {
                value: this.formGroup.value.target
              }),
              {
                name: find(this.storageOptions, {
                  value: this.formGroup.value.target
                })?.label
              }
            ),
      requestParams: this.getParams()
    };
  }

  restore(): Observable<void> {
    return new Observable<void>((observer: Observer<void>) => {
      const params = this.getParams();
      this.restoreV2Service
        .CreateRestoreTask({ CreateRestoreTaskRequestBody: params as any })
        .subscribe(
          res => {
            observer.next();
            observer.complete();
          },
          err => {
            observer.error(err);
            observer.complete();
          }
        );
    });
  }

  disableOkBtn() {
    if (this.restoreType !== RestoreType.FileRestore) {
      this.modal.getInstance().lvOkDisabled = this.formGroup.invalid;
    } else {
      this.onStatusChange.emit();
    }
  }
}
