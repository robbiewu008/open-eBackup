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
import { Component, Input, OnInit } from '@angular/core';
import {
  AbstractControl,
  FormControl,
  FormGroup,
  ValidatorFn
} from '@angular/forms';
import { DataMap } from 'app/shared/consts';
import {
  BaseUtilService,
  DataMapService,
  I18NService
} from 'app/shared/services';
import { assign, trim, toString, isEmpty, isNumber, each } from 'lodash';
import { pairwise } from 'rxjs/operators';

@Component({
  selector: 'aui-live-mount-advanced-performance',
  templateUrl: './live-mount-advanced-performance.component.html',
  styleUrls: ['./live-mount-advanced-performance.component.less']
})
export class LiveMountAdvancedPerformanceComponent implements OnInit {
  @Input() formGroup: FormGroup;
  dataMap = DataMap;
  latencyOptions = this.dataMapService
    .toArray('LiveMount_Latency')
    .filter(v => (v.isLeaf = true));
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
    private dataMapService: DataMapService,
    public baseUtilService: BaseUtilService
  ) {}

  ngOnInit() {
    this.initForm();
  }

  initForm() {
    this.formGroup.addControl('bindWidthStatus', new FormControl(false));
    this.formGroup.addControl('iopsStatus', new FormControl(false));
    this.formGroup.addControl('latencyStatus', new FormControl(false));
    this.formGroup.addControl('bindWidthMin', new FormControl(false));
    this.formGroup.addControl('min_bandwidth', new FormControl(''));
    this.formGroup.addControl('bindWidthMax', new FormControl(false));
    this.formGroup.addControl('max_bandwidth', new FormControl(''));
    this.formGroup.addControl('bindWidthBurst', new FormControl(false));
    this.formGroup.addControl('burst_bandwidth', new FormControl(''));
    this.formGroup.addControl('iopsMin', new FormControl(false));
    this.formGroup.addControl('min_iops', new FormControl(''));
    this.formGroup.addControl('iopsMax', new FormControl(false));
    this.formGroup.addControl('max_iops', new FormControl(''));
    this.formGroup.addControl('iopsBurst', new FormControl(false));
    this.formGroup.addControl('burst_iops', new FormControl(''));
    this.formGroup.addControl('burst_time', new FormControl(''));
    this.formGroup.addControl(
      'latency',
      new FormControl(DataMap.LiveMount_Latency.zeroDotsFive.value)
    );
    this.listenParamter();
  }

  bindWidthMinChange() {
    if (this.formGroup.value.bindWidthMin) {
      this.formGroup
        .get('min_bandwidth')
        .setValidators([
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.integer(),
          this.baseUtilService.VALID.rangeValue(1, 999999999),
          this.validMinBandWidth()
        ]);
    } else {
      this.formGroup.get('min_bandwidth').clearValidators();
    }
    this.formGroup.get('min_bandwidth').updateValueAndValidity();
  }

  bindWidthMaxChange() {
    if (this.formGroup.value.bindWidthMax) {
      this.formGroup
        .get('max_bandwidth')
        .setValidators([
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.integer(),
          this.baseUtilService.VALID.rangeValue(1, 999999999),
          this.validMaxBandWidth()
        ]);
    } else {
      this.formGroup.get('max_bandwidth').clearValidators();
    }
    this.formGroup.get('max_bandwidth').updateValueAndValidity();
  }

  bindWidthBurstChange() {
    if (this.formGroup.value.bindWidthBurst) {
      this.formGroup
        .get('burst_bandwidth')
        .setValidators([
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.integer(),
          this.baseUtilService.VALID.rangeValue(1, 999999999),
          this.validBurstBandWidth()
        ]);
    } else {
      this.formGroup.get('burst_bandwidth').clearValidators();
    }
    this.formGroup.get('burst_bandwidth').updateValueAndValidity();
  }

  iopsMinChange() {
    if (this.formGroup.value.iopsMin) {
      this.formGroup
        .get('min_iops')
        .setValidators([
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.integer(),
          this.baseUtilService.VALID.rangeValue(100, 999999999),
          this.validMinIops()
        ]);
    } else {
      this.formGroup.get('min_iops').clearValidators();
    }
    this.formGroup.get('min_iops').updateValueAndValidity();
  }

  iopsMaxChange() {
    if (this.formGroup.value.iopsMax) {
      this.formGroup
        .get('max_iops')
        .setValidators([
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.integer(),
          this.baseUtilService.VALID.rangeValue(100, 999999999),
          this.validMaxIops()
        ]);
    } else {
      this.formGroup.get('max_iops').clearValidators();
    }
    this.formGroup.get('max_iops').updateValueAndValidity();
  }

  iopsBurstChange() {
    if (this.formGroup.value.iopsBurst) {
      this.formGroup
        .get('burst_iops')
        .setValidators([
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.integer(),
          this.baseUtilService.VALID.rangeValue(100, 999999999),
          this.validBurstIops()
        ]);
    } else {
      this.formGroup.get('burst_iops').clearValidators();
    }
    this.formGroup.get('burst_iops').updateValueAndValidity();
  }

  listenParamter() {
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
        if (this.formGroup.value.bindWidthMin) {
          this.formGroup
            .get('min_bandwidth')
            .setValidators([
              this.baseUtilService.VALID.required(),
              this.baseUtilService.VALID.integer(),
              this.baseUtilService.VALID.rangeValue(1, 999999999),
              this.validMinBandWidth()
            ]);
        } else {
          this.formGroup.get('min_bandwidth').clearValidators();
        }
        if (this.formGroup.value.bindWidthMax) {
          this.formGroup
            .get('max_bandwidth')
            .setValidators([
              this.baseUtilService.VALID.required(),
              this.baseUtilService.VALID.integer(),
              this.baseUtilService.VALID.rangeValue(1, 999999999),
              this.validMaxBandWidth()
            ]);
        } else {
          this.formGroup.get('max_bandwidth').clearValidators();
        }
        if (this.formGroup.get('burst_bandwidth')) {
          if (this.formGroup.value.bindWidthBurst) {
            this.formGroup
              .get('burst_bandwidth')
              .setValidators([
                this.baseUtilService.VALID.integer(),
                this.baseUtilService.VALID.rangeValue(1, 999999999),
                this.validBurstBandWidth()
              ]);
          } else {
            this.formGroup.get('burst_bandwidth').clearValidators();
          }
          this.formGroup.get('burst_bandwidth').updateValueAndValidity();
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
        if (this.formGroup.value.iopsMin) {
          this.formGroup
            .get('min_iops')
            .setValidators([
              this.baseUtilService.VALID.required(),
              this.baseUtilService.VALID.integer(),
              this.baseUtilService.VALID.rangeValue(100, 999999999),
              this.validMinIops()
            ]);
        } else {
          this.formGroup.get('min_iops').clearValidators();
        }
        if (this.formGroup.value.iopsMax) {
          this.formGroup
            .get('max_iops')
            .setValidators([
              this.baseUtilService.VALID.required(),
              this.baseUtilService.VALID.integer(),
              this.baseUtilService.VALID.rangeValue(100, 999999999),
              this.validMaxIops()
            ]);
        } else {
          this.formGroup.get('max_iops').clearValidators();
        }
        if (this.formGroup.get('burst_iops')) {
          if (this.formGroup.value.iopsBurst) {
            this.formGroup
              .get('burst_iops')
              .setValidators([
                this.baseUtilService.VALID.integer(),
                this.baseUtilService.VALID.rangeValue(100, 999999999),
                this.validBurstIops()
              ]);
          } else {
            this.formGroup.get('burst_iops').clearValidators();
          }
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
        if (
          toString(res[0]) === toString(res[1]) ||
          !this.formGroup.value.bindWidthMin
        ) {
          return;
        }
        setTimeout(() => {
          this.updateMinAndMaxBandwidthValidity('min_bandwidth');
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
        if (
          toString(res[0]) === toString(res[1]) ||
          !this.formGroup.value.bindWidthMax
        ) {
          return;
        }
        setTimeout(() => {
          this.updateMinAndMaxBandwidthValidity('max_bandwidth');
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
        if (
          toString(res[0]) === toString(res[1]) ||
          !this.formGroup.value.bindWidthBurst
        ) {
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
        if (
          toString(res[0]) === toString(res[1]) ||
          !this.formGroup.value.iopsMin
        ) {
          return;
        }
        setTimeout(() => {
          this.updateMinAndMaxIopsValidity('min_iops');
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
        if (
          toString(res[0]) === toString(res[1]) ||
          !this.formGroup.value.iopsMax
        ) {
          return;
        }
        setTimeout(() => {
          this.updateMinAndMaxIopsValidity('max_iops');
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
        if (
          toString(res[0]) === toString(res[1]) ||
          !this.formGroup.value.iopsBurst
        ) {
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
      if (!this.formGroup || !trim(control.value)) {
        return null;
      }

      if (
        isEmpty(toString(this.formGroup.value.max_bandwidth)) ||
        !isNumber(+control.value) ||
        !isNumber(+this.formGroup.value.max_bandwidth) ||
        !this.formGroup.value.bindWidthMax
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
      if (!this.formGroup || !trim(control.value)) {
        return null;
      }

      if (
        isEmpty(toString(this.formGroup.value.min_bandwidth)) ||
        !isNumber(+control.value) ||
        !isNumber(+this.formGroup.value.min_bandwidth) ||
        !this.formGroup.value.bindWidthMin
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

  updateMinAndMaxBandwidthValidity(focusKey) {
    this.formGroup.get('min_bandwidth').clearValidators();
    this.formGroup.get('max_bandwidth').clearValidators();
    if (
      (focusKey === 'min_bandwidth' &&
        !trim(this.formGroup.value.max_bandwidth)) ||
      (focusKey === 'max_bandwidth' &&
        !trim(this.formGroup.value.max_bandwidth) &&
        trim(this.formGroup.value.min_bandwidth))
    ) {
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
          this.baseUtilService.VALID.integer(),
          this.baseUtilService.VALID.rangeValue(1, 999999999),
          this.validMaxBandWidth()
        ]);
    } else {
      this.formGroup
        .get('min_bandwidth')
        .setValidators([
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
    }
    this.formGroup.get('min_bandwidth').updateValueAndValidity();
    this.formGroup.get('max_bandwidth').updateValueAndValidity();
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
        !isNumber(+this.formGroup.value.max_bandwidth) ||
        !this.formGroup.value.bindWidthMax
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
      if (!this.formGroup || !trim(control.value)) {
        return null;
      }

      if (
        isEmpty(toString(this.formGroup.value.max_iops)) ||
        !isNumber(+control.value) ||
        !isNumber(+this.formGroup.value.max_iops) ||
        !this.formGroup.value.iopsMax
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
      if (!this.formGroup || !trim(control.value)) {
        return null;
      }

      if (
        isEmpty(toString(this.formGroup.value.min_iops)) ||
        !isNumber(+control.value) ||
        !isNumber(+this.formGroup.value.min_iops) ||
        !this.formGroup.value.iopsMin
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

  updateMinAndMaxIopsValidity(focusKey) {
    this.formGroup.get('min_iops').clearValidators();
    this.formGroup.get('max_iops').clearValidators();
    if (
      (focusKey === 'min_iops' && !trim(this.formGroup.value.max_iops)) ||
      (focusKey === 'max_iops' &&
        !trim(this.formGroup.value.max_iops) &&
        trim(this.formGroup.value.min_iops))
    ) {
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
          this.baseUtilService.VALID.integer(),
          this.baseUtilService.VALID.rangeValue(100, 999999999),
          this.validMaxIops()
        ]);
    } else {
      this.formGroup
        .get('min_iops')
        .setValidators([
          this.baseUtilService.VALID.integer(),
          this.baseUtilService.VALID.rangeValue(100, 999999999),
          this.validMinIops()
        ]);
      this.formGroup
        .get('max_iops')
        .setValidators([
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.integer(),
          this.baseUtilService.VALID.rangeValue(100, 999999999),
          this.validMaxIops()
        ]);
    }
    this.formGroup.get('min_iops').updateValueAndValidity();
    this.formGroup.get('max_iops').updateValueAndValidity();
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
        !isNumber(+this.formGroup.value.max_iops) ||
        !this.formGroup.value.iopsMax
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

  updateIopsItems(value, type: 'min' | 'max' | 'burst') {
    each(this.iopsItems, item => {
      const obj = {};
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
}
