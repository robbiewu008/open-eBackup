import { Component, EventEmitter, Input, OnInit, Output } from '@angular/core';
import { FormGroup } from '@angular/forms';
import {
  ApplicationType,
  BaseUtilService,
  ClustersApiService,
  CommonConsts,
  CookieService,
  DataMap,
  DataMapService,
  I18NService,
  MultiCluster,
  NasDistributionStoragesApiService,
  QosService,
  StorageUnitService,
  StorageUserAuthService
} from 'app/shared';
import { AppUtilsService } from 'app/shared/services/app-utils.service';
import { filter, find, get, isEqual, map, remove, size } from 'lodash';
@Component({
  selector: 'aui-specify-destination-location',
  templateUrl: './specify-destination-location.component.html',
  styleUrls: ['./specify-destination-location.component.less']
})
export class SpecifyDestinationLocationComponent implements OnInit {
  find = find;
  size = size;
  specialClassName;
  backupStorageUnitNames = [];
  backupStorageUnitGroupNames = [];
  applicationType = '';
  showDestination = true;
  @Input() isSlaDetail: boolean;
  @Input() data: any;
  @Input() action: any;
  @Input() formGroup: FormGroup;
  @Input() isRetry: boolean;
  @Input() isUsed: boolean;
  @Output() isDisableQos = new EventEmitter<any>();
  backupStorageTypes = this.dataMapService.toArray(
    'storagePoolBackupStorageType'
  );
  isHcsUser = this.cookieService.get('userType') === CommonConsts.HCS_USER_TYPE;

  constructor(
    public baseUtilService: BaseUtilService,
    public i18n: I18NService,
    private qosServiceApi: QosService,
    private cookieService: CookieService,
    public dataMapService: DataMapService,
    private clusterApiService: ClustersApiService,
    private nasDistributionStoragesApiService: NasDistributionStoragesApiService,
    private storageUnitService: StorageUnitService,
    private storageUserAuthService: StorageUserAuthService,
    public appUtilsService?: AppUtilsService
  ) {}

  ngOnInit(): void {
    this.formGroup
      .get('storage_id')
      .setValidators([this.baseUtilService.VALID.required()]);
    this.formGroup
      .get('storage_type')
      .setValidators([this.baseUtilService.VALID.required()]);

    this.getBackupStorageNames();
    if (!this.isSlaDetail) {
      this.updateData();
      this.listenChanges();
      this.applicationType = get(this.data, 'applicationData');
    } else {
      this.applicationType = get(this.data[0], 'applicationData');
    }
    this.destinationOps();
  }

  destinationOps() {
    // 单机场景非dws屏蔽备份存储单元组
    if (
      !MultiCluster.isMulti &&
      this.applicationType !== ApplicationType.GaussDBDWS
    ) {
      remove(
        this.backupStorageTypes,
        item => item.value === DataMap.backupStorageType.group.value
      );
    }
  }

  updateData() {
    // 各个应用初始化会传入“none”，多存储池需求后必须指定存储位置,不能再使用“none”
    if (this.formGroup.get('storage_type').value === 'none') {
      this.formGroup.get('storage_type').setValue('');
    }
  }

  getBackupStorageNames() {
    // 查询备份存储单元组
    this.storageUserAuthService
      .getStorageUserAuthRelationsByUserId({
        userId: this.cookieService.get('userId'),
        authType: 2
      })
      .subscribe(res => {
        this.backupStorageUnitGroupNames = map(res.records, item => {
          return {
            isLeaf: true,
            label: item.storageName,
            disabled: false,
            value: item.storageId,
            ...item
          };
        });
        if (this.applicationType !== ApplicationType.GaussDBDWS) {
          this.backupStorageUnitGroupNames = this.backupStorageUnitGroupNames.filter(
            v => v.hasEnableParallelStorage === false
          );
        }
        if (
          this.applicationType === ApplicationType.GaussDBDWS &&
          !MultiCluster.isMulti
        ) {
          this.backupStorageUnitGroupNames = this.backupStorageUnitGroupNames.filter(
            v => v.hasEnableParallelStorage === true
          );
        }
        this.backupStorageUnitGroupNames = [
          ...this.backupStorageUnitGroupNames
        ];
      });

    // 查询备份存储单元
    this.storageUserAuthService
      .getStorageUserAuthRelationsByUserId({
        userId: this.cookieService.get('userId'),
        authType: 1
      })
      .subscribe(res => {
        this.backupStorageUnitNames = map(res.records, item => {
          return {
            isLeaf: true,
            label: item.storageName,
            disabled: false,
            value: item.storageId,
            ...item
          };
        });
        // x系列只能指定本地存储单元
        this.backupStorageUnitNames = filter(
          this.backupStorageUnitNames,
          item => {
            return (
              this.appUtilsService.isDecouple ||
              item.generatedType ===
                DataMap.backupStorageGeneratedType.local.value
            );
          }
        );
      });
  }
  listenChanges() {
    this.formGroup.get('backupTeams').valueChanges.subscribe(res => {
      this.updateBackupStorageNames(res);
    });

    this.formGroup.get('storage_type').valueChanges.subscribe(res => {
      this.formGroup.get('storage_id').setValue('');
      if (!this.formGroup.value.storage_type) {
        this.formGroup.get('storage_id').clearValidators();
      } else {
        this.formGroup
          .get('storage_id')
          .setValidators(this.baseUtilService.VALID.required());
      }
      this.formGroup.get('storage_id').updateValueAndValidity();
      this.updateBackupStorageNames(this.formGroup.get('backupTeams').value);
    });

    this.formGroup.get('storage_id').valueChanges.subscribe(res => {
      if (!res) {
        return;
      }
      const flag =
        find(this.backupStorageUnitNames, { value: res }).storageType ===
        'BasicDisk';
      this.isDisableQos.emit(flag);

      if (flag) {
        this.formGroup.get('qos_id').setValue('');
      }
    });
  }
  updateBackupStorageNames(data) {
    if (
      find(data, { action: 'log' }) &&
      this.formGroup.value.storage_type == 'storage_unit_group'
    ) {
      this.backupStorageUnitGroupNames = map(
        this.backupStorageUnitGroupNames,
        item => {
          return {
            isLeaf: true,
            label: item.clusterName,
            ...item,
            disabled: isEqual(
              item.storageStrategyType,
              DataMap.newBackupPolicy.balance.value
            )
          };
        }
      );
    } else {
      this.backupStorageUnitGroupNames = map(
        this.backupStorageUnitGroupNames,
        item => {
          return {
            isLeaf: true,
            label: item.clusterName,
            ...item,
            disabled: false
          };
        }
      );
    }
  }
}
