import { Component, Input, OnInit } from '@angular/core';
import { I18NService, PermissionTable, RoleAuthApiService } from 'app/shared';
import { AppUtilsService } from 'app/shared/services/app-utils.service';
import { cloneDeep } from 'lodash';

@Component({
  selector: 'aui-role-auth-tree',
  templateUrl: './role-auth-tree.component.html',
  styleUrls: ['./role-auth-tree.component.less']
})
export class RoleAuthTreeComponent implements OnInit {
  @Input() data;
  tableData = [];

  constructor(
    public i18n: I18NService,
    private appUtilsService: AppUtilsService,
    private roleAuthApiService: RoleAuthApiService
  ) {}

  ngOnInit() {
    this.getData();
  }

  getData() {
    this.roleAuthApiService
      .getAuthListByRoleIdUsingGet({
        id: this.data.roleId
      })
      .subscribe(res => {
        const authSet = new Set(res.map(item => item.uuid));
        const permissionTable = cloneDeep(PermissionTable).filter(
          item =>
            !(
              this.appUtilsService.isDistributed &&
              item.value === 'DataSecurity'
            )
        );
        permissionTable.forEach(
          item =>
            (item.children = item.children.filter(subItem =>
              authSet.has(subItem.value)
            ))
        );
        this.tableData = permissionTable.filter(item => item.children.length);
      });
  }
}
