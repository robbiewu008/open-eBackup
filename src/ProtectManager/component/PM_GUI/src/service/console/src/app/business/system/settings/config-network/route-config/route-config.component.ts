import { Component, EventEmitter, Input, OnInit, Output } from '@angular/core';
import { FormArray, FormBuilder, FormControl, FormGroup } from '@angular/forms';
import {
  BaseUtilService,
  CommonConsts,
  DataMap,
  DataMapService,
  I18NService,
  SystemApiService,
  WarningMessageService
} from 'app/shared';
import { assign, each, map } from 'lodash';

@Component({
  selector: 'aui-route-config',
  templateUrl: './route-config.component.html',
  styleUrls: ['./route-config.component.less']
})
export class RouteConfigComponent implements OnInit {
  @Input() isNetwork = false;
  @Input() ipType;
  @Input() routeData;
  @Output() routeStatus = new EventEmitter<any>();
  data;
  componentData;
  memberEsn;
  formGroup: FormGroup;
  pageIndex = CommonConsts.PAGE_START;
  pageSize = CommonConsts.PAGE_SIZE;
  tableData = [];
  routeOptions = this.dataMapService.toArray('initRouteType');
  isIpv4;

  prefixErrorTip = assign(
    {},
    this.baseUtilService.rangeErrorTip,
    {
      invalidRang: this.i18n.get('common_valid_rang_label', [1, 128])
    },
    this.baseUtilService.requiredErrorTip,
    {
      invalidName: this.i18n.get('common_invalid_inputtext_label')
    }
  );

  constructor(
    public baseUtilService: BaseUtilService,
    private i18n: I18NService,
    private fb: FormBuilder,
    private dataMapService: DataMapService,
    private systemApiService: SystemApiService,
    private warningMessageService: WarningMessageService
  ) {}

  ngOnInit(): void {
    if (!this.isNetwork) {
      this.getRoute();
      this.initConfig();
    } else {
      this.isIpv4 = this.ipType === '0';
    }
    this.initForm();
  }

  initConfig() {
    this.isIpv4 = this.data.ipType === DataMap.IP_Type.ipv4.value;
    if (!!this.data?.gateWay) {
      this.routeOptions = this.routeOptions.map(item => {
        return {
          ...item,
          disabled: item.value === DataMap.initRouteType.default.value
        };
      });
    }
  }

  initRoute() {
    if (!!this.routeData.length) {
      each(this.routeData, item => {
        const routeItem = this.getTargetRouteFormGroup() as FormGroup;
        routeItem.patchValue({
          gateWay: item.gateway,
          targetAddress: item.destination,
          mask: item.mask,
          type: item.routeType
        });
        if (item.routeType === DataMap.initRouteType.default.value) {
          routeItem.get('mask').clearValidators();
          routeItem.get('targetAddress').clearValidators();
          routeItem.get('mask').updateValueAndValidity();
          routeItem.get('targetAddress').updateValueAndValidity();
        }
        this.listenItem(routeItem);
        routeItem.statusChanges.subscribe(res => {
          this.routeStatus.emit(this.targetRoute);
        });
        (this.formGroup.get('targetRoute') as FormArray).push(routeItem);
      });
    }
  }

  getRoute() {
    this.systemApiService
      .getPortRoutes({
        portName: this.data.name,
        akOperationTips: false,
        memberEsn: this.memberEsn || ''
      })
      .subscribe(res => {
        each(res, item => {
          const routeItem = this.getTargetRouteFormGroup() as FormGroup;
          routeItem.patchValue({
            gateWay: item.GATEWAY,
            targetAddress: item.DESTINATION,
            mask: item.MASK,
            type: item.TYPE,
            isExist: true
          });
          (this.formGroup.get('targetRoute') as FormArray).push(routeItem);
        });
      });
  }

  initForm() {
    this.formGroup = this.fb.group({
      targetRoute: this.fb.array([])
    });
    if (this.isNetwork) {
      this.initRoute();
    }
  }

  get targetRoute() {
    return (this.formGroup.get('targetRoute') as FormArray).controls;
  }

