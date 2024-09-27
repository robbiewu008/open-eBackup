import {
  AfterViewInit,
  ChangeDetectorRef,
  Component,
  Input,
  OnDestroy,
  OnInit,
  TemplateRef,
  ViewChild
} from '@angular/core';
import { FormBuilder, FormControl, FormGroup } from '@angular/forms';
import { Router } from '@angular/router';
import { MenuComponent, MessageboxService, MessageService } from '@iux/live';
import {
  BaseUtilService,
  ClustersApiService,
  CommonConsts,
  CookieService,
  DataMap,
  DataMapService,
  I18NService,
  SystemApiService,
  WarningMessageService
} from 'app/shared';
import { AppUtilsService } from 'app/shared/services/app-utils.service';
import {
  assign,
  each,
  find,
  get,
  includes,
  isFunction,
  set,
  size
} from 'lodash';
import { Subject, Subscription, timer } from 'rxjs';
import { switchMap, takeUntil } from 'rxjs/operators';
import { ConfiguringPortsComponent } from './configuring-ports/configuring-ports.component';
import { ManualConfigPortComponent } from './configuring-ports/manual-config-port/manual-config-port.component';
import { InitConfigProcessComponent } from './init-config-process/init-config-process.component';

@Component({
  selector: 'aui-init',
  templateUrl: './init.component.html',
  styleUrls: ['./init.component.less']
})
export class InitComponent implements OnInit, AfterViewInit, OnDestroy {
  @Input() isModify; // 用于网络配置的判断参数
  modifying = false; // 判断是否进入修改配置网络
  isEnglish = this.i18n.language !== 'zh-cn';
  nextBtnDisabled: boolean = true;
  activeIndex = 0;
  componentData = {};
  okBtnDisabled = true;
  isCertify = false;
  formGroup: FormGroup;
  dataMap = DataMap;
  selected = 0;
  truePassword;

  clusterMenus = [];
  nodeName = '';
  activeNode;
  timeSub$: Subscription;
  destroy$ = new Subject();
  timeSub3$: Subscription;
  destroy3$ = new Subject();
  roleList = [
    this.dataMap.Target_Cluster_Role.primaryNode.value,
    this.dataMap.Target_Cluster_Role.backupNode.value,
    this.dataMap.Target_Cluster_Role.memberNode.value
  ];

  cardData = [
    {
      label: this.i18n.get('common_manual_config_label'),
      content: this.i18n.get('common_manual_config_tip_label'),
      value: 0
    },
    {
      label: this.i18n.get('common_lld_config_label'),
      content: this.i18n.get('common_lld_config_tip_label'),
      value: 1
    }
  ];

  @ViewChild(InitConfigProcessComponent, { static: false })
  initConfigProcessComponent: InitConfigProcessComponent;
  @ViewChild(ConfiguringPortsComponent, { static: false })
  configuringPortsComponent: ConfiguringPortsComponent;
  @ViewChild(ManualConfigPortComponent, { static: false })
  manualConfigPortComponent: ManualConfigPortComponent;
  @ViewChild('messageTpl', { static: false }) messageTpl: TemplateRef<any>;
  @ViewChild('menu') menuInstance: MenuComponent;

