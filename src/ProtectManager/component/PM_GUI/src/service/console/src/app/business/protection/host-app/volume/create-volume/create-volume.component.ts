import { Component, Input, OnInit, ViewChild } from '@angular/core';
import { FormBuilder, FormControl, FormGroup } from '@angular/forms';
import { DatatableComponent, MessageService } from '@iux/live';
import {
  BaseUtilService,
  CommonConsts,
  DataMapService,
  I18NService
} from 'app/shared';
import {
  EnvironmentsService,
  HostService,
  ProtectedEnvironmentApiService,
  ProtectedResourceApiService
} from 'app/shared/api/services';
import { CAPACITY_UNIT, DataMap, MultiCluster } from 'app/shared/consts';
import { cacheGuideResource } from 'app/shared/consts/guide-config';
import {
  assign,
  cloneDeep,
  defer,
  each,
  filter,
  find,
  get,
  includes,
  isEmpty,
  isNumber,
  map,
  size,
  startsWith,
  uniq
} from 'lodash';
import { Observable, Observer } from 'rxjs';

@Component({
  selector: 'aui-create-volume',
  templateUrl: './create-volume.component.html',
  styleUrls: ['./create-volume.component.less']
})
export class CreateVolumeComponent implements OnInit {
  unitconst = CAPACITY_UNIT;
  formGroup: FormGroup;
  usedHostOptions = [];
  hostOptions = [];
  volumesData = [];
  tempVolumesData = [];
  selectionVolumes = [];
  selectedVolumes = [];
  pageSize = CommonConsts.PAGE_SIZE_SMALL;
  pageSizeOptions = [CommonConsts.PAGE_SIZE_SMALL];
  pageIndex = CommonConsts.PAGE_START;
  dataMap = DataMap;
  originalPaths = [];
  osType;
  name;
  mountName;
  isProtected = false;
  hasSystemVolume = true; // 用来判断是否有系统卷

  typeFilterMap = this.dataMapService.toArray('volumeType');

