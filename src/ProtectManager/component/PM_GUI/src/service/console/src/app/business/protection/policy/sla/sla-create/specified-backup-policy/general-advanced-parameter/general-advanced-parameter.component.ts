import { Component, Input, OnInit } from '@angular/core';
import { FormControl, FormGroup } from '@angular/forms';
import {
  BaseUtilService,
  ClustersApiService,
  DataMapService,
  I18NService,
  NasDistributionStoragesApiService,
  ProtectResourceAction
} from 'app/shared';
import { AppUtilsService } from 'app/shared/services/app-utils.service';
import { assign, find, first, map, size } from 'lodash';

@Component({
  selector: 'aui-general-advanced-parameter',
  templateUrl: './general-advanced-parameter.component.html',
  styleUrls: ['./general-advanced-parameter.component.less']
})
export class GeneralAdvancedParameterComponent implements OnInit {
  find = find;
  @Input() isSlaDetail: boolean;
  @Input() action: any;
  @Input() data: any;
  @Input() formGroup: FormGroup;
  @Input() isUsed: boolean;
  retryTimesErrorTip = assign({}, this.baseUtilService.rangeErrorTip, {
    invalidRang: this.i18n.get('common_valid_rang_label', [1, 5])
  });
  waitTimesErrorTip = assign({}, this.baseUtilService.rangeErrorTip, {
    invalidRang: this.i18n.get('common_valid_rang_label', [1, 30])
  });
  isRetry = true;
  protectResourceAction = ProtectResourceAction;

  constructor(
    private baseUtilService: BaseUtilService,
    public i18n: I18NService,
    public dataMapService: DataMapService,
    private clusterApiService: ClustersApiService,
    private nasDistributionStoragesApiService: NasDistributionStoragesApiService,
    public appUtilsService?: AppUtilsService
  ) {}

  ngOnInit() {
    this.updateForm();
    this.updateData();
  }

  updateForm() {
    this.formGroup.addControl('storage_type', new FormControl(''));
    this.formGroup.addControl('storage_id', new FormControl(''));
    this.formGroup.addControl('alarm_over_time_window', new FormControl(false));
    this.formGroup.addControl('alarm_after_failure', new FormControl(true));
    this.formGroup.addControl('auto_retry', new FormControl(true));
    this.formGroup.addControl(
      'enable_deduption_compression',
      new FormControl(true)
    );
    this.formGroup.addControl(
      'auto_retry_times',
      new FormControl(3, {
        validators: [
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.integer(),
          this.baseUtilService.VALID.rangeValue(1, 5)
        ]
      })
    );
    this.formGroup.addControl(
      'auto_retry_wait_minutes',
      new FormControl(5, {
        validators: [
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.integer(),
          this.baseUtilService.VALID.rangeValue(1, 30)
        ]
      })
    );
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
            ]
          })
        );
        this.formGroup.addControl(
          'auto_retry_wait_minutes',
          new FormControl(5, {
            validators: [
              this.baseUtilService.VALID.required(),
              this.baseUtilService.VALID.integer(),
              this.baseUtilService.VALID.rangeValue(1, 30)
            ]
          })
        );
      }
    });
  }

  updateData() {
    if (!this.data || !size(this.data)) {
      return;
    }

    const data = first(map(this.data, 'ext_parameters'));
    if (data) {
      this.formGroup.patchValue(data);
    }
    this.isRetry = data.auto_retry;
  }
}
