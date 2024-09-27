import { Component, Input, OnInit } from '@angular/core';
import { FormBuilder, FormControl, FormGroup } from '@angular/forms';
import { ModalRef } from '@iux/live';
import {
  BaseUtilService,
  CommonConsts,
  DataMap,
  InstanceType,
  RestoreV2LocationType,
  RestoreV2Type,
  VmFileReplaceStrategy
} from 'app/shared';
import {
  ProtectedResourceApiService,
  RestoreApiV2Service
} from 'app/shared/api/services';
import {
  assign,
  each,
  filter,
  find,
  isEmpty,
  isNumber,
  isString,
  set
} from 'lodash';
import { Observable, Observer } from 'rxjs';

@Component({
  selector: 'aui-saphana-restore',
  templateUrl: './saphana-restore.component.html',
  styleUrls: ['./saphana-restore.component.less']
})
export class SaphanaRestoreComponent implements OnInit {
  resourceData;
  targetOptions = [];
  databaseOptions = [];
  formGroup: FormGroup;
  restoreLocationType = RestoreV2LocationType;
  fileReplaceStrategy = VmFileReplaceStrategy;

  @Input() rowCopy;
  @Input() childResType;
  @Input() restoreType;

  constructor(
    private fb: FormBuilder,
    private modal: ModalRef,
    private baseUtilService: BaseUtilService,
    private restoreV2Service: RestoreApiV2Service,
    private protectedResourceApiService: ProtectedResourceApiService
  ) {}

  ngOnInit() {
    this.resourceData = isString(this.rowCopy.resource_properties)
      ? JSON.parse(this.rowCopy.resource_properties)
      : {};
    this.initForm();
    this.getInstanceOptions();
  }

  initForm() {
    this.formGroup = this.fb.group({
      restoreLocation: new FormControl(RestoreV2LocationType.ORIGIN),
      originLocation: new FormControl({
        value: this.resourceData?.name,
        disabled: true
      }),
      target: new FormControl(
        { value: '', disabled: true },
        {
          validators: this.baseUtilService.VALID.required()
        }
      ),
      database: new FormControl(
        { value: '', disabled: true },
        {
          validators: this.baseUtilService.VALID.required()
        }
      )
    });

    this.listenForm();
    this.modal.getInstance().lvOkDisabled = false;
    if (
      this.rowCopy?.resource_status === DataMap.Resource_Status.notExist.value
    ) {
      this.formGroup.get('restoreLocation').setValue(RestoreV2LocationType.NEW);
    }
  }

  listenForm() {
    this.formGroup.statusChanges.subscribe(res => this.disableOkBtn());

    this.formGroup.get('restoreLocation').valueChanges.subscribe(res => {
      if (res === RestoreV2LocationType.ORIGIN) {
        this.formGroup.get('target').disable();
        this.formGroup.get('database').disable();
      } else {
        this.formGroup.get('target').enable();
        this.formGroup.get('database').enable();
      }
    });

    this.formGroup.get('target').valueChanges.subscribe(res => {
      this.formGroup.get('database').setValue('');
      this.databaseOptions = [];

      if (isEmpty(res)) {
        return;
      }

      this.getDatabaseOptions(res);
    });
  }

  getDatabaseOptions(uuid, recordsTemp?, startPage?) {
    const params = {
      pageNo: startPage || CommonConsts.PAGE_START,
      pageSize: CommonConsts.PAGE_SIZE * 10,
      conditions: JSON.stringify({
        parentUuid: uuid,
        subType: [DataMap.Resource_Type.saphanaDatabase.value]
      })
    };

    this.protectedResourceApiService.ListResources(params).subscribe(res => {
      if (!recordsTemp) {
        recordsTemp = [];
      }
      if (!isNumber(startPage)) {
        startPage = CommonConsts.PAGE_START;
      }
      startPage++;
      recordsTemp = [...recordsTemp, ...res.records];
      if (
        startPage ===
          Math.ceil(res.totalCount / (CommonConsts.PAGE_SIZE * 10)) ||
        res.totalCount === 0
      ) {
        const databaseArray = [];

        each(recordsTemp, item => {
          databaseArray.push({
            ...item,
            key: item.uuid,
            value: item.uuid,
            label: item.name,
            isLeaf: true
          });
        });
        this.databaseOptions = filter(
          databaseArray,
          item =>
            item.uuid !== this.resourceData.uuid &&
            item.extendInfo?.sapHanaDbType ===
              this.resourceData.extendInfo?.sapHanaDbType
        );
        return;
      }
      this.getDatabaseOptions(uuid, recordsTemp, startPage);
    });
  }

  getInstanceOptions(recordsTemp?, startPage?) {
    const params = {
      pageNo: startPage || CommonConsts.PAGE_START,
      pageSize: CommonConsts.PAGE_SIZE * 10,
      conditions: JSON.stringify({
        subType: [DataMap.Resource_Type.saphanaInstance.value],
        isTopInstance: InstanceType.TopInstance
      })
    };

    this.protectedResourceApiService.ListResources(params).subscribe(res => {
      if (!recordsTemp) {
        recordsTemp = [];
      }
      if (!isNumber(startPage)) {
        startPage = CommonConsts.PAGE_START;
      }
      startPage++;
      recordsTemp = [...recordsTemp, ...res.records];
      if (
        startPage ===
          Math.ceil(res.totalCount / (CommonConsts.PAGE_SIZE * 10)) ||
        res.totalCount === 0
      ) {
        const instanceArray = [];

        each(recordsTemp, item => {
          instanceArray.push({
            ...item,
            key: item.uuid,
            value: item.uuid,
            label: item.name,
            isLeaf: true
          });
        });
        this.targetOptions = instanceArray;
        return;
      }
      this.getInstanceOptions(recordsTemp, startPage);
    });
  }

  getParams() {
    const params = {
      copyId: this.rowCopy.uuid,
      targetEnv:
        this.formGroup.value.restoreLocation === RestoreV2LocationType.ORIGIN
          ? this.resourceData?.parentUuid || this.resourceData?.parent_uuid
          : this.formGroup.value.target,
      restoreType:
        this.restoreType === RestoreV2Type.CommonRestore
          ? RestoreV2Type.CommonRestore
          : RestoreV2Type.FileRestore,
      targetLocation: this.formGroup.value.restoreLocation,
      targetObject:
        this.formGroup.value.restoreLocation === RestoreV2LocationType.ORIGIN
          ? this.resourceData?.uuid
          : this.formGroup.value.database
    };

    if (this.rowCopy.backup_type === DataMap.CopyData_Backup_Type.log.value) {
      assign(params, {
        extendInfo: {
          restoreTimestamp: this.rowCopy.restoreTimeStamp
        }
      });
    }
    return params;
  }

  getTargetPath() {
    return this.formGroup.value.restoreLocation === RestoreV2LocationType.ORIGIN
      ? this.resourceData?.name
      : `${
          find(this.databaseOptions, {
            value: this.formGroup.value.database
          })['label']
        }`;
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
    this.modal.getInstance().lvOkDisabled = this.formGroup.invalid;
  }
}
