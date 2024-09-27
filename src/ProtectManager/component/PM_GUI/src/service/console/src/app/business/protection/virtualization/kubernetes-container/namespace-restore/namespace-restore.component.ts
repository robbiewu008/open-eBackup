import { Component, OnInit } from '@angular/core';
import {
  AbstractControl,
  FormArray,
  FormBuilder,
  FormControl,
  FormGroup,
  ValidatorFn
} from '@angular/forms';
import { OptionItem } from '@iux/live';
import {
  BaseUtilService,
  CommonConsts,
  DataMap,
  DataMapService,
  FileReplaceStrategy,
  I18NService,
  ProtectedResourceApiService,
  RestoreApiV2Service,
  RestoreV2LocationType
} from 'app/shared';
import { AppUtilsService } from 'app/shared/services/app-utils.service';
import {
  assign,
  defer,
  each,
  filter,
  find,
  get,
  includes,
  isEmpty,
  map,
  set
} from 'lodash';
import { Observable, Observer } from 'rxjs';

@Component({
  selector: 'aui-namespace-restore',
  templateUrl: './namespace-restore.component.html',
  styleUrls: ['./namespace-restore.component.less']
})
export class NamespaceRestoreComponent implements OnInit {
  rowCopy;
  childResType;
  restoreType;

  resource;
  formGroup: FormGroup;
  restoreToNewLocationOnly = false;
  restoreLocationType = RestoreV2LocationType;
  fileReplaceStrategy = FileReplaceStrategy;
  originalClusterOptions: OptionItem[] = [];
  originalNamespaceOptions: OptionItem[] = [];
  clusterOptions = [];
  namespaceOptions = [];
  workLoadTypeOptions = this.dataMapService
    .toArray('workLoadType')
    .filter(item => {
      item.isLeaf = true;
      return !includes(
        [DataMap.workLoadType.job.value, DataMap.workLoadType.cronJob.value],
        item.value
      );
    });
  nameErrorTip = {
    ...this.baseUtilService.requiredErrorTip,
    invalidName: this.i18n.get('protetion_work_load_name_label', [1, 50])
  };
  envValueErrorTip = {
    ...this.baseUtilService.requiredErrorTip,
    invalidName: this.i18n.get('protetion_work_load_name_label', [1, 100])
  };
  targetClusterErrorTip = {
    ...this.baseUtilService.requiredErrorTip,
    invalidTargetCluster: this.i18n.get(
      'protection_k8s_cluster_version_error_label'
    )
  };

  constructor(
    public baseUtilService: BaseUtilService,
    private fb: FormBuilder,
    private i18n: I18NService,
    private dataMapService: DataMapService,
    private appUtilsService: AppUtilsService,
    private restoreV2Service: RestoreApiV2Service,
    private protectedResourceApiService: ProtectedResourceApiService
  ) {}

  ngOnInit() {
    this.initResource();
    this.initForm();
  }

  initResource() {
    this.resource = JSON.parse(this.rowCopy?.resource_properties || '{}');
    this.restoreToNewLocationOnly = includes(
      [
        DataMap.CopyData_generatedType.replicate.value,
        DataMap.CopyData_generatedType.cascadedReplication.value
      ],
      this.rowCopy.generated_by
    );
    defer(() => {
      this.originalClusterOptions = [
        {
          label: this.resource.environment_name,
          value: this.resource.environment_uuid,
          isLeaf: true
        }
      ];
      if (
        this.rowCopy.resource_sub_type ===
        DataMap.Resource_Type.kubernetesNamespaceCommon.value
      ) {
        this.originalNamespaceOptions = [
          {
            label: this.resource.name,
            value: this.resource.uuid,
            isLeaf: true
          }
        ];
      } else {
        this.originalNamespaceOptions = [
          {
            label: this.resource.parent_name,
            value: this.resource.parent_uuid,
            isLeaf: true
          }
        ];
      }
    });
  }

  getEnvVariablesFormGroup(addValid?) {
    return this.fb.group({
      envKey: new FormControl('', {
        validators: addValid
          ? [
              this.baseUtilService.VALID.required(),
              this.baseUtilService.VALID.name(CommonConsts.REGEX.envValue)
            ]
          : null
      }),
      envValue: new FormControl('', {
        validators: addValid
          ? [
              this.baseUtilService.VALID.required(),
              this.baseUtilService.VALID.name(CommonConsts.REGEX.envValue)
            ]
          : null
      })
    });
  }

