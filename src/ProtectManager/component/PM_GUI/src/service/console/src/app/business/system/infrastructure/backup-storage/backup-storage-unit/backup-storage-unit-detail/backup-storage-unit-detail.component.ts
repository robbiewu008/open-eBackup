import {
  ChangeDetectorRef,
  Component,
  Input,
  OnInit,
  TemplateRef,
  ViewChild
} from '@angular/core';
import { FormBuilder, FormControl, FormGroup } from '@angular/forms';
import {
  BaseUtilService,
  CAPACITY_UNIT,
  ColorConsts,
  CommonConsts,
  DataMap,
  I18NService,
  NasDistributionStoragesApiService
} from 'app/shared';
import { StoragePoolService } from 'app/shared/api/services/storage-pool.service';
import { TableCols, TableConfig } from 'app/shared/components/pro-table';
import { AppUtilsService } from 'app/shared/services/app-utils.service';
import { assign, size } from 'lodash';

@Component({
  selector: 'aui-backup-storage-unit-detail',
  templateUrl: './backup-storage-unit-detail.component.html',
  styleUrls: ['./backup-storage-unit-detail.component.less']
})
export class BackupStorageUnitDetailComponent implements OnInit {
  hosts;
  formItems = [];
  unitconst = CAPACITY_UNIT;
  dataMap = DataMap;
  tableData;
  resSubType;
  tableConfig: TableConfig;
  progressBarColor = [[0, ColorConsts.NORMAL]];
  isEdit = false;
  isView = true;
  lessThanLabel = this.i18n.get('common_less_than_label');
  formGroup: FormGroup;
  thresholdErrorTip = {
    ...this.baseUtilService.rangeErrorTip,
    invalidRang: this.i18n.get('common_valid_rang_label', [1, 95])
  };
  preLimitValue;
  protected readonly Math = Math;

  @ViewChild('capacity', { static: true })
  capacity: TemplateRef<any>;
  @ViewChild('thresholdTpl', { static: true })
  thresholdTpl: TemplateRef<any>;

  @ViewChild('progressLabelTpl', { static: true })
  progressLabelTpl: TemplateRef<any>;

  @Input() data: any;
  constructor(
    private i18n: I18NService,
    private cdr: ChangeDetectorRef,
    private nasDistributionStoragesApiService: NasDistributionStoragesApiService,
    public baseUtilService: BaseUtilService,
    private fb: FormBuilder,
    public storagePoolService: StoragePoolService,
    public appUtilsService: AppUtilsService
  ) {}

  ngOnInit() {
    if (this.data.deviceType === 'BasicDisk') {
      // 服务器类型的值在下发时不一样，所以需要单独转换
      this.data.deviceType = DataMap.poolStorageDeviceType.Server.value;
    }
    this.initConfig();
    this.initForm();
    this.getData();
  }

  initConfig() {
    const cols: TableCols[] = [
      {
        key: 'id',
        name: 'id',
        hidden: true
      },
      {
        key: 'clusterId',
        name: 'ClusterId',
        hidden: true
      },
      {
        key: 'clusterName',
        name: this.i18n.get('common_name_label')
      },
      {
        key: 'healthStatus',
        name: this.i18n.get('healthStatus')
      },
      {
        key: 'runningStatus',
        name: this.i18n.get('runningStatus')
      },
      {
        key: 'deviceType',
        name: this.i18n.get('common_equipment_type_label')
      },
      {
        key: 'deviceName',
        name: this.i18n.get('protection_storage_device_label')
      },
      {
        key: 'poolName',
        name: 'poolName',
        filter: {
          type: 'search',
          filterMode: 'contains'
        }
      },
      {
        key: 'threshold',
        name: this.i18n.get('system_capacity_threshold_label'),
        cellRender: this.thresholdTpl
      },
      {
        key: 'progressLabelTpl',
        name: this.i18n.get('common_capacity_label'),
        cellRender: this.progressLabelTpl
      }
    ];

    this.tableConfig = {
      table: {
        compareWith: 'clusterId',
        size: 'default',
        columns: cols,
        virtualScroll: true,
        scrollFixed: true,
        scroll: { y: '420px' },
        colDisplayControl: false,
        autoPolling: CommonConsts.TIME_INTERVAL,
        fetchData: () => {
          this.getData();
        }
      },
      pagination: null
    };
  }

  modifyThreshold(opt = false) {
    this.isEdit = opt;
    if (!opt) {
      this.formGroup.get('limitValue').setValue(this.preLimitValue);
    }
    if (this.isEdit) {
      this.formGroup
        .get('limitValue')
        .setValidators([
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.integer(),
          this.baseUtilService.VALID.rangeValue(1, 95)
        ]);
    } else {
      this.formGroup.get('limitValue').clearValidators();
    }
    this.formGroup.get('limitValue').updateValueAndValidity();
  }

  updateThreshold() {
    if (this.formGroup.get('limitValue').valid) {
      const params: any = {};
      assign(params, {
        deviceId: this.data.deviceId,
        threshold: +this.formGroup.value.limitValue,
        poolId: this.data.poolId
      });

      this.storagePoolService
        .modifyPoolThresholdPUT({
          storagePoolThresholdRequest: params
        })
        .subscribe(res => {
          this.isEdit = false;
          this.preLimitValue = this.formGroup.value.limitValue;
        });
    }
  }

  getData() {
    this.preLimitValue = this.data.threshold;
    this.formGroup.get('limitValue').setValue(this.preLimitValue);
    this.tableData = {
      data: this.data,
      total: size(this.data)
    };
    this.cdr.detectChanges();
  }

  initForm() {
    this.formGroup = this.fb.group({
      limitValue: new FormControl(this.data.threshold)
    });
  }
}
