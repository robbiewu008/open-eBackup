import {
  AfterViewInit,
  Component,
  EventEmitter,
  OnInit,
  Output,
  ViewChild
} from '@angular/core';
import {
  CookieService,
  DataMap,
  DataMapService,
  getPermissionMenuItem,
  I18NService,
  MODAL_COMMON,
  OperateItems,
  RoleType,
  SecurityApiService,
  UsersApiService,
  WarningMessageService
} from 'app/shared';
import { ProButton } from 'app/shared/components/pro-button/interface';
import {
  Filters,
  ProTableComponent,
  TableCols
} from 'app/shared/components/pro-table';
import { BatchOperateService } from 'app/shared/services/batch-operate.service';
import { DrawModalService } from 'app/shared/services/draw-modal.service';
import { VirtualScrollService } from 'app/shared/services/virtual-scroll.service';
import { assign, extend, includes, isEmpty, map, size } from 'lodash';
import { SetEmailComponent } from '../../user-role/set-email/set-email.component';
import { UnlockComponent } from '../../user-role/unlock/unlock.component';
import { WarningComponent } from '../../user-role/user-role.component';
import { ResetpwdComponent } from './resetpwd/resetpwd.component';

@Component({
  selector: 'aui-users',
  templateUrl: './users.component.html',
  styleUrls: ['./users.component.less']
})
export class UsersComponent implements OnInit, AfterViewInit {
  optsConfig;
  tableConfig;
  tableData;
  selectionData = [];
  isSysAdmin = this.cookieService.role === RoleType.SysAdmin;

  @Output() openPage = new EventEmitter();

  @ViewChild('dataTable', { static: false }) dataTable: ProTableComponent;

  constructor(
    public i18n: I18NService,
    public drawModalService: DrawModalService,
    public usersApiService: UsersApiService,
    public virtualScroll: VirtualScrollService,
    public warningMessageService: WarningMessageService,
    public batchOperateService: BatchOperateService,
    private cookieService: CookieService,
    private securityApiService: SecurityApiService,
    private dataMapService: DataMapService
  ) {}

  ngOnInit() {
    this.initConfig();
  }

  ngAfterViewInit() {
    this.dataTable.fetchData();
  }

