import { Component, Input, OnInit } from '@angular/core';
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
  extendParams,
  I18NService,
  OverWriteOption,
  RestoreType,
  RestoreV2LocationType,
  RestoreV2Type
} from 'app/shared';
import {
  ProtectedResourceApiService,
  RestoreApiV2Service
} from 'app/shared/api/services';
import { AppUtilsService } from 'app/shared/services/app-utils.service';
import {
  each,
  filter,
  find,
  get,
  includes,
  isEmpty,
  isNumber,
  isString,
  map,
  set,
  trim
} from 'lodash';
import { Observable, Observer } from 'rxjs';

@Component({
  selector: 'aui-sql-server-restore',
  templateUrl: './sql-server-restore.component.html',
  styleUrls: ['./sql-server-restore.component.less']
})
export class SQLServerRestoreComponent implements OnInit {
  @Input() rowCopy;
  @Input() childResType;
  @Input() restoreType;
  isDrill;
  copyLocation;
  hostData = [];
  instData = [];
  filterParams = [];
  options = [];
  instanceOptions = [];
  dataMap = DataMap;
  restoreLocationType = RestoreV2LocationType;
  fileReplaceStrategy = OverWriteOption;
  formGroup: FormGroup;
  resourceData;
  location = this.i18n.get('common_location_label');
  isClusterInstance = false;
  originalLocation;
  disableOriginLocation = false;

