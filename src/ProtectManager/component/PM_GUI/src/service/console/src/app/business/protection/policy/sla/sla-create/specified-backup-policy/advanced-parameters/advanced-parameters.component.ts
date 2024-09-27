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
import { FormControl, FormGroup } from '@angular/forms';
import {
  ApplicationType,
  BaseUtilService,
  CommonConsts,
  CookieService,
  DataMapService,
  I18NService,
  ProtectResourceAction,
  QosService
} from 'app/shared';
import { AppUtilsService } from 'app/shared/services/app-utils.service';
import {
  assign,
  find,
  first,
  get,
  includes,
  isUndefined,
  map,
  size
} from 'lodash';

@Component({
  selector: 'aui-advanced-parameters',
  templateUrl: './advanced-parameters.component.html',
  styleUrls: ['./advanced-parameters.component.less']
})
export class AdvancedParametersComponent implements OnInit {
  find = find;
  size = size;
  qosNames = [];
  @Input() appType: any;
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
  channelNumberErrorTip = {
    ...this.baseUtilService.rangeErrorTip,
    ...this.baseUtilService.sizeErrorTip,
    invalidRang: this.i18n.get('common_valid_rang_label', [1, 254]),
    invalidMaxSize: this.i18n.get('common_valid_rang_label', [1, 254]),
    invalidMinSize: this.i18n.get('common_valid_rang_label', [1, 254])
  };
  concurrentNumberErrorTip = {
    ...this.baseUtilService.rangeErrorTip,
    invalidRang: this.i18n.get('common_valid_rang_label', [1, 256])
  };
  parallelNumberErrorTip = {
    invalidInteger: this.i18n.get('common_valid_integer_label'),
    invalidRang: this.i18n.get('common_valid_rang_label', [1, 16])
  };
  parallelNumberPlaceHolder = '1~16';
  memoryErrorTip = {
    invalidInteger: this.i18n.get('common_cannot_decimal_label'),
    invalidRang: this.i18n.get('common_valid_rang_label', [1, 2048])
  };
  capacityThresholdErrorTip = {};
  capacityThresholdPlaceHolder = '0~100';
  capacityThresholdToolTip = this.i18n.get(
    'protection_vmware_capacity_threshold_tip_label'
  );
  rateLimitTip = {
    ...this.baseUtilService.rangeErrorTip,
    invalidRang: this.i18n.get('common_valid_rang_label', [1, 1024]),
    invalidInteger: this.i18n.get('common_cannot_decimal_label')
  };

  protectResourceAction = ProtectResourceAction;
  applicationType = ApplicationType;
  isRetry = true;
  isHcsUser = this.cookieService.get('userType') === CommonConsts.HCS_USER_TYPE;
  isVirtualCloud = false; // 判断是否为一部分虚拟化和云平台应用
  isSourceDeduplication = true; // 判断源端重删
  isDeduplication = false; // 判断目标端重删
  isDeduplicationDisabled = false; // 判断目标端重删是否禁止修改
  isSpecifiedLocation = true; // 判断指定目标位置
  isConcurrent = false; // 并发数判断
  hasConcurrentTip = false; // 并发数问号提示判断
  isBackupNodeFirst = false; // 优先使用备节点备份判断
  hasBackupNodeTip = false; // 优先使用备节点问号提示判断
  isCopyVerify = false; // 副本校验判断
  isCopyVerifyInfo = false; // 副本校验打开后提示信息判断
  isArchiveLogDelete = false; // 归档日志删除判断
  isLogFailTransformFull = false; // 日志备份失败后自动转全量判断
  isParallelNumber = false; // 并发进程数判断
  showParallelNumberTip = false; // 并发进程数提示显示判断
  isUseMemory = false; // 备份运行内存判断
  isAutoIndex = false; // 自动索引判断
  isCapacityThreshold = false; // 生产存储剩余容量阈值
  isBackupGroup = false; // 用于轻量云gauss和hcsgauss的特别参数判断
  isRateLimit = false; // 用于流量控制一类的参数
  isThread = false; // 线程数
  isDisableQos = false; // 如果选择了本地盘的存储单元就禁止限速策略