  getEnvConfigFormGroup(addValid?) {
    return this.fb.group({
      workLoadType: new FormControl('', {
        validators: addValid ? [this.baseUtilService.VALID.required()] : null
      }),
      workLoadName: new FormControl('', {
        validators: addValid
          ? [
              this.baseUtilService.VALID.required(),
              this.baseUtilService.VALID.name(CommonConsts.REGEX.workloadName)
            ]
          : null
      }),
      containerName: new FormControl('', {
        validators: addValid
          ? [
              this.baseUtilService.VALID.required(),
              this.baseUtilService.VALID.name(CommonConsts.REGEX.workloadName)
            ]
          : null
      }),
      envVariables: this.fb.array([this.getEnvVariablesFormGroup(addValid)])
    });
  }

  getScConfigFormGroup(addScValid?) {
    return this.fb.group({
      scName: new FormControl('', {
        validators: addScValid ? [this.baseUtilService.VALID.required()] : null
      }),
      scParameterMap: this.fb.array([this.getEnvVariablesFormGroup(addScValid)])
    });
  }

  initForm() {
    this.formGroup = this.fb.group({
      restoreTo: new FormControl(RestoreV2LocationType.ORIGIN),
      originalCluster: new FormControl(this.resource.environment_uuid),
      originalNamespace: new FormControl(
        this.rowCopy.resource_sub_type ===
        DataMap.Resource_Type.kubernetesNamespaceCommon.value
          ? this.resource.uuid
          : this.resource.parent_uuid
      ),
      targetCluster: new FormControl(''),
      targetNamespace: new FormControl(''),
      overwriteType: new FormControl(FileReplaceStrategy.Replace),
      changeEnv: new FormControl(false),
      changeSc: new FormControl(false),
      envConfigs: this.fb.array([this.getEnvConfigFormGroup()]),
      scConfigs: this.fb.array([this.getScConfigFormGroup()])
    });

    this.formGroup.get('restoreTo').valueChanges.subscribe(res => {
      if (res === RestoreV2LocationType.ORIGIN) {
        this.formGroup.get('targetCluster').clearValidators();
        this.formGroup.get('targetNamespace').clearValidators();
      } else {
        this.formGroup
          .get('targetCluster')
          .setValidators([
            this.baseUtilService.VALID.required(),
            this.validTargetCluster()
          ]);
        this.formGroup
          .get('targetNamespace')
          .setValidators([this.baseUtilService.VALID.required()]);
        this.getCluster();
      }
      this.formGroup.get('targetCluster').updateValueAndValidity();
      this.formGroup.get('targetNamespace').updateValueAndValidity();
    });

    this.formGroup.get('targetCluster').valueChanges.subscribe(res => {
      this.getNameSpace(res);
      this.updateTargetClusterErrorTip(res);
    });

    this.formGroup.get('changeEnv').valueChanges.subscribe(res => {
      if (res) {
        each((this.formGroup.get('envConfigs') as FormArray).controls, form => {
          form
            .get('workLoadType')
            .setValidators([this.baseUtilService.VALID.required()]);
          form
            .get('workLoadName')
            .setValidators([
              this.baseUtilService.VALID.required(),
              this.baseUtilService.VALID.name(CommonConsts.REGEX.workloadName)
            ]);
          form
            .get('containerName')
            .setValidators([
              this.baseUtilService.VALID.required(),
              this.baseUtilService.VALID.name(CommonConsts.REGEX.workloadName)
            ]);
          each(
            (form.get('envVariables') as FormArray).controls,
            variablesForm => {
              variablesForm
                .get('envKey')
                .setValidators([
                  this.baseUtilService.VALID.required(),
                  this.baseUtilService.VALID.name(CommonConsts.REGEX.envValue)
                ]);
              variablesForm
                .get('envValue')
                .setValidators([
                  this.baseUtilService.VALID.required(),
                  this.baseUtilService.VALID.name(CommonConsts.REGEX.envValue)
                ]);
              variablesForm.get('envKey').updateValueAndValidity();
              variablesForm.get('envValue').updateValueAndValidity();
            }
          );
          form.get('workLoadType').updateValueAndValidity();
          form.get('workLoadName').updateValueAndValidity();
          form.get('containerName').updateValueAndValidity();
        });
      } else {
        each((this.formGroup.get('envConfigs') as FormArray).controls, form => {
          form.get('workLoadType').clearValidators();
          form.get('workLoadName').clearValidators();
          form.get('containerName').clearValidators();
          each(
            (form.get('envVariables') as FormArray).controls,
            variablesForm => {
              variablesForm.get('envKey').clearValidators();
              variablesForm.get('envValue').clearValidators();
              variablesForm.get('envKey').updateValueAndValidity();
              variablesForm.get('envValue').updateValueAndValidity();
            }
          );
          form.get('workLoadType').updateValueAndValidity();
          form.get('workLoadName').updateValueAndValidity();
          form.get('containerName').updateValueAndValidity();
        });
      }
    });

    this.formGroup.get('changeSc').valueChanges.subscribe(res => {
      if (res) {
        each((this.formGroup.get('scConfigs') as FormArray).controls, form => {
          form
            .get('scName')
            .setValidators([
              this.baseUtilService.VALID.required(),
              this.baseUtilService.VALID.name(CommonConsts.REGEX.workloadName)
            ]);
          each(
            (form.get('scParameterMap') as FormArray).controls,
            variablesForm => {
              variablesForm
                .get('envKey')
                .setValidators([
                  this.baseUtilService.VALID.required(),
                  this.baseUtilService.VALID.name(CommonConsts.REGEX.envValue)
                ]);
              variablesForm
                .get('envValue')
                .setValidators([
                  this.baseUtilService.VALID.required(),
                  this.baseUtilService.VALID.name(CommonConsts.REGEX.envValue)
                ]);
              variablesForm.get('envKey').updateValueAndValidity();
              variablesForm.get('envValue').updateValueAndValidity();
            }
          );
          form.get('scName').updateValueAndValidity();
        });
      } else {
        each((this.formGroup.get('scConfigs') as FormArray).controls, form => {
          form.get('scName').clearValidators();
          each(
            (form.get('scParameterMap') as FormArray).controls,
            variablesForm => {
              variablesForm.get('envKey').clearValidators();
              variablesForm.get('envValue').clearValidators();
              variablesForm.get('envKey').updateValueAndValidity();
              variablesForm.get('envValue').updateValueAndValidity();
            }
          );
          form.get('scName').updateValueAndValidity();
        });
      }
    });

    if (this.restoreToNewLocationOnly) {
      defer(() =>
        this.formGroup.get('restoreTo').setValue(RestoreV2LocationType.NEW)
      );
    }
  }

