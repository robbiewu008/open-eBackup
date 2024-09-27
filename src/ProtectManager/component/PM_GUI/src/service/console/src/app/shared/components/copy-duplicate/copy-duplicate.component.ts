import { Component, OnInit, TemplateRef, ViewChild } from '@angular/core';
import { FormBuilder, FormControl, FormGroup } from '@angular/forms';
import { MessageService, OptionItem } from '@iux/live';
import { TargetClusterComponent } from 'app/business/system/infrastructure/cluster-management/target-cluster/target-cluster.component';
import {
  BaseUtilService,
  ClustersApiService,
  CommonConsts,
  CookieService,
  DataMap,
  DataMapService,
  I18NService,
  MODAL_COMMON,
  MultiCluster,
  NasDistributionStoragesApiService,
  PolicyType,
  ReplicationApiService,
  ReplicationModeType,
  StorageUserAuthService,
  UsersApiService,
  WarningMessageService
} from 'app/shared';
import { DrawModalService } from 'app/shared/services/draw-modal.service';
import { InfoMessageService } from 'app/shared/services/info-message.service';
import {
  assign,
  filter,
  find,
  isEmpty,
  isNumber,
  map,
  toString as _toString
} from 'lodash';
import { Observable, Observer } from 'rxjs';

@Component({
  selector: 'aui-copy-duplicate',
  templateUrl: './copy-duplicate.component.html',
  styleUrls: ['./copy-duplicate.component.less']
})
export class CopyDuplicateComponent implements OnInit {
  rowItem: any;
  formGroup: FormGroup;
  externalOption: any[] = [];
  externalStorage: any[] = [];
  _find = find;
  dataMap = DataMap;
  specifyUserOptionsMap = {};
  isAuth = false;
  externalStorageMap = {};
  externalStorageUnitMap = {};
  backupStorageTypesAll = this.dataMapService.toArray('backupStorageTypeSla');
  backupStorageTypesWithoutGroup = this.backupStorageTypesAll.filter(type => {
    return !(type.value === DataMap.backupStorageTypeSla.group.value);
  });
  isDwsCopy = false;
  isHcsUser = this.cookieService.get('userType') === CommonConsts.HCS_USER_TYPE;
  isHcsCrossCloud = false;
  slaProperties;
  vdcTenantOptions = [];

  retentionDurationErrorTip = assign({}, this.baseUtilService.rangeErrorTip, {
    invalidRang: this.i18n.get('common_valid_rang_label', [1, 365])
  });

  retentionDurations = this.dataMapService
    .toArray('Interval_Unit')
    .filter((v: OptionItem) => {
      return v.value !== 'm' && v.value !== 'h';
    })
    .filter((v: OptionItem) => {
      return (v.isLeaf = true);
    });

  @ViewChild('tipTpl', { static: false }) tipTpl: TemplateRef<any>;

  constructor(
    private fb: FormBuilder,
    public i18n: I18NService,
    private cookieService: CookieService,
    public dataMapService: DataMapService,
    public baseUtilService: BaseUtilService,
    private drawModalService: DrawModalService,
    public infoMessageService: InfoMessageService,
    private clusterApiService: ClustersApiService,
    public warningMessageService: WarningMessageService,
    private replicationApiService: ReplicationApiService,
    private usersApiService: UsersApiService,
    private messageService: MessageService,
    private storageUserAuthService: StorageUserAuthService,
    private nasDistributionStoragesApiService: NasDistributionStoragesApiService
  ) {}

  ngOnInit(): void {
    this.isDwsCopy =
      this.rowItem.resource_sub_type ===
      DataMap.Resource_Type.GaussDB_DWS.value;
    if (this.isHcsUser) {
      // 跨云复制做单独处理
      this.slaProperties = JSON.parse(this.rowItem.sla_properties);
      let replicationPolicy = find(this.slaProperties.policy_list, {
        type: PolicyType.REPLICATION
      });
      this.isHcsCrossCloud =
        replicationPolicy?.ext_parameters.replication_target_mode ===
        ReplicationModeType.CROSS_CLOUD;
    }
    this.initForm();
    this.getExtStorage();
  }