  initConfig() {
    const opts: ProButton[] = [
      {
        id: 'create',
        type: 'primary',
        permission: OperateItems.CreateUserComponent,
        label: this.i18n.get('common_create_label'),
        onClick: () => {
          this.create();
        }
      },
      {
        id: 'delete',
        label: this.i18n.get('common_delete_label'),
        permission: OperateItems.DeletingUser,
        onClick: data => {
          this.delete(data);
        },
        disableCheck: data => !data.length
      }
    ];
    this.optsConfig = getPermissionMenuItem(opts, this.cookieService.role);
    const itemOpts: ProButton[] = [
      {
        id: 'modify',
        label: this.i18n.get('common_modify_label'),
        permission: OperateItems.ModifyingUser,
        onClick: rowData => {
          this.create(rowData[0]);
        },
        disableCheck: rowData => rowData[0].defaultUser
      },
      {
        id: 'restPwd',
        label: this.i18n.get('system_user_restpwd_label'),
        permission: OperateItems.ResetPassword,
        disableCheck: rowData => {
          return (
            rowData[0].userId === this.cookieService.get('userId') ||
            includes(
              [
                DataMap.loginUserType.saml.value,
                DataMap.loginUserType.ldap.value,
                DataMap.loginUserType.ldapGroup.value,
                DataMap.loginUserType.hcs.value,
                DataMap.loginUserType.adfs.value
              ],
              rowData[0].userType
            )
          );
        },
        onClick: rowData => {
          this.restPwd(rowData);
        }
      },
      {
        id: 'setEmail',
        label: this.i18n.get('system_reset_password_email_settings_label'),
        disableCheck: rowData => {
          return (
            rowData[0].userId !== this.cookieService.get('userId') ||
            includes(
              [
                DataMap.loginUserType.saml.value,
                DataMap.loginUserType.ldap.value,
                DataMap.loginUserType.ldapGroup.value,
                DataMap.loginUserType.hcs.value,
                DataMap.loginUserType.adfs.value
              ],
              rowData[0].userType
            )
          );
        },
        displayCheck: () => {
          return !includes(
            [
              DataMap.Deploy_Type.cloudbackup.value,
              DataMap.Deploy_Type.cloudbackup2.value,
              DataMap.Deploy_Type.hyperdetect.value
            ],
            this.i18n.get('deploy_type')
          );
        },
        permission: OperateItems.SetRestPswdEmail,
        onClick: rowData => {
          this.setEmail(rowData);
        }
      },
      {
        id: 'lock',
        label: this.i18n.get('common_lock_label'),
        permission: OperateItems.LockingUser,
        disableCheck: rowData => {
          return (
            rowData[0].lock ||
            rowData[0].defaultUser ||
            rowData[0].userName === 'sysadmin' ||
            rowData[0].userId === this.cookieService.get('userId') ||
            includes(
              [
                DataMap.loginUserType.ldap.value,
                DataMap.loginUserType.ldapGroup.value,
                DataMap.loginUserType.hcs.value,
                DataMap.loginUserType.adfs.value
              ],
              rowData[0].userType
            )
          );
        },
        onClick: rowData => {
          this.warningMessageService.create({
            content: this.i18n.get('system_user_lock_tip_label', [
              rowData[0].userName
            ]),
            onOK: () => {
              const userId = rowData[0].userId;
              this.usersApiService
                .lockUsingPUT({ userId })
                .subscribe(() => this.dataTable.fetchData());
            }
          });
        }
      },
      {
        id: 'unLock',
        label: this.i18n.get('common_unlock_label'),
        permission: OperateItems.UnlockingUser,
        disableCheck: rowData => {
          return (
            !rowData[0].lock ||
            rowData[0].userId === this.cookieService.get('userId') ||
            includes(
              [
                DataMap.loginUserType.ldap.value,
                DataMap.loginUserType.ldapGroup.value,
                DataMap.loginUserType.hcs.value
              ],
              rowData[0].userType
            )
          );
        },
        onClick: rowData => {
          this.unLock(rowData);
        }
      },
      {
        id: 'delete',
        label: this.i18n.get('common_delete_label'),
        permission: OperateItems.DeletingUser,
        onClick: rowData => {
          this.delete(rowData);
        },
        disableCheck: ([data]) =>
          data.defaultUser ||
          data.userId === this.cookieService.get('userId') ||
          includes([DataMap.loginUserType.hcs.value], data.userType)
      }
    ];
    const cols: TableCols[] = [
      {
        key: 'userName',
        name: this.i18n.get('common_name_label'),
        width: '300px',
        sort: true,
        filter: {
          type: 'search',
          filterMode: 'contains'
        },
        cellRender: {
          type: 'text',
          config: {
            click: data => this.detail(data)
          }
        }
      },
      {
        key: 'roleName',
        name: this.i18n.get('common_roles_label'),
        width: '300px'
      },
      {
        key: 'login',
        name: this.i18n.get('common_status_label'),
        cellRender: {
          type: 'status',
          config: this.dataMapService.toArray('UserLoginStatusRBAC')
        },
        width: '160px'
      },
      {
        key: 'lock',
        name: this.i18n.get('common_lock_status_label'),
        cellRender: {
          type: 'status',
          config: this.dataMapService.toArray('USRE_LOCK')
        },
        width: '160px'
      },
      {
        key: 'description',
        name: this.i18n.get('common_desc_label')
      },
      {
        key: 'operation',
        name: this.i18n.get('common_operation_label'),
        hidden: !this.isSysAdmin,
        width: '160px',
        cellRender: {
          type: 'operation',
          config: {
            maxDisplayItems: 2,
            items: getPermissionMenuItem(itemOpts, this.cookieService.role)
          }
        }
      }
    ];
    this.tableConfig = {
      table: {
        compareWith: 'userId',
        columns: cols,
        rows: {
          selectionMode: 'multiple',
          selectionTrigger: 'selector',
          showSelector: this.isSysAdmin
        },
        fetchData: (filter: Filters) => {
          this.getData(filter);
        },
        selectionChange: selection => {
          this.selectionData = selection;
        }
      }
    };
  }

  drawModal(header, component, params) {
    this.drawModalService.create(
      assign({}, MODAL_COMMON.generateDrawerOptions(), {
        lvWidth: MODAL_COMMON.normalWidth,
        lvHeader: header,
        lvContent: component,
        lvComponentParams: params,
        lvOkDisabled: true,
        lvAfterOpen: modal => {
          const content = modal.getContentComponent();
          const modalIns = modal.getInstance();
          content.formGroup.statusChanges.subscribe(result => {
            modalIns.lvOkDisabled = result !== 'VALID';
          });
        },
        lvOk: modal => {
          return new Promise(resolve => {
            const content = modal.getContentComponent();
            content.onOK().subscribe({
              next: () => {
                resolve(true);
                this.dataTable.fetchData();
              },
              error: error => resolve(false)
            });
          });
        }
      })
    );
  }

  private unLock(rowData) {
    this.securityApiService.getUsingGET1({}).subscribe(res => {
      this.drawModal(
        this.i18n.get('system_permission_auth_label'),
        UnlockComponent,
        {
          user: extend({ ...rowData[0], ...res })
        }
      );
    });
  }

  private setEmail(rowData) {
    this.usersApiService
      .getEmailUsingGET({ userId: rowData[0].userId })
      .subscribe(res => {
        const emailAddress = res.emailAddress;
        this.drawModal(
          this.i18n.get('system_reset_password_email_settings_label'),
          SetEmailComponent,
          {
            userId: rowData[0].userId,
            emailAddress
          }
        );
      });
  }

  private restPwd(rowData) {
    this.securityApiService.getUsingGET1({}).subscribe(res => {
      this.drawModal(
        this.i18n.get('system_user_restpwd_label'),
        ResetpwdComponent,
        {
          user: extend({ ...rowData[0], ...res })
        }
      );
    });
  }

