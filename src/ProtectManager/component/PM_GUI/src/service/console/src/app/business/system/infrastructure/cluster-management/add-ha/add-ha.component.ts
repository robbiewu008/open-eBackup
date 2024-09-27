import { Component, OnInit, ViewChild } from '@angular/core';
import {
  FormBuilder,
  FormGroup,
  FormControl,
  ValidatorFn,
  AbstractControl,
  FormArray
} from '@angular/forms';
import {
  BackupClustersHaApiService,
  BaseUtilService,
  CommonConsts,
  I18NService
} from 'app/shared';
import {
  ProTableComponent,
  TableCols,
  TableConfig,
  TableData
} from 'app/shared/components/pro-table';
import { cloneDeep, defer, get, trim } from 'lodash';
import { Observable, Observer } from 'rxjs';

@Component({
  selector: 'aui-add-ha',
  templateUrl: './add-ha.component.html',
  styleUrls: ['./add-ha.component.less']
})
export class AddHaComponent implements OnInit {
  @ViewChild('dataTable', { static: false }) dataTable: ProTableComponent;
  drawData: any;
  isEdit: boolean;
  constructor(
    public fb: FormBuilder,
    private i18n: I18NService,
    public baseUtilService: BaseUtilService,
    public haApiService: BackupClustersHaApiService
  ) {}

  addForm: FormGroup;
  baseInfoForm: FormGroup;
  haData: any[];
  tableData: TableData;
  tableConfig: TableConfig;
  primaryInfo = {
    name: '',
    status: 0,
    netMask: '',
    gaussIp: '',
    infraIp: ''
  };
  standByInfo = {
    name: '',
    status: 0,
    netMask: '',
    gaussIp: '',
    infraIp: ''
  };

  gatewayErrorTip = {
    invalidStart: this.i18n.get('system_ha_gateway_error_tips_label'),
    invalidSameName: this.i18n.get('system_same_gateway_tip_label'),
    ...this.baseUtilService.ipErrorTip
  };

  ngOnInit(): void {
    this.initForm();
    this.initData();
    this.initConfig();
    if (this.isEdit) {
      const arr = cloneDeep(this.drawData.gatewayIpList).slice(1);
      for (let item of arr) {
        this.addGateway(item);
      }
    }
  }

  initForm() {
    this.addForm = this.fb.group({
      ip: new FormControl(get(this.drawData, 'floatIpAddress', ''), {
        validators: [
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.ip()
        ],
        updateOn: 'change'
      }),
      gatewayArr: this.fb.array([
        this.getGatewayArrFormGroup(get(this.drawData, 'gatewayIpList[0]', ''))
      ])
    });
  }

  validGateway(ipv4Re?: RegExp, ipv6Re?: RegExp): ValidatorFn {
    return (control: AbstractControl): { [key: string]: any } | null => {
      const value = trim(control.value);
      if (!value) {
        return null;
      }
      if (!ipv4Re) {
        ipv4Re = CommonConsts.REGEX.ipv4;
      }
      if (!ipv6Re) {
        ipv6Re = CommonConsts.REGEX.ipv6;
      }

      if (ipv4Re.test(value) || ipv6Re.test(value)) {
        // 不能够以127开头
        const firstIp = value.split('.')[0];
        if (firstIp !== '127') {
          return null;
        } else {
          return { invalidStart: { value: value } };
        }
      }
      return { invalidName: { value: value } };
    };
  }

  validSameGateway() {
    return (
      control: AbstractControl
    ): Promise<{ [key: string]: any } | null> => {
      return new Promise(resolve => {
        defer(() => {
          if (!trim(control.value)) {
            resolve(null);
          }
          const arr = this.addForm.value.gatewayArr;
          if (arr.length === 1) resolve(null);
          let num = 0;
          for (let i = 0; i < arr.length; i++) {
            if (trim(arr[i].gateway) === trim(control.value)) {
              num++;
            }
          }

          if (num > 1) {
            resolve({ invalidSameName: { value: trim(control.value) } });
          }
          resolve(null);
        });
      });
    };
  }

  get gatewayArr() {
    return (this.addForm.get('gatewayArr') as FormArray).controls;
  }

  getGatewayArrFormGroup(item) {
    return this.fb.group({
      gateway: new FormControl(item || '', {
        validators: [
          this.baseUtilService.VALID.required(),
          this.validGateway()
        ],
        asyncValidators: this.validSameGateway(),
        updateOn: 'change'
      })
    });
  }

  addGateway(item?) {
    (this.addForm.get('gatewayArr') as FormArray).push(
      this.getGatewayArrFormGroup(item)
    );
  }

  removeRow(i) {
    (this.addForm.get('gatewayArr') as FormArray).removeAt(i);
  }

  initData() {
    const clusters = this.drawData.clusters;
    this.haData = clusters.filter(
      v => v.status === 27 && v.backupRoleType === 'MEMBER'
    );
    this.primaryInfo = clusters.filter(
      item => item.backupRoleType === 'PRIMARY'
    )[0];
    if (this.isEdit) {
      this.standByInfo = clusters.filter(
        v => v.backupRoleType === 'STANDBY'
      )[0];
    } else {
      this.standByInfo = this.haData[0];
      defer(() => {
        this.dataTable.setSelections([this.haData[0]]);
      });
    }
  }

  initConfig() {
    const cols: TableCols[] = [
      {
        key: 'name',
        name: this.i18n.get('common_name_label'),
        filter: {
          type: 'search',
          filterMode: 'contains'
        }
      },
      {
        key: 'gaussIp',
        name: this.i18n.get('system_network_plane_ip1_label')
      },
      {
        key: 'infraIp',
        name: this.i18n.get('system_network_plane_ip2_label')
      },
      {
        key: 'netMask',
        name: this.i18n.get('common_mask_ip_label')
      }
    ];
    this.tableConfig = {
      pagination: {
        mode: 'simple',
        showPageSizeOptions: false,
        winTablePagination: true
      },
      table: {
        async: false,
        columns: cols,
        scroll: { y: '192px' },
        rows: {
          selectionMode: 'single',
          selectionTrigger: 'selector',
          showSelector: true,
          keepRadioLogic: true
        },
        colDisplayControl: false,
        selectionChange: val => {
          this.standByInfo = val[0];
        }
      }
    };
    this.tableData = {
      data: this.haData,
      total: this.haData.length
    };
  }

  getParams() {
    let arr = [];
    this.addForm.value.gatewayArr.forEach(item => {
      arr.push(trim(item.gateway));
    });
    const params = {
      clusterEsn: this.isEdit ? '' : this.dataTable.getAllSelections()[0].esn,
      floatIpAddress: trim(this.addForm.value.ip),
      gatewayIpList: arr
    };
    return params;
  }

  saveHaInfo(): Observable<void> {
    if (this.addForm.invalid) return;
    const params = this.getParams();
    if (this.isEdit) {
      return new Observable<void>((observer: Observer<void>) => {
        this.haApiService.updateHaConfig({ request: params }).subscribe({
          next: () => {
            observer.next();
            observer.complete();
          },
          error: error => {
            observer.error(error);
            observer.complete();
          }
        });
      });
    } else {
      return new Observable<void>((observer: Observer<void>) => {
        this.haApiService.addHaConfig({ request: params }).subscribe({
          next: () => {
            observer.next();
            observer.complete();
          },
          error: error => {
            observer.error(error);
            observer.complete();
          }
        });
      });
    }
  }
}