  initForm() {
    this.formGroup = this.fb.group({
      external_system_id: new FormControl('', {
        validators: [this.baseUtilService.VALID.required()]
      }),
      retention_duration: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.integer(),
          this.baseUtilService.VALID.rangeValue(1, 365)
        ]
      }),
      duration_unit: new FormControl(DataMap.Interval_Unit.day.value),
      link_deduplication: new FormControl(true),
      link_compression: new FormControl(true),
      alarm_after_failure: new FormControl(true),
      specifyUser: new FormControl('', {
        validators: !this.isHcsUser
          ? [this.baseUtilService.VALID.required()]
          : null
      }),
      userName: new FormControl(''),
      authPassword: new FormControl('', {
        validators: !this.isHcsUser
          ? [this.baseUtilService.VALID.required()]
          : null
      }),
      replication_storage_type: new FormControl('', {
        validators: this.isHcsUser
          ? null
          : [this.baseUtilService.VALID.required()]
      }),
      replication_storage_id: new FormControl(''),
      external_group_storage_id: new FormControl('')
    });

    this.listenForm();
  }

  listenForm() {
    if (!isEmpty(this.rowItem.storage_id)) {
      this.formGroup.addControl(
        'external_storage_id',
        new FormControl('', {
          validators: [this.baseUtilService.VALID.required()]
        })
      );
    }

    this.formGroup.get('external_system_id').valueChanges.subscribe(res => {
      if (!res) {
        return;
      }
      if (!isEmpty(this.rowItem.storage_id)) {
        this.getRepository(res);
      }
      if (this.isHcsUser) {
        return;
      }
      this.formGroup.get('specifyUser').setValue('');
      this.formGroup.get('authPassword').setValue('');
      this.formGroup.get('external_group_storage_id').setValue('');
      this.formGroup.get('replication_storage_id').setValue('');
      this.formGroup.get('replication_storage_type').setValue('');
      this.getSpecifyUser(res);
    });

    if (!this.isHcsUser) {
      this.formGroup
        .get('replication_storage_type')
        .valueChanges.subscribe(res => {
          if (res === DataMap.backupStorageTypeSla.group.value) {
            this.formGroup
              .get('external_group_storage_id')
              .setValidators([this.baseUtilService.VALID.required()]);
            this.formGroup.get('replication_storage_id').clearValidators();
          } else {
            this.formGroup
              .get('replication_storage_id')
              .setValidators([this.baseUtilService.VALID.required()]);
            this.formGroup.get('external_group_storage_id').clearValidators();
          }
          this.formGroup.get('replication_storage_id').updateValueAndValidity();
          this.formGroup
            .get('external_group_storage_id')
            .updateValueAndValidity();
        });
    }
  }

  getSpecifyUser(external_system_id) {
    this.usersApiService
      .remoteDPUserUsingGET({
        clusterId: external_system_id
      })
      .subscribe((res: any) => {
        // 当前只保留COMMON类型用户并且没有双因子认证
        res.userList = filter(
          res.userList,
          item =>
            item.userType === DataMap.loginUserType.local.value &&
            isEmpty(item.dynamicCodeEmail)
        );
        if (isEmpty(this.specifyUserOptionsMap[external_system_id])) {
          assign(this.specifyUserOptionsMap, {
            [external_system_id]: map(res.userList, item => {
              return assign(item, {
                label: item.userName,
                value: item.userId,
                isLeaf: true
              });
            })
          });
        }
      });
  }

  authPassword() {
    this.formGroup
      .get('userName')
      .setValue(
        find(
          this.specifyUserOptionsMap[this.formGroup.value.external_system_id],
          { value: this.formGroup.value.specifyUser }
        )?.userName
      );
    const params = {
      username: this.formGroup.value.userName,
      password: this.formGroup.value.authPassword,
      clusterId: this.formGroup.value.external_system_id
    };
    this.clusterApiService
      .verifyUsernameAndPasswordUsingPost({
        verifyUserDto: params,
        akOperationTips: false
      })
      .subscribe((res: any) => {
        this.isAuth = res;
        if (res) {
          this.messageService.success(
            this.i18n.get('protection_user_verify_success_label')
          );
          this.externalStorageMap[this.formGroup.value.specifyUser] = [];
          this.externalStorageUnitMap[this.formGroup.value.specifyUser] = [];
          this.getStorageUnitOptions();
        } else {
          this.messageService.error(
            this.i18n.get('protection_user_verify_fail_label')
          );
        }
      });
  }

  getStorageUnitOptions() {
    this.storageUserAuthService
      .getRemoteStorageUserAuthRelationsByUserId({
        pageNo: CommonConsts.PAGE_START,
        pageSize: CommonConsts.PAGE_SIZE_MAX,
        userId: this.formGroup.value.specifyUser,
        authType: 2,
        clusterId: this.formGroup.value.external_system_id
      })
      .subscribe(res => {
        if (
          isEmpty(this.externalStorageMap[this.formGroup.value.specifyUser])
        ) {
          this.externalStorageMap[this.formGroup.value.specifyUser] = map(
            res.records,
            item => {
              return assign(item, {
                label: item.storageName,
                value: item.storageId,
                isLeaf: true
              });
            }
          );
          if (!this.isDwsCopy) {
            this.externalStorageMap[
              this.formGroup.value.specifyUser
            ] = this.externalStorageMap[
              this.formGroup.value.specifyUser
            ].filter(v => !v.hasEnableParallelStorage);
          }
          if (this.isDwsCopy && !MultiCluster.isMulti) {
            this.externalStorageMap[
              this.formGroup.value.specifyUser
            ] = this.externalStorageMap[
              this.formGroup.value.specifyUser
            ].filter(v => v.hasEnableParallelStorage);
          }
        }
      });
    this.storageUserAuthService
      .getRemoteStorageUserAuthRelationsByUserId({
        pageNo: CommonConsts.PAGE_START,
        pageSize: CommonConsts.PAGE_SIZE_MAX,
        userId: this.formGroup.value.specifyUser,
        authType: 1,
        clusterId: this.formGroup.value.external_system_id
      })
      .subscribe(res => {
        if (
          isEmpty(this.externalStorageUnitMap[this.formGroup.value.specifyUser])
        ) {
          this.externalStorageUnitMap[this.formGroup.value.specifyUser] = map(
            res.records,
            item => {
              return assign(item, {
                label: item.storageName,
                value: item.storageId,
                isLeaf: true
              });
            }
          );
        }
      });
  }

  setRetentionDurationValidators(max: number) {
    this.formGroup
      .get('retention_duration')
      .setValidators([
        this.baseUtilService.VALID.required(),
        this.baseUtilService.VALID.integer(),
        this.baseUtilService.VALID.rangeValue(1, max)
      ]);
    this.retentionDurationErrorTip = assign(
      {},
      this.baseUtilService.rangeErrorTip,
      {
        invalidRang: this.i18n.get('common_valid_rang_label', [1, max])
      }
    );
  }

  // 单机场景，普通应用只能选存储单元；其他情况可以选单元和单元组。
  backupStorageTypes() {
    return !MultiCluster.isMulti && !this.isDwsCopy
      ? this.backupStorageTypesWithoutGroup
      : this.backupStorageTypesAll;
  }

  changeTimeUnits(value) {
    this.formGroup.get('retention_duration').enable();
    switch (value) {
      case DataMap.Interval_Unit.day.value:
        this.setRetentionDurationValidators(365);
        break;
      case DataMap.Interval_Unit.week.value:
        this.setRetentionDurationValidators(54);
        break;
      case DataMap.Interval_Unit.month.value:
        this.setRetentionDurationValidators(24);
        break;
      case DataMap.Interval_Unit.year.value:
        this.setRetentionDurationValidators(10);
        break;
      case DataMap.Interval_Unit.persistent.value:
        this.formGroup.get('retention_duration').disable();
        this.formGroup.get('retention_duration').setValue('');
        break;
      default:
        break;
    }
    this.formGroup.get('retention_duration').updateValueAndValidity();
  }

  addStorage() {
    const externalStorageComponent = new TargetClusterComponent(
      this.i18n,
      this.drawModalService,
      this.clusterApiService,
      this.dataMapService,
      this.warningMessageService,
      this.infoMessageService,
      this.cookieService
    );
    externalStorageComponent.addTargetCluster(() => this.getExtStorage());
  }

  getExtStorage() {
    this.clusterApiService
      .getClustersInfoUsingGET({
        startPage: CommonConsts.PAGE_START,
        pageSize: CommonConsts.PAGE_SIZE * 10,
        typeList: [DataMap.Cluster_Type.target.value],
        statusList: [DataMap.Cluster_Status.online.value],
        roleList: [DataMap.Target_Cluster_Role.replication.value]
      })
      .subscribe(res => {
        if (this.isHcsCrossCloud) {
          // hcs跨云只支持反向复制，所以要确定所选的集群是原来的集群
          res.records = filter(res.records, item => {
            let tmpIp = item.clusterIp.split(',');
            let replicationPolicy = find(this.slaProperties.policy_list, {
              type: PolicyType.REPLICATION
            });
            return tmpIp.includes(
              replicationPolicy.ext_parameters.source_cluster_ip
            );
          });
        }
        this.externalOption = map(res.records, (item: any) => {
          return assign(item, {
            isLeaf: true,
            label: item.clusterName
          });
        });
      });
  }

  getRepository(clusterId, recordsTemp?, startPage?) {
    this.nasDistributionStoragesApiService
      .ListNasDistributionStorages({
        pageNo: startPage || CommonConsts.PAGE_START,
        pageSize: CommonConsts.PAGE_SIZE,
        clustersId: clusterId,
        clustersType: _toString(DataMap.Cluster_Type.target.value)
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
          startPage === Math.ceil(res.totalCount / CommonConsts.PAGE_SIZE) ||
          res.totalCount === 0
        ) {
          this.externalStorage = map(recordsTemp, item => {
            return assign(item, {
              isLeaf: true,
              label: item.name
            });
          });
        }
        this.getRepository(clusterId, recordsTemp, startPage);
      });
  }

  onOK(): Observable<any> {
    return new Observable<void>((observer: Observer<void>) => {
      const params = {
        copy_id: this.rowItem.uuid,
        external_system_id: this.formGroup.value.external_system_id,
        retention_type:
          this.formGroup.value.duration_unit ===
          DataMap.Interval_Unit.persistent.value
            ? 1
            : 2,
        duration_unit:
          this.formGroup.value.duration_unit ===
          DataMap.Interval_Unit.persistent.value
            ? null
            : this.formGroup.value.duration_unit,
        retention_duration: +this.formGroup.value.retention_duration,
        link_compression: this.formGroup.value.link_compression,
        link_deduplication: this.formGroup.value.link_deduplication,
        alarm_after_failure: this.formGroup.value.alarm_after_failure,
        user_id: this.formGroup.value.specifyUser,
        username: this.formGroup.value.userName,
        password: this.formGroup.value.authPassword,
        storage_id:
          this.formGroup.value.replication_storage_type ===
          DataMap.backupStorageTypeSla.group.value
            ? this.formGroup.value.external_group_storage_id
            : this.formGroup.value.replication_storage_id,
        storage_type:
          this.formGroup.value.replication_storage_type ===
          DataMap.backupStorageTypeSla.group.value
            ? 'storage_unit_group'
            : 'storage_unit'
      } as any;
      if (!isEmpty(this.rowItem.storage_id)) {
        assign(params, {
          external_storage_id: this.formGroup.value.external_storage_id
        });
      }
      if (this.isHcsCrossCloud) {
        assign(params, {
          user_id: this.slaProperties.user_id
        });
        delete params.storage_id;
        delete params.username;
        delete params.password;
      }
      this.drawModalService.create({
        ...MODAL_COMMON.generateDrawerOptions(),
        lvModalKey: 'copy-info-message',
        ...{
          lvType: 'dialog',
          lvDialogIcon: 'lv-icon-popup-info-48',
          lvHeader: this.i18n.get('protection_copy_window_title_label'),
          lvContent: this.tipTpl,
          lvWidth: 500,
          lvOkType: 'primary',
          lvCancelType: 'default',
          lvOkDisabled: false,
          lvFocusButtonId: 'cancel',
          lvCloseButtonDisplay: true,
          lvOk: () => {
            this.replicationApiService
              .replicationUsingPOST({
                replicationReq: params
              })
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
          },
          lvCancel: () => {
            observer.error(null);
            observer.complete();
          },
          lvAfterClose: result => {
            if (result && result.trigger === 'close') {
              observer.error(null);
              observer.complete();
            }
          }
        }
      });
    });
  }
}
