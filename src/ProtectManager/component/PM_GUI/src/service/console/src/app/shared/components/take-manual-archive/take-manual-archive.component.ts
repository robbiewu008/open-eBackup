/*
 * This file is a part of the open-eBackup project.
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
 * If a copy of the MPL was not distributed with this file, You can obtain one at
 * http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) [2024] Huawei Technologies Co.,Ltd.
 *
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 */
import { ChangeDetectorRef, Component, OnInit } from '@angular/core';
import { FormBuilder, FormControl, FormGroup } from '@angular/forms';
import { OptionItem } from '@iux/live';
import { ArchiveStorageComponent } from 'app/business/system/infrastructure/archive-storage/archive-storage.component';
import { StoragePoolListComponent } from 'app/business/system/infrastructure/archive-storage/storage-pool-list/storage-pool-list.component';
import {
  ApplicationType,
  ClustersApiService,
  CommonConsts,
  DataMap,
  MediaSetApiService,
  QosService,
  RetentionType,
  StoragesApiService
} from 'app/shared';
import { ArchiveService } from 'app/shared/api/services';
import {
  BaseUtilService,
  CookieService,
  DataMapService,
  I18NService,
  WarningMessageService
} from 'app/shared/services';
import { AppUtilsService } from 'app/shared/services/app-utils.service';
import { BatchOperateService } from 'app/shared/services/batch-operate.service';
import { DrawModalService } from 'app/shared/services/draw-modal.service';
import { VirtualScrollService } from 'app/shared/services/virtual-scroll.service';
import {
  assign,
  each,
  filter,
  find,
  findKey,
  isArray,
  isNumber,
  isUndefined,
  map,
  set,
  size
} from 'lodash';
import { Observable, Observer } from 'rxjs';

@Component({
  selector: 'aui-take-manual-archive',
  templateUrl: './take-manual-archive.component.html',
  styleUrls: ['./take-manual-archive.component.less']
})
export class TakeManualArchiveComponent implements OnInit {
  data;
  application;

  formGroup: FormGroup;
  dataMap = DataMap;
  applicationType = ApplicationType;
  clusterNodeNames = [];
  qosNames = [];
  tapeStorageNames = [];
  s3StorageNames = [];
  storageNames = [];
  mediaSetOptions = {};
  mediaSetData = [];
  curNodeInfo = {
    esn: '',
    clusterId: '',
    nodeName: ''
  };

  // 保留70年
  maxRetentionDay = 25550;
  maxRetentionWeek = 3650;
  maxRetentionMonth = 840;
  maxRetentionYear = 70;
  protocolOptions = this.dataMapService
    .toArray('Archival_Protocol')
    .filter((v: OptionItem) => {
      return (v.isLeaf = true);
    });
  retentionDurationUnit = this.dataMapService
    .toArray('Interval_Unit')
    .filter((v: OptionItem) => {
      return v.value !== 'm' && v.value !== 'h';
    })
    .filter((v: OptionItem) => {
      return (v.isLeaf = true);
    });
  retentionDurationErrorTip = assign({}, this.baseUtilService.rangeErrorTip, {
    invalidRang: this.i18n.get('common_valid_rang_label', [
      1,
      this.maxRetentionDay
    ])
  });

  constructor(
    private fb: FormBuilder,
    public i18n: I18NService,
    private cdr: ChangeDetectorRef,
    private qosServiceApi: QosService,
    private cookieService: CookieService,
    private dataMapService: DataMapService,
    public baseUtilService: BaseUtilService,
    private virtualScroll: VirtualScrollService,
    private storageApiService: StoragesApiService,
    private clusterApiService: ClustersApiService,
    private mediaSetApiService: MediaSetApiService,
    private batchOperateService: BatchOperateService,
    private drawModalService: DrawModalService,
    private warningMessageService: WarningMessageService,
    private archiveService: ArchiveService,
    public appUtilsService: AppUtilsService
  ) {}

  ngOnInit() {
    this.application = this.getTypeKey(
      this.data.resource_sub_type,
      this.appUtilsService.findResourceTypeByKey('slaId')
    );
    this.getClusterNodes();
    this.initForm();
    this.getQosNames();
  }