  validTargetCluster(): ValidatorFn {
    return (control: AbstractControl): { [key: string]: any } | null => {
      if (!control.value) {
        return null;
      }

      // 新位置恢复限制：相同大版本间的同类型集群
      const originCluster: any = filter(
        this.clusterOptions,
        item => item.value === this.resource.environment_uuid
      );
      const clusterType = get(originCluster[0]?.extendInfo, 'clusterType', '');
      const version = get(originCluster[0], 'version', '')
        .split('.')
        .slice(0, 2)
        .join('.');
      const targetCluser = find(this.clusterOptions, { value: control.value });
      if (
        targetCluser?.extendInfo?.clusterType !== clusterType ||
        targetCluser.version
          .split('.')
          .slice(0, 2)
          .join('.') !== version
      ) {
        return { invalidTargetCluster: { value: control.value } };
      }
      return null;
    };
  }

  updateTargetClusterErrorTip(res) {
    const originCluster: any = filter(
      this.clusterOptions,
      item => item.value === this.resource.environment_uuid
    );
    const targetCluser = find(this.clusterOptions, { value: res });
    this.targetClusterErrorTip.invalidTargetCluster = this.i18n.get(
      'protection_k8s_cluster_version_error_label',
      [originCluster[0]?.name, targetCluser?.name]
    );
  }

  get envConfigs() {
    return (this.formGroup.get('envConfigs') as FormArray).controls;
  }

  addConfig() {
    (this.formGroup.get('envConfigs') as FormArray).push(
      this.getEnvConfigFormGroup(true)
    );
  }

  deleteConfig(i) {
    if (this.envConfigs.length === 1) {
      return;
    }
    (this.formGroup.get('envConfigs') as FormArray).removeAt(i);
  }

