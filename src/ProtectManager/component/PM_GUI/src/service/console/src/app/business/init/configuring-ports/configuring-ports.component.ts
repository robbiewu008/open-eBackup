import {
  ChangeDetectionStrategy,
  ChangeDetectorRef,
  Component,
  EventEmitter,
  Input,
  OnInit,
  Output,
  ViewChild
} from '@angular/core';
import { FormGroup } from '@angular/forms';
import { DatatableComponent, MessageService, UploadFile } from '@iux/live';
import {
  BaseUtilService,
  DataMap,
  DataMapService,
  I18NService,
  LogManagerApiService,
  SystemApiService
} from 'app/shared';
import { TableConfig, TableData } from 'app/shared/components/pro-table';
import { DrawModalService } from 'app/shared/services/draw-modal.service';
import {
  assign,
  cloneDeep,
  each,
  filter,
  find,
  includes,
  result,
  size
} from 'lodash';
import { Subject } from 'rxjs';

@Component({
  selector: 'aui-configuring-ports',
  templateUrl: './configuring-ports.component.html',
  styleUrls: ['./configuring-ports.component.less'],
  changeDetection: ChangeDetectionStrategy.OnPush
})
export class ConfiguringPortsComponent implements OnInit {
  @Input() componentData;
  @Output() onStatusChange = new EventEmitter<any>();
  data;
  rawData;
  dataMap = DataMap;
  newConfigData = [];
  formGroup: FormGroup;
  tableConfig: TableConfig;
  tableData: TableData;

  originalBackupNetworks;
  oringinalArchiveNetworks;
  oringinalReplicationNetworks;
  backupNetworks = [];
  archivedNetworks = {} as any;
  replicationNetworks = {} as any;
  bondPortList: [];
  logicalData: [];
  networkSize = false;

  selectFile;
  valid$ = new Subject<boolean>();
  fileReceive = false;
  filters = [];
  uploadable = true;
  columns = [];
  controllers = [];

  isCloudBackup = includes(
    [
      DataMap.Deploy_Type.cloudbackup2.value,
      DataMap.Deploy_Type.cloudbackup.value
    ],
    this.i18n.get('deploy_type')
  );
  @ViewChild(DatatableComponent, { static: false }) lvTable: DatatableComponent;

  constructor(
    public message: MessageService,
    public baseUtilService: BaseUtilService,
    private i18n: I18NService,
    private cdr: ChangeDetectorRef,
    private messageService: MessageService,
    private dataMapService: DataMapService,
    private drawModalService: DrawModalService,
    private systemApiService: SystemApiService,
    private logManagerApiService: LogManagerApiService
  ) {}

  ngOnInit(): void {
    this.initForm();
    this.initColumns();
    this.getControl();
  }

  initColumns() {
    this.columns = [
      {
        label: this.i18n.get('common_name_label'),
        key: 'name',
        isShow: true
      },
      {
        label: this.i18n.get('common_role_label'),
        key: 'role',
        isShow: true,
        filter: true,
        filterMap: this.dataMapService.toArray('initRoleTable'),
        width: 120
      },
      {
        key: 'portType',
        label: this.i18n.get('common_ports_type_label'),
        isShow: true
      },
      {
        key: 'ip',
        label: this.i18n.get('common_ip_address_label'),
        isShow: true
      },
      {
        key: 'mask',
        label: this.i18n.get('common_subnet_mask_prefix_label'),
        isShow: true
      },
      {
        key: 'gateWay',
        label: this.i18n.get('common_gateway_label'),
        isShow: true
      },
      {
        key: 'vlan',
        label: this.i18n.get('common_vlan_use_label'),
        isShow: true
      },
      {
        key: 'route',
        label: this.i18n.get('common_route_config_label'),
        isShow: true
      },
      {
        key: 'mtu',
        label: this.i18n.get('common_mtu_label'),
        isShow: true
      },
      {
        key: 'ethernetPort',
        label: this.i18n.get('common_bounded_ethernet_port_label'),
        isShow: true,
        width: 240
      }
    ];
  }

  initForm() {
    this.filters = [
      {
        name: 'suffix',
        filterFn: (files: UploadFile[]) => {
          const supportSuffix = ['xls'];
          let validFiles = files.filter(file => {
            let suffix = file.name.split('.').pop();
            return supportSuffix.includes(suffix);
          });

          if (validFiles.length !== files.length) {
            this.message.error(
              this.i18n.get('common_format_error_label', ['xls']),
              {
                lvMessageKey: 'formatErrorKey',
                lvShowCloseButton: true
              }
            );
            this.selectFile = '';
            this.valid$.next(false);
            return;
          }
          if (files[0].size > 1024 * 1024 * 2) {
            this.message.error(
              this.i18n.get('common_max_size_file_label', ['2MB']),
              {
                lvMessageKey: 'maxSizeFileErrorKey',
                lvShowCloseButton: true
              }
            );
            this.selectFile = '';
            this.valid$.next(false);
            return;
          }
          this.valid$.next(true);
          return validFiles;
        }
      }
    ];
  }

