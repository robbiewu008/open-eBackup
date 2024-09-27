import { Component, OnInit, TemplateRef, ViewChild } from '@angular/core';
import { FormBuilder, FormControl, FormGroup } from '@angular/forms';
import { MessageService, ModalRef, UploadFile } from '@iux/live';
import {
  BaseUtilService,
  CommonConsts,
  DataMap,
  I18NService,
  MODAL_COMMON,
  ProtectedEnvironmentApiService,
  ProtectedResourceApiService,
  ResourceType,
  MultiCluster,
  getMultiHostOps
} from 'app/shared';
import { ProButton } from 'app/shared/components/pro-button/interface';
import {
  ProTableComponent,
  TableConfig,
  TableData
} from 'app/shared/components/pro-table';
import { AppUtilsService } from 'app/shared/services/app-utils.service';
import { DrawModalService } from 'app/shared/services/draw-modal.service';
import {
  assign,
  difference,
  each,
  filter,
  first,
  includes,
  intersection,
  isEmpty,
  map,
  size,
  isEqual,
  get,
  isUndefined,
  some
} from 'lodash';
import { Observable, Observer } from 'rxjs';
import { AddStorageComponent } from './add-storage/add-storage.component';

@Component({
  selector: 'aui-register-cluster',
  templateUrl: './register-cluster.component.html',
  styleUrls: ['./register-cluster.component.less']
})
export class RegisterClusterComponent implements OnInit {
  rowItem: any;
  formGroup: FormGroup;
  configFileFilter: object[];
  hostOptions = [];
  selectConfigFile;
  selectionData = [];
  optsConfig: ProButton[];
  tableConfig: TableConfig;
  tableData: TableData;
  dataMap = DataMap;