  scriptErrorTip = {
    invalidName: this.i18n.get('common_script_error_label'),
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [8192])
  };
  newNameErrorTip = {
    ...this.baseUtilService.requiredErrorTip,
    ...this.baseUtilService.nameErrorTip,
    invalidName: this.i18n.get('protection_new_name_error_tips_label'),
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [64]),
    specialNameInvaild: this.i18n.get(
      'protection_sqlserver_new_name_error_tips_label'
    )
  };

  constructor(
    public i18n: I18NService,
    private appUtilsService: AppUtilsService,
    private fb: FormBuilder,
    private modal: ModalRef,
    public baseUtilService: BaseUtilService,
    private restoreV2Service: RestoreApiV2Service,
    private protectedResourceApiService: ProtectedResourceApiService
  ) {}

  initForm() {
    this.formGroup = this.fb.group({
      restoreTo: new FormControl(RestoreV2LocationType.ORIGIN),
      originalHost: new FormControl({
        value: this.resourceData.extendInfo?.hostName,
        disabled: true
      }),
      host: new FormControl(this.resourceData.root_uuid, {
        validators: [this.baseUtilService.VALID.required()]
      }),
      instance: new FormControl(this.rowCopy.resource_id, {
        validators: [this.baseUtilService.VALID.required()]
      }),
      replaceDatabase: new FormControl(false),
      newName: new FormControl(''),
      preScript: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.maxLength(8192),
          this.baseUtilService.VALID.name(
            CommonConsts.REGEX.windowsScript,
            false
          )
        ]
      }),
      postScript: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.maxLength(8192),
          this.baseUtilService.VALID.name(
            CommonConsts.REGEX.windowsScript,
            false
          )
        ]
      }),
      executeScript: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.maxLength(8192),
          this.baseUtilService.VALID.name(
            CommonConsts.REGEX.windowsScript,
            false
          )
        ]
      })
    });

    this.watch();
    if (this.disableOriginLocation) {
      this.formGroup.get('restoreTo').setValue(RestoreV2LocationType.NEW);
    } else {
      this.formGroup.get('restoreTo').setValue(RestoreV2LocationType.ORIGIN);
    }
  }

  watch() {
    this.formGroup.get('restoreTo').valueChanges.subscribe(res => {
      this.instanceOptions = [];
      if (res === RestoreV2LocationType.ORIGIN) {
        this.formGroup.get('host').setValue(this.resourceData.environment_uuid);
        this.formGroup.get('host').disable();
        this.formGroup.get('instance').disable();
        this.location = this.i18n.get('common_location_label');
      } else {
        this.formGroup.get('host').setValue('');
        this.formGroup.get('host').enable();
        this.formGroup.get('instance').enable();
        this.location = this.i18n.get('explore_target_host_cluster_label');
      }
    });

    this.formGroup.get('host').valueChanges.subscribe(res => {
      this.formGroup.get('instance').setValue('');
      this.getInstance();
    });

    this.formGroup.get('replaceDatabase').valueChanges.subscribe(res => {
      if (res) {
        this.formGroup
          .get('newName')
          .setValidators([
            this.baseUtilService.VALID.required(),
            this.validNewName()
          ]);
      } else {
        this.formGroup.get('newName').clearValidators();
      }

      this.formGroup.get('newName').setValue('');
    });

    this.formGroup.statusChanges.subscribe(res => {
      this.disableOkBtn();
    });
  }

  getResourceData() {
    this.resourceData = isString(this.rowCopy.resource_properties)
      ? JSON.parse(this.rowCopy.resource_properties)
      : {};
  }

  getHosts(labelParams?: any) {
    const conditions = {
      type: 'Plugin',
      subType: [`${DataMap.Resource_Type.SQLServerInstance.value}Plugin`]
    };
    extendParams(conditions, labelParams);
    const extParams = {
      conditions: JSON.stringify(conditions)
    };
    this.appUtilsService.getResourceByRecursion(
      extParams,
      params => this.protectedResourceApiService.ListResources(params),
      resource => {
        resource = filter(resource, item => !isEmpty(item.environment));
        const hostArray = [];
        each(resource, item => {
          const tmp = item.environment;
          if (
            tmp.osType === DataMap.Os_Type.windows.value &&
            tmp.extendInfo.scenario === DataMap.proxyHostType.external.value
          ) {
            hostArray.push({
              ...tmp,
              key: tmp.uuid,
              value: tmp.uuid,
              label: `${tmp.name}(${tmp.endpoint})`,
              isLeaf: true
            });
          }
        });
        this.options = hostArray;
        this.updateDrillData();
      }
    );
  }

  getClusters(recordsTemp?: any[], startPage?: number) {
    const params = {
      pageNo: startPage || CommonConsts.PAGE_START,
      pageSize: CommonConsts.PAGE_SIZE,
      conditions: JSON.stringify({
        subType: DataMap.Resource_Type.SQLServerCluster.value
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
        startPage === Math.ceil(res.totalCount / CommonConsts.PAGE_SIZE) ||
        res.totalCount === 0
      ) {
        this.options = map(recordsTemp, item => {
          return {
            ...item,
            key: item.uuid,
            value: item.uuid,
            label: item.name || '',
            isLeaf: true
          };
        });

        return;
      }
      this.getClusters(recordsTemp, startPage);
    });
  }

  getInstance(recordsTemp?: any[], startPage?: number) {
    const endpoint = get(
      find(this.options, item => item.uuid === this.formGroup.value.host),
      'endpoint'
    );
    const conditions = {
      subType: [
        DataMap.Resource_Type.SQLServerInstance.value,
        DataMap.Resource_Type.SQLServerClusterInstance.value
      ],
      environment: {
        endpoint: [['~~'], endpoint]
      }
    };

    const params = {
      pageNo: startPage || CommonConsts.PAGE_START,
      pageSize: CommonConsts.PAGE_SIZE,
      conditions: JSON.stringify(conditions)
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
        startPage === Math.ceil(res.totalCount / CommonConsts.PAGE_SIZE) ||
        res.totalCount === 0
      ) {
        this.instanceOptions = map(recordsTemp, item => {
          return {
            ...item,
            key: item.uuid,
            value: item.uuid,
            label: item.name,
            isLeaf: true
          };
        });
        this.instanceOptions = filter(this.instanceOptions, item => {
          return (
            item.path === endpoint || item.environment.endpoint === endpoint
          );
        });
        return;
      }
      this.getInstance(recordsTemp, startPage);
    });
  }

  validNewName(): ValidatorFn {
    return (control: AbstractControl): { [key: string]: any } | null => {
      if (!trim(control.value)) {
        return { required: { value: control.value } };
      }

      if (
        includes(['master', 'model', 'msdb', 'tempdb'], trim(control.value))
      ) {
        return { specialNameInvaild: { value: control.value } };
      }

      return null;
    };
  }

  getParams() {
    const params = {
      copyId: this.rowCopy.uuid,
      targetEnv:
        this.formGroup.value.restoreTo === RestoreV2LocationType.ORIGIN
          ? this.resourceData.environment_uuid
          : this.formGroup.value.host,
      restoreType:
        this.restoreType === RestoreType.CommonRestore
          ? RestoreV2Type.CommonRestore
          : RestoreV2Type.FileRestore,
      targetLocation: this.formGroup.value.restoreTo,
      scripts: {
        preScript: this.formGroup.value.preScript,
        postScript: this.formGroup.value.postScript,
        failPostScript: this.formGroup.value.executeScript
      }
    };

    if (this.formGroup.value.restoreTo === RestoreV2LocationType.NEW) {
      set(params, 'targetObject', this.formGroup.value.instance);
    }

    if (this.rowCopy.backup_type === DataMap.CopyData_Backup_Type.log.value) {
      set(
        params,
        'extendInfo.restoreTimestamp',
        this.rowCopy.restoreTimeStamp || ''
      );
    }

    if (this.formGroup.value.replaceDatabase) {
      set(params, 'extendInfo.newDatabaseName', this.formGroup.value.newName);
    } else {
      set(params, 'extendInfo.newDatabaseName', '');
    }

    return params;
  }

  getTargetParams() {
    return {
      ...this.formGroup.value,
      originalHost: this.formGroup.get('originalHost')?.value,
      resource:
        this.formGroup.value.restoreTo === RestoreV2LocationType.ORIGIN
          ? {
              name: this.resourceData.environment_name,
              value: this.resourceData.environment_uuid
            }
          : {
              name: find(
                this.options,
                item => item.uuid === this.formGroup.value.host
              )['label'],
              value: this.formGroup.value.host
            },
      restoreLocation: this.formGroup.value.restoreTo,
      requestParams: this.getParams()
    };
  }

  getTargetPath() {
    return this.formGroup.value.restoreTo === RestoreV2LocationType.ORIGIN
      ? this.resourceData.environment_name
      : `${
          find(this.options, item => item.uuid === this.formGroup.value.host)[
            'label'
          ]
        }/${
          find(
            this.instanceOptions,
            item => item.uuid === this.formGroup.value.instance
          )['label']
        }`;
  }

  restore(): Observable<void> {
    return new Observable<void>((observer: Observer<void>) => {
      this.restoreV2Service
        .CreateRestoreTask({
          CreateRestoreTaskRequestBody: this.getParams() as any
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
    });
  }

  ngOnInit() {
    this.disableOriginLocation =
      this.rowCopy?.resource_status ===
        DataMap.Resource_Status.notExist.value ||
      this.rowCopy?.generated_by ===
        DataMap.CopyData_generatedType.cascadedReplication.value;

    this.getResourceData();
    this.initForm();
    if (this.resourceData.environment_is_cluster === 'False') {
      this.getHosts();
    } else {
      this.getHosts();
    }
    this.disableOkBtn();
  }

  disableOkBtn() {
    this.modal.getInstance().lvOkDisabled = this.formGroup.invalid;
  }

  updateDrillData() {
    if (this.isDrill && !isEmpty(this.rowCopy?.drillRecoveryConfig)) {
      const config = this.rowCopy?.drillRecoveryConfig;
      this.formGroup.get('host').setValue(config.targetEnv);
      this.formGroup.get('instance').setValue(config.targetObject);
      this.formGroup
        .get('replaceDatabase')
        .setValue(!isEmpty(config.extendInfo?.newDatabaseName));
      this.formGroup
        .get('newName')
        .setValue(config.extendInfo?.newDatabaseName);
    }
  }

  updateTable(event?) {
    // 根据筛选条件更新表格
    this.getHosts(event);
  }
}
