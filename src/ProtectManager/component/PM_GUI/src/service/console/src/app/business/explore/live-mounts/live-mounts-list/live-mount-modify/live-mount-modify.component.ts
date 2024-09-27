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
import { Component, OnInit } from '@angular/core';
import {
  AbstractControl,
  FormBuilder,
  FormControl,
  FormGroup,
  ValidatorFn
} from '@angular/forms';
import { ModalRef } from '@iux/live';
import {
  BaseUtilService,
  CommonConsts,
  DataMapService,
  I18NService,
  LiveMountApiService,
  LiveMountPolicyApiService,
  RetentionPolicy,
  DataMap,
  LANGUAGE
} from 'app/shared';
import {
  assign,
  each,
  includes,
  isEmpty,
  isNumber,
  map as _map,
  omit,
  toString,
  trim,
  forOwn,
  pick,
  first
} from 'lodash';
import { Observable, Observer } from 'rxjs';
import { pairwise } from 'rxjs/operators';

@Component({
  selector: 'aui-live-mount-modify',
  templateUrl: './live-mount-modify.component.html',
  styleUrls: ['./live-mount-modify.component.less']
})
export class LiveMountModifyComponent implements OnInit {
  item;
  componentData;
  dataMap = DataMap;
  formGroup: FormGroup;
  policyOptions = [];
  latencyOptions = this.dataMapService
    .toArray('LiveMount_Latency')
    .filter(v => (v.isLeaf = true));

