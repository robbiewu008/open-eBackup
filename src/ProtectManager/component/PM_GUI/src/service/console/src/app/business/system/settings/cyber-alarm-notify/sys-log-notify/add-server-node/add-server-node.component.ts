import { Component } from '@angular/core';
import {
  AbstractControl,
  FormBuilder,
  FormGroup,
  ValidatorFn
} from '@angular/forms';
import {
  BaseUtilService,
  ComponentRestApiService,
  DataMap,
  DataMapService,
  I18NService,
  SyslogApiService
} from 'app/shared';
import { assign, isEmpty, map, set, trim } from 'lodash';
import { Observable, Observer } from 'rxjs';

@Component({
  selector: 'aui-add-server-node',
  templateUrl: './add-server-node.component.html',
  styleUrls: ['./add-server-node.component.less']
})
export class AddServerNodeComponent {
  rowData;
  ipSet: Set<string>;
  formGroup: FormGroup;
  certificationOptions = [];
  isHyper =
    this.i18n.get('deploy_type') === DataMap.Deploy_Type.hyperdetect.value;
  protocolTypes = this.dataMapService.toArray('AlarmProtocolType');
  certRequired = true; // 动态控制证书是否必填
  ipErrorTip = {
    ...this.baseUtilService.ipErrorTip,
    repeatIp: this.i18n.get('common_create_logical_port_same_ip_label')
  };
  portErrorTip = {
    ...this.baseUtilService.rangeErrorTip,
    invalidRang: this.i18n.get('common_valid_rang_label', [1, 65535])
  };
  protocolTipsLabel = '';

  constructor(
    private fb: FormBuilder,
    private i18n: I18NService,
    private dataMapService: DataMapService,
    public baseUtilService: BaseUtilService,
    private syslogApiService: SyslogApiService,
    private certApiService: ComponentRestApiService
  ) {}

  ngOnInit() {
    this.initForm();
    this.getCertificates();
  }

  initForm() {
    this.formGroup = this.fb.group({
      ip: [
        '',
        [
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.ipv4(),
          this.validRepeatIp()
        ]
      ],
      port: [
        514,
        [
          this.baseUtilService.VALID.integer(),
          this.baseUtilService.VALID.rangeValue(1, 65535)
        ]
      ],
      protocol: [
        DataMap.AlarmProtocolType.TCPAndSSL.value,
        [this.baseUtilService.VALID.required()]
      ],
      cert: ['', [this.baseUtilService.VALID.required()]]
    });
    this.listenChange();
    if (this.rowData) {
      this.formGroup.patchValue({
        ip: this.rowData?.ip,
        port: this.rowData?.port,
        protocol: this.rowData?.protocol,
        cert: this.rowData?.component_id.split('/')[1] || ''
      });
      this.ipSet.delete(this.rowData.ip);
    }
  }

  validRepeatIp(): ValidatorFn {
    return (control: AbstractControl): { [key: string]: any } | null => {
      if (!trim(control.value)) {
        return null;
      }
      if (this.ipSet.has(control.value)) {
        return { repeatIp: { value: control.value } };
      }
      return null;
    };
  }

  listenChange() {
    this.formGroup.get('protocol').valueChanges.subscribe(res => {
      const formItem: AbstractControl = this.formGroup.get('cert');
      formItem.setValue('');
      if (res === DataMap.AlarmProtocolType.TCPAndSSL.value) {
        formItem.setValidators([this.baseUtilService.VALID.required()]);
        this.certRequired = true;
      } else {
        formItem.clearValidators();
        this.certRequired = false;
        this.protocolTipsLabel = this.i18n.get('system_syslog_protocol_label', [
          this.dataMapService.getLabel('AlarmProtocolType', res)
        ]);
      }
      formItem.updateValueAndValidity();
    });
  }

  getCertificates() {
    this.certApiService.queryComponentsUsingGET({}).subscribe(result => {
      const res = result.filter(
        item => item.type === DataMap.Component_Type.syslog.value
      );
      this.certificationOptions = map(res, item => {
        return assign(item, {
          label: item.name,
          value: item.componentId,
          isLeaf: true
        });
      });
    });
  }

  getParams() {
    const { ip, port, protocol, cert } = this.formGroup.value;
    return {
      syslogIpParam: {
        ip,
        port,
        protocol,
        component_id: cert,
        is_test_server: false
      }
    };
  }

  modifyNode(params) {
    return this.syslogApiService.modifySyslogIpUsingPUT(params);
  }

  createNode(params) {
    return this.syslogApiService.addSyslogIpUsingPost(params);
  }

  onOk(): Observable<void> {
    return new Observable((observer: Observer<void>) => {
      const params = this.getParams();
      let observable: Observable<string>;
      if (!isEmpty(this.rowData)) {
        set(params.syslogIpParam, 'id', this.rowData.id);
        observable = this.modifyNode(params);
      } else {
        observable = this.createNode(params);
      }
      observable.subscribe({
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
