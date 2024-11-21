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
import { Component, EventEmitter, Input, OnInit, Output } from '@angular/core';
import {
  AbstractControl,
  FormBuilder,
  FormControl,
  FormGroup,
  ValidatorFn
} from '@angular/forms';
import {
  BaseUtilService,
  DataMap,
  DataMapService,
  DatastoreType,
  I18NService,
  LANGUAGE,
  MountTargetLocation,
  TargetCPU,
  TargetMemory,
  VirtualResourceService,
  VmRestoreOptionType,
  VmwareService
} from 'app/shared';
import {
  assign,
  each,
  find,
  first,
  isEmpty,
  isNumber,
  isUndefined,
  map as _map,
  pick,
  toString,
  trim,
  includes
} from 'lodash';
import { Observable, Observer } from 'rxjs';
import { map, pairwise } from 'rxjs/operators';

@Component({
  selector: 'aui-live-mount-vmware-options',
  templateUrl: './live-mount-options.component.html',
  styleUrls: ['./live-mount-options.component.less']
})
export class LiveMountOptionsComponent implements OnInit {
  formGroup: FormGroup;
  rowProperties = {
    net_work: []
  };
  latencyOptions = this.dataMapService
    .toArray('LiveMount_Latency')
    .filter(v => (v.isLeaf = true));

  originName;
  originLocation;
  selectedHost;
  cacheSelectedHost;

  targetStorageLocationOptions = [];
  datastoreNoData = false;
  networkNoData = false;
  networkTableData = [];

  targetCPU = TargetCPU;
  targetMemory = TargetMemory;
  datastoreType = DatastoreType;
  vmRestoreOptionType = VmRestoreOptionType;
  mountTargetLocation = MountTargetLocation;
  restoreToNewLocationOnly = false;
  isEn = this.i18n.language.toLowerCase() === LANGUAGE.EN;