  /**
   * 根据subType取出对应的slaType
   * @param type
   * @param typeMap
   */
  getTypeKey(type: string, typeMap) {
    const key = findKey(typeMap, item => {
      if (isArray(item)) {
        return item.includes(type);
      } else {
        return item === type;
      }
    });
    return key || type;
  }

  getClusterNodes() {
    const cluster = JSON.parse(
      decodeURIComponent(this.cookieService.get('currentCluster'))
    ) || {
      clusterId: DataMap.Cluster_Type.local.value,
      clusterType: DataMap.Cluster_Type.local.value
    };
    const params = {
      clusterId: cluster.clusterId,
      startPage: CommonConsts.PAGE_START,
      pageSize: CommonConsts.PAGE_SIZE,
      roleList: [
        DataMap.Target_Cluster_Role.primaryNode.value,
        DataMap.Target_Cluster_Role.backupNode.value,
        DataMap.Target_Cluster_Role.memberNode.value
      ]
    };

    this.clusterApiService
      .getClustersInfoUsingGET(params)
      .subscribe((res: any) => {
        const nodes = map(res.records, item => {
          this.getTapeStorageNames(item);

          return {
            ...item,
            key: item.clusterId,
            value: item.clusterId,
            label: item.clusterName,
            isLeaf: true
          };
        });

        this.clusterNodeNames = nodes;
      });
  }