  spaceLabel = this.i18n.language === LANGUAGE.CN ? '' : ' ';
  minBandwidthErrorTip = assign({}, this.baseUtilService.rangeErrorTip, {
    invalidRang: this.i18n.get('common_valid_rang_label', [1, 999999999]),
    invalidMin: this.i18n.get('explore_min_max_valid_label')
  });
  maxBandwidthErrorTip = assign({}, this.baseUtilService.rangeErrorTip, {
    invalidRang: this.i18n.get('common_valid_rang_label', [1, 999999999]),
    invalidMax: this.i18n.get('explore_max_min_valid_label')
  });
  burstBandwidthErrorTip = assign({}, this.baseUtilService.rangeErrorTip, {
    invalidRang: this.i18n.get('common_valid_rang_label', [1, 999999999]),
    invalidBurst: this.i18n.get('explore_burst_valid_label')
  });
  minIopsErrorTip = assign({}, this.baseUtilService.rangeErrorTip, {
    invalidRang: this.i18n.get('common_valid_rang_label', [100, 999999999]),
    invalidMin: this.i18n.get('explore_min_max_valid_label')
  });
  maxIopsErrorTip = assign({}, this.baseUtilService.rangeErrorTip, {
    invalidRang: this.i18n.get('common_valid_rang_label', [100, 999999999]),
    invalidMax: this.i18n.get('explore_max_min_valid_label')
  });
  burstIopsErrorTip = assign({}, this.baseUtilService.rangeErrorTip, {
    invalidRang: this.i18n.get('common_valid_rang_label', [100, 999999999]),
    invalidBurst: this.i18n.get('explore_burst_valid_label')
  });
  burstTimeErrorTip = assign({}, this.baseUtilService.rangeErrorTip, {
    invalidRang: this.i18n.get('common_valid_rang_label', [1, 999999999])
  });
  targetHostErrorTip = assign({}, this.baseUtilService.rangeErrorTip, {
    invalidSameHost: this.i18n.get('explore_target_same_host_label')
  });
  scriptNameErrorTip = assign({}, this.baseUtilService.scriptNameErrorTip, {
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [64])
  });
  mysqlPortErrorTip = assign({}, this.baseUtilService.rangeErrorTip, {
    invalidRang: this.i18n.get('common_valid_rang_label', [1024, 65535])
  });
  iopsItems = [
    {
      id: 850000,
      header: '16KB'
    },
    {
      id: 680000,
      header: '32KB'
    },
    {
      id: 450000,
      header: '64KB'
    },
    {
      id: 225000,
      header: '128KB'
    },
    {
      id: 112500,
      header: '256KB'
    },
    {
      id: 56250,
      header: '512KB'
    },
    {
      id: 28125,
      header: '1024KB'
    },
    {
      id: 14063,
      header: '2048KB'
    }
  ];
  latencyItems = [
    {
      id: 850000,
      header: '16KB'
    },
    {
      id: 680000,
      header: '32KB'
    },
    {
      id: 450000,
      header: '64KB'
    },
    {
      id: 225000,
      header: '128KB'
    },
    {
      id: 112500,
      header: '256KB'
    },
    {
      id: 56250,
      header: '512KB'
    },
    {
      id: 28125,
      header: '1024KB'
    },
    {
      id: 14063,
      header: '2048KB'
    }
  ];

  constructor(
    private i18n: I18NService,
    private fb: FormBuilder,
    private modal: ModalRef,
    private dataMapService: DataMapService,
    public baseUtilService: BaseUtilService,
    private liveMountApiService: LiveMountApiService,
    private liveMountPolicyApiService: LiveMountPolicyApiService
  ) {}

  ngOnInit() {
    this.initForm();
    this.getPolicies();
    this.updateForm();
  }

  updateForm() {
    const parameters = forOwn(
      JSON.parse(this.item.parameters),
      (value, key, object) => {
        return !value ? (object[key] = '') : (object[key] = value);
      }
    );

    if (
      this.item.resource_sub_type === DataMap.Resource_Type.virtualMachine.value
    ) {
      assign(parameters, {
        ...parameters.performance
      });
    }

    this.formGroup.patchValue({
      ...parameters,
      ...parameters.performance,
      bindWidthStatus:
        !isEmpty(toString(parameters.performance.min_bandwidth)) &&
        !isEmpty(toString(parameters.performance.max_bandwidth)),
      iopsStatus:
        !isEmpty(toString(parameters.performance.min_iops)) &&
        !isEmpty(toString(parameters.performance.max_iops)),
      latencyStatus: !isEmpty(toString(parameters.performance.latency)),
      policyId: this.item.policy_id
    });
    setTimeout(() => {
      this.modal.getInstance().lvOkDisabled = this.formGroup.invalid;
    }, 0);
  }

  updateIopsItems(value, type: 'min' | 'max' | 'burst') {
    each(this.iopsItems, item => {
      let obj = {};
      obj[type] =
        isNaN(+value) || !trim(value)
          ? '--'
          : Math.round((item.id * value) / 1000000);
      assign(item, obj);
    });
  }

  updateLatencyData(value) {
    each(this.latencyItems, item => {
      const max = isNaN(+value)
        ? '--'
        : Math.round((1000000 * value) / item.id) / 1000 + 'ms';
      assign(item, { max });
    });
  }

  initForm() {
    this.formGroup = this.fb.group({
      bindWidthStatus: new FormControl(false),
      iopsStatus: new FormControl(false),
      latencyStatus: new FormControl(false),
      min_bandwidth: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.integer(),
          this.baseUtilService.VALID.rangeValue(1, 999999999),
          this.validMinBandWidth()
        ],
        updateOn: 'change'
      }),
      max_bandwidth: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.integer(),
          this.baseUtilService.VALID.rangeValue(1, 999999999),
          this.validMaxBandWidth()
        ],
        updateOn: 'change'
      }),
      burst_bandwidth: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.integer(),
          this.baseUtilService.VALID.rangeValue(1, 999999999),
          this.validBurstBandWidth()
        ],
        updateOn: 'change'
      }),
      min_iops: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.integer(),
          this.baseUtilService.VALID.rangeValue(100, 999999999),
          this.validMinIops()
        ],
        updateOn: 'change'
      }),
      max_iops: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.integer(),
          this.baseUtilService.VALID.rangeValue(100, 999999999),
          this.validMaxIops()
        ],
        updateOn: 'change'
      }),
      burst_iops: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.integer(),
          this.baseUtilService.VALID.rangeValue(100, 999999999),
          this.validBurstIops()
        ],
        updateOn: 'change'
      }),
      burst_time: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.integer(),
          this.baseUtilService.VALID.rangeValue(1, 999999999)
        ],
        updateOn: 'change'
      }),
      latency: new FormControl('', {
        validators: [this.baseUtilService.VALID.required()],
        updateOn: 'change'
      }),
      pre_script: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.maxLength(64),
          this.baseUtilService.VALID.name(CommonConsts.REGEX.linuxScript, false)
        ],
        updateOn: 'change'
      }),
      post_script: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.maxLength(64),
          this.baseUtilService.VALID.name(CommonConsts.REGEX.linuxScript, false)
        ],
        updateOn: 'change'
      }),
      failed_script: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.maxLength(64),
          this.baseUtilService.VALID.name(CommonConsts.REGEX.linuxScript, false)
        ],
        updateOn: 'change'
      }),
      policyId: new FormControl('')
    });

    if (
      this.componentData.childResourceType &&
      this.dataMap.Resource_Type.tdsqlInstance.value.includes(
        this.componentData.childResourceType[0]
      )
    ) {
      this.formGroup.addControl(
        'mysql_port',
        new FormControl(
          { value: '', disabled: true },
          {
            validators: [
              this.baseUtilService.VALID.required(),
              this.baseUtilService.VALID.integer(),
              this.baseUtilService.VALID.rangeValue(1024, 65535)
            ]
          }
        )
      );
    }
    this.formGroup.get('bindWidthStatus').valueChanges.subscribe(res => {
      if (!res) {
        this.formGroup.get('min_bandwidth').clearValidators();
        this.formGroup.get('max_bandwidth').clearValidators();
        if (this.formGroup.get('burst_bandwidth')) {
          this.formGroup.get('burst_bandwidth').clearValidators();
        }
        if (
          !(
            (this.formGroup.value.iopsStatus &&
              this.formGroup.value.burst_iops) ||
            (this.formGroup.value.bindWidthStatus &&
              this.formGroup.value.burst_bandwidth)
          )
        ) {
          this.formGroup.get('burst_time').clearValidators();
        }
      } else {
        this.formGroup
          .get('min_bandwidth')
          .setValidators([
            this.baseUtilService.VALID.required(),
            this.baseUtilService.VALID.integer(),
            this.baseUtilService.VALID.rangeValue(1, 999999999),
            this.validMinBandWidth()
          ]);
        this.formGroup
          .get('max_bandwidth')
          .setValidators([
            this.baseUtilService.VALID.required(),
            this.baseUtilService.VALID.integer(),
            this.baseUtilService.VALID.rangeValue(1, 999999999),
            this.validMaxBandWidth()
          ]);
        if (this.formGroup.get('burst_bandwidth')) {
          this.formGroup
            .get('burst_bandwidth')
            .setValidators([
              this.baseUtilService.VALID.integer(),
              this.baseUtilService.VALID.rangeValue(1, 999999999),
              this.validBurstBandWidth()
            ]);
        }

        if (
          (this.formGroup.value.iopsStatus &&
            this.formGroup.value.burst_iops) ||
          (this.formGroup.value.bindWidthStatus &&
            this.formGroup.value.burst_bandwidth)
        ) {
          this.formGroup
            .get('burst_time')
            .setValidators([
              this.baseUtilService.VALID.required(),
              this.baseUtilService.VALID.integer(),
              this.baseUtilService.VALID.rangeValue(1, 999999999)
            ]);
        }
      }
      this.formGroup.get('burst_time').updateValueAndValidity();
      this.formGroup.get('min_bandwidth').updateValueAndValidity();
      this.formGroup.get('max_bandwidth').updateValueAndValidity();
    });
    this.formGroup.get('iopsStatus').valueChanges.subscribe(res => {
      if (!res) {
        this.formGroup.get('min_iops').clearValidators();
        this.formGroup.get('max_iops').clearValidators();
        if (this.formGroup.get('burst_iops')) {
          this.formGroup.get('burst_iops').clearValidators();
        }
        if (
          !(
            (this.formGroup.value.iopsStatus &&
              this.formGroup.value.burst_iops) ||
            (this.formGroup.value.bindWidthStatus &&
              this.formGroup.value.burst_bandwidth)
          )
        ) {
          this.formGroup.get('burst_time').clearValidators();
        }
      } else {
        this.formGroup
          .get('min_iops')
          .setValidators([
            this.baseUtilService.VALID.required(),
            this.baseUtilService.VALID.integer(),
            this.baseUtilService.VALID.rangeValue(100, 999999999),
            this.validMinIops()
          ]);
        this.formGroup
          .get('max_iops')
          .setValidators([
            this.baseUtilService.VALID.required(),
            this.baseUtilService.VALID.integer(),
            this.baseUtilService.VALID.rangeValue(100, 999999999)
          ]);
        if (this.formGroup.get('burst_iops')) {
          this.formGroup
            .get('burst_iops')
            .setValidators([
              this.baseUtilService.VALID.integer(),
              this.baseUtilService.VALID.rangeValue(100, 999999999),
              this.validBurstIops()
            ]);
          this.formGroup.get('burst_iops').updateValueAndValidity();
        }

        if (
          (this.formGroup.value.iopsStatus &&
            this.formGroup.value.burst_iops) ||
          (this.formGroup.value.bindWidthStatus &&
            this.formGroup.value.burst_bandwidth)
        ) {
          this.formGroup
            .get('burst_time')
            .setValidators([
              this.baseUtilService.VALID.required(),
              this.baseUtilService.VALID.integer(),
              this.baseUtilService.VALID.rangeValue(1, 999999999)
            ]);
        }
      }
      this.formGroup.get('burst_time').updateValueAndValidity();
      this.formGroup.get('min_iops').updateValueAndValidity();
      this.formGroup.get('max_iops').updateValueAndValidity();
    });
    this.formGroup.get('latencyStatus').valueChanges.subscribe(res => {
      if (!res) {
        this.formGroup.get('latency').clearValidators();
      } else {
        this.formGroup
          .get('latency')
          .setValidators([this.baseUtilService.VALID.required()]);
      }
      this.formGroup.get('latency').updateValueAndValidity();
    });
    this.formGroup
      .get('min_bandwidth')
      .valueChanges.pipe(pairwise())
      .subscribe(res => {
        if (toString(res[0]) === toString(res[1])) {
          return;
        }
        setTimeout(() => {
          if (
            this.formGroup.value.min_bandwidth &&
            this.formGroup.value.max_bandwidth
          ) {
            this.formGroup.get('max_bandwidth').markAsTouched();
            this.formGroup.get('max_bandwidth').updateValueAndValidity();
            if (this.formGroup.value.burst_bandwidth) {
              this.formGroup.get('burst_bandwidth').markAsTouched();
              this.formGroup.get('burst_bandwidth').updateValueAndValidity();
            }
          }
        }, 0);
      });
    this.formGroup
      .get('max_bandwidth')
      .valueChanges.pipe(pairwise())
      .subscribe(res => {
        if (toString(res[0]) === toString(res[1])) {
          return;
        }
        setTimeout(() => {
          if (
            this.formGroup.value.max_bandwidth &&
            this.formGroup.value.min_bandwidth
          ) {
            this.formGroup.get('min_bandwidth').markAsTouched();
            this.formGroup.get('min_bandwidth').updateValueAndValidity();
            if (this.formGroup.value.burst_bandwidth) {
              this.formGroup.get('burst_bandwidth').markAsTouched();
              this.formGroup.get('burst_bandwidth').updateValueAndValidity();
            }
          }
        }, 0);
      });
    this.formGroup
      .get('burst_bandwidth')
      .valueChanges.pipe(pairwise())
      .subscribe(res => {
        if (toString(res[0]) === toString(res[1])) {
          return;
        }
        setTimeout(() => {
          if (this.formGroup.value.min_bandwidth) {
            this.formGroup.get('min_bandwidth').markAsTouched();
            this.formGroup.get('min_bandwidth').updateValueAndValidity();
          }

          if (this.formGroup.value.max_bandwidth) {
            this.formGroup.get('max_bandwidth').markAsTouched();
            this.formGroup.get('max_bandwidth').updateValueAndValidity();
          }

          if (
            (this.formGroup.value.iopsStatus &&
              this.formGroup.value.burst_iops) ||
            (this.formGroup.value.bindWidthStatus &&
              this.formGroup.value.burst_bandwidth)
          ) {
            this.formGroup
              .get('burst_time')
              .setValidators([
                this.baseUtilService.VALID.required(),
                this.baseUtilService.VALID.integer(),
                this.baseUtilService.VALID.rangeValue(1, 999999999)
              ]);
          } else {
            this.formGroup.get('burst_time').clearValidators();
          }
          this.formGroup.get('burst_time').updateValueAndValidity();
        }, 0);
      });
    this.formGroup
      .get('min_iops')
      .valueChanges.pipe(pairwise())
      .subscribe(res => {
        if (this.formGroup.get('min_iops').invalid) {
          this.updateIopsItems('--', 'min');
        } else {
          this.updateIopsItems(res[1], 'min');
        }
        if (toString(res[0]) === toString(res[1])) {
          return;
        }
        setTimeout(() => {
          if (this.formGroup.value.min_iops && this.formGroup.value.max_iops) {
            this.formGroup.get('max_iops').markAsTouched();
            this.formGroup.get('max_iops').updateValueAndValidity();
            if (this.formGroup.value.burst_iops) {
              this.formGroup.get('burst_iops').markAsTouched();
              this.formGroup.get('burst_iops').updateValueAndValidity();
            }
          }
        }, 0);
      });
    this.formGroup
      .get('max_iops')
      .valueChanges.pipe(pairwise())
      .subscribe(res => {
        if (this.formGroup.get('max_iops').invalid) {
          this.updateIopsItems('--', 'max');
        } else {
          this.updateIopsItems(res[1], 'max');
        }
        if (toString(res[0]) === toString(res[1])) {
          return;
        }
        setTimeout(() => {
          if (this.formGroup.value.max_iops && this.formGroup.value.min_iops) {
            this.formGroup.get('min_iops').markAsTouched();
            this.formGroup.get('min_iops').updateValueAndValidity();
            if (this.formGroup.value.burst_iops) {
              this.formGroup.get('burst_iops').markAsTouched();
              this.formGroup.get('burst_iops').updateValueAndValidity();
            }
          }
        }, 0);
      });
    this.formGroup
      .get('burst_iops')
      .valueChanges.pipe(pairwise())
      .subscribe(res => {
        if (this.formGroup.get('burst_iops').invalid) {
          this.updateIopsItems('--', 'burst');
        } else {
          this.updateIopsItems(res[1], 'burst');
        }
        if (toString(res[0]) === toString(res[1])) {
          return;
        }
        setTimeout(() => {
          if (this.formGroup.value.min_iops) {
            this.formGroup.get('min_iops').markAsTouched();
            this.formGroup.get('min_iops').updateValueAndValidity();
          }
          if (this.formGroup.value.max_iops) {
            this.formGroup.get('max_iops').markAsTouched();
            this.formGroup.get('max_iops').updateValueAndValidity();
          }

          if (
            (this.formGroup.value.iopsStatus &&
              this.formGroup.value.burst_iops) ||
            (this.formGroup.value.bindWidthStatus &&
              this.formGroup.value.burst_bandwidth)
          ) {
            this.formGroup
              .get('burst_time')
              .setValidators([
                this.baseUtilService.VALID.required(),
                this.baseUtilService.VALID.integer(),
                this.baseUtilService.VALID.rangeValue(1, 999999999)
              ]);
          } else {
            this.formGroup.get('burst_time').clearValidators();
          }
          this.formGroup.get('burst_time').updateValueAndValidity();
        }, 0);
      });

    this.formGroup
      .get('latency')
      .valueChanges.pipe(pairwise())
      .subscribe(res => {
        if (this.formGroup.get('latency').invalid) {
          this.updateLatencyData('--');
        } else {
          this.updateLatencyData(res[1]);
        }
      });
  }

  validMinBandWidth(): ValidatorFn {
    return (control: AbstractControl): { [key: string]: any } | null => {
      if (!this.formGroup) {
        return null;
      }

      if (
        isEmpty(toString(this.formGroup.value.max_bandwidth)) ||
        !isNumber(+control.value) ||
        !isNumber(+this.formGroup.value.max_bandwidth)
      ) {
        return null;
      }

      return +control.value <= +this.formGroup.value.max_bandwidth
        ? null
        : {
            invalidMin: { value: control.value }
          };
    };
  }

  validMaxBandWidth(): ValidatorFn {
    return (control: AbstractControl): { [key: string]: any } | null => {
      if (!this.formGroup) {
        return null;
      }
      if (
        isEmpty(toString(this.formGroup.value.min_bandwidth)) ||
        !isNumber(+control.value) ||
        !isNumber(+this.formGroup.value.min_bandwidth)
      ) {
        return null;
      }

      return +control.value >= +this.formGroup.value.min_bandwidth
        ? null
        : {
            invalidMax: { value: control.value }
          };
    };
  }

  validBurstBandWidth(): ValidatorFn {
    return (control: AbstractControl): { [key: string]: any } | null => {
      if (!this.formGroup) {
        return null;
      }

      if (!trim(control.value)) {
        return null;
      }

      if (
        !isNumber(+control.value) ||
        isEmpty(toString(this.formGroup.value.max_bandwidth)) ||
        !isNumber(+this.formGroup.value.max_bandwidth)
      ) {
        return null;
      }

      return +control.value > +this.formGroup.value.max_bandwidth
        ? null
        : {
            invalidBurst: { value: control.value }
          };
    };
  }

  validMinIops(): ValidatorFn {
    return (control: AbstractControl): { [key: string]: any } | null => {
      if (!this.formGroup) {
        return null;
      }

      if (
        isEmpty(toString(this.formGroup.value.max_iops)) ||
        !isNumber(+control.value) ||
        !isNumber(+this.formGroup.value.max_iops)
      ) {
        return null;
      }

      return +control.value <= +this.formGroup.value.max_iops
        ? null
        : {
            invalidMin: { value: control.value }
          };
    };
  }

  validMaxIops(): ValidatorFn {
    return (control: AbstractControl): { [key: string]: any } | null => {
      if (!this.formGroup) {
        return null;
      }

      if (
        isEmpty(toString(this.formGroup.value.min_iops)) ||
        !isNumber(+control.value) ||
        !isNumber(+this.formGroup.value.min_iops)
      ) {
        return null;
      }

      return +control.value >= +this.formGroup.value.min_iops
        ? null
        : {
            invalidMax: { value: control.value }
          };
    };
  }

  validBurstIops(): ValidatorFn {
    return (control: AbstractControl): { [key: string]: any } | null => {
      if (!this.formGroup) {
        return null;
      }

      if (!trim(control.value)) {
        return null;
      }

      if (
        !isNumber(+control.value) ||
        isEmpty(toString(this.formGroup.value.max_iops)) ||
        !isNumber(+this.formGroup.value.max_iops)
      ) {
        return null;
      }

      return +control.value > +this.formGroup.value.max_iops
        ? null
        : {
            invalidBurst: { value: control.value }
          };
    };
  }

  getPolicies(recordsTemp?, startPage?) {
    this.liveMountPolicyApiService
      .getPoliciesUsingGET({
        size: 20,
        page: startPage || 0
      })
      .subscribe(res => {
        if (!recordsTemp) {
          recordsTemp = [];
        }
        if (!isNumber(startPage)) {
          startPage = CommonConsts.PAGE_START;
        }
        startPage++;
        recordsTemp = [...recordsTemp, ...res.items];
        if (
          startPage === Math.ceil(res.total / CommonConsts.PAGE_SIZE) ||
          res.total === 0
        ) {
          this.policyOptions = _map(recordsTemp, item => {
            return assign(item, {
              key: item.policyId,
              label: this.i18n.get('common_two_colon_label', [
                item.name,
                item.retentionPolicy === RetentionPolicy.Permanent
                  ? this.i18n.get('explore_permanent_retention_label')
                  : item.retentionPolicy === RetentionPolicy.LatestOne
                  ? this.i18n.get('explore_always_latest_label')
                  : this.i18n.get('common_retention_label') +
                    this.spaceLabel +
                    item.retentionValue +
                    this.spaceLabel +
                    this.dataMapService.getLabel(
                      'Interval_Unit',
                      item.retentionUnit
                    )
              ]),
              isLeaf: true
            });
          });
          return;
        }
        this.getPolicies(recordsTemp, startPage);
      });
  }

  onOK(): Observable<void> {
    return new Observable<any>((observer: Observer<void>) => {
      if (this.formGroup.invalid) {
        return;
      }

      const parameters = {} as any,
        advanceParameters = omit(this.formGroup.value, [
          'bindWidthStatus',
          'iopsStatus',
          'latencyStatus',
          'policyId'
        ]);
      assign(advanceParameters, {
        mysql_port: this.formGroup.get('mysql_port')?.value
      });

      if (!this.formGroup.value.bindWidthStatus) {
        advanceParameters.min_bandwidth = null;
        advanceParameters.max_bandwidth = null;
        advanceParameters.burst_bandwidth = null;
      }

      if (!this.formGroup.value.iopsStatus) {
        advanceParameters.min_iops = null;
        advanceParameters.max_iops = null;
        advanceParameters.burst_iops = null;
      }

      if (
        !(
          (this.formGroup.value.iopsStatus &&
            this.formGroup.value.burst_iops) ||
          (this.formGroup.value.bindWidthStatus &&
            this.formGroup.value.burst_bandwidth)
        )
      ) {
        advanceParameters.burst_time = null;
      }

      if (!this.formGroup.value.latencyStatus) {
        advanceParameters.latency = null;
      }

      each(advanceParameters, (v, k) => {
        if (isEmpty(trim(toString(v)))) {
          if (!includes(['pre_script', 'post_script', 'failed_script'], k)) {
            parameters[k] = null;
          }
          return;
        }

        if (!includes(['pre_script', 'post_script', 'failed_script'], k)) {
          parameters[k] = +v;
        } else {
          parameters[k] = v;
        }
      });

      if (
        this.item.resource_sub_type ===
        DataMap.Resource_Type.virtualMachine.value
      ) {
        delete parameters.pre_script;
        delete parameters.post_script;
        delete parameters.failed_script;
      }

      let liveMountParam = { parameters: {}, policyId: '' };
      if (
        includes(
          [DataMap.Resource_Type.tdsqlInstance.value],
          first(this.componentData.childResourceType)
        )
      ) {
        liveMountParam = {
          parameters: {
            performance: omit(parameters, [
              'pre_script',
              'post_script',
              'failed_script',
              'mysql_port'
            ]),
            ...pick(parameters, [
              'pre_script',
              'post_script',
              'failed_script',
              'mysql_port'
            ])
          },
          policyId: this.formGroup.value.policyId
        };
      } else {
        liveMountParam = {
          parameters: {
            performance: omit(parameters, [
              'pre_script',
              'post_script',
              'failed_script'
            ]),
            ...pick(parameters, ['pre_script', 'post_script', 'failed_script'])
          },
          policyId: this.formGroup.value.policyId
        };
      }
      if (!this.formGroup.value.policyId) {
        delete liveMountParam.policyId;
      }

      this.liveMountApiService
        .modifyLiveMountUsingPUT({
          liveMountId: this.item.id,
          liveMountParam
        })
        .subscribe({
          next: () => {
            observer.next();
            observer.complete();
          },
          error: () => {
            observer.error({});
            observer.complete();
          }
        });
    });
  }
}
