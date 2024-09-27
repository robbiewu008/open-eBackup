import { Component, OnInit, Output, EventEmitter, Input } from '@angular/core';
import { FormGroup, FormBuilder, FormControl } from '@angular/forms';
import {
  BaseUtilService,
  I18NService,
  CommonConsts,
  DataMap,
  ProtectedResourceApiService,
  RestoreApiV2Service,
  RestoreV2LocationType,
  AgentsSubType,
  HdfsFilesetReplaceOptions,
  ProtectedEnvironmentApiService
} from 'app/shared';
import { Observable, Observer } from 'rxjs';
import {
  each,
  isNumber,
  filter,
  assign,
  map,
  toString,
  isEmpty,
  find,
  size,
  intersectionWith,
  split,
  last,
  isEqual,
  includes
} from 'lodash';
import { MessageService } from '@iux/live';

@Component({
  selector: 'aui-backup-set-restore',
  templateUrl: './backup-set-restore.component.html',
  styleUrls: ['./backup-set-restore.component.less']
})
export class BackupSetRestoreComponent implements OnInit {
  agents = [];
  dataMap = DataMap;
  clusterOptions = [];
  namespaceOptions = [];
  restoreToNewLocationOnly;
  formGroup: FormGroup;
  restoreLocationType = RestoreV2LocationType;
  directoryErrorTip = {
    ...this.baseUtilService.requiredErrorTip,
    invalidName: this.i18n.get('common_path_error_label')
  };
  filesetReplaceOptions = HdfsFilesetReplaceOptions;

  @Input() rowCopy;
  @Input() childResType;
  @Input() restoreType;
  @Output() restoreParamsChange = new EventEmitter();

  constructor(
    private fb: FormBuilder,
    private i18n: I18NService,
    private messageService: MessageService,
    private restoreV2Service: RestoreApiV2Service,
    private protectedResourceApiService: ProtectedResourceApiService,
    private protectedEnvironmentApiService: ProtectedEnvironmentApiService,
    public baseUtilService: BaseUtilService
  ) {}

  ngOnInit() {
    this.getAgents();
    this.getClusters();
    this.initForm();
    this.updateData();
  }

  initForm() {
    this.formGroup = this.fb.group({
      restoreTo: new FormControl(this.restoreLocationType.ORIGIN),
      cluster: new FormControl(
        this.rowCopy.environment_uuid ? this.rowCopy.environment_uuid : ''
      ),
      temporary_directory: new FormControl(''),
      namespace: new FormControl(''),
      originalType: new FormControl(this.filesetReplaceOptions.Skip)
    });
    setTimeout(() => {
      if (this.rowCopy.backup_type === DataMap.CopyData_Backup_Type.log.value) {
        this.formGroup
          .get('temporary_directory')
          .setValidators([
            this.baseUtilService.VALID.name(CommonConsts.REGEX.linuxPath, true)
          ]);
        this.formGroup.get('temporary_directory').updateValueAndValidity();
      }
    }, 500);
    this.formGroup.get('cluster').valueChanges.subscribe(res => {
      setTimeout(() => {
        if (
          !res ||
          this.formGroup.value.restoreTo === this.restoreLocationType.ORIGIN
        ) {
          return;
        }
        this.getNameSpace(res);
      }, 0);
    });
    this.restoreToNewLocationOnly =
      includes(
        [
          DataMap.CopyData_generatedType.replicate.value,
          DataMap.CopyData_generatedType.cascadedReplication.value
        ],
        this.rowCopy.generated_by
      ) || this.rowCopy.is_replicated;
    this.listenFormGroup();
  }

  updateData() {
    if (this.restoreToNewLocationOnly) {
      setTimeout(() => {
        this.formGroup.get('restoreTo').setValue(this.restoreLocationType.NEW);
      }, 100);
    }
  }

