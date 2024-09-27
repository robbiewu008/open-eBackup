import { Component, Input, OnInit, ViewChild } from '@angular/core';
import { FormArray, FormBuilder, FormControl, FormGroup } from '@angular/forms';
import {
  BaseUtilService,
  CommonConsts,
  DataMap,
  DataMapService,
  I18NService,
  MODAL_COMMON,
  ProtectedEnvironmentApiService,
  ProtectedResourceApiService,
  ResourceType
} from 'app/shared';
import { AppUtilsService } from 'app/shared/services/app-utils.service';
import { each, filter, first, get, map, size, isNil } from 'lodash';
import { MessageboxService } from '@iux/live';
import {
  ProTableComponent,
  TableCols,
  TableConfig,
  TableData
} from 'app/shared/components/pro-table';
import { Observable, Observer } from 'rxjs';

@Component({
  selector: 'aui-register-distributed-instance',
  templateUrl: './register-distributed-instance.component.html',
  styleUrls: ['./register-distributed-instance.component.less']
})
export class RegisterDistributedInstanceComponent implements OnInit {
  formGroup: FormGroup;
  instanceTableConfig: TableConfig;
  instanceTableData: TableData;
  dataNodesData;
  clusterOptions: any[];
  proxyOptions;
  nameErrorTip = {
    ...this.baseUtilService.nameErrorTip,
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [32])
  };
  usernameErrorTip = {
    ...this.baseUtilService.requiredErrorTip,
    ...this.baseUtilService.lengthErrorTip,
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [32])
  };
  agentsErrorTip = {
    ...this.baseUtilService.requiredErrorTip,
    invalidMinLength: this.i18n.get('common_host_number_least_2_label')
  };
  passwordErrorTip = {
    ...this.baseUtilService.requiredErrorTip,
    ...this.baseUtilService.lengthErrorTip,
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [32])
  };
  @Input() rowData: any;
  @ViewChild('instanceTable', { static: false })
  instanceTable: ProTableComponent;
  constructor(
    public i18n: I18NService,
    public baseUtilService: BaseUtilService,
    private fb: FormBuilder,
    private appUtilsService: AppUtilsService,
    private messageBox: MessageboxService,
    private dataMapService: DataMapService,
    private protectedResourceApiService: ProtectedResourceApiService,
    private protectedEnvironmentApiService: ProtectedEnvironmentApiService
  ) {}

  ngOnInit(): void {
    this.initForm();
    this.initTable();
    this.getClusterOptions();
    this.getProxyOptions();
  }

  initForm() {
    this.formGroup = this.fb.group({
      name: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.name(),
          this.baseUtilService.VALID.maxLength(64)
        ]
      }),
      cluster: new FormControl('', {
        validators: [this.baseUtilService.VALID.required()]
      }),
      groupId: new FormControl(
        {
          value: '',
          disabled: !!this.rowData
        },
        {
          validators: [this.baseUtilService.VALID.required()]
        }
      ),
      dataNodes: this.fb.array([], this.baseUtilService.VALID.required())
    });
    if (!!this.rowData) {
      this.getDetail();
    }
  }

  initTable() {
    const cols: TableCols[] = [
      {
        key: 'setId',
        name: this.i18n.get('SETID')
      }
    ];
    this.instanceTableConfig = {
      table: {
        columns: cols,
        showLoading: false,
        colDisplayControl: false,
        async: false
      },
      pagination: {
        mode: 'simple',
        showTotal: false,
        winTablePagination: true,
        showPageSizeOptions: false
      }
    };
  }

  get dataNodes() {
    return (this.formGroup.get('dataNodes') as FormArray).controls;
  }

  getDetail() {
    this.protectedResourceApiService
      .ShowResource({ resourceId: this.rowData.uuid })
      .subscribe(res => {
        const clusterGroupInfo = JSON.parse(
          get(res, 'extendInfo.clusterGroupInfo')
        );
        this.formGroup.patchValue({
          name: res.name,
          cluster: res.parentUuid,
          groupId: clusterGroupInfo.id
        });
        this.formatResData(clusterGroupInfo, true, res);
      });
  }

  addDataRow(rowData: { ip: string; parentUuid: string }, modify: boolean) {
    (this.formGroup.get('dataNodes') as FormArray).push(
      this.fb.group({
        host: new FormControl(rowData.ip),
        proxy: new FormControl(modify ? rowData.parentUuid : '', {
          validators: [this.baseUtilService.VALID.required()]
        })
      })
    );
  }

  scanDataNodes() {
    this.scancanDataNodes(size(this.dataNodes) !== 0);
  }

  scancanDataNodes(isRepeat = false) {
    const params = {
      pageNo: CommonConsts.PAGE_START,
      pageSize: CommonConsts.PAGE_SIZE * 10,
      envId: this.formGroup.value.cluster,
      resourceType: DataMap.Resource_Type.tdsqlCluster.value,
      conditions: JSON.stringify({
        type: DataMap.tdsqlInstanceType.distributed.value,
        id: this.formGroup.get('groupId').value
      })
    };
    if (isRepeat) {
      this.messageBox.confirm({
        lvDialogIcon: 'lv-icon-popup-danger-48',
        lvContent: this.i18n.get('protection_confirm_scan_data_node_label'),
        lvWidth: MODAL_COMMON.smallWidth + 50,
        lvCancelType: 'default',
        lvOkType: 'primary',
        lvOk: () => {
          if (size(this.dataNodes)) {
            (this.formGroup.get('dataNodes') as FormArray).clear();
          }
          this.instanceTableData = {
            data: [],
            total: 0
          };
          this.getNodes(params);
        }
      });
    } else {
      this.getNodes(params);
    }
  }

  getNodes(params) {
    this.protectedEnvironmentApiService
      .ListEnvironmentResource(params)
      .subscribe(res => {
        const clusterGroupInfo = JSON.parse(
          get(first(res.records), 'extendInfo.clusterGroupInfo', '{}')
        );
        this.formatResData(clusterGroupInfo, false, res);
      });
  }

  getClusterOptions() {
    const extParams = {
      conditions: JSON.stringify({
        subType: DataMap.Resource_Type.tdsqlCluster.value
      })
    };
    this.appUtilsService.getResourceByRecursion(
      extParams,
      param => this.protectedResourceApiService.ListResources(param),
      res => {
        this.clusterOptions = map(res, item => {
          return {
            key: item.uuid,
            value: item.uuid,
            label: item.name,
            isLeaf: true,
            ...item
          };
        });
      }
    );
  }

  formatResData(data, modify: boolean, res) {
    this.dataNodesData = [{}];
    const group = get(data, 'group');
    const originalNode = get(group, 'dataNodes');
    const instanceData = get(group, 'setIds');
    const instanceNodes = map(instanceData, item => {
      return {
        setId: item
      };
    });
    this.instanceTableData = {
      data: instanceNodes,
      total: size(instanceNodes)
    };
    each(originalNode, node => {
      this.addDataRow(node, modify);
    });
  }

  getParams() {
    const agents = map(this.formGroup.value.dataNodes, node => {
      return {
        uuid: node.proxy
      };
    });
    const dataNodes = map(this.formGroup.value.dataNodes, item => {
      return {
        ip: item.host,
        parentUuid: item.proxy
      };
    });
    const clusterGroupInfo = {
      id: this.formGroup.get('groupId').value,
      name: this.formGroup.get('name').value,
      cluster: this.formGroup.get('cluster').value,
      type: DataMap.tdsqlInstanceType.distributed.value,
      group: {
        groupId: this.formGroup.get('groupId').value,
        setIds: map(this.instanceTableData.data, item => item.setId),
        dataNodes
      }
    };
    return {
      name: this.formGroup.get('name').value,
      parentUuid: this.formGroup.get('cluster').value,
      type: ResourceType.DATABASE,
      subType: DataMap.Resource_Type.tdsqlDistributedInstance.value,
      extendInfo: {
        clusterGroupInfo: JSON.stringify(clusterGroupInfo)
      },
      dependencies: {
        agents
      }
    };
  }

  getProxyOptions() {
    const extParams = {
      conditions: JSON.stringify({
        type: 'Plugin',
        subType: [`${DataMap.Resource_Type.tdsqlCluster.value}Plugin`]
      })
    };
    this.appUtilsService.getResourceByRecursion(
      extParams,
      params => this.protectedResourceApiService.ListResources(params),
      resource => {
        const hostArray = [];
        resource = filter(resource, item => {
          return (
            (!isNil(item.environment) &&
              item.environment.extendInfo.scenario === '0') ||
            item.extendInfo.scenario === '0'
          );
        });
        each(resource, item => {
          const tmp = item.environment || item;
          hostArray.push({
            ...tmp,
            key: tmp.uuid,
            value: tmp.uuid,
            label: `${tmp.name}(${tmp.endpoint})`,
            isLeaf: true
          });
        });
        this.proxyOptions = [...hostArray];
      }
    );
  }

  onOK(): Observable<void> {
    return new Observable<void>((observer: Observer<void>) => {
      if (this.formGroup.invalid) {
        return;
      }
      const params = this.getParams();
      if (this.rowData) {
        this.protectedResourceApiService
          .UpdateResource({
            resourceId: this.rowData.uuid,
            UpdateResourceRequestBody: params
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
      } else {
        this.protectedResourceApiService
          .CreateResource({
            CreateResourceRequestBody: params
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
      }
    });
  }
}
