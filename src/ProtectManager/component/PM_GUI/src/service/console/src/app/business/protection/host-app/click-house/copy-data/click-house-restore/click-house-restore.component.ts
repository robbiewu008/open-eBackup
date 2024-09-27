import { Component, Input, OnInit } from '@angular/core';
import { FormBuilder, FormControl, FormGroup } from '@angular/forms';
import {
  BaseUtilService,
  CommonConsts,
  DataMap,
  I18NService,
  ResourceType,
  RestoreV2LocationType
} from 'app/shared';
import {
  ProtectedResourceApiService,
  RestoreApiV2Service
} from 'app/shared/api/services';
import { defer, find, includes, isNumber, isString, map } from 'lodash';
import { Observable, Observer } from 'rxjs';

@Component({
  selector: 'aui-click-house-restore',
  templateUrl: './click-house-restore.component.html',
  styleUrls: ['./click-house-restore.component.less']
})
export class ClickHouseRestoreComponent implements OnInit {
  @Input() rowCopy;
  @Input() childResType;
  @Input() restoreType;
  filterParams = [];
  clusterOptions = [];
  dataMap = DataMap;
  restoreLocationType = RestoreV2LocationType;
  formGroup: FormGroup;
  resourceData;
  location = this.i18n.get('common_location_label');
  restoreToNewLocationOnly = false;

  constructor(
    public i18n: I18NService,
    private fb: FormBuilder,
    public baseUtilService: BaseUtilService,
    private restoreV2Service: RestoreApiV2Service,
    private protectedResourceApiService: ProtectedResourceApiService
  ) {}

  ngOnInit() {
    this.getResourceData();
    this.initForm();
    this.getClusters();
  }

  initForm() {
    this.restoreToNewLocationOnly = includes(
      [
        DataMap.CopyData_generatedType.replicate.value,
        DataMap.CopyData_generatedType.cascadedReplication.value
      ],
      this.rowCopy.generated_by
    );
    this.formGroup = this.fb.group({
      restoreTo: new FormControl(RestoreV2LocationType.ORIGIN),
      cluster: new FormControl(
        this.rowCopy.resource_id ? this.rowCopy.resource_id : ''
      )
    });
    this.watch();
    if (this.restoreToNewLocationOnly) {
      defer(() => {
        this.formGroup.patchValue({
          restoreTo: RestoreV2LocationType.NEW
        });
      });
    }
  }

  watch() {
    this.formGroup.get('restoreTo').valueChanges.subscribe(res => {
      if (res === RestoreV2LocationType.ORIGIN) {
        this.updateOld();
        this.location = this.i18n.get('common_location_label');
      } else {
        this.updateNew();
        this.location = this.i18n.get('common_target_to_cluster_label');
      }
      this.formGroup.get('cluster').updateValueAndValidity();
      this.formGroup.updateValueAndValidity();
    });
  }

  updateOld() {
    this.formGroup.get('cluster').clearValidators();
    this.formGroup
      .get('cluster')
      .setValue(this.rowCopy.resource_id ? this.rowCopy.resource_id : '');
  }

  updateNew() {
    this.formGroup.get('cluster').setValue('');
    this.formGroup
      .get('cluster')
      .setValidators([this.baseUtilService.VALID.required()]);
  }

  getResourceData() {
    this.resourceData = isString(this.rowCopy.resource_properties)
      ? JSON.parse(this.rowCopy.resource_properties)
      : {};
  }

  getClusters(recordsTemp?: any[], startPage?: number) {
    const conditions = {
      subType: [DataMap.Resource_Type.ClickHouse.value],
      type: ResourceType.CLUSTER
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
        this.clusterOptions = map(recordsTemp, item => {
          return {
            ...item,
            key: item.uuid,
            value: item.uuid,
            label: item.name,
            isLeaf: true
          };
        });

        this.formGroup.updateValueAndValidity();
        return;
      }
      this.getClusters(recordsTemp, startPage);
    });
  }

  restore(): Observable<void> {
    return new Observable<void>((observer: Observer<void>) => {
      const targetObj = find(this.clusterOptions, {
        value: this.formGroup.value.cluster
      });
      const params = {
        copyId: this.rowCopy.uuid,
        targetEnv: targetObj?.rootUuid || this.resourceData.environment_uuid,
        restoreType: this.restoreType,
        targetLocation: this.formGroup.value.restoreTo,
        targetObject: this.formGroup.value.cluster
      };
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

  getTargetParams() {
    const targetObj = find(this.clusterOptions, {
      value: this.formGroup.value.cluster
    });
    const cluster = find(this.clusterOptions, {
      value: this.formGroup.value.cluster
    });
    const params = {
      copyId: this.rowCopy.uuid,
      targetEnv: targetObj?.rootUuid || this.resourceData.environment_uuid,
      restoreType: this.restoreType,
      targetLocation: this.formGroup.value.restoreTo,
      targetObject: this.formGroup.value.cluster
    };
    return {
      ...this.formGroup.value,
      ...params,
      resource: {
        name: cluster?.label || this.resourceData.environment_name,
        label: cluster?.label || this.resourceData.environment_name
      },
      requestParams: params
    };
  }

  getTargetPath() {
    const selectedCluster = find(this.clusterOptions, {
      uuid: this.formGroup.value.cluster
    });
    return `${selectedCluster?.label || this.rowCopy.resource_location}`;
  }
}