  getNameSpace(clusterId, recordsTemp?, startPage?) {
    const params = {
      pageNo: startPage || CommonConsts.PAGE_START + 1,
      pageSize: CommonConsts.PAGE_SIZE,
      envId: clusterId,
      parentId: clusterId,
      resourceType: DataMap.Resource_Type.HBaseNameSpace.value
    };
    this.protectedEnvironmentApiService
      .ListEnvironmentResource(params)
      .subscribe(res => {
        if (!recordsTemp) {
          recordsTemp = [];
        }
        if (!isNumber(startPage)) {
          startPage = CommonConsts.PAGE_START + 1;
        }
        startPage++;
        recordsTemp = [...recordsTemp, ...res.records];
        if (
          startPage ===
            Math.ceil(res.totalCount / CommonConsts.PAGE_SIZE) + 1 ||
          res.totalCount === 0
        ) {
          const clusterArray = [];
          each(recordsTemp, item => {
            clusterArray.push({
              ...item,
              key: item.uuid,
              value: item.name,
              label: item.extendInfo.nameSpace,
              isLeaf: true
            });
          });
          this.namespaceOptions = clusterArray;
          return;
        }
        this.getNameSpace(clusterId, recordsTemp, startPage);
      });
  }

  listenFormGroup() {
    this.formGroup.get('restoreTo').valueChanges.subscribe(res => {
      if (res === this.restoreLocationType.ORIGIN) {
        this.formGroup.get('cluster').clearValidators();
        this.formGroup.get('namespace').clearValidators();
        this.formGroup
          .get('cluster')
          .setValue(
            this.rowCopy.environment_uuid ? this.rowCopy.environment_uuid : ''
          );
        this.formGroup.get('namespace').setValue('');
        this.namespaceOptions = [];
      } else {
        this.formGroup
          .get('cluster')
          .setValidators([this.baseUtilService.VALID.required()]);
        this.formGroup.get('cluster').setValue('');
        this.formGroup.get('namespace').setValue('');
      }
      this.getClusters();
      this.formGroup.get('cluster').updateValueAndValidity();
    });

    this.formGroup.statusChanges.subscribe(res => {
      this.restoreParamsChange.emit(res === 'VALID');
    });
  }