  filesetNameErrorTip = {
    ...this.baseUtilService.requiredErrorTip,
    invalidName: this.i18n.get('protection_fileset_name_vaild_label'),
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [256])
  };

  // 系统卷判定
  systemVolumeList = [
    '/bin',
    '/etc',
    '/lib',
    '/lib64',
    '/opt',
    '/root',
    '/sbin',
    '/usr',
    '/var',
    '/home'
  ];

  @ViewChild('pageA', { static: false }) pageA;
  @ViewChild('pageS', { static: false }) pageS;
  @ViewChild('mountPopover', { static: false }) mountPopover;
  @ViewChild('namePopover', { static: false }) namePopover;
  @ViewChild('lvTable', { static: false }) lvTable: DatatableComponent;

  @Input() rowData;
  constructor(
    public fb: FormBuilder,
    public i18n: I18NService,
    public hostApiService: HostService,
    public dataMapService: DataMapService,
    private messageService: MessageService,
    public baseUtilService: BaseUtilService,
    public environmentsApiService: EnvironmentsService,
    private protectedResourceApiService: ProtectedResourceApiService,
    private protectedEnvironmentApiService: ProtectedEnvironmentApiService
  ) {}

  ngOnInit() {
    this.initForm();
    if (this.rowData) {
      // 保护中并且开了操作系统备份则置灰，创建中并且创建时已选中操作系统备份则也置灰
      this.isProtected =
        (this.rowData.protectionStatus ===
          DataMap.Protection_Status.protected.value &&
          this.rowData?.protectedObject?.extParameters?.system_backup_flag) ||
        (this.rowData.protectionStatus ===
          DataMap.Protection_Status.creating.value &&
          this.rowData.extendInfo.system_backup_flag === 'true');
      this.getHosts();
    } else {
      this.getUsedHosts();
    }
  }

  initForm() {
    this.formGroup = this.fb.group({
      name: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.maxLength(256),
          this.baseUtilService.VALID.name(
            /^[\u4e00-\u9fa5a-zA-Z_][\u4e00-\u9fa5a-zA-Z_0-9-]*$/
          )
        ]
      }),
      selectedHost: new FormControl('', {
        validators: [this.baseUtilService.VALID.required()]
      }),
      selectedVolumes: new FormControl([], {
        validators: [
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.maxLength(64)
        ]
      }),
      osBackup: new FormControl(false)
    });

    this.formGroup.get('selectedHost').valueChanges.subscribe(res => {
      if (!res) {
        return;
      }
      this.hasSystemVolume = true;
      this.osType = find(this.hostOptions, {
        key: res
      })?.os_type;
      this.removeAll();
      this.volumesData = [];
      this.getVolumes();
    });

    this.formGroup.get('osBackup').valueChanges.subscribe(res => {
      defer(() => this.selectionChange());
    });
  }

  getUsedHosts(recordsTemp?, startPage?) {
    this.protectedResourceApiService
      .ListResources({
        pageNo: startPage || 0,
        pageSize: CommonConsts.PAGE_SIZE * 10,
        conditions: JSON.stringify({
          subType: DataMap.Resource_Type.volume.value
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
          startPage ===
            Math.ceil(res.totalCount / (CommonConsts.PAGE_SIZE * 10)) ||
          res.totalCount === 0
        ) {
          const hostArray = [];
          each(res.records, item => {
            hostArray.push(item.environment.uuid);
          });
          this.usedHostOptions = [...uniq(hostArray)];
          this.getHosts();
          return;
        }
        this.getUsedHosts(recordsTemp, startPage);
      });
  }

  getHosts(recordsTemp?, startPage?) {
    this.protectedResourceApiService
      .ListResources({
        pageSize: 20,
        pageNo: startPage || 0,
        conditions: JSON.stringify({
          subType: ['VolumePlugin']
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
          startPage === Math.ceil(res.totalCount / CommonConsts.PAGE_SIZE) ||
          res.totalCount === 0
        ) {
          const hostArr = [];
          recordsTemp = filter(
            recordsTemp,
            item =>
              !isEmpty(item.environment) &&
              !find(this.usedHostOptions, val => val === item.environment.uuid)
          );
          if (!MultiCluster.isMulti) {
            recordsTemp = filter(
              recordsTemp,
              item =>
                item.environment?.linkStatus ===
                DataMap.resource_LinkStatus_Special.normal.value
            );
          } else {
            recordsTemp = filter(recordsTemp, item => {
              const val = item.environment;
              const connection = val?.extendInfo?.connection_result;
              const target = JSON.parse(connection)[MultiCluster.esn];
              if (target?.link_status) {
                return true;
              }
              return (
                val.linkStatus ===
                DataMap.resource_LinkStatus_Special.normal.value
              );
            });
          }
          each(recordsTemp, item => {
            hostArr.push({
              key: item.uuid,
              value: item.uuid,
              label: !isEmpty(item.environment?.endpoint)
                ? `${item.environment?.name}(${item.environment?.endpoint})`
                : item.environment?.name,
              os_type: item.environment?.osType,
              parentUuid: item.parentUuid,
              isLeaf: true
            });
          });
          this.hostOptions = hostArr;

          if (!!this.rowData) {
            this.formGroup.patchValue({
              name: this.rowData.name,
              selectedHost: get(
                find(
                  this.hostOptions,
                  host => host.parentUuid === this.rowData.environment?.uuid
                ),
                'value'
              ),
              osBackup: this.rowData.extendInfo.system_backup_flag === 'true'
            });
            this.originalPaths = map(
              JSON.parse(this.rowData.extendInfo.paths),
              item => item.name
            );
          }
          return;
        }
        this.getHosts(recordsTemp, startPage);
      });
  }

  getVolumes(recordsTemp?, startPage?) {
    this.protectedEnvironmentApiService
      .ListEnvironmentResource({
        envId: find(this.hostOptions, {
          key: this.formGroup.value.selectedHost
        })?.parentUuid,
        pageNo: startPage || CommonConsts.PAGE_START,
        pageSize: CommonConsts.PAGE_SIZE * 10,
        parentId: '',
        resourceType: DataMap.Resource_Type.volume.value
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
          startPage ===
            Math.ceil(res.totalCount / (CommonConsts.PAGE_SIZE * 10)) ||
          res.totalCount === 0
        ) {
          const tableData = map(recordsTemp, item => {
            return {
              ...item,
              size: get(item, 'extendInfo.size'),
              volume: get(item, 'extendInfo.path'),
              volumeMountPoints: get(item, 'extendInfo.volumeMountPoints', '[]')
            };
          });

          // 标识卷的类型
          this.markVolume(tableData);

          // 判断所有卷里有没有系统卷
          if (!find(tableData, { type: true })) {
            this.hasSystemVolume = false;
            this.formGroup.get('osBackup').setValue(false);
          }

          this.volumesData = cloneDeep(tableData);
          this.tempVolumesData = this.volumesData;

          if (!!size(this.originalPaths)) {
            this.selectionVolumes = filter(this.volumesData, volume => {
              return includes(this.originalPaths, volume.volume);
            });
            this.selectionChange();
            this.originalPaths = [];
          }
          if (this.formGroup.value.osBackup) {
            // 获取时已经开启开关则自动选中系统卷
            this.selectionChange();
          }
          return;
        }
        this.getVolumes(recordsTemp, startPage);
      });
  }

  markVolume(tableData) {
    each(tableData, item => {
      let tmp = item.volumeMountPoints.split(',');
      if (!tmp) {
        item.type = false;
      }
      each(tmp, volume => {
        assign(item, {
          type: !!find(this.systemVolumeList, val => {
            return startsWith(volume, val) || volume === '/';
          })
        });
      });
    });
  }

  selectionChange() {
    if (!!this.formGroup.value.osBackup) {
      // 开启了开关则必定选中系统卷
      this.volumesData.forEach(item => {
        if (
          item.type &&
          !this.selectionVolumes.some(v => v.volume === item.volume)
        ) {
          this.selectionVolumes.push(item);
        }
        item.disabled = item.type;
      });
      this.selectionVolumes.forEach(item => {
        item.disabled = item.type;
      });
    } else {
      this.volumesData.forEach(item => {
        item.disabled = false;
      });
      this.selectionVolumes.forEach(item => {
        item.disabled = false;
      });
    }
    this.selectionVolumes = [...this.selectionVolumes];
    this.selectedVolumes = [...this.selectionVolumes];
    this.formGroup.get('selectedVolumes').setValue(this.selectedVolumes);
  }

  remove(item) {
    this.selectionVolumes = filter(
      this.selectionVolumes,
      volume => item.volume !== volume.volume
    );
    this.selectionChange();
  }

  removeAll() {
    this.selectionVolumes = [];
    this.selectionChange();
  }

  pageChange(page) {
    this.pageSize = page.pageSize;
    this.pageIndex = page.pageIndex;
  }

  searchMount(e) {
    if (e === '') {
      this.volumesData = this.tempVolumesData;
    }
    let displayData = [];
    each(this.tempVolumesData, item => {
      if (item.volumeMountPoints.indexOf(e) !== -1) {
        displayData.push({
          ...item
        });
      }
    });
    this.volumesData = displayData;
  }

  searchName(e) {
    if (e !== '') {
      this.namePopover.hide();
    }
    this.lvTable.filter({ key: 'volume', value: e, filterMode: 'contains' });
  }

  filterChange(options: any) {
    this.lvTable.filter(options);
  }

  getParams() {
    const params = {
      name: this.formGroup.value.name,
      parentUuid: this.formGroup.value.selectedHost,
      type: DataMap.Resource_Type.fileset.value,
      subType: DataMap.Resource_Type.volume.value,
      extendInfo: {
        paths: JSON.stringify(
          map(this.selectedVolumes, volume => {
            return {
              name: volume.volume
            };
          })
        ),
        system_backup_flag: this.formGroup.value.osBackup
      }
    };

    return params;
  }

  onOK(): Observable<void> {
    return new Observable<void>((observer: Observer<void>) => {
      const params = this.getParams();

      if (!!this.rowData) {
        this.protectedResourceApiService
          .UpdateResource({
            resourceId: this.rowData.uuid,
            UpdateResourceRequestBody: params
          })
          .subscribe({
            next: () => {
              observer.next();
              observer.complete();
            },
            error: error => {
              observer.error(error);
              observer.complete();
            }
          });
      } else {
        this.protectedResourceApiService
          .CreateResource({
            CreateResourceRequestBody: params
          })
          .subscribe({
            next: res => {
              cacheGuideResource(res);
              observer.next();
              observer.complete();
            },
            error: error => {
              observer.error(error);
              observer.complete();
            }
          });
      }
    });
  }
}