  nameErrorTip = assign(this.baseUtilService.nameErrorTip, {
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [32])
  });
  pwdErrorTip = assign(this.baseUtilService.pwdErrorTip, {
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [32])
  });

  constructor(
    public appUtilsService: AppUtilsService,
    public baseUtilService: BaseUtilService,
    public clusterApiService: ClustersApiService,
    private router: Router,
    private fb: FormBuilder,
    private i18n: I18NService,
    private cdr: ChangeDetectorRef,
    private cookieSerive: CookieService,
    private messageBox: MessageboxService,
    private dataMapService: DataMapService,
    private messageService: MessageService,
    private systemApiService: SystemApiService,
    private warningMessageService: WarningMessageService
  ) {}

  ngOnInit() {
    this.initFormGroup();
    if (this.isModify) {
      // 如果进入修改后用户刷新界面，重新进入时要重新设定修改状态
      this.appUtilsService.setCacheValue(
        'networkModify',
        DataMap.networkModifyingStatus.normal.value
      );
      this.getClusterNodes();
    }
  }

  ngAfterViewInit() {
    if (!this.isModify) {
      this.getStatus();
    } else {
      this.getModifyStatus();
    }
  }

  ngOnDestroy() {
    this.appUtilsService.clearCacheValue('networkModify');
  }

  initFormGroup() {
    this.formGroup = this.fb.group({
      username: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.maxLength(32)
        ]
      }),
      password: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.maxLength(32)
        ]
      })
    });
  }

  modifyChange() {
    if (this.modifying) {
      this.warningMessageService.create({
        content: this.i18n.get('common_leave_network_config_tip_label'),
        onOK: () => {
          this.modifying = false;
          this.manualConfigPortComponent.modifyChange(this.modifying);
          this.appUtilsService.setCacheValue(
            'networkModify',
            DataMap.networkModifyingStatus.normal.value
          );
        }
      });
    } else {
      this.modifying = true;
      this.appUtilsService.setCacheValue(
        'networkModify',
        DataMap.networkModifyingStatus.modify.value
      );
      this.manualConfigPortComponent.modifyChange(this.modifying);
    }
  }

  certification() {
    if (!this.formGroup.valid) {
      return;
    }
    const ListAllPortUsingPOSTRequestBody = {
      password: this.formGroup.value.password,
      username: this.formGroup.value.username
    };
    this.systemApiService
      .checkAuth({
        storageAuth: ListAllPortUsingPOSTRequestBody,
        memberEsn: this.activeNode || ''
      })
      .subscribe((res: any) => {
        this.isCertify = true;
        this.formGroup.get('username').disable();
        this.formGroup.get('password').disable();
        this.nextBtnDisabled = false;
        set(this.componentData, 'storageAuth', ListAllPortUsingPOSTRequestBody);
        this.truePassword = this.formGroup.get('password').value;
        this.formGroup.get('password').setValue('*********');
      });
  }

  onResetChange(event) {
    if (event === 'modifyFail') {
      this.activeIndex = 0;
      this.manualComponentChange();
    } else if (event === 'modifyFinish') {
      this.activeIndex = 0;
      this.modifying = false;
      this.manualConfigPortComponent.modifyChange(this.modifying);
      this.appUtilsService.setCacheValue(
        'networkModify',
        DataMap.networkModifyingStatus.normal.value
      );
      this.manualComponentChange();
    } else {
      this.systemApiService.getInitConfigUsingGET({}).subscribe(res => {
        if (res.status === DataMap.System_Init_Status.no.value) {
          this.router.navigate(['/home']);
          return;
        }
        this.activeIndex = 0;
      });
    }
  }

  configPortStatusChange(e) {
    this.okBtnDisabled = e;
  }

  manualConfigPortStatusChange(e) {
    this.okBtnDisabled = e;
  }

  getClusterNodes(nodeName?) {
    const params = {
      startPage: CommonConsts.PAGE_START,
      pageSize: CommonConsts.PAGE_SIZE,
      clusterName: nodeName,
      roleList: this.roleList
    };

    if (this.timeSub3$) {
      this.timeSub3$.unsubscribe();
    }
    this.timeSub3$ = timer(0, CommonConsts.TIME_INTERVAL)
      .pipe(
        switchMap(index => {
          return this.clusterApiService.getClustersInfoUsingGET({
            ...params,
            akLoading: !index
          });
        }),
        takeUntil(this.destroy3$)
      )
      .subscribe(res => {
        if (!size(res)) {
          this.activeNode = null;
          this.clusterMenus = [];
          this.cdr.detectChanges();
          return;
        }
        const nodes = [];
        each(res.records, item => {
          nodes.push({
            ...item,
            id: item.storageEsn,
            label: item.clusterName,
            disabled: item.status !== DataMap.Cluster_Status.online.value
          });
        });
        this.clusterMenus = nodes;
        this.activeNode = this.activeNode || get(this.clusterMenus, '[0].id');
        this.cdr.detectChanges();
      });
  }

  getRoleLabel(item) {
    if (item === DataMap.Target_Cluster_Role.primaryNode.value) {
      return this.i18n.get('system_backup_cluster_primary_node_label');
    } else if (item === DataMap.Target_Cluster_Role.backupNode.value) {
      return this.i18n.get('system_backup_cluster_standby_node_label');
    } else {
      return this.i18n.get('system_backup_cluster_member_node_label');
    }
  }

  search() {
    this.getClusterNodes(this.nodeName);
  }

  refresh() {
    this.getClusterNodes(this.nodeName);
  }

  nodeChange(event) {
    if (this.activeNode === event.data.storageEsn) {
      return;
    }
    if (this.modifying) {
      this.warningMessageService.create({
        content: this.i18n.get('common_leave_network_config_tip_label'),
        onOK: () => {
          this.modifying = false;
          this.manualConfigPortComponent.modifyChange(this.modifying);
          this.appUtilsService.setCacheValue(
            'networkModify',
            DataMap.networkModifyingStatus.normal.value
          );
          this.activeNode = event.data.storageEsn;
          this.manualComponentChange();
        },
        onCancel: () => {
          this.menuInstance.lvActiveItemId = this.activeNode;
        },
        lvAfterClose: () => {
          this.menuInstance.lvActiveItemId = this.activeNode;
        }
      });
    } else {
      this.activeNode = event.data.storageEsn;
      this.getModifyStatus(() => {
        this.manualComponentChange();
      });
    }
  }

  manualComponentChange() {
    this.manualConfigPortComponent.memberEsn = this.activeNode;
    this.manualConfigPortComponent.getControl();
  }

  getCurrentNodeName() {
    return (
      find(this.clusterMenus, { storageEsn: this.activeNode })?.clusterName ||
      ''
    );
  }

  getStatus() {
    if (
      this.cookieSerive.initedStatus ===
      DataMap.System_Init_Status.running.value
    ) {
      this.activeIndex = 3;
      this.initConfigProcessComponent.getStatus();
    } else if (
      this.cookieSerive.initedStatus ===
      DataMap.System_Init_Status.archiveFailed.value
    ) {
      this.activeIndex = 3;
      this.initConfigProcessComponent.status =
        DataMap.System_Init_Status.archiveFailed.value;
      this.initConfigProcessComponent.result = {
        code: DataMap.System_Init_Status.archiveFailed.label,
        params: []
      };
    }
  }

  getModifyStatus(callback?) {
    this.systemApiService
      .getInitStatusInfoUsingGET({ memberEsn: this.activeNode || '' })
      .subscribe(res => {
        if (res.status === DataMap.networkModifyStatus.running.value) {
          this.modifying = true;
          this.appUtilsService.setCacheValue(
            'networkModify',
            DataMap.networkModifyingStatus.modifying.value
          );
          this.activeIndex = 3;
          this.initConfigProcessComponent.getStatus();
        } else {
          if (isFunction(callback)) {
            callback();
          }
        }
      });
  }

  clearComponentData() {
    for (let key in this.componentData) {
      if (key !== 'storageAuth') {
        delete this.componentData[key];
      }
    }
  }

  createInitConfig() {
    // 清理上一次的残留数据
    this.clearComponentData();
    if (this.selected === 1) {
      assign(
        this.componentData,
        this.configuringPortsComponent.getComponentData()
      );
    } else {
      assign(
        this.componentData,
        this.manualConfigPortComponent.getComponentData()
      );
    }

    if (this.isModify) {
      this.warningMessageService.create({
        content: this.i18n.get('common_network_config_warning_tip_label'),
        onOK: () => {
          this.systemApiService
            .createInitConfigUsingPOST({
              initNetworkBody: this.componentData,
              memberEsn: this.activeNode || ''
            })
            .subscribe({
              next: () => {
                this.appUtilsService.setCacheValue(
                  'networkModify',
                  DataMap.networkModifyingStatus.modifying.value
                );
                this.activeIndex = 3;
                this.initConfigProcessComponent.getStatus();
              },
              error: error => {}
            });
        }
      });
    } else {
      this.systemApiService
        .createInitConfigUsingPOST({
          initNetworkBody: this.componentData,
          akOperationTips: false
        })
        .subscribe(res => {
          if (res.status === DataMap.System_Init_Status.ok.value) {
            this.router.navigate(['/home']);
            return;
          }

          if (res.status === DataMap.System_Init_Status.running.value) {
            this.activeIndex = 3;
            this.initConfigProcessComponent.getStatus();
            return;
          }

          if (
            includes(
              [
                DataMap.System_Init_Status.archiveFailed.value,
                DataMap.System_Init_Status.backupFailed.value,
                DataMap.System_Init_Status.authFailed.value,
                DataMap.System_Init_Status.failed.value
              ],
              res.status
            )
          ) {
            this.messageService.error(
              this.i18n.get(
                this.dataMapService.getLabel('System_Init_Status', res.status)
              ),
              {
                lvMessageKey: 'system_Init_Status_key',
                lvShowCloseButton: true
              }
            );
            return;
          }
        });
    }
  }
}