  nameErrorTip = assign({}, this.baseUtilService.requiredErrorTip, {
    ...this.baseUtilService.nameErrorTip,
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [64])
  });

  @ViewChild('modalFooter', { static: true }) modalFooter: TemplateRef<any>;
  @ViewChild('dataTable', { static: false }) dataTable: ProTableComponent;

  constructor(
    private appUtilsService: AppUtilsService,
    private modal: ModalRef,
    private fb: FormBuilder,
    private i18n: I18NService,
    private message: MessageService,
    public baseUtilService: BaseUtilService,
    private drawModalService: DrawModalService,
    private protectedResourceApiService: ProtectedResourceApiService,
    private protectedEnvironmentApiService: ProtectedEnvironmentApiService
  ) {}

  ngOnInit() {
    this.initForm();
    this.getProxyOptions();
    this.initConfigFileFilter();
    this.initTableConfig();
  }
  getProxyOptions() {
    const extParams = {
      conditions: JSON.stringify({
        type: 'Plugin',
        subType: [`${DataMap.Resource_Type.Kubernetes.value}Plugin`]
      })
    };
    this.appUtilsService.getResourceByRecursion(
      extParams,
      params => this.protectedResourceApiService.ListResources(params),
      resource => {
        resource = filter(resource, item => !isEmpty(item.environment));
        const hostArray = [];
        resource = filter(
          resource,
          item =>
            item.environment.extendInfo.scenario ===
            DataMap.proxyHostType.external.value
        );
        if (MultiCluster.isMulti) {
          resource = filter(resource, item => {
            const val = item.environment;
            const connection = val?.extendInfo?.connection_result;
            const targetObj = JSON.parse(connection || '{}');
            const linkFlag = some(
              targetObj,
              item =>
                item.link_status ===
                Number(DataMap.resource_LinkStatus_Special.normal.value)
            );

            if (
              linkFlag ||
              includes(
                map(this.rowItem?.dependencies?.agents, 'uuid'),
                val.uuid
              )
            ) {
              return true;
            }
            return (
              val.linkStatus ===
              DataMap.resource_LinkStatus_Special.normal.value
            );
          });
        }
        resource = filter(
          resource,
          item =>
            item.environment?.linkStatus ===
              DataMap.resource_LinkStatus_Special.normal.value ||
            includes(
              map(this.rowItem?.dependencies?.agents, 'uuid'),
              item.environment?.uuid
            )
        );
        each(resource, item => {
          const tmp = item.environment;
          hostArray.push({
            ...tmp,
            key: tmp.uuid,
            value: tmp.uuid,
            label: `${tmp.name}(${tmp.endpoint})`,
            isLeaf: true
          });
        });
        this.hostOptions = hostArray;
        if (!isEmpty(this.rowItem)) {
          this.formGroup
            .get('clusterNode')
            .setValue(
              intersection(
                map(this.rowItem.dependencies?.agents, 'uuid'),
                map(this.hostOptions, 'uuid')
              ),
              {
                emitEvent: false
              }
            );
        }
      }
    );
  }

  initForm() {
    if (this.rowItem?.uuid) {
      const tableData = [];
      for (let key in this.rowItem.extendInfo) {
        if (key.indexOf('storage_') !== -1) {
          tableData.push(JSON.parse(this.rowItem.extendInfo[key]));
        }
      }
      each(tableData, item => {
        if (isUndefined(item.ipList)) {
          assign(item, { ipList: item.ip });
        }
      });
      this.tableData = {
        data: tableData,
        total: size(tableData)
      };
    }
    this.formGroup = this.fb.group({
      name: new FormControl(this.rowItem ? this.rowItem.name : '', {
        validators: [
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.name()
        ]
      }),
      clusterNode: new FormControl([], {
        validators: this.baseUtilService.VALID.required()
      })
    });
    this.formGroup.statusChanges.subscribe(res => this.disableOkBtn());
  }

  disableOkBtn() {
    this.modal.getInstance().lvOkDisabled =
      this.formGroup.invalid ||
      isEmpty(this.selectConfigFile) ||
      isEmpty(this.tableData) ||
      size(this.tableData?.data) === 0;
  }

  initConfigFileFilter() {
    this.configFileFilter = [
      {
        name: 'suffix',
        filterFn: (files: UploadFile[]) => {
          if (files.length !== files.length) {
            this.message.error(
              this.i18n.get('common_format_error_label', ['xml']),
              {
                lvMessageKey: 'formatErrorKey3',
                lvShowCloseButton: true
              }
            );
            this.selectConfigFile = '';
            this.disableOkBtn();
            return [];
          }
          if (files[0].size > 1024 * 1024) {
            this.message.error(
              this.i18n.get('common_max_size_file_label', ['1MB']),
              {
                lvMessageKey: 'maxSizeFileErrorKey3',
                lvShowCloseButton: true
              }
            );
            this.selectConfigFile = '';
            this.disableOkBtn();
            return [];
          }
          const reader = new FileReader();
          reader.onloadend = () => {
            btoa(
              (this.selectConfigFile = (reader.result as any)
                .replace('data:', '')
                .replace(/^.+,/, ''))
            );
            this.disableOkBtn();
          };
          reader.readAsDataURL(files[0].originFile);
          return files;
        }
      }
    ];
  }

  initTableConfig() {
    this.optsConfig = [
      {
        id: 'add',
        label: this.i18n.get('common_add_label'),
        type: 'primary',
        disableCheck: () => {
          return this.tableData?.total === 32;
        },
        disabledTips: this.i18n.get('protection_number_limited_label'),
        onClick: () => this.addStorage()
      },
      {
        id: 'remove',
        label: this.i18n.get('common_remove_label'),
        disableCheck: data => {
          return !size(data);
        },
        onClick: data => this.removeStorage(data)
      }
    ];
    this.tableConfig = {
      table: {
        async: false,
        columns: [
          {
            key: 'ipList',
            name: this.i18n.get('common_ip_domain_name_label')
          },
          {
            key: 'operation',
            width: 130,
            hidden: 'ignoring',
            name: this.i18n.get('common_operation_label'),
            cellRender: {
              type: 'operation',
              config: {
                maxDisplayItems: 1,
                items: [
                  {
                    id: 'edit',
                    label: this.i18n.get('common_modify_label'),
                    onClick: ([data]) => this.addStorage(data)
                  },
                  this.optsConfig[1]
                ]
              }
            }
          }
        ],
        rows: {
          selectionMode: 'multiple',
          selectionTrigger: 'selector'
        },
        compareWith: 'ip',
        size: 'small',
        colDisplayControl: false,
        selectionChange: data => {
          this.selectionData = data;
        }
      },
      pagination: {
        mode: 'simple',
        pageSize: CommonConsts.PAGE_SIZE_SMALL,
        pageSizeOptions: CommonConsts.SIMPLE_PAGE_SIZE_OPTIONS,
        showPageSizeOptions: false,
        winTablePagination: true
      }
    };
  }

  addStorage(rowItem?) {
    this.drawModalService.create(
      assign({}, MODAL_COMMON.generateDrawerOptions(), {
        lvModalKey: 'add-storage',
        lvWidth: MODAL_COMMON.normalWidth + 100,
        lvHeader: rowItem
          ? this.i18n.get('common_modify_label')
          : this.i18n.get('protection_add_storage_resource_label'),
        lvContent: AddStorageComponent,
        lvOkDisabled: true,
        lvComponentParams: { rowItem },
        lvAfterOpen: modal => {
          const content = modal.getContentComponent() as AddStorageComponent;
          const modalIns = modal.getInstance();
          content.formGroup.statusChanges.subscribe(res => {
            modalIns.lvOkDisabled = res !== 'VALID';
          });
        },
        lvOk: modal => {
          return new Promise(resolve => {
            const content = modal.getContentComponent() as AddStorageComponent;
            content.onOK().subscribe(res => {
              if (
                !isEmpty(this.tableData) &&
                !this.checkIpList(res) &&
                !isEqual(res.ipList, rowItem?.ipList)
              ) {
                this.message.error(
                  this.i18n.get('protection_storage_exist_error_label'),
                  {
                    lvMessageKey: 'vaild-storage',
                    lvShowCloseButton: true
                  }
                );
                resolve(false);
                return;
              }
              resolve(true);
              if (isEmpty(this.tableData)) {
                this.tableData = {
                  data: [res],
                  total: 1
                };
              } else {
                // 修改
                if (rowItem) {
                  each(this.tableData.data, v => {
                    if (v.ip === rowItem.ip) {
                      assign(v, res);
                    }
                  });
                  this.tableData = {
                    data: [...this.tableData.data],
                    total: this.tableData.total
                  };
                } else {
                  this.tableData = {
                    data: [...this.tableData.data, res],
                    total: ++this.tableData.total
                  };
                }
              }
              this.selectionData = [...this.selectionData];
              this.disableOkBtn();
            });
          });
        }
      })
    );
  }

  checkIpList(res) {
    for (let item of this.tableData.data) {
      let ipListArray = item.ipList.split(',');
      if (ipListArray.includes(res.ip)) {
        return false; // 找到匹配，返回false
      }
    }
    return true; // 没有找到匹配，返回true
  }

  removeStorage(data: any[]) {
    const firstIps = data.map(item => item.ip);
    const filterData = this.tableData?.data.filter(item => {
      return !firstIps.includes(item.ip);
    });
    this.tableData = {
      data: filterData,
      total: size(filterData)
    };
    this.selectionData = filter(this.selectionData, item => {
      return includes(map(filterData, 'ip'), item.ip);
    });
    this.dataTable.setSelections(this.selectionData);
    this.disableOkBtn();
  }

  getParams() {
    const storageObj = {};
    each(this.tableData.data, item => {
      const ipListArr = item.ipList.split(',');
      assign(storageObj, {
        [`storage_${first(ipListArr)}`]: JSON.stringify({
          username: item.username,
          password: item.password,
          ip: first(ipListArr),
          ipList: ipListArr.join(','),
          port: 8088
        })
      });
    });
    let reduceAgents = [];
    if (!isEmpty(this.rowItem)) {
      reduceAgents = difference(
        this.rowItem.dependencies?.agents.map(item => item.uuid),
        this.formGroup.value.clusterNode
      );
    }
    const params = {
      name: this.formGroup.value.name,
      type: ResourceType.Container,
      subType: DataMap.Resource_Type.Kubernetes.value,
      auth: {
        authType: this.formGroup.value.loginMode,
        extendInfo: {
          config: this.selectConfigFile,
          ...storageObj
        }
      },
      dependencies: {
        agents: map(this.formGroup.value.clusterNode, item => {
          return { uuid: item };
        })
      }
    };

    if (!isEmpty(this.rowItem)) {
      assign(params.dependencies, {
        '-agents': reduceAgents.map(item => {
          return { uuid: item };
        })
      });
    }

    return params;
  }

  onOK(): Observable<void> {
    return new Observable<void>((observer: Observer<void>) => {
      if (this.formGroup.invalid) {
        return;
      }
      const params = this.getParams();
      if (this.rowItem?.uuid) {
        this.protectedEnvironmentApiService
          .UpdateProtectedEnvironment({
            envId: this.rowItem.uuid,
            UpdateProtectedEnvironmentRequestBody: params
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
        this.protectedEnvironmentApiService
          .RegisterProtectedEnviroment({
            RegisterProtectedEnviromentRequestBody: params as any
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