  getAgents(recordsTemp?, startPage?) {
    this.protectedResourceApiService
      .ListResources({
        pageSize: 200,
        pageNo: startPage || 0,
        conditions: JSON.stringify({
          type: 'Plugin',
          subType:
            this.rowCopy.resource_sub_type ===
            DataMap.Resource_Type.NASFileSystem.value
              ? [AgentsSubType.NasFileSystem]
              : [AgentsSubType.NasShare],
          environment: {
            linkStatus: [
              ['in'],
              toString(DataMap.resource_LinkStatus.normal.value)
            ]
          }
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
          this.agents = recordsTemp;
          return;
        }
        this.getAgents(recordsTemp, startPage);
      });
  }

  getClusters(recordsTemp?, startPage?) {
    const params = {
      pageNo: startPage || CommonConsts.PAGE_START,
      pageSize: CommonConsts.PAGE_SIZE,
      conditions: JSON.stringify({
        subType: DataMap.Resource_Type.HBase.value
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
        const clusterArray = [];
        each(recordsTemp, item => {
          clusterArray.push({
            key: item.uuid,
            value: item.uuid,
            label: item.name,
            isLeaf: true
          });
        });
        this.clusterOptions = clusterArray;
        return;
      }
      this.getClusters(recordsTemp, startPage);
    });
  }

  getComponent() {
    const requestParams = this.formGroup.value;
    const recoveryOptions = [
      {
        label: this.i18n.get('protection_restore_to_label'),
        value:
          this.formGroup.value.restoreTo === this.restoreLocationType.ORIGIN
            ? this.i18n.get('common_restore_to_origin_location_label')
            : this.i18n.get('common_restore_to_new_location_label')
      },
      {
        label: this.i18n.get('common_location_label'),
        value: this.formGroup.value.cluster
      }
    ];

    return {
      recoveryOptions: filter(recoveryOptions, item => {
        return this.formGroup.value.restoreTo ===
          this.restoreLocationType.ORIGIN
          ? item.label === this.i18n.get('protection_restore_to_label')
          : item;
      }),
      requestParams
    };
  }

  getTargetParams() {
    const params = {
      copyId: this.rowCopy.uuid,
      restoreType: this.restoreType,
      targetLocation: this.formGroup.value.restoreTo,
      targetEnv:
        this.formGroup.value.restoreTo === this.restoreLocationType.ORIGIN
          ? this.rowCopy.environment_uuid
          : this.formGroup.value.cluster,
      targetObject: isEmpty(this.formGroup.value.namespace)
        ? this.rowCopy.resource_id
        : this.formGroup.value.namespace,
      agents: [],
      extendInfo: {
        restoreOption: this.formGroup.value.originalType
      }
    };
    if (this.rowCopy.backup_type === DataMap.CopyData_Backup_Type.log.value) {
      assign(params.extendInfo, {
        temporary_directory: this.formGroup.value.temporary_directory
      });
    }
    const namespace = find(this.namespaceOptions, {
      value: this.formGroup.value.namespace
    });
    const cluster = find(this.clusterOptions, {
      value: this.formGroup.value.cluster
    });
    return {
      ...this.formGroup.value,
      resource:
        this.formGroup.value.restoreTo === this.restoreLocationType.NEW
          ? namespace
            ? {
                ...namespace,
                label: `${cluster?.label}${namespace.name}`
              }
            : {
                label: cluster?.label
              }
          : assign({}, cluster, {
              name: cluster?.label,
              label: cluster?.label
            }),
      requestParams: params
    };
  }

  getTargetPath() {
    return find(this.clusterOptions, {
      value: this.formGroup.value.cluster
    })?.label;
  }

  restore(body): Observable<void> {
    return new Observable<void>((observer: Observer<void>) => {
      if (this.formGroup.invalid) {
        return;
      }
      let tables = split(
        JSON.parse(this.rowCopy.resource_properties).extendInfo?.table,
        ','
      );
      tables = filter(tables, item => {
        return size(split(item, '/')) > 2;
      });
      if (
        DataMap.Resource_Type.HBaseBackupSet.value === this.childResType &&
        !isEmpty(this.formGroup.value.namespace) &&
        size(
          intersectionWith(tables, (a, b) => {
            return last(split(a, '/')) === last(split(b, '/'));
          })
        ) !== size(tables) &&
        size(tables) >= 2
      ) {
        this.messageService.error(
          this.i18n.get('common_same_table_to_namespace_label'),
          {
            lvShowCloseButton: true,
            lvMessageKey: 'resSameTableMesageKey'
          }
        );
        observer.error('');
        observer.complete();
        return;
      }
      const params = {
        copyId: this.rowCopy.uuid,
        restoreType: this.restoreType,
        targetLocation: this.formGroup.value.restoreTo,
        targetEnv:
          this.formGroup.value.restoreTo === this.restoreLocationType.ORIGIN
            ? this.rowCopy.environment_uuid
            : this.formGroup.value.cluster,
        targetObject: isEmpty(this.formGroup.value.namespace)
          ? this.rowCopy.resource_id
          : this.formGroup.value.namespace,
        agents: [],
        extendInfo: {
          restoreOption: this.formGroup.value.originalType
        }
      };
      if (this.rowCopy.backup_type === DataMap.CopyData_Backup_Type.log.value) {
        assign(params, {
          extendInfo: {
            restoreTimestamp: this.rowCopy.restoreTimeStamp,
            restoreOption: this.formGroup.value.originalType,
            temporary_directory: this.formGroup.value.temporary_directory
          }
        });
      }

      this.restoreV2Service
        .CreateRestoreTask({ CreateRestoreTaskRequestBody: params })
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
}
