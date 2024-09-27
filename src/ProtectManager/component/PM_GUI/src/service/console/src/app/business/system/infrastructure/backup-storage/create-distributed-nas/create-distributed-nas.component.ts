import {
  ChangeDetectorRef,
  Component,
  Input,
  OnInit,
  TemplateRef,
  ViewChild
} from '@angular/core';
import {
  AbstractControl,
  FormBuilder,
  FormControl,
  FormGroup,
  ValidatorFn
} from '@angular/forms';
import {
  DatatableComponent,
  MessageService,
  ModalRef,
  OptionItem
} from '@iux/live';
import {
  BaseUtilService,
  CAPACITY_UNIT,
  ClustersApiService,
  ColorConsts,
  CommonConsts,
  CookieService,
  DataMap,
  DataMapService,
  I18NService,
  NasDistributionStoragesApiService,
  StoragesApiService,
  StorageUnitService
} from 'app/shared';
import { ProTableComponent } from 'app/shared/components/pro-table';
import { AppUtilsService } from 'app/shared/services/app-utils.service';
import {
  assign,
  defer,
  each,
  filter,
  find,
  get,
  isNumber,
  isUndefined,
  map,
  remove
} from 'lodash';
import { Observable, Observer, Subject } from 'rxjs';
import { SetStoragePolicyComponent } from '../set-storage-policy/set-storage-policy.component';

@Component({
  selector: 'aui-create-distributed-nas',
  templateUrl: './create-distributed-nas.component.html',
  styleUrls: ['./create-distributed-nas.component.less']
})
export class CreateDistributedNasComponent implements OnInit {
  isEdit: boolean;
  dataMap = DataMap;
  title = this.i18n.get('common_selected_label');
  formGroup: FormGroup;
  allSourceData = []; // 用来保存所有的数据
  sourceData = []; //非并行
  targetData = [];
  localSourceData = []; //本地集群
  localTargetData = [];
  nonLocalSourceData = []; //非本地集群
  nonLocalTargetData = [];
  selection = [];
  activeStep = 1;
  componentData;
  activeIndex: string = 'localCluster';
  typeData = {
    storageStrategyType: 4,
    timeoutPeriod: null,
    timeUnit: 1
  };
  dataTable;
  tempData = [];
  progressBarColor = [[0, ColorConsts.NORMAL]];
  lessThanLabel = this.i18n.get('common_less_than_label');
  protected readonly CAPACITY_UNIT = CAPACITY_UNIT;
  protected readonly Math = Math;
  unitconst = CAPACITY_UNIT;

  isDistributed = this.appUtilsService.isDistributed;
  isDecouple = this.appUtilsService.isDecouple;

  deviceTypeOptions = this.dataMapService
    .toArray('poolStorageDeviceType')
    .filter((v: OptionItem) => {
      return (
        (v.isLeaf = true) &&
        v.value !==
          (this.isDistributed
            ? DataMap.poolStorageDeviceType.OceanProtectX.value
            : DataMap.poolStorageDeviceType.OceanPacific.value)
      );
    });