  getTapeStorageNames(node, recordsTemp?, startPage?) {
    const params = {
      pageNo: startPage || CommonConsts.PAGE_START + 1,
      pageSize: CommonConsts.PAGE_SIZE * 10,
      memberEsn: node?.storageEsn || node?.remoteEsn
    };
    this.mediaSetApiService.getMediaSetAllUsingGET(params).subscribe(res => {
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
          Math.ceil(res.totalCount / (CommonConsts.PAGE_SIZE * 10)) + 1 ||
        res.totalCount === 0
      ) {
        const storageNames = [];
        each(recordsTemp, item => {
          storageNames.push({
            ...item,
            isLeaf: true,
            label: item.mediaSetName,
            value: item.mediaSetId
          });
        });
        if (!node) {
          this.tapeStorageNames = storageNames;
        } else {
          set(this.mediaSetOptions, node?.clusterId, storageNames);
        }

        return;
      }
      this.getTapeStorageNames(node, recordsTemp, startPage);
    });
  }

  initForm() {
    this.formGroup = this.fb.group({
      protocol: new FormControl('', {
        validators: [this.baseUtilService.VALID.required()]
      }),
      node_id: new FormControl(''),
      mediaSet: new FormControl(''),
      storage_id: new FormControl('', {
        validators: [this.baseUtilService.VALID.required()],
        updateOn: 'change'
      }),
      retention_duration: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.integer(),
          this.baseUtilService.VALID.rangeValue(1, this.maxRetentionDay)
        ],
        updateOn: 'change'
      }),
      duration_unit: new FormControl(DataMap.Interval_Unit.day.value),
      auto_index: new FormControl(true),
      qos_id: new FormControl(''),
      esn: new FormControl(''),
      network_access: new FormControl(true)
    });

    this.listenForm();
    this.formGroup
      .get('protocol')
      .setValue(DataMap.Archival_Protocol.objectStorage.value);
  }

  listenForm() {
    this.formGroup.get('protocol').valueChanges.subscribe(res => {
      if (!res) {
        return;
      }
      if (res === DataMap.Archival_Protocol.objectStorage.value) {
        this.formGroup
          .get('storage_id')
          .setValidators([this.baseUtilService.VALID.required()]);
        this.formGroup.get('retention_duration').enable();
        this.formGroup
          .get('duration_unit')
          .setValue(DataMap.Interval_Unit.day.value);
        this.formGroup.get('node_id').clearValidators();
        this.formGroup.get('mediaSet').clearValidators();
      } else {
        this.formGroup
          .get('node_id')
          .setValidators([this.baseUtilService.VALID.required()]);
        this.formGroup.get('storage_id').clearValidators();
        this.formGroup
          .get('mediaSet')
          .setValidators([this.baseUtilService.VALID.required()]);
        this.formGroup.get('qos_id').setValue('');
        this.formGroup
          .get('duration_unit')
          .setValue(DataMap.Interval_Unit.persistent.value);
      }
      this.formGroup.get('storage_id').setValue('');
      this.formGroup.get('storage_id').updateValueAndValidity();
      this.formGroup.get('node_id').updateValueAndValidity();
      this.formGroup.get('mediaSet').updateValueAndValidity();
      this.formGroup.get('retention_duration').updateValueAndValidity();
      this.getStorageNames(res);
    });

    this.formGroup.get('node_id').valueChanges.subscribe(res => {
      if (!res) {
        this.curNodeInfo = {
          esn: '',
          clusterId: '',
          nodeName: ''
        };
        return;
      }

      this.formGroup.get('mediaSet').setValue('');
      this.formGroup.get('mediaSet').updateValueAndValidity();
      const cluster = find(this.clusterNodeNames, { value: res });
      this.curNodeInfo = {
        esn: cluster?.storageEsn,
        clusterId: cluster?.clusterId,
        nodeName: cluster?.clusterName
      };
      this.formGroup.get('esn').setValue(cluster?.storageEsn);
    });
  }

  getStorageNames(storageType) {
    if (storageType === DataMap.Archival_Protocol.objectStorage.value) {
      this.getS3StorageNames();
    }
  }

  getS3StorageNames() {
    this.storageApiService
      .storageUsingGET({
        startPage: 0,
        pageSize: 200
      })
      .subscribe(res => {
        if (!size(res.records)) {
          this.storageNames = [];
        }
        res.records = filter(res.records, item => {
          return item.status === DataMap.Archive_Storage_Status.online.value;
        });
        this.s3StorageNames = map(res.records, item => {
          return assign(item, {
            isLeaf: true,
            label: item.name,
            value: item.repositoryId
          });
        });
      });
  }

  addStorage(type) {
    if (type === DataMap.Archival_Protocol.objectStorage.value) {
      this.addS3Storage();
    } else {
      this.addMediaSet(this.curNodeInfo);
    }
  }

  addS3Storage() {
    const archiveStorageComponent = new ArchiveStorageComponent(
      this.appUtilsService,
      this.i18n,
      this.drawModalService,
      this.baseUtilService,
      this.warningMessageService,
      this.storageApiService,
      this.dataMapService,
      this.cookieService
    );
    archiveStorageComponent.createArchiveStorage(() =>
      this.getS3StorageNames()
    );
  }

  addMediaSet(data?) {
    const node = {
      clusterId: data?.clusterId || '',
      remoteEsn: data?.esn || '',
      clusterName: data?.nodeName || ''
    };
    const storagePoolListComponent = new StoragePoolListComponent(
      this.i18n,
      this.cdr,
      this.dataMapService,
      this.mediaSetApiService,
      this.batchOperateService,
      this.warningMessageService,
      this.drawModalService,
      this.virtualScroll,
      this.cookieService
    );
    storagePoolListComponent.node = node;
    storagePoolListComponent.createPool(() => this.getTapeStorageNames(node));
  }

  getMediaRetentionTip(options, v) {
    const mediaSet = find(options, item => item.mediaSetId === v);
    if (isUndefined(mediaSet)) {
      return;
    }
    const retentionLabel = this.dataMapService.getLabel(
      'Tape_Retention_Type',
      mediaSet.retentionType
    );
    const retentionUnit = this.dataMapService.getLabel(
      'Tape_Retention_Unit',
      mediaSet.retentionUnit
    );
    if (
      mediaSet.retentionType !== DataMap.Tape_Retention_Type.temporary.value
    ) {
      return this.i18n.get('protection_archive_media_set_retention_tip_label', [
        retentionLabel
      ]);
    } else {
      let resultLabel = this.i18n.get(
        'protection_archive_media_set_retention_tip_label',
        [`${retentionLabel}（${mediaSet.retentionDuration + retentionUnit}）`]
      );
      if (this.i18n.isEn) {
        return resultLabel.replace('（', '(').replace('）', ')');
      }
      return resultLabel;
    }
  }

  changeTimeUnits(value) {
    this.formGroup.get('retention_duration').enable();
    const baseLimit = [
      this.baseUtilService.VALID.required(),
      this.baseUtilService.VALID.integer()
    ];
    const maxRetention = {
      [DataMap.Interval_Unit.day.value]: this.maxRetentionDay,
      [DataMap.Interval_Unit.week.value]: this.maxRetentionWeek,
      [DataMap.Interval_Unit.month.value]: this.maxRetentionMonth,
      [DataMap.Interval_Unit.year.value]: this.maxRetentionYear
    };

    const retentionDurationControl = this.formGroup.get('retention_duration');
    const retentionDurationValidators = [
      ...baseLimit,
      this.baseUtilService.VALID.rangeValue(1, maxRetention[value])
    ];
    const retentionDurationErrorTip = assign(
      {},
      this.baseUtilService.rangeErrorTip,
      {
        invalidRang: this.i18n.get('common_valid_rang_label', [
          1,
          maxRetention[value]
        ])
      }
    );

    if (value === DataMap.Interval_Unit.persistent.value) {
      retentionDurationControl.disable();
      retentionDurationControl.setValue('');
    } else {
      retentionDurationControl.setValidators(retentionDurationValidators);
      retentionDurationControl.enable();
      this.retentionDurationErrorTip = retentionDurationErrorTip;
    }
    this.formGroup.get('retention_duration').updateValueAndValidity();
  }

  getQosNames() {
    this.qosServiceApi
      .queryResourcesV1QosGet({
        pageNo: 0,
        pageSize: 100
      })
      .subscribe(res => {
        this.qosNames = map(res.items, (item: any) => {
          item['isLeaf'] = true;
          item['label'] = item.name;
          return item;
        });
      });
  }

  getParams() {
    const params: any = {
      copy_id: this.data.uuid,
      retention_type:
        this.formGroup.get('protocol').value ===
        DataMap.Archival_Protocol.tapeLibrary.value
          ? RetentionType.PERMANENTLY_RETAINED
          : DataMap.Interval_Unit.persistent.value ===
            this.formGroup.get('duration_unit').value
          ? RetentionType.PERMANENTLY_RETAINED
          : RetentionType.TEMPORARY_RESERVATION,
      network_access: this.formGroup.get('network_access').value,
      storage_list: [
        {
          storage_id: this.formGroup.get('storage_id').value,
          protocol: this.formGroup.get('protocol').value
        }
      ]
    };

    if (!!this.formGroup.get('qos_id').value) {
      assign(params, {
        qos_id: this.formGroup.get('qos_id').value
      });
    }

    if (
      this.formGroup.get('protocol').value ===
      DataMap.Archival_Protocol.tapeLibrary.value
    ) {
      assign(params.storage_list[0], {
        esn: this.formGroup.get('esn').value,
        storage_id: this.formGroup.get('mediaSet').value
      });
    } else {
      assign(params, {
        retention_duration: +this.formGroup.get('retention_duration').value,
        duration_unit: this.formGroup.get('duration_unit').value
      });
      if (
        DataMap.Interval_Unit.persistent.value ===
        this.formGroup.get('duration_unit').value
      ) {
        delete params.duration_unit;
      }
    }

    if (
      [
        ApplicationType.NASFileSystem,
        ApplicationType.NASShare,
        ApplicationType.HDFS,
        ApplicationType.ImportCopy,
        ApplicationType.Fileset
      ].includes(this.application)
    ) {
      assign(params, {
        auto_index: this.formGroup.get('auto_index').value
      });
    }

    return params;
  }

  onOK(): Observable<void> {
    return new Observable<void>((observer: Observer<void>) => {
      if (this.formGroup.invalid) {
        return;
      }

      this.archiveService
        .manualArchive({
          archiveRequest: this.getParams()
        })
        .subscribe({
          next: () => {
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
}
