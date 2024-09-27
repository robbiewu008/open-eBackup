import { Component, OnInit } from '@angular/core';
import { CommonConsts, I18NService } from 'app/shared';
import { RoleApiService } from 'app/shared/api/services';
import { map, includes } from 'lodash';

@Component({
  selector: 'cdm-associatedusers',
  templateUrl: './associatedusers.component.html'
})
export class AssociatedusersComponent implements OnInit {
  role;
  userData = [];
  total = 0;
  pageIndex = CommonConsts.PAGE_START;
  pageSize = CommonConsts.PAGE_SIZE;
  sizeOptions = CommonConsts.PAGE_SIZE_OPTIONS;

  constructor(
    public i18n: I18NService,
    public roleApiService: RoleApiService
  ) {}

  closeLabel = this.i18n.get('common_close_label');
  associatedUsersLabel = this.i18n.get('system_role_associatedusers_label');

  lvFooter = [
    {
      id: 'close',
      label: this.closeLabel,
      onClick: (modal, button) => {
        modal.close();
      }
    }
  ];

  ngOnInit() {
    this.initUser();
  }

  initUser() {
    this.roleApiService
      .getUserListByRoleIdUsingGET({
        id: this.role.roleId,
        startIndex: this.pageIndex + 1,
        pageSize: this.pageSize
      })
      .subscribe(r => {
        this.userData = map(r.userList, item => {
          item.description =
            item.defaultUser &&
            includes(
              ['sysadmin', 'mmdp_admin', 'mm_audit', 'cluster_admin'],
              item.description
            )
              ? this.i18n.get(`common_${item.description}_label`)
              : item.description;
          return item;
        });
        this.total = r.total;
      });
  }

  userPageChange(page) {
    this.pageSize = page.pageSize;
    this.pageIndex = page.pageIndex;
    this.initUser();
  }
}