  addEnvVariable(i) {
    ((this.formGroup.get('envConfigs') as FormArray).controls[i].get(
      'envVariables'
    ) as FormArray).push(this.getEnvVariablesFormGroup(true));
  }

  deleteEnvVariable(i, j) {
    ((this.formGroup.get('envConfigs') as FormArray).controls[i].get(
      'envVariables'
    ) as FormArray).removeAt(j);
  }

  get scConfigs() {
    return (this.formGroup.get('scConfigs') as FormArray).controls;
  }

  addScConfig() {
    (this.formGroup.get('scConfigs') as FormArray).push(
      this.getScConfigFormGroup(true)
    );
  }

  deleteScConfig(i) {
    if (this.scConfigs.length === 1) {
      return;
    }
    (this.formGroup.get('scConfigs') as FormArray).removeAt(i);
  }

  addScParams(i) {
    ((this.formGroup.get('scConfigs') as FormArray).controls[i].get(
      'scParameterMap'
    ) as FormArray).push(this.getEnvVariablesFormGroup(true));
  }

  deleteScParams(i, j) {
    ((this.formGroup.get('scConfigs') as FormArray).controls[i].get(
      'scParameterMap'
    ) as FormArray).removeAt(j);
  }

  getOptions(subType, params?) {
    const extParams = {
      conditions: JSON.stringify({
        subType: [subType],
        ...params
      })
    };
    this.appUtilsService.getResourceByRecursion(
      extParams,
      params => this.protectedResourceApiService.ListResources(params),
      cluster => {
        let options = map(cluster, item => {
          return assign(item, {
            label: item.name,
            value: item.uuid,
            isLeaf: true
          });
        });
        if (subType === DataMap.Resource_Type.kubernetesClusterCommon.value) {
          this.clusterOptions = options;
        } else {
          this.namespaceOptions = options;
        }
      }
    );
  }

  getCluster() {
    if (!isEmpty(this.clusterOptions)) {
      return;
    }
    this.getOptions(DataMap.Resource_Type.kubernetesClusterCommon.value);
  }

  getNameSpace(parentUuid) {
    if (!parentUuid) {
      return;
    }
    this.getOptions(DataMap.Resource_Type.kubernetesNamespaceCommon.value, {
      parentUuid
    });
  }

  getParams() {
    const params = {
      copyId: this.rowCopy?.uuid,
      restoreType: this.restoreType,
      targetEnv:
        this.formGroup.value.restoreTo === RestoreV2LocationType.ORIGIN
          ? this.formGroup.value.originalCluster
          : this.formGroup.value.targetCluster,
      targetLocation: this.formGroup.value.restoreTo,
      targetObject:
        this.formGroup.value.restoreTo === RestoreV2LocationType.ORIGIN
          ? this.resource?.name
          : this.formGroup.value.targetNamespace,
      extendInfo: {
        restoreOption: this.formGroup.value.overwriteType,
        isEnableChangeEnv: this.formGroup.value.changeEnv,
        isEnableChangeScParameter: this.formGroup.value.changeSc
      }
    };
    const advancedConfigReqList = [];
    if (this.formGroup.value.changeEnv) {
      each(this.formGroup.value.envConfigs, item => {
        advancedConfigReqList.push({
          workLoadType: item.workLoadType,
          workLoadName: item.workLoadName,
          containerName: item.containerName,
          envMap: map(item.envVariables, env => {
            return {
              key: env.envKey,
              value: env.envValue
            };
          })
        });
      });
      assign(params.extendInfo, {
        advancedConfigReqList: JSON.stringify(advancedConfigReqList)
      });
    }
    const scParameterList = [];
    if (this.formGroup.value.changeSc) {
      each(this.formGroup.value.scConfigs, item => {
        let paramsMap = {};
        each(item.scParameterMap, item => {
          set(paramsMap, item.envKey, item.envValue);
        });
        scParameterList.push({
          scName: item.scName,
          paramsMap: paramsMap
        });
      });
      assign(params.extendInfo, {
        scParameterList: JSON.stringify(scParameterList)
      });
    }
    return params;
  }

  restore(): Observable<void> {
    return new Observable<void>((observer: Observer<void>) => {
      const params = this.getParams();
      this.restoreV2Service
        .CreateRestoreTask({ CreateRestoreTaskRequestBody: params })
        .subscribe(
          () => {
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
}
