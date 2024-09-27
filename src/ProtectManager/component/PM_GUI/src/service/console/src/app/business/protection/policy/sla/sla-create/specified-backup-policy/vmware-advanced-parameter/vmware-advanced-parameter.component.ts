import { Component, Input, OnInit } from '@angular/core';
import { FormBuilder, FormControl, FormGroup } from '@angular/forms';
import { ModalRef } from '@iux/live';
import {
  BaseUtilService,
  CommonConsts,
  CookieService,
  DataMap,
  DataMapService,
  I18NService,
  ProtectResourceAction,
  QosService
} from 'app/shared';
import { AppUtilsService } from 'app/shared/services/app-utils.service';
import { assign, find, first, map, size } from 'lodash';

@Component({
  selector: 'aui-vmware-advanced-parameter',
  templateUrl: './vmware-advanced-parameter.component.html',
  styleUrls: ['./vmware-advanced-parameter.component.less']
})
export class VmwareAdvancedParameterComponent implements OnInit {
  find = find;
  size = size;
  qosNames = [];
  protectResourceAction = ProtectResourceAction;
  dataMap = DataMap;
  @Input() isSlaDetail: boolean;
  @Input() action: any;
  @Input() data: any;
  @Input() formGroup: FormGroup;
  @Input() isUsed: boolean;

  capacityThresholdErrorTip = assign({}, this.baseUtilService.rangeErrorTip, {
    invalidRang: this.i18n.get('common_valid_rang_label', [0, 100])
  });

  retryTimesErrorTip = assign({}, this.baseUtilService.rangeErrorTip, {
    invalidRang: this.i18n.get('common_valid_rang_label', [1, 5])
  });
  waitTimesErrorTip = assign({}, this.baseUtilService.rangeErrorTip, {
    invalidRang: this.i18n.get('common_valid_rang_label', [1, 30])
  });
  filesErrorTip = {
    ...this.baseUtilService.requiredErrorTip
  };

  isHcsUser = this.cookieService.get('userType') === CommonConsts.HCS_USER_TYPE;
  isRetry = true;
  isDisableQos = false; // 如果选择了本地盘的存储单元就禁止限速策略

  constructor(
    public baseUtilService: BaseUtilService,
    public i18n: I18NService,
    public fb: FormBuilder,
    public modal: ModalRef,
    private qosServiceApi: QosService,
    private cookieService: CookieService,
    public dataMapService: DataMapService,
    public appUtilsService?: AppUtilsService
  ) {}

  ngOnInit() {
    this.getQosNames();
    this.updateForm();
    this.updateData();
  }

  updateData() {
    if (!this.data || !size(this.data)) {
      return;
    }

    const data = first(map(this.data, 'ext_parameters'));
    if (!data.specifies_transfer_mode) {
      data.specifies_transfer_mode = DataMap.vmwareTransferMode.san.value;
    }
    if (data) {
      this.formGroup.patchValue(data);
    }
    this.isRetry = data.auto_retry;
  }

  updateForm() {
    this.formGroup.addControl('storage_type', new FormControl(''));
    this.formGroup.addControl('storage_id', new FormControl(''));
    this.formGroup.addControl('fine_grained_restore', new FormControl(false));
    this.formGroup.addControl(
      'enable_deduption_compression',
      new FormControl(true)
    );
    this.formGroup.addControl(
      'ensure_consistency_backup',
      new FormControl(true)
    );

    this.formGroup.addControl('qos_id', new FormControl(''));
    this.formGroup.addControl('auto_retry', new FormControl(true));
    this.formGroup.addControl('alarm_over_time_window', new FormControl(false));
    this.formGroup.addControl('alarm_after_failure', new FormControl(true));
    this.formGroup.addControl('source_deduplication', new FormControl(false));
    this.formGroup.addControl(
      'ensure_storage_layer_backup',
      new FormControl(false)
    );

    this.formGroup.addControl('ensure_deleted_data', new FormControl(false));
    this.formGroup.addControl(
      'ensure_specifies_transfer_mode',
      new FormControl(false)
    );
    this.formGroup.addControl(
      'specifies_transfer_mode',
      new FormControl(DataMap.vmwareTransferMode.san.value)
    );

    this.formGroup.addControl(
      'available_capacity_threshold',
      new FormControl(20, {
        validators: [
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.integer(),
          this.baseUtilService.VALID.rangeValue(0, 100)
        ],
        updateOn: 'change'
      })
    );

    this.formGroup.addControl(
      'auto_retry_times',
      new FormControl(3, {
        validators: [
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.integer(),
          this.baseUtilService.VALID.rangeValue(1, 5)
        ],
        updateOn: 'change'
      })
    );
    this.formGroup.addControl(
      'auto_retry_wait_minutes',
      new FormControl(5, {
        validators: [
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.integer(),
          this.baseUtilService.VALID.rangeValue(1, 30)
        ],
        updateOn: 'change'
      })
    );

    this.formGroup
      .get('ensure_storage_layer_backup')
      .valueChanges.subscribe(res => {
        if (res) {
          this.formGroup
            .get('ensure_deleted_data')
            .setValue(false, { emitEvent: false });
          this.formGroup
            .get('ensure_specifies_transfer_mode')
            .setValue(false, { emitEvent: false });
        }
      });

    this.formGroup.get('ensure_deleted_data').valueChanges.subscribe(res => {
      if (res) {
        this.formGroup
          .get('ensure_storage_layer_backup')
          .setValue(false, { emitEvent: false });
      }
    });

    this.formGroup
      .get('ensure_specifies_transfer_mode')
      .valueChanges.subscribe(res => {
        if (res) {
          this.formGroup
            .get('ensure_storage_layer_backup')
            .setValue(false, { emitEvent: false });
        }
      });

    this.formGroup.get('auto_retry').valueChanges.subscribe(res => {
      if (!res) {
        this.formGroup.removeControl('auto_retry_times');
        this.formGroup.removeControl('auto_retry_wait_minutes');
      } else {
        this.formGroup.addControl(
          'auto_retry_times',
          new FormControl(3, {
            validators: [
              this.baseUtilService.VALID.required(),
              this.baseUtilService.VALID.integer(),
              this.baseUtilService.VALID.rangeValue(1, 5)
            ],
            updateOn: 'change'
          })
        );
        this.formGroup.addControl(
          'auto_retry_wait_minutes',
          new FormControl(5, {
            validators: [
              this.baseUtilService.VALID.required(),
              this.baseUtilService.VALID.integer(),
              this.baseUtilService.VALID.rangeValue(1, 30)
            ],
            updateOn: 'change'
          })
        );
      }
    });
  }

  storageTypeChange(e) {
    this.isDisableQos = e;
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
}