  create(data?) {
    this.openPage.emit({
      name: 'createUser',
      data: data
    });
  }

  delete(datas: any[]) {
    this.drawModalService.create({
      lvModalKey: 'warningMessage',
      lvType: 'dialog',
      lvDialogIcon: 'lv-icon-popup-danger-48',
      lvHeader: this.i18n.get('common_danger_label'),
      lvContent: WarningComponent,
      lvComponentParams: {
        content: this.i18n.get('system_user_delete_label', [
          map(datas, 'userName').join(this.i18n.get('common_comma_label'))
        ]),
        userRole: datas[0]?.rolesSet[0]?.roleName
      },
      lvWidth: MODAL_COMMON.normalWidth,
      lvOkType: 'primary',
      lvCancelType: 'default',
      lvOkDisabled: true,
      lvFocusButtonId: 'cancel',
      lvCloseButtonDisplay: true,
      lvAfterOpen: modal => {
        const component = modal.getContentComponent() as WarningComponent;
        const modalIns = modal.getInstance();
        component.isChecked$.subscribe(e => {
          modalIns.lvOkDisabled = !e;
        });
      },
      lvOk: modal => {
        this.deleteConfirmed(modal, datas);
      }
    });
  }

  private deleteConfirmed(modal, datas: any[]) {
    const component = modal.getContentComponent() as WarningComponent;
    if (datas.length > 1) {
      const deleteData = map(datas, item => {
        return {
          ...item,
          name: item.userName
        };
      });
      this.batchOperateService.selfGetResults(
        item => {
          return this.usersApiService.deleteUserUsingDELETE1({
            userId: item.userId,
            isCleanAllResources: component.deleteResource,
            akDoException: false,
            akOperationTips: false,
            akLoading: false
          });
        },
        deleteData,
        () => {
          this.dataTable.fetchData();
          this.selectionData = [];
          this.dataTable.setSelections([]);
        }
      );
    } else {
      const userId = datas[0]?.userId;
      this.usersApiService
        .deleteUserUsingDELETE1({
          userId,
          isCleanAllResources: component.deleteResource
        })
        .subscribe(() => {
          this.dataTable.fetchData();
          this.selectionData = [];
          this.dataTable.setSelections([]);
        });
    }
  }

  detail(rowData) {
    this.openPage.emit({
      name: 'userDetail',
      data: rowData
    });
  }

  getData(filter) {
    const conditions = !isEmpty(filter.conditions)
      ? JSON.parse(filter.conditions)
      : {};
    const params = {
      filter: JSON.stringify({
        isDefault: true,
        ...conditions
      }),
      startIndex: filter.paginator.pageIndex + 1,
      pageSize: filter.paginator.pageSize
    };
    if (!!size(filter.sort)) {
      assign(params, {
        orderBy: filter.sort.key,
        orderType: filter.sort.direction
      });
    }
    if (
      [RoleType.SysAdmin, RoleType.Auditor].includes(this.cookieService.role)
    ) {
      this.usersApiService.getAllUserUsingGET(params).subscribe(res => {
        res.userList.forEach(user => this.preProcess(user));
        this.tableData = {
          data: res.userList,
          total: res.total
        };
      });
    } else {
      this.initDataProtectUser();
    }
  }

  private preProcess(user) {
    assign(user, { roleName: user.rolesSet[0]?.roleName });
    const roleNameRes = this.dataMapService.getLabel(
      'defaultRoleName',
      user.roleName
    );
    user.roleName = roleNameRes === '--' ? user.roleName : roleNameRes;
    user.description =
      user.defaultUser &&
      includes(
        ['sysadmin', 'mmdp_admin', 'mm_audit', 'cluster_admin'],
        user.description
      )
        ? this.i18n.get(`common_${user.description}_label`)
        : user.description;
    assign(user, {
      disabled:
        user.defaultUser ||
        user.userId === this.cookieService.get('userId') ||
        includes([DataMap.loginUserType.hcs.value], user.userType)
    });
  }

  initDataProtectUser() {
    const userId = this.cookieService.get('userId');
    const currentCluster = this.cookieService.filterCluster
      ? this.cookieService.filterCluster
      : JSON.parse(
          decodeURIComponent(this.cookieService.get('currentCluster'))
        );

    const isLocal =
      isEmpty(currentCluster) ||
      (currentCluster['clusterId'] === DataMap.Cluster_Type.local.value &&
        currentCluster['clusterType'] === DataMap.Cluster_Type.local.value);
    if (isLocal) {
      this.usersApiService.getUsingGET2({ userId }).subscribe(res => {
        this.parseOtherUser(res);
      });
    } else {
      this.usersApiService.getLoginUserUsingGET({}).subscribe(res => {
        this.parseOtherUser(res);
      });
    }
  }

  private parseOtherUser(res: any) {
    this.preProcess(res);
    assign(res, {
      login: true
    });
    this.tableData = {
      data: [res],
      total: 1
    };
  }
}
