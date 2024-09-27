import { Component, Input, OnInit } from '@angular/core';
import { FormArray, FormBuilder, FormControl, FormGroup } from '@angular/forms';
import { ModalRef } from '@iux/live';
import {
  BaseUtilService,
  CapacityCalculateLabel,
  ClientManagerApiService,
  CommonConsts,
  CopiesService,
  DataMap,
  I18NService,
  isJson,
  ProtectedEnvironmentApiService,
  RestoreV2LocationType,
  RestoreV2Type,
  VmFileReplaceStrategy
} from 'app/shared';
import {
  ProtectedResourceApiService,
  RestoreApiV2Service
} from 'app/shared/api/services';
import { AppUtilsService } from 'app/shared/services/app-utils.service';
import {
  assign,
  each,
  filter,
  find,
  first,
  get,
  includes,
  intersection,
  isEmpty,
  isNumber,
  isString,
  isUndefined,
  map,
  size
} from 'lodash';
import { Observable, Observer } from 'rxjs';

@Component({
  selector: 'aui-tdsql-restore',
  templateUrl: './tdsql-restore.component.html',
  styleUrls: ['./tdsql-restore.component.less'],
  providers: [CapacityCalculateLabel]
})
export class TdsqlRestoreComponent implements OnInit {
  _isEmpty = isEmpty;
  resourceData;
  resourcePro;
  targetOptions = [];
  instanceOptions = [];
  proxyOptions = [];
  cacheProxyOptions = [];
  machineOptions = [];
  nodeOptions = [];
  formGroupValidateInfo = {}; // 当前界面的校验信息，恢复位置切换时需要同步更新。
  formGroup: FormGroup;
  restoreLocationType = RestoreV2LocationType;
  fileReplaceStrategy = VmFileReplaceStrategy;
  rowCopySqlVersion;
  rowCopyMachineInfo;
  hideCreateInstance = true;
  instanceNameErrorTip = {
    ...this.baseUtilService.nameErrorTip,
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [64])
  };
  InstanceStatus = {
    NEW: {
      value: 'new',
      label: 'common_create_instance_label'
    },
    EXISTED: {
      value: 'existed',
      label: 'common_existed_instance_label'
    }
  };
  @Input() rowCopy;
  @Input() childResType;
  @Input() restoreType;
  isDrill;

  constructor(
    private fb: FormBuilder,
    private i18n: I18NService,
    private modal: ModalRef,
    private copiesService: CopiesService,
    private appUtilsService: AppUtilsService,
    public baseUtilService: BaseUtilService,
    private clientManagerApiService: ClientManagerApiService,
    private capacityCalculate: CapacityCalculateLabel,
    private restoreV2Service: RestoreApiV2Service,
    private protectedResourceApiService: ProtectedResourceApiService,
    private protectedEnvironmentApiService: ProtectedEnvironmentApiService
  ) {}

  ngOnInit() {
    this.initData();
    this.initForm();
    this.getClusterOptions();
    this.getProxyOptions();
  }

  initData() {
    this.resourceData = isString(this.rowCopy.resource_properties)
      ? JSON.parse(this.rowCopy.resource_properties)
      : {};
    this.resourcePro = isString(this.rowCopy.properties)
      ? JSON.parse(this.rowCopy.properties || '{}')
      : {};
    // 副本信息中携带mysql版本，用于和目标主机的进行比较
    this.rowCopySqlVersion = get(this.resourcePro, 'mysql_version', '');
    this.rowCopyMachineInfo = get(this.resourcePro, 'instance_config_info', {});
    // 1.5版本的副本不会有这个字段，所以低版本需要屏蔽新建实例
    // 因为无法取到原机型配置信息
    this.hideCreateInstance = isEmpty(this.rowCopyMachineInfo);
    // 如果是日志副本，需要去找到最近的数据副本，然后判断该数据副本是否支持配置机型信息
    if (this.rowCopy.backup_type === DataMap.CopyData_Backup_Type.log.value) {
      // 日志副本的LOG仓里会记录最近一次数据副本的id--UBC逻辑
      const repositories = get(this.resourcePro, 'repositories', []);
      const logRepository = find(repositories, {
        type: DataMap.RepositoryDataTypeEnum.LOG.value
      });
      const latestDataCopyId = get(
        logRepository,
        'extendInfo.logBackup.latestDataCopyId'
      );
      this.queryNearestFullCopy(latestDataCopyId).subscribe(res => {
        const copyInfo = res.items[0];
        let resourcePro = isJson(copyInfo.properties)
          ? JSON.parse(copyInfo.properties || '{}')
          : {};
        this.rowCopyMachineInfo = get(resourcePro, 'instance_config_info', {});
        this.hideCreateInstance = isEmpty(this.rowCopyMachineInfo);
        this.initMachineInfo();
        this.getProxyOptions();
      });
    } else {
      this.initMachineInfo();
    }
  }

  initMachineInfo() {
    if (!this.hideCreateInstance) {
      const { cpu, memory, data_disk, log_disk } = this.rowCopyMachineInfo;
      this.rowCopyMachineInfo = {
        cpu: (Number(cpu) * 10) / 1000 || 1,
        memory: Number(memory) / 1000 || 1,
        dataDisk: Number(data_disk) / 1000 || 1,
        logDisk: Number(log_disk) / 1000 || 1
      };
    }
  }

  initForm() {
    this.formGroup = this.fb.group({
      restoreLocation: new FormControl(RestoreV2LocationType.ORIGIN),
      originLocation: new FormControl({
        value: this.resourceData?.name,
        disabled: true
      }),
      target: new FormControl(
        { value: '', disabled: true },
        {
          validators: this.baseUtilService.VALID.required()
        }
      ),
      instance: new FormControl(
        { value: '', disabled: true },
        {
          validators: this.baseUtilService.VALID.required()
        }
      ),
      instanceStatus: new FormControl(this.InstanceStatus.EXISTED.value),
      drMode: new FormControl(''),
      newInstanceName: new FormControl(''),
      dataNodes: this.fb.array([]),
      machineInfo: new FormGroup({
        machine: new FormControl(''),
        cpu: new FormControl(''),
        memory: new FormControl(''),
        dataDisk: new FormControl(''),
        logDisk: new FormControl('')
      })
    });

    this.listenForm();
    this.modal.getInstance().lvOkDisabled = false;
    if (
      this.rowCopy?.resource_status === DataMap.Resource_Status.notExist.value
    ) {
      this.formGroup.get('restoreLocation').setValue(RestoreV2LocationType.NEW);
    }
  }

  listenForm() {
    this.formGroup.statusChanges.subscribe(res => this.disableOkBtn());

    this.formGroup.get('restoreLocation').valueChanges.subscribe(res => {
      if (res === RestoreV2LocationType.ORIGIN) {
        this.formGroup.get('dataNodes').disable();
        this.formGroup.get('target').disable();
        this.formGroup.get('instance').disable();
      } else {
        if (
          this.formGroup.get('instanceStatus').value ===
          this.InstanceStatus.NEW.value
        ) {
          this.formGroup.get('dataNodes').enable();
        } else {
          this.formGroup.get('instance').enable();
        }
        this.formGroup.get('target').enable();
      }
    });

    this.formGroup.get('target').valueChanges.subscribe(res => {
      this.formGroup.get('instance').setValue('');
      this.instanceOptions = [];

      if (isEmpty(res)) {
        return;
      }
      this.formGroup.get('drMode').setValue('');
      this.getMachineAndHostById(res);
      this.getInstancesOptions(res);
    });

    this.formGroup.get('instanceStatus').valueChanges.subscribe(res => {
      if (res === this.InstanceStatus.NEW.value) {
        this.formGroup.get('instance').disable();
        this.formGroup.get('dataNodes').enable();
        this.formGroup
          .get('dataNodes')
          .setValidators([this.baseUtilService.VALID.required()]);
      } else {
        this.formGroup.get('dataNodes').disable();
        this.formGroup.get('instance').enable();
        this.formGroup.get('dataNodes').clearValidators();
      }
      this.formGroup.get('dataNodes').updateValueAndValidity();
    });

    this.formGroup.get('drMode').valueChanges.subscribe(res => {
      this.addDataRow(this.getNumArray(res));
    });

    this.formGroup.get('dataNodes').valueChanges.subscribe(res => {
      if (!res || isEmpty(res)) {
        this.proxyOptions = map(this.proxyOptions, item =>
          assign(item, {
            disabled: false
          })
        );
        return;
      }
      const cacheArr = map(res, 'parentUuid');
      this.proxyOptions = map(this.proxyOptions, item =>
        assign(item, {
          disabled: includes(cacheArr, item.uuid)
        })
      );
    });

    this.formGroup.get('machineInfo.machine').valueChanges.subscribe(res => {
      if (!res) {
        this.formGroup.get('machineInfo').clearValidators();
        this.formGroup.get('machineInfo').updateValueAndValidity();
        return;
      }
      // 返回值是{machine,cpu,memory,dataDisk,logDisk}
      const { machine, ...extraMachineParams } = res;
      each(extraMachineParams, (rawValue, key) => {
        let value = rawValue;
        if (key === 'cpu') {
          value = rawValue * 1000;
        }
        const endValue = value / 1000;
        const startNumber = endValue > 1 ? 1 : 0;
        this.formGroupValidateInfo[key] = {
          rangePlaceholder: this.i18n.get('protection_input_range_tips_label', [
            startNumber,
            endValue
          ]),
          rangeErrorTips: {
            ...this.baseUtilService.rangeErrorTip,
            invalidRang: this.i18n.get('common_valid_rang_label', [
              startNumber,
              endValue
            ])
          },
          defaultValue: '',
          // cpu、数据磁盘、日志磁盘必须是整数，内存可以为小数
          validateFn: includes(['cpu', 'logDisk', 'dataDisk'], key)
            ? [
                this.baseUtilService.VALID.integer(),
                this.baseUtilService.VALID.rangeValue(1, value),
                this.baseUtilService.VALID.required()
              ]
            : [
                this.baseUtilService.VALID.rangeValue(startNumber, endValue),
                this.baseUtilService.VALID.required()
              ]
        };
        this.formGroup
          .get(['machineInfo', key])
          .setValue(this.rowCopyMachineInfo[key] || 1);
        this.formGroup
          .get(['machineInfo', key])
          .setValidators([...this.formGroupValidateInfo[key].validateFn]);
      });
    });
  }

  get dataNodes() {
    return (this.formGroup.get('dataNodes') as FormArray).controls;
  }

  getNumArray(num: number) {
    return Array.from({ length: num }, (_, i) => i + 1);
  }

  capacityCalculateLabel(rawData) {
    return map(rawData, item =>
      this.capacityCalculate.transform(item, '1.0-3', 'MB', false, false, 1000)
    );
  }

  getMachineAndHostById(envId: string) {
    if (this.hideCreateInstance) {
      return;
    }
    const extParams = {
      envId,
      resourceType: DataMap.Resource_Type.tdsqlCluster.value,
      conditions: JSON.stringify({ queryType: 'resource' })
    };
    this.appUtilsService.getResourceByRecursion(
      extParams,
      params =>
        this.protectedEnvironmentApiService.ListEnvironmentResource(params),
      res => {
        if (isEmpty(res)) return;
        const machineStr = get(first(res), 'extendInfo.machineSpec', '[]');
        const hostStr = get(first(res), 'extendInfo.hosts', '[]');
        const machineData = JSON.parse(machineStr);
        const hostData = JSON.parse(hostStr);
        const machineArr = map(machineData, item => {
          const { machine, cpu, ...rest } = item;
          const formattedLabel = this.capacityCalculateLabel(rest);
          return {
            key: machine,
            value: { ...item },
            isLeaf: true,
            label: this.i18n.get('protection_tdsql_machine_type_label', [
              machine,
              cpu,
              ...formattedLabel
            ])
          };
        });
        this.proxyOptions = [...this.cacheProxyOptions];
        const filterArr = filter(this.proxyOptions, item => {
          const intersectionArr = intersection(item.ipList, hostData);
          if (!!size(intersectionArr)) {
            return assign(item, { ip: intersectionArr[0] });
          }
        });
        this.proxyOptions = [...filterArr];
        this.initNodeOptions(size(filterArr));
        this.machineOptions = [...machineArr];
      }
    );
  }

  getProxyOptions() {
    if (this.hideCreateInstance) {
      return;
    }
    const extParams = {
      conditions: JSON.stringify({
        pluginType: `${DataMap.Resource_Type.tdsqlCluster.value}Plugin`
      })
    };
    this.appUtilsService.getResourceByRecursion(
      extParams,
      params => this.clientManagerApiService.queryAgentListInfoUsingGET(params),
      resource => {
        const hostArray = [];
        each(resource, item => {
          hostArray.push({
            ...item,
            key: item.uuid,
            value: item.uuid,
            label: `${item.name}(${item.endpoint})`,
            ipList: item.extendInfo.agentIpList.split(','),
            isLeaf: true
          });
        });
        this.proxyOptions = [...hostArray];
        this.cacheProxyOptions = this.proxyOptions; // 缓存代理主机
      }
    );
  }

  initNodeOptions(maxNumber: number) {
    const chineseNums = ['零', '一', '两', '三', '四', '五'];
    const nodeOpts = chineseNums.slice(0, maxNumber);
    this.nodeOptions = map(nodeOpts, (item, index) => {
      return {
        key: index + 1,
        label: this.i18n.get(
          'protection_instance_node_option_label',
          this.i18n.isEn ? [index] : [item]
        ),
        value: index + 1,
        isLeaf: true
      };
    });
  }

  getInstancesOptions(uuid, recordsTemp?, startPage?) {
    const params = {
      pageNo: startPage || CommonConsts.PAGE_START,
      pageSize: CommonConsts.PAGE_SIZE * 10,
      conditions: JSON.stringify({
        parentUuid: uuid,
        subType: [DataMap.Resource_Type.tdsqlInstance.value]
      })
    };

    this.protectedResourceApiService.ListResources(params).subscribe(res => {
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
        const instanceArray = [];

        each(recordsTemp, item => {
          instanceArray.push({
            ...item,
            key: item.uuid,
            value: item.uuid,
            label: item.name,
            isLeaf: true,
            disabled: this.disableTargetInstance(item)
          });
        });
        this.instanceOptions = filter(
          instanceArray,
          item => item.uuid !== this.resourceData.uuid
        );
        return;
      }
      this.getInstancesOptions(uuid, recordsTemp, startPage);
    });
  }

  disableTargetInstance(data) {
    // d首个ataNodes中节点的defaultsFile用'/'分割，第5位就是数据库版本
    const instanceInfoStr = get(data, 'extendInfo.clusterInstanceInfo', '{}');
    const clusterGroups = get(JSON.parse(instanceInfoStr), 'groups', [])[0];
    const dataNode = get(clusterGroups, 'dataNodes', [])[0];
    const version = get(dataNode, 'defaultsFile', '').split('/');
    return this.rowCopySqlVersion !== version[4];
  }

  getClusterOptions(recordsTemp?, startPage?) {
    const params = {
      pageNo: startPage || CommonConsts.PAGE_START,
      pageSize: CommonConsts.PAGE_SIZE * 10,
      conditions: JSON.stringify({
        subType: [DataMap.Resource_Type.tdsqlCluster.value]
      })
    };

    this.protectedResourceApiService.ListResources(params).subscribe(res => {
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
        const clusterArray = [];

        each(recordsTemp, item => {
          clusterArray.push({
            ...item,
            key: item.uuid,
            value: item.uuid,
            label: item.name,
            isLeaf: true
          });
        });
        this.targetOptions = clusterArray;
        return;
      }
      this.getClusterOptions(recordsTemp, startPage);
    });
  }

  queryNearestFullCopy(uuid) {
    const params = {
      pageSize: 1,
      pageNo: CommonConsts.PAGE_START,
      conditions: JSON.stringify({
        resource_sub_type: [DataMap.Resource_Type.tdsqlInstance.value],
        uuid
      })
    };
    return this.copiesService.queryResourcesV1CopiesGet(params);
  }

  getParams() {
    const params: any = {
      copyId: this.rowCopy.uuid,
      targetEnv:
        this.formGroup.value.restoreLocation === RestoreV2LocationType.ORIGIN
          ? this.resourceData?.parentUuid || this.resourceData?.parent_uuid
          : this.formGroup.value.target,
      restoreType:
        this.restoreType === RestoreV2Type.CommonRestore
          ? RestoreV2Type.CommonRestore
          : RestoreV2Type.FileRestore,
      targetLocation: this.formGroup.value.restoreLocation,
      targetObject:
        this.formGroup.value.restoreLocation === RestoreV2LocationType.ORIGIN
          ? this.resourceData?.uuid
          : this.formGroup.value.instance
    };

    if (this.rowCopy.backup_type === DataMap.CopyData_Backup_Type.log.value) {
      const properties = isString(this.rowCopy.properties)
        ? JSON.parse(this.rowCopy.properties)
        : {};
      assign(params, {
        extendInfo: {
          restoreTimestamp:
            get(this.rowCopy, 'restoreTimeStamp') || get(properties, 'endTime')
        }
      });
    }

    if (
      this.formGroup.get('instanceStatus').value ===
      this.InstanceStatus.NEW.value
    ) {
      const {
        machine: selectedMachine,
        ...inputParams
      } = this.formGroup.value.machineInfo;
      const restoreHosts = map(this.formGroup.get('dataNodes').value, item => {
        return assign(item, {
          ...item,
          ip: find(this.cacheProxyOptions, { uuid: item.parentUuid }).ip
        });
      });
      assign(params, {
        extendInfo: {
          newInstanceName: this.formGroup.get('newInstanceName').value,
          create_new_instance: true,
          restoreHosts: JSON.stringify(restoreHosts),
          restoreLocation: find(this.targetOptions, { uuid: params.targetEnv })[
            'name'
          ]
        }
      });
      const machineValue = this.formGroup.get('machineInfo').get('machine')
        .value;
      const extParameter = isEmpty(machineValue)
        ? ''
        : JSON.stringify({ machine: selectedMachine.machine, ...inputParams });

      assign(params, {
        extendInfo: {
          ...params.extendInfo,
          extParameter
        }
      });
    }
    return params;
  }

  getTargetPath() {
    const form = this.formGroup;
    if (form.get('restoreLocation').value === RestoreV2LocationType.ORIGIN) {
      return this.resourceData?.name;
    } else {
      if (form.get('instanceStatus').value === this.InstanceStatus.NEW.value) {
        return find(this.targetOptions, { uuid: form.get('target').value })[
          'name'
        ];
      } else {
        return find(this.instanceOptions, {
          value: this.formGroup.value.instance
        })['label'];
      }
    }
  }

  getTargetPort() {
    const target = find(this.instanceOptions, {
      value: this.formGroup.value.instance
    });
    const clusterInstanceInfo = get(target, 'extendInfo.clusterInstanceInfo');
    if (!isUndefined(clusterInstanceInfo)) {
      const dataNodes = JSON.parse(clusterInstanceInfo).groups[0].dataNodes;
      return find(dataNodes, {
        isMaster: '1'
      }).port;
    } else return '0';
  }

  addDataRow(rowData) {
    const dataNodesForm = this.formGroup.get('dataNodes') as FormArray;
    dataNodesForm.clear();
    each(rowData, item => {
      dataNodesForm.push(
        this.fb.group({
          parentUuid: new FormControl('', {
            validators: [this.baseUtilService.VALID.required()]
          }),
          priority: DataMap.tdsqlNodePriority.medium.value,
          nodeType: DataMap.tdsqlNodeType.dataNode.value
        })
      );
    });
  }

  restore(): Observable<void> {
    return new Observable<void>((observer: Observer<void>) => {
      const params = this.getParams();
      this.restoreV2Service
        .CreateRestoreTask({ CreateRestoreTaskRequestBody: params as any })
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

  disableOkBtn() {
    this.modal.getInstance().lvOkDisabled = this.formGroup.invalid;
  }
}