  @Input() activeIndex;
  @Input() componentData;
  @Output() selectMountOptionChange = new EventEmitter<any>();

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
  virtualSocketsErrorTip = assign({}, this.baseUtilService.rangeErrorTip, {
    invalidRang: this.i18n.get('common_valid_rang_label', [1, 128])
  });
  coresPerErrorTip = assign({}, this.baseUtilService.rangeErrorTip, {
    invalidMinSize: this.i18n.get('common_valid_minsize_label', [1]),
    invalidDivisorNum: this.i18n.get('explore_ivalid_number_cores_label')
  });
  memorysErrorTip = assign({}, this.baseUtilService.rangeErrorTip, {
    invalidRang: this.i18n.get('common_valid_rang_label', ['4MB', '6128GB']),
    invalidMemory: this.i18n.get('explore_ivalid_memory_label')
  });
  nameErrorTip = assign({}, this.baseUtilService.nameErrorTip, {
    invalidSameName: this.i18n.get('common_duplicate_name_label'),
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [80])
  });

  constructor(
    private i18n: I18NService,
    private fb: FormBuilder,
    private dataMapService: DataMapService,
    public baseUtilService: BaseUtilService,
    private vmwareService: VmwareService,
    private virtualResourceService: VirtualResourceService
  ) {}

  ngOnInit() {
    this.initForm();
    this.updateForm();
  }

  updateForm() {
    this.restoreToNewLocationOnly =
      this.componentData &&
      this.componentData.selectionCopy &&
      includes(
        [
          DataMap.CopyData_generatedType.replicate.value,
          DataMap.CopyData_generatedType.reverseReplication.value,
          DataMap.CopyData_generatedType.cascadedReplication.value
        ],
        this.componentData.selectionCopy.generated_by
      );
    if (this.restoreToNewLocationOnly) {
      this.formGroup
        .get('target_location')
        .setValue(MountTargetLocation.Others);
    }
  }

  initForm() {
    this.formGroup = this.fb.group({
      target_location: new FormControl(MountTargetLocation.Original),
      name: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.maxLength(80),
          this.validIsSameName()
        ],
        updateOn: 'change'
      }),
      location: new FormControl(''),
      targetStorageLocationName: new FormControl(''),
      power_on: new FormControl(true),
      startup_network_adaptor: new FormControl(false),
      bindWidthStatus: new FormControl(false),
      iopsStatus: new FormControl(false),
      latencyStatus: new FormControl(false),
      min_bandwidth: new FormControl(''),
      max_bandwidth: new FormControl(''),
      burst_bandwidth: new FormControl(''),
      min_iops: new FormControl(''),
      max_iops: new FormControl(''),
      burst_iops: new FormControl(''),
      burst_time: new FormControl(''),
      latency: new FormControl(''),
      targetCPU: new FormControl(TargetCPU.OriginalConfig),
      targetMemory: new FormControl(TargetCPU.OriginalConfig),
      deleteOriginalVM: new FormControl(false)
    });

    this.listenRestoreLocation();
    this.listenFormStatus();
    this.listenAdvancedParamter();
    this.listenTargetCPU();
    this.listenTargetMemory();
  }

  initData() {
    this.originName = this.componentData.selectionResource.resource_name;
    this.originLocation = this.componentData.selectionResource.original_location;
    this.rowProperties = JSON.parse(
      this.componentData.selectionCopy.properties
    ).vmware_metadata;

    let environmentSubType = this.componentData.selectionResource
      .environment_sub_type;
    if (this.componentData.selectionResource.resource_properties) {
      environmentSubType = JSON.parse(
        this.componentData.selectionResource.resource_properties
      ).environment_sub_type;
    }
    this.updateForm();
    this.selectMountOptionChange.emit(this.formGroup.valid);
  }

  listenRestoreLocation() {
    this.formGroup.get('target_location').valueChanges.subscribe(res => {
      if (res === MountTargetLocation.Original) {
        this.updateOriginForm();
        this.selectedHost = '';
        this.cacheSelectedHost = '';
        this.datastoreNoData = false;
        this.networkNoData = false;
      } else {
        this.networkTableData = [];
        this.formGroup.patchValue({ location: '' });
        this.formGroup.patchValue({ targetStorageLocationName: '' });
        each(this.rowProperties.net_work, (name, index) => {
          this.networkTableData.push({ name, uuid: index });
        });
        this.updateNewForm();
      }
    });
  }

  listenFormStatus() {
    this.formGroup.statusChanges.subscribe(res => {
      this.selectMountOptionChange.emit(res === 'VALID');
    });
  }

  listenAdvancedParamter() {
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
        this.updateMinAndMaxBandwidthValidity('min_bandwidth');
        if (this.formGroup.get('burst_bandwidth')) {
          this.formGroup
            .get('burst_bandwidth')
            .setValidators([
              this.baseUtilService.VALID.integer(),
              this.baseUtilService.VALID.rangeValue(1, 999999999),
              this.validBurstBandWidth()
            ]);
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
            this.baseUtilService.VALID.rangeValue(100, 999999999),
            this.validMaxIops()
          ]);
        this.updateMinAndMaxIopsValidity('min_iops');
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
        if (toString(res[0]) === toString(res[1])) {
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
        if (toString(res[0]) === toString(res[1])) {
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

  listenTargetCPU() {
    this.formGroup.get('targetCPU').valueChanges.subscribe(res => {
      if (res === TargetCPU.OriginalConfig) {
        this.formGroup.removeControl('num_virtual_sockets');
        this.formGroup.removeControl('num_cores_per_virtual');
      } else {
        this.formGroup.addControl(
          'num_virtual_sockets',
          new FormControl('', {
            validators: [
              this.baseUtilService.VALID.integer(),
              this.baseUtilService.VALID.required(),
              this.baseUtilService.VALID.rangeValue(1, 128)
            ]
          })
        );
        this.formGroup.addControl(
          'num_cores_per_virtual',
          new FormControl('', {
            validators: [
              this.baseUtilService.VALID.integer(),
              this.baseUtilService.VALID.minSize(0),
              this.baseUtilService.VALID.required(),
              this.validNumberCores()
            ]
          })
        );
        this.formGroup
          .get('num_virtual_sockets')
          .valueChanges.subscribe(res => {
            setTimeout(() => {
              if (!this.formGroup.get('num_cores_per_virtual').value) {
                return;
              }
              this.formGroup.get('num_cores_per_virtual').markAsTouched();
              this.formGroup
                .get('num_cores_per_virtual')
                .updateValueAndValidity();
            }, 0);
          });
      }
    });
  }

  listenTargetMemory() {
    this.formGroup.get('targetMemory').valueChanges.subscribe(res => {
      if (res === TargetMemory.OriginalConfig) {
        this.formGroup.removeControl('memory_size');
      } else {
        this.formGroup.addControl(
          'memory_size',
          new FormControl('', {
            validators: [
              this.baseUtilService.VALID.integer(),
              this.baseUtilService.VALID.required(),
              this.baseUtilService.VALID.rangeValue(4, 6128 * 1024),
              this.vavalidTargetMemory()
            ]
          })
        );
      }
    });
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

  vavalidTargetMemory(): ValidatorFn {
    return (control: AbstractControl): { [key: string]: any } | null => {
      if (!trim(control.value)) {
        return null;
      }

      return +control.value % 4 === 0
        ? null
        : { invalidMemory: { value: control.value } };
    };
  }

  validNumberCores(): ValidatorFn {
    return (control: AbstractControl): { [key: string]: any } | null => {
      if (!trim(control.value)) {
        return null;
      }

      if (!this.formGroup.value.num_virtual_sockets) {
        return { invalidDivisorNum: { value: control.value } };
      }

      return +control.value !== 0 &&
        +this.formGroup.value.num_virtual_sockets % +control.value === 0
        ? null
        : { invalidDivisorNum: { value: control.value } };
    };
  }

  validIsSameName(): ValidatorFn {
    return (control: AbstractControl): { [key: string]: any } | null => {
      if (!trim(control.value)) {
        return null;
      }

      return this.originName === trim(control.value)
        ? { invalidSameName: { value: control.value } }
        : null;
    };
  }

  validMinBandWidth(): ValidatorFn {
    return (control: AbstractControl): { [key: string]: any } | null => {
      if (!this.formGroup || !trim(control.value)) {
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
      if (!this.formGroup || !trim(control.value)) {
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
      if (!this.formGroup || !trim(control.value)) {
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
      if (!this.formGroup || !trim(control.value)) {
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

  updateOriginForm() {
    this.formGroup.removeControl('targetStorageLocation');
    this.formGroup.get('targetCPU').setValue(TargetCPU.OriginalConfig);
    this.formGroup.get('targetMemory').setValue(TargetMemory.OriginalConfig);
    this.formGroup.get('targetCPU').updateValueAndValidity();
    this.formGroup.get('targetMemory').updateValueAndValidity();
  }

  updateNewForm() {
    this.formGroup.addControl(
      'targetStorageLocation',
      new FormControl('', {
        validators: [this.baseUtilService.VALID.required()],
        updateOn: 'change'
      })
    );
    this.formGroup.get('targetStorageLocation').valueChanges.subscribe(res => {
      const storageLocation = this.targetStorageLocationOptions.find(
        location => location.key === res
      );
      storageLocation &&
        this.formGroup
          .get('targetStorageLocationName')
          .setValue(storageLocation.label);
    });
    this.targetStorageLocationOptions = [];
  }

  changeLocation(event) {
    this.formGroup.patchValue({ location: event.path });
    this.selectedHost = event.uuid;
    if (this.selectedHost === this.cacheSelectedHost) {
      return;
    }
    this.cacheSelectedHost = this.selectedHost;

    if (event.unselected) {
      this.targetStorageLocationOptions = [];
      this.formGroup.patchValue({
        targetStorageLocation: null
      });
      return;
    }

    this.getStorageTableData();
    this.getNetworkTableData();
  }

  getVirtualResource(): Observable<any> {
    return new Observable<any>((observer: Observer<any>) => {
      this.originLocation = '';
      const resourceProperties = JSON.parse(
        this.componentData.selectionResource.resource_properties
      );
      this.virtualResourceService
        .queryResourcesV1VirtualResourceGet({
          pageSize: 10,
          pageNo: 0,
          conditions: JSON.stringify({
            uuid: resourceProperties.parent_uuid
          })
        })
        .pipe(map(res => first(res.items)))
        .subscribe(res => {
          assign(this.componentData.selectionResource, {
            original_location: !res
              ? this.componentData.selectionResource.resource_location
              : res.path
          });
          observer.next({});
          observer.complete();
        });
    });
  }

  getStorageTableData() {
    this.vmwareService
      .listComputeResDatastoreV1ComputeResourcesComputeResUuidDatastoresGet({
        computeResUuid: this.selectedHost
      })
      .subscribe({
        next: res => {
          if (!res.length) {
            this.datastoreNoData = true;
            this.storageServiceError();
            return;
          }
          this.targetStorageLocationOptions = [];
          res.forEach(item => {
            this.targetStorageLocationOptions.push({
              key: item.uuid,
              label: item.name,
              isLeaf: true
            });
          });
        },
        error: ex => {
          this.datastoreNoData = false;
          this.storageServiceError();
        }
      });
  }

  storageServiceError() {
    this.targetStorageLocationOptions = [];
    this.formGroup.patchValue({ targetStorageLocation: '' });
  }

  getNetworkTableData() {
    this.vmwareService
      .listComputeResNetworkV1ComputeResourcesComputeResUuidNetworksGet({
        computeResUuid: this.selectedHost
      })
      .subscribe({
        next: res => {
          if (!res.length) {
            this.networkNoData = true;
            this.networkServiceError();
            return;
          }
          const options = [];
          res.forEach(item => {
            options.push({
              key: item.uuid,
              label: item.name,
              isLeaf: true
            });
          });
          this.networkTableData.forEach(item => {
            item['options'] = options;
          });
        },
        error: ex => {
          this.networkNoData = false;
          this.networkServiceError();
        }
      });
  }

  networkServiceError() {
    this.networkTableData.forEach(item => {
      item['options'] = [];
      item.selection = '';
      item.selectionName = '';
    });
  }

  changeTargetNetwork(value, networkUuid) {
    _map(this.networkTableData, data => {
      if (networkUuid === data.uuid) {
        const obj = find(data.options, { key: value });
        if (!isUndefined(obj)) {
          data.selection = obj.key;
          data.selectionName = obj.label;
        }
      }
    });
  }

  getComponentData() {
    const requestParams = {
      target_location: this.formGroup.value.target_location
    };
    const parameters = {} as any;
    const performance = {},
      performanceParams = pick(this.formGroup.value, [
        'min_bandwidth',
        'max_bandwidth',
        'burst_bandwidth',
        'min_iops',
        'max_iops',
        'burst_iops',
        'burst_time',
        'latency'
      ]);
    each(performanceParams, (v, k) => {
      if (isEmpty(trim(String(v)))) {
        return;
      }
      performance[k] = v;
    });
    assign(parameters, { performance });

    if (this.formGroup.value.target_location === MountTargetLocation.Original) {
      let resourceProperties;
      try {
        resourceProperties = JSON.parse(
          this.componentData.selectionResource.resource_properties
        );
      } catch (error) {
        resourceProperties = this.componentData.selectionResource;
      }
      assign(requestParams, {
        target_resource_uuid_list: [resourceProperties.parent_uuid]
      });
      assign(parameters, {
        config: {
          power_on: this.formGroup.value.power_on,
          startup_network_adaptor: this.formGroup.value.startup_network_adaptor
        },
        name: trim(this.formGroup.value.name),
        isDeleteOriginalVM: this.formGroup.value.deleteOriginalVM + ''
      });
    } else {
      const network = [];
      each(this.networkTableData, data => {
        network.push({
          adapter_name: data.name,
          target_network_uuid: data.selection
        });
      });
      assign(parameters, {
        name: trim(this.formGroup.value.name),
        config: {
          power_on: this.formGroup.value.power_on,
          startup_network_adaptor: this.formGroup.value.startup_network_adaptor,
          specify_location_config: {
            storage_location: this.formGroup.value.targetStorageLocation,
            network
          }
        }
      });
      assign(requestParams, {
        target_resource_uuid_list: [this.selectedHost]
      });
    }
    const cpu = {} as any;
    if (this.formGroup.value.targetCPU === TargetCPU.SpecifyConfig) {
      if (this.formGroup.value.num_virtual_sockets) {
        assign(cpu, {
          num_virtual_sockets: this.formGroup.value.num_virtual_sockets
        });
      }
      if (this.formGroup.value.num_cores_per_virtual) {
        assign(cpu, {
          num_cores_per_virtual: this.formGroup.value.num_cores_per_virtual
        });
      }
      assign(cpu, {
        use_original: false
      });
    } else {
      assign(cpu, {
        use_original: true
      });
      delete cpu.num_virtual_sockets;
      delete cpu.num_cores_per_virtual;
    }
    assign(parameters.config, { cpu });

    const memory = {} as any;
    if (this.formGroup.value.targetMemory === TargetMemory.SpecifyConfig) {
      if (this.formGroup.value.memory_size) {
        assign(memory, {
          memory_size: this.formGroup.value.memory_size
        });
      }
      assign(memory, {
        use_original: false
      });
    } else {
      assign(memory, {
        use_original: true
      });
      delete memory.memory_size;
    }
    assign(parameters.config, { memory });
    assign(requestParams, { parameters });

    return assign(this.componentData, {
      requestParams: assign(this.componentData.requestParams, requestParams),
      selectionMount: {
        ...this.formGroup.value,
        networkTableData: this.networkTableData
      }
    });
  }
}