  nameErrorTip = {
    ...this.baseUtilService.nameErrorTip,
    invalidName: this.i18n.get('common_valid_distributed_nas_name_label'),
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [32])
  };
  descErrorTip = {
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [255])
  };
  valid$ = new Subject<boolean>();
  @ViewChild(SetStoragePolicyComponent, { static: false })
  SetStoragePolicyComponent: SetStoragePolicyComponent;
  @ViewChild('allTable', { static: false }) allTable: ProTableComponent;
  @ViewChild('selectedTable', { static: false })
  selectedTable: ProTableComponent;
  @ViewChild('page', { static: false }) page;
  @ViewChild('operation', { static: true }) operation: TemplateRef<any>;
  @ViewChild('lvTable') lvTable: DatatableComponent;
  @Input() rowData;

  constructor(
    public modal: ModalRef,
    public i18n: I18NService,
    public fb: FormBuilder,
    public dataMapService: DataMapService,
    public baseUtilService: BaseUtilService,
    public storageApiService: StoragesApiService,
    public cookieService: CookieService,
    private clusterApiService: ClustersApiService,
    private messageService: MessageService,
    private nasDistributionStoragesApiService: NasDistributionStoragesApiService,
    private cdr: ChangeDetectorRef,
    private storageUnitService: StorageUnitService,
    public appUtilsService: AppUtilsService
  ) {}

  ngOnInit() {
    assign(this.componentData, {});
    this.initForm();
    this.getCluster();
  }

  previous(callback?: () => void) {
    this.activeStep = 1;
    this.modal.setProperty({
      lvFooter: [
        {
          type: 'primary',
          label: this.i18n.get('common_next_label'),
          onClick: modal => {
            this.next(...[, () => !isUndefined(callback) && callback()]);
          }
        },
        {
          label: this.i18n.get('common_cancel_label'),
          onClick: modal => {
            modal.close();
          }
        }
      ]
    });
    this.cdr.detectChanges();
  }
  next(parallelVal?, callback?: () => void) {
    this.activeStep = 2;
    const form = this.SetStoragePolicyComponent.timeForm;
    this.changeFooter(() => callback());
    if (this.isEdit && !this.rowData?.storageStrategyType && !parallelVal) {
      form.get('storageStrategyType').setValue(1);
    }
    this.cdr.detectChanges();
  }
  changeFooter(callback?: () => void) {
    this.modal.setProperty({
      lvFooter: [
        {
          label: this.i18n.get('common_previous_label'),
          disabled: false,
          onClick: modal => {
            this.previous(() => callback());
          }
        },
        {
          type: 'primary',
          label: this.i18n.get('common_ok_label'),
          disabled: false,
          onClick: modal => {
            if (this.isEdit) {
              this.modify().subscribe(() => this.dataTable.fetchData());
            } else {
              this.create().subscribe(() => {
                this.dataTable?.fetchData();
                callback();
              });
            }
            modal.close();
          }
        },
        {
          label: this.i18n.get('common_cancel_label'),
          onClick: modal => {
            modal.close();
          }
        }
      ]
    });
  }

  getCluster(recordsTemp?: any[], startPage?: number) {
    this.storageUnitService
      .queryBackUnitGET({
        pageNo: startPage || CommonConsts.PAGE_START,
        pageSize: CommonConsts.PAGE_SIZE * 10
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
          const cluster = [];
          each(recordsTemp, item => {
            cluster.push({
              uuid: item.id,
              threshold: item['availableCapacityRatio'] || 90,
              usedPercent: (item.usedCapacity * 100) / item.capacity,
              ...item
            });
          });
          cluster.sort((a, b) => a.status - b.status);
          if (this.appUtilsService.isDecouple) {
            this.sourceData = cluster;
          } else {
            this.sourceData = cluster.filter(item => {
              return (
                item.generatedType ===
                DataMap.backupStorageGeneratedType.local.value
              );
            });
          }
          this.localSourceData = cluster.filter(item => {
            return (
              item.generatedType ===
              DataMap.backupStorageGeneratedType.local.value
            );
          });
          this.nonLocalSourceData = cluster.filter(item => {
            return (
              item.generatedType !==
              DataMap.backupStorageGeneratedType.local.value
            );
          });
          this.allSourceData = cluster;
          if (this.rowData) {
            this.getData();
          } else {
            this.targetData = [...this.targetData];
            this.localTargetData = [...this.localTargetData];
            this.nonLocalTargetData = [...this.nonLocalTargetData];
          }
          if (this.appUtilsService.isDecouple && this.rowData) {
            // 有设备类型时要根据设备类型去过滤存储单元
            this.formGroup.get('deviceType').disable();
            defer(() =>
              this.formGroup.get('deviceType').updateValueAndValidity()
            );
          }
          this.cdr.detectChanges();
          return;
        }
        this.getCluster(recordsTemp, startPage);
      });
  }

  getData() {
    this.nasDistributionStoragesApiService
      .NasDistributionStorageInfo({ id: this.rowData.uuid })
      .subscribe(res => {
        const content = this.SetStoragePolicyComponent;
        content.timeForm.patchValue({
          storageStrategyType: res.storageStrategyType,
          timeoutPeriod: res.timeoutPeriod || ''
        });
        each(res.unitList, item => {
          // 开了并行计算
          if (
            res.hasEnableParallelStorage &&
            !this.appUtilsService.isDecouple
          ) {
            this.localTargetData = [
              ...this.localTargetData,
              assign(
                find(
                  [...this.localSourceData, ...this.nonLocalSourceData],
                  data => data.id === item.unitId
                ),
                {
                  threshold: get(item, 'availableCapacityRatio')
                }
              )
            ];
            this.nonLocalTargetData = [...this.localTargetData];
            // 没开并行计算
          } else {
            this.targetData = [
              ...this.targetData,
              assign(
                find(this.sourceData, data => data.id === item.unitId),
                {
                  threshold: get(item, 'availableCapacityRatio')
                }
              )
            ];
          }
        });
        this.formGroup.get('selected').setValue(this.targetData);
        this.formGroup
          .get('nonLocalSelected')
          .setValue(this.nonLocalTargetData);
        this.formGroup.get('localSelected').setValue(this.localTargetData);
        if (res.hasEnableParallelStorage && !this.appUtilsService.isDecouple) {
          this.selection = [...this.localTargetData];
        } else {
          this.selection = [...this.targetData];
        }
        this.selection.forEach(item => this.checkChange(true, item));

        this.cdr.detectChanges();
      });
  }

  initForm() {
    const reg = /^[a-zA-Z_0-9]{0,32}$/;

    this.formGroup = this.fb.group({
      name: new FormControl(
        { value: '', disabled: this.rowData ? true : false },
        {
          validators: [
            this.baseUtilService.VALID.maxLength(32),
            this.baseUtilService.VALID.name(reg)
          ]
        }
      ),
      deviceType: ['', { validators: [this.baseUtilService.VALID.required()] }],
      desc: new FormControl('', {
        validators: [this.baseUtilService.VALID.maxLength(255)]
      }),
      timeoutPeriod: new FormControl(15, {
        validators: [
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.integer(),
          this.baseUtilService.VALID.rangeValue(1, 120)
        ]
      }),
      selected: new FormControl([], {
        validators: [
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.minLength(1),
          this.baseUtilService.VALID.maxLength(32)
        ]
      }),
      localSelected: new FormControl([]),
      nonLocalSelected: new FormControl([]),
      hasEnableParallelStorage: new FormControl(false)
    });

    this.formGroup.valueChanges.subscribe(res => {
      if (this.formGroup.value.selected.length > 32) {
        this.messageService.error(
          this.i18n.get('protection_distributed_nas_limit_label'),
          {
            lvMessageKey: 'distributed_nas_limit_key',
            lvShowCloseButton: true
          }
        );
      }
    });

    if (!this.appUtilsService.isDecouple) {
      this.formGroup
        .get('hasEnableParallelStorage')
        .valueChanges.subscribe(res => {
          this.selection = this.selection.filter(
            item =>
              item.generatedType ===
              DataMap.backupStorageGeneratedType.local.value
          );
          this.targetData = [...this.selection];
          this.localTargetData = [...this.selection];
          this.nonLocalTargetData = [...this.selection];
          this.formGroup.get('localSelected').setValue(this.localTargetData);
          this.formGroup
            .get('nonLocalSelected')
            .setValue(this.nonLocalTargetData);
          this.formGroup.get('selected').setValue(this.targetData);
          if (res) {
            this.formGroup
              .get('localSelected')
              .setValidators([
                this.baseUtilService.VALID.required(),
                this.validLocalSelection(),
                this.baseUtilService.VALID.maxLength(32)
              ]);
            this.formGroup
              .get('nonLocalSelected')
              .setValidators([this.baseUtilService.VALID.maxLength(32)]);
            this.formGroup.get('selected').clearValidators();
          } else {
            this.formGroup
              .get('selected')
              .setValidators([
                this.baseUtilService.VALID.required(),
                this.baseUtilService.VALID.minLength(1),
                this.baseUtilService.VALID.maxLength(32)
              ]);
            this.formGroup.get('localSelected').clearValidators();
            this.formGroup.get('nonLocalSelected').clearValidators();
          }
          this.formGroup.get('selected').updateValueAndValidity();
          this.formGroup.get('localSelected').updateValueAndValidity();
          this.formGroup.get('nonLocalSelected').updateValueAndValidity();
          each(this.nonLocalSourceData, item => {
            item.disabled = false;
          });
        });
    }

    if (this.isDecouple || this.isDistributed) {
      this.formGroup.get('deviceType').valueChanges.subscribe(res => {
        // 下面要过滤对应设备的存储单元
        this.sourceData = filter(
          this.allSourceData,
          item =>
            item.deviceType === res ||
            (res === DataMap.poolStorageDeviceType.Server.value &&
              item.deviceType === 'BasicDisk')
        );
        this.selection = [];
        this.targetData = [];
        each(this.sourceData, item => {
          item.disabled = false;
        });
      });
    } else {
      this.formGroup.get('deviceType').clearValidators();
      this.formGroup.get('deviceType').updateValueAndValidity();
    }

    if (this.rowData) {
      this.formGroup.patchValue({
        name: this.rowData.name,
        desc: this.rowData.description,
        deviceType: this.rowData?.deviceType,
        hasEnableParallelStorage: this.rowData.hasEnableParallelStorage,
        timeoutPeriod: this.rowData?.timeoutPeriod
      });
    }
  }

  validLocalSelection(): ValidatorFn {
    return (control: AbstractControl): { [key: string]: any } | null => {
      if (!control.value) {
        return null;
      }
      if (
        !control.value.some(
          item =>
            item.generatedType ===
            DataMap.backupStorageGeneratedType.local.value
        )
      ) {
        return { errMsg: '' };
      }
      return null;
    };
  }

  dataChange() {
    this.formGroup.get('selected').setValue(this.targetData);
  }

  remove(source) {
    const deleteKeys = map([source], 'id');
    remove(this.selection, item => deleteKeys.includes(item.id));
    this.selection = [...this.selection];
    this.changeTransferData(this.selection);
  }

  selectionChange(selection) {
    this.changeTransferData(selection);
  }

  removeAll() {
    this.selection = [];
    this.changeTransferData(this.selection);
  }
  changeTransferData(selectedData) {
    if (
      this.formGroup.value.hasEnableParallelStorage &&
      !this.appUtilsService.isDecouple
    ) {
      this.localTargetData = [...selectedData];
      this.formGroup.get('localSelected').setValue(this.localTargetData);
      this.nonLocalTargetData = [...selectedData];
      this.formGroup.get('nonLocalSelected').setValue(this.nonLocalTargetData);
    } else {
      this.targetData = [...selectedData];
      this.formGroup.get('selected').setValue(this.targetData);
    }
  }

  tabChange(newVal) {
    if (newVal === 'localCluster') {
      this.selection = [...this.localTargetData];
    } else {
      this.selection = [...this.nonLocalTargetData];
    }
  }

  checkChange(event, checkItem) {
    const currentData =
      this.formGroup.value.hasEnableParallelStorage &&
      !this.appUtilsService.isDecouple
        ? [...this.localSourceData, ...this.nonLocalSourceData]
        : this.sourceData;
    each(currentData, item => {
      if (item.deviceId === checkItem.deviceId && item.id !== checkItem.id) {
        item.disabled = !!event;
      }
    });
  }

  getParms() {
    const content = this.SetStoragePolicyComponent;
    const hasEnable = this.formGroup.get('hasEnableParallelStorage').value;
    let clusterIdList = [];
    if (!hasEnable) {
      clusterIdList = map(content.tableData, (item, index) => {
        if (content.timeForm.value.storageStrategyType !== 4) {
          return {
            unitId: item.id,
            availableCapacityRatio: item.threshold,
            strategyOrder: index
          };
        } else {
          return {
            unitId: item.id,
            availableCapacityRatio: item.threshold
          };
        }
      });
    } else {
      const data = [...this.localTargetData];
      clusterIdList = map(data, item => {
        return {
          unitId: item.id,
          availableCapacityRatio: item.threshold
        };
      });
    }

    const params = {
      name: this.formGroup.get('name').value,
      description: this.formGroup.get('desc').value,
      hasEnableParallelStorage: hasEnable,
      storageUnitReqs: clusterIdList,
      timeoutPeriod: this.formGroup.get('timeoutPeriod').value
    };
    if (!hasEnable) {
      assign(params, {
        storageStrategyType: content.timeForm.value.storageStrategyType
      });
    }
    if (this.isDecouple || this.isDistributed) {
      assign(params, {
        deviceType: this.formGroup.get('deviceType').value
      });
    }
    return params;
  }
  modify(): Observable<void> {
    return new Observable<void>((observer: Observer<void>) => {
      const params = this.getParms();
      this.nasDistributionStoragesApiService
        .UpdateNasDistributionStorage({
          id: this.rowData.uuid,
          UpdateClusterRepositoryRequestBody: params as any
        })
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

  create(): Observable<void> {
    return new Observable<void>((observer: Observer<void>) => {
      const params = this.getParms();
      this.nasDistributionStoragesApiService
        .CreateNasDistributionStorage({
          CreateNasDistributionStorageRequestBody: params as any
        })
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