  constructor(
    public i18n: I18NService,
    public dataMapService: DataMapService,
    public appUtilsService: AppUtilsService,
    private qosServiceApi: QosService,
    private baseUtilService: BaseUtilService,
    private cookieService: CookieService
  ) {}

  ngOnInit(): void {
    this.getQosNames();
    this.updateForm();
    // 根据应用类型增加表单控件
    this.updateSpecialControl();
    this.updateData();
  }

  updateForm() {
    this.formGroup.addControl('storage_type', new FormControl(''));
    this.formGroup.addControl('storage_id', new FormControl(''));
    this.formGroup.addControl('qos_id', new FormControl(''));
    this.formGroup.addControl('alarm_over_time_window', new FormControl(false));
    this.formGroup.addControl('alarm_after_failure', new FormControl(true));
    this.formGroup.addControl('source_deduplication', new FormControl(false));
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

  updateSpecialControl() {
    // 删除源端重删
    if (
      includes(
        [
          ApplicationType.KubernetesDatasetCommon,
          ApplicationType.ActiveDirectory,
          ApplicationType.NASFileSystem,
          ApplicationType.Ndmp,
          ApplicationType.NASShare,
          ApplicationType.CommonShare,
          ApplicationType.OceanBase
        ],
        this.appType
      )
    ) {
      this.isSourceDeduplication = false;
      this.formGroup.get('source_deduplication').disable();
    }

    // 删除指定目标位置
    if (includes([ApplicationType.CommonShare], this.appType)) {
      this.isSpecifiedLocation = false;
      this.formGroup.get('storage_type').disable();
      this.formGroup.get('storage_id').disable();
    }

    // 目标端重删
    if (
      includes(
        [
          ApplicationType.NASFileSystem,
          ApplicationType.NASShare,
          ApplicationType.ObjectStorage,
          ApplicationType.Fileset,
          ApplicationType.Volume
        ],
        this.appType
      )
    ) {
      this.isDeduplication = true;
      if (
        [ApplicationType.NASFileSystem, ApplicationType.NASShare].includes(
          this.appType
        )
      ) {
        this.isDeduplicationDisabled =
          this.action === ProtectResourceAction.Modify;
      }
      this.formGroup.addControl('deduplication', new FormControl(true));
      if (this.appType === ApplicationType.Fileset) {
        // 文件集不打开目标端重删则源端重删无用
        this.formGroup.get('deduplication').valueChanges.subscribe(res => {
          if (!res) {
            this.formGroup.get('source_deduplication').setValue(false);
          }
        });
      }
    }

    // oracle通道数和传输和存储加密
    if (this.appType === ApplicationType.Oracle) {
      this.formGroup.addControl(
        'channel_number',
        new FormControl(1, {
          validators: [
            this.baseUtilService.VALID.integer(),
            this.baseUtilService.VALID.rangeValue(1, 254)
          ]
        })
      );
      this.formGroup.addControl('encryption', new FormControl(false));
      this.formGroup.addControl(
        'encryption_algorithm',
        new FormControl('AES256')
      );
      this.formGroup.get('encryption').valueChanges.subscribe(res => {
        if (!res) {
          this.formGroup.removeControl('encryption_algorithm');
        } else {
          this.formGroup.addControl(
            'encryption_algorithm',
            new FormControl('AES256')
          );
        }
        this.formGroup.updateValueAndValidity();
      });
    }

    // 自动索引
    if (
      includes(
        [
          ApplicationType.HDFS,
          ApplicationType.NASFileSystem,
          ApplicationType.Ndmp,
          ApplicationType.NASShare,
          ApplicationType.ObjectStorage,
          ApplicationType.Fileset,
          ApplicationType.Volume
        ],
        this.appType
      )
    ) {
      this.isAutoIndex = true;
      this.formGroup.addControl('auto_index', new FormControl(false));
    }

    // 虚拟化和云平台的自动索引
    if (
      includes(
        [
          ApplicationType.CNware,
          ApplicationType.FusionCompute,
          ApplicationType.FusionOne,
          ApplicationType.HCSCloudHost,
          ApplicationType.OpenStack,
          ApplicationType.ApsaraStack,
          ApplicationType.HyperV
        ],
        this.appType
      )
    ) {
      this.isAutoIndex = true;
      this.isVirtualCloud = true;
      this.formGroup.addControl('fine_grained_restore', new FormControl(false));
    }

    // 并发数
    if (
      includes(
        [
          ApplicationType.MySQL,
          ApplicationType.SQLServer,
          ApplicationType.DB2,
          ApplicationType.OpenGauss,
          ApplicationType.TDSQL,
          ApplicationType.Dameng
        ],
        this.appType
      )
    ) {
      this.isConcurrent = true;
      if (
        includes(
          [
            ApplicationType.SQLServer,
            ApplicationType.OpenGauss,
            ApplicationType.TDSQL
          ],
          this.appType
        )
      ) {
        this.hasConcurrentTip = true;
      }
      this.formGroup.addControl(
        'channel_number',
        new FormControl(1, {
          validators: [
            this.baseUtilService.VALID.integer(),
            this.baseUtilService.VALID.rangeValue(1, 256)
          ]
        })
      );
      if (this.appType === ApplicationType.Dameng) {
        this.formGroup
          .get('channel_number')
          .setValidators([
            this.baseUtilService.VALID.integer(),
            this.baseUtilService.VALID.rangeValue(1, 128)
          ]);
        this.concurrentNumberErrorTip = {
          ...this.baseUtilService.rangeErrorTip,
          invalidRang: this.i18n.get('common_valid_rang_label', [1, 128])
        };
        this.formGroup.get('channel_number').updateValueAndValidity();
      }
    }

    // 优先使用备节点备份
    if (
      includes(
        [
          ApplicationType.MySQL,
          ApplicationType.OpenGauss,
          ApplicationType.GaussDBT,
          ApplicationType.GoldenDB
        ],
        this.appType
      )
    ) {
      this.isBackupNodeFirst = true;
      if (
        includes(
          [
            ApplicationType.MySQL,
            ApplicationType.OpenGauss,
            ApplicationType.GoldenDB
          ],
          this.appType
        )
      ) {
        this.hasBackupNodeTip = true;
      }
      this.formGroup.addControl('slave_node_first', new FormControl(true));
    }

    // 流控类的参数
    if (
      includes(
        [
          ApplicationType.TiDB,
          ApplicationType.LightCloudGaussDB,
          ApplicationType.GaussDBForOpenGauss
        ],
        this.appType
      )
    ) {
      this.isRateLimit = true;
      this.formGroup.addControl(
        'rate_limit',
        new FormControl(75, {
          validators: [
            this.baseUtilService.VALID.rangeValue(1, 1024),
            this.baseUtilService.VALID.integer()
          ]
        })
      );
      if (includes([ApplicationType.TiDB], this.appType)) {
        // Tidb的限速默认不填就没有值
        this.formGroup.get('rate_limit').setValue('');
      }
    }

    // gaussdb的备机备份和数据压缩
    if (
      includes(
        [
          ApplicationType.LightCloudGaussDB,
          ApplicationType.GaussDBForOpenGauss
        ],
        this.appType
      )
    ) {
      this.isBackupGroup = true;
      this.formGroup.addControl(
        'enable_standby_backup',
        new FormControl(false)
      );
      this.formGroup.addControl('close_compression', new FormControl(false));
    }

    // 副本校验
    if (
      includes(
        [
          ApplicationType.DB2,
          ApplicationType.CNware,
          ApplicationType.FusionCompute,
          ApplicationType.FusionOne,
          ApplicationType.KubernetesStatefulSet,
          ApplicationType.HCSCloudHost,
          ApplicationType.OpenStack,
          ApplicationType.ApsaraStack,
          ApplicationType.HyperV
        ],
        this.appType
      )
    ) {
      this.isCopyVerify = true;
      if (
        includes(
          [
            ApplicationType.CNware,
            ApplicationType.FusionCompute,
            ApplicationType.FusionOne,
            ApplicationType.KubernetesStatefulSet,
            ApplicationType.HCSCloudHost,
            ApplicationType.OpenStack,
            ApplicationType.ApsaraStack,
            ApplicationType.HyperV
          ],
          this.appType
        )
      ) {
        this.isCopyVerifyInfo = true;
      }
      this.formGroup.addControl('copy_verify', new FormControl(false));
    }

    // 归档日志删除
    if (includes([ApplicationType.DB2], this.appType)) {
      this.isArchiveLogDelete = true;
      this.formGroup.addControl('delete_log', new FormControl(false));
    }

    // 日志备份失败后自动转全量
    if (includes([ApplicationType.GaussDBT], this.appType)) {
      this.isLogFailTransformFull = true;
      this.formGroup.addControl('autoFullBackup', new FormControl(false));
    }

    // 并发进程数
    if (
      includes(
        [ApplicationType.GaussDBT, ApplicationType.KingBase],
        this.appType
      )
    ) {
      this.isParallelNumber = true;
      let maxNumLimit = 16;
      if (this.appType === ApplicationType.KingBase) {
        maxNumLimit = 999;
        this.showParallelNumberTip = true;
        this.parallelNumberErrorTip.invalidRang = this.i18n.get(
          'common_valid_rang_label',
          [1, 999]
        );
        this.parallelNumberPlaceHolder = '1~999';
      }
      this.formGroup.addControl(
        'parallel_process',
        new FormControl('', {
          validators: [
            this.baseUtilService.VALID.integer(),
            this.baseUtilService.VALID.rangeValue(1, maxNumLimit)
          ]
        })
      );
    }

    // 线程数
    if (includes([ApplicationType.PostgreSQL], this.appType)) {
      this.isThread = true;
      this.formGroup.addControl(
        'thread_number',
        new FormControl(1, {
          validators: [
            this.baseUtilService.VALID.integer(),
            this.baseUtilService.VALID.rangeValue(1, 256)
          ]
        })
      );
    }

    // 线程数
    if (includes([ApplicationType.PostgreSQL], this.appType)) {
      this.isThread = true;
      this.formGroup.addControl(
        'thread_number',
        new FormControl(1, {
          validators: [
            this.baseUtilService.VALID.integer(),
            this.baseUtilService.VALID.rangeValue(1, 256)
          ]
        })
      );
    }

    // 备份运行内存
    if (includes([ApplicationType.TDSQL], this.appType)) {
      this.isUseMemory = true;
      this.formGroup.addControl(
        'use_memory',
        new FormControl('', {
          validators: [
            this.baseUtilService.VALID.integer(),
            this.baseUtilService.VALID.rangeValue(1, 2048)
          ]
        })
      );
    }

    // 生产存储剩余容量阈值
    if (
      includes(
        [ApplicationType.KubernetesStatefulSet, ApplicationType.Exchange],
        this.appType
      )
    ) {
      this.isCapacityThreshold = true;
      switch (this.appType) {
        case ApplicationType.Exchange:
          this.capacityThresholdToolTip = this.i18n.get(
            'protection_exchange_capacity_threshold_tip_label'
          );
          this.createCapacityFormGroup(5, 1, 100);
          break;
        default:
          this.createCapacityFormGroup();
      }
    }
  }

  getAutoIndexTip() {
    // 获取自动索引问号提示内容
    if (this.appType === ApplicationType.HDFS) {
      return this.i18n.get('protection_hdfs_auto_index_tip_label');
    } else {
      return this.i18n.get('protection_auto_index_tip_label');
    }
  }

  getDeduplicationTip() {
    if (this.appType === ApplicationType.NASFileSystem) {
      return this.i18n.get(
        'protection_deduplication_nas_filesystem_desc_label'
      );
    } else {
      return this.i18n.get('protection_deduplication_desc_label');
    }
  }

  getConcurrentTip() {
    // 获取并发数问号提示内容
    if (this.appType === ApplicationType.SQLServer) {
      return this.i18n.get('protection_sql_server_channel_number_tips_label');
    } else if (this.appType === ApplicationType.OpenGauss) {
      return this.i18n.get('protection_channel_number_help_label');
    } else if (this.appType === ApplicationType.TDSQL) {
      return this.i18n.get(
        'protection_tdsql_distributed_concurrent_tips_label'
      );
    }
  }

  getConcurrentPlaceholder() {
    // 获取并发数占位符
    if (this.appType === ApplicationType.Dameng) {
      return '1~128';
    } else {
      return '1~256';
    }
  }

  getBackupNodeTip() {
    // 获取备节点备份问号提示
    if (this.appType === ApplicationType.MySQL) {
      return this.i18n.get('protection_sla_mysql_slave_node_first_help_label');
    } else if (this.appType === ApplicationType.GoldenDB) {
      return this.i18n.get('protection_sla_slave_node_first_help_label');
    } else if (this.appType === ApplicationType.OpenGauss) {
      return this.i18n.get('protection_backup_node_preferred_tip_label');
    }
  }

  getRateLimitLabel() {
    // 用于获取ratelimit代表的流控词条
    if (includes([ApplicationType.TiDB], this.appType)) {
      return this.i18n.get('protection_tidb_sla_rate_limit_label');
    } else {
      return this.i18n.get('protection_gaussdb_backup_flow_control_label');
    }
  }

  getRateLimitUnit() {
    // 用于获取ratelimit代表的单位
    if (includes([ApplicationType.TiDB], this.appType)) {
      return '(MiB/s)';
    } else {
      return '(MB/s)';
    }
  }

  createCapacityFormGroup(defaultValue = 20, minValue = 0, maxValue = 100) {
    this.formGroup.addControl(
      'available_capacity_threshold',
      new FormControl(defaultValue, {
        validators: [
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.integer(),
          this.baseUtilService.VALID.rangeValue(minValue, maxValue)
        ]
      })
    );
    this.capacityThresholdPlaceHolder = `${minValue}~${maxValue}`;
    assign(this.capacityThresholdErrorTip, this.baseUtilService.rangeErrorTip, {
      invalidRang: this.i18n.get('common_valid_rang_label', [
        minValue,
        maxValue
      ])
    });
  }

  storageTypeChange(e) {
    this.isDisableQos = e;
  }

  updateData() {
    if (!this.data || !size(this.data)) {
      return;
    }

    const data = first(map(this.data, 'ext_parameters'));
    if (data) {
      this.formGroup.patchValue(data);
      if (this.appType === ApplicationType.TDSQL) {
        this.formGroup
          .get('channel_number')
          .setValue(get(data, 'channel_number', ''));
        this.formGroup
          .get('use_memory')
          .setValue(get(data, 'use_memory') || '1024');
      }

      if (this.appType === ApplicationType.SQLServer) {
        // 升级上来的sla应该展示默认值12，而不是'--'
        this.formGroup
          .get('channel_number')
          .setValue(get(data, 'channel_number', 1));
      }

      if (this.appType === ApplicationType.PostgreSQL) {
        // 升级上来的sla应该展示默认值1
        this.formGroup
          .get('thread_number')
          .setValue(get(data, 'thread_number', 1));
      }

      if (this.appType === ApplicationType.Exchange) {
        // 升级上来的available_capacity_threshold应该展示默认值5
        this.formGroup
          .get('available_capacity_threshold')
          .setValue(get(data, 'available_capacity_threshold', 5));
      }
    }
    this.isRetry = data.auto_retry;
    if (this.appType === ApplicationType.OpenGauss) {
      if (isUndefined(this.formGroup.value.storage_type)) {
        this.formGroup.get('storage_type').setValue('none');
      }
    }
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