  getTargetRouteFormGroup() {
    return this.fb.group({
      type: new FormControl(
        !!this.data?.gateWay
          ? DataMap.initRouteType.client.value
          : DataMap.initRouteType.default.value,
        {
          validators: this.baseUtilService.VALID.required()
        }
      ),
      targetAddress: new FormControl(this.isIpv4 ? '0.0.0.0' : '::', {
        validators: [
          this.baseUtilService.VALID.required(),
          this.isIpv4
            ? this.baseUtilService.VALID._ipv4()
            : this.baseUtilService.VALID._ipv6()
        ]
      }),
      mask: new FormControl(this.isIpv4 ? '0.0.0.0' : '0', {
        validators: [
          this.baseUtilService.VALID.required(),
          this.isIpv4
            ? this.baseUtilService.VALID.name(CommonConsts.REGEX.mask)
            : this.baseUtilService.VALID.rangeValue(1, 128)
        ]
      }),
      gateWay: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.required(),
          this.isIpv4
            ? this.baseUtilService.VALID._ipv4()
            : this.baseUtilService.VALID._ipv6()
        ]
      }),
      isExist: new FormControl(false)
    });
  }

  ipTypeChange() {
    each(this.targetRoute, item => {
      item.get('type').setValue(item.value.type);
      item
        .get('gateWay')
        .setValidators([
          this.baseUtilService.VALID.required(),
          this.isIpv4
            ? this.baseUtilService.VALID._ipv4()
            : this.baseUtilService.VALID._ipv6()
        ]);
      item.get('gateWay').updateValueAndValidity();
    });
  }

  pageChange(page) {
    this.pageSize = page.pageSize;
    this.pageIndex = page.pageIndex;
  }

  add() {
    const routeItem = this.getTargetRouteFormGroup() as FormGroup;
    this.listenItem(routeItem);
    routeItem.get('type').updateValueAndValidity();
    (this.formGroup.get('targetRoute') as FormArray).push(routeItem);
    if (this.isNetwork) {
      this.routeStatus.emit(this.targetRoute);
      routeItem.statusChanges.subscribe(res => {
        this.routeStatus.emit(this.targetRoute);
      });
    }
  }

  listenItem(routeItem) {
    routeItem.get('type').valueChanges.subscribe(res => {
      const targetAddressValidators = [
        this.baseUtilService.VALID.required(),
        this.isIpv4
          ? this.baseUtilService.VALID._ipv4()
          : this.baseUtilService.VALID._ipv6()
      ];
      const maskValidators = [
        this.baseUtilService.VALID.required(),
        this.isIpv4
          ? this.baseUtilService.VALID.name(CommonConsts.REGEX.mask)
          : this.baseUtilService.VALID.rangeValue(1, 128)
      ];
      if (res === DataMap.initRouteType.network.value) {
        routeItem.get('mask').setValue('');
        routeItem.get('targetAddress').setValue('');
      } else if (res === DataMap.initRouteType.client.value) {
        routeItem.get('mask').setValue(this.isIpv4 ? '255.255.255.255' : '128');
        routeItem.get('targetAddress').setValue('');
      } else if (res === DataMap.initRouteType.default.value) {
        routeItem.get('mask').setValue(this.isIpv4 ? '0.0.0.0' : '0');
        routeItem.get('targetAddress').setValue(this.isIpv4 ? '0.0.0.0' : '::');
        targetAddressValidators.pop();
        maskValidators.pop();
      }
      routeItem.get('mask').setValidators(maskValidators);
      routeItem.get('mask').updateValueAndValidity();
      routeItem.get('targetAddress').setValidators(targetAddressValidators);
      routeItem.get('targetAddress').updateValueAndValidity();
    });
  }

  getParams(item) {
    const value = item.value;
    const params = {
      portName: this.data.name,
      route: {
        DESTINATION: value.targetAddress,
        GATEWAY: value.gateWay,
        MASK: value.mask,
        TYPE: value.type
      }
    };
    return params;
  }

  getTargetParams() {
    // 用于添加网络平面时批量处理数据
    return !!this.targetRoute.length
      ? map(this.targetRoute, item => ({
          destination: item.value.targetAddress,
          gateway: item.value.gateWay,
          mask: item.value.mask,
          routeType: item.value.type
        }))
      : [];
  }

  remove(i) {
    const row = (this.formGroup.get('targetRoute') as FormArray).at(i);
    if (row.value.isExist) {
      this.warningMessageService.create({
        content: this.i18n.get('common_delete_route_warning_tip_label', [
          this.data.name
        ]),
        onOK: () => {
          this.systemApiService
            .deletePortRoute({
              modifyLogicPortRouteRequest: this.getParams(row),
              memberEsn: this.memberEsn || ''
            })
            .subscribe(res => {
              (this.formGroup.get('targetRoute') as FormArray).removeAt(i);
            });
        }
      });
    } else {
      (this.formGroup.get('targetRoute') as FormArray).removeAt(i);
      if (this.isNetwork) {
        this.routeStatus.emit(this.targetRoute);
      }
    }
  }

  confirm(item) {
    if (!item.valid) {
      return;
    }
    this.warningMessageService.create({
      content: this.i18n.get('common_add_route_waring_tip_label', [
        this.data.name
      ]),
      onOK: () => {
        this.systemApiService
          .addPortRoute({
            modifyLogicPortRouteRequest: this.getParams(item),
            memberEsn: this.memberEsn || ''
          })
          .subscribe(res => {
            item.get('isExist').setValue(true);
          });
      }
    });
  }
}