  getControl() {
    this.logManagerApiService.collectNodeInfo({}).subscribe(
      res => {
        each(res.data, item => {
          this.controllers.push({
            controllerData: [],
            controllerName: item.nodeName,
            tableCols: cloneDeep(this.columns)
          });
        });
      },
      err => {}
    );
  }

  upload() {
    this.systemApiService
      .queryinitConfigInfoUSingPost({
        lld: this.selectFile
      })
      .subscribe(res => {
        this.rawData = res;
        this.fileReceive = true;
        this.onStatusChange.emit(true);
        this.clearData();
        if (!!res.archiveNetworkConfig) {
          this.parseData(res.archiveNetworkConfig);
        }
        if (!!res.backupNetworkConfig) {
          this.parseData(res.backupNetworkConfig);
        }
        if (!!res.copyNetworkConfig) {
          this.parseData(res.copyNetworkConfig);
        }
        this.nameChange();
      });
  }

  clearData() {
    each(this.controllers, control => {
      control.controllerData = [];
    });
  }

  parseData(item) {
    each(item.logicPorts, item => {
      each(this.controllers, control => {
        if (control.controllerName === item?.currentControllerId) {
          let backupData = filter(
            control.controllerData,
            val => val.role === item.role
          );
          let nameNum: any = size(backupData) + 1;
          if (nameNum < 10) {
            nameNum = `0${nameNum}`;
          }
          let typeName =
            item.role === DataMap.initRole.data.value
              ? 'Backup'
              : item.role === DataMap.initRole.copy.value
              ? 'Replication'
              : 'Archive';
          while (
            find(
              control.contollerData,
              item =>
                item.name === `${control.controllerName}-${typeName}${nameNum}`
            )
          ) {
            nameNum = Number(nameNum) + 1;
            if (nameNum < 10) {
              nameNum = `0${nameNum}`;
            }
          }
          control.controllerData.push(
            assign(item, {
              name: `${control.controllerName}-${typeName}${nameNum}`
            })
          );
        }
        control.controllerData = [...control.controllerData];
      });
    });
    this.cdr.detectChanges();
  }

  uploadChange(e) {
    if (e.activeFiles[0].size === 0) {
      this.messageService.error(result(e, 'file.error.message'));
    } else if (e.action === 'select') {
      this.selectFile = e.activeFiles[0].originFile;
    } else if (e.action === 'remove') {
      this.selectFile = '';
    } else {
    }
  }

  getLld() {
    const a = document.createElement('a');
    if (this.i18n.isEn) {
      a.href = 'assets/LLD_Design_En.xls';
    } else {
      a.href = 'assets/LLD_Design.xls';
    }
    a.download = 'LLD_Design.xls';
    a.click();
  }

  nameChange(e?) {
    let redFlag = false;
    each(this.controllers, control => {
      if (find(control.controllerData, item => !item.name)) {
        redFlag = true;
      }
    });
    this.onStatusChange.emit(redFlag);
  }

  getNetworkData(type, data) {
    each(this.controllers, control => {
      each(control.controllerData, item => {
        if (item?.role === type) {
          let tempPort = cloneDeep(item);
          data.push(tempPort);
        }
      });
    });
    return data;
  }

  getComponentData() {
    let backupPort = [];
    let archivePort = [];
    let replicatePort = [];
    backupPort = this.getNetworkData(DataMap.initRole.data.value, backupPort);
    archivePort = this.getNetworkData(
      DataMap.initRole.dataManage.value,
      archivePort
    );
    replicatePort = this.getNetworkData(
      DataMap.initRole.copy.value,
      replicatePort
    );
    const params = assign(this.componentData, {
      configLanguage: {
        language: this.i18n.language
      },
      backupNetworkConfig: {
        logicPorts: backupPort
      }
    });
    if (!!archivePort.length) {
      assign(params, {
        archiveNetworkConfig: {
          logicPorts: archivePort
        }
      });
    }
    if (!!replicatePort.length) {
      assign(params, {
        copyNetworkConfig: {
          logicPorts: replicatePort
        }
      });
    }
    return params;
  }
}
