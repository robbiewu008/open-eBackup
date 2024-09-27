import { Component, OnInit } from '@angular/core';
import {
  CommonConsts,
  UsersApiService,
  DataMap,
  ProtectedResourceApiService
} from 'app/shared';
import { DataMapService, I18NService } from 'app/shared/services';
import { assign, isEmpty, each, toString, size, isNumber } from 'lodash';
import { ModalRef } from '@iux/live';
import { TableCols, TableConfig, TableData } from '../pro-table';

@Component({
  selector: 'aui-hcs-user-list',
  templateUrl: './hcs-user-list.component.html',
  styleUrls: ['./hcs-user-list.component.less']
})
export class HCSUserListComponent implements OnInit {
  proTableConfig: TableConfig;
  projectTableData;
  userTableConfig: TableConfig;
  userTableData: TableData;
  authorizeParams;
  startIndex = CommonConsts.PAGE_START;
  pageSize = CommonConsts.PAGE_SIZE;
  filterParams = { roleName: ['Role_DP_Admin'] };
  sortSources = {};
  projectSelect = [];
  userSelect = [];

  constructor(
    private i18n: I18NService,
    private dataMapService: DataMapService,
    private protectedResourceApiService: ProtectedResourceApiService,
    private usersApiService: UsersApiService,
    private modalRef: ModalRef
  ) {}

  ngOnInit() {
    this.initConfig();
    this.getProjects();
    this.getUsers();
  }

  initConfig() {
    const cols1: TableCols[] = [
      {
        key: 'name',
        name: this.i18n.get('common_name_label'),
        filter: {
          type: 'search',
          filterMode: 'contains'
        }
      },
      {
        key: 'owner',
        name: this.i18n.get('common_hcs_manager_label'),
        filter: {
          type: 'search',
          filterMode: 'contains'
        }
      }
    ];

    this.proTableConfig = {
      table: {
        async: false,
        columns: cols1,
        size: 'small',
        colDisplayControl: false,
        rows: {
          selectionMode: 'multiple',
          selectionTrigger: 'selector',
          showSelector: true
        },
        selectionChange: section => {
          this.projectSelect = section;
          if (this.authorizeParams.type) {
            this.modalRef.getInstance().lvOkDisabled =
              isEmpty(this.projectSelect) || isEmpty(this.userSelect);
          } else {
            this.modalRef.getInstance().lvOkDisabled = isEmpty(
              this.projectSelect
            );
          }
        }
      },
      pagination: {
        mode: 'simple',
        showPageSizeOptions: false,
        winTablePagination: true
      }
    };

    const cols2: TableCols[] = [
      {
        key: 'userName',
        name: this.i18n.get('common_name_label'),
        sort: true,
        filter: {
          type: 'search',
          filterMode: 'contains'
        }
      },
      {
        key: 'login',
        name: this.i18n.get('common_status_label'),
        filter: {
          type: 'select',
          isMultiple: true,
          showCheckAll: true,
          options: this.dataMapService.toArray('User_Login_Status')
        },
        cellRender: {
          type: 'status',
          config: this.dataMapService.toArray('User_Login_Status')
        }
      },
      {
        key: 'lock',
        name: this.i18n.get('common_lock_status_label'),
        filter: {
          type: 'select',
          isMultiple: true,
          showCheckAll: true,
          options: this.dataMapService.toArray('USRE_LOCK')
        },
        cellRender: {
          type: 'status',
          config: this.dataMapService.toArray('USRE_LOCK')
        }
      }
    ];
    this.userTableConfig = {
      table: {
        async: false,
        columns: cols2,
        size: 'small',
        colDisplayControl: false,
        rows: {
          selectionMode: 'single',
          selectionTrigger: 'selector',
          showSelector: true
        },
        selectionChange: section => {
          this.userSelect = section;
          if (this.authorizeParams.type) {
            this.modalRef.getInstance().lvOkDisabled =
              isEmpty(this.projectSelect) || isEmpty(this.userSelect);
          } else {
            this.modalRef.getInstance().lvOkDisabled = isEmpty(
              this.projectSelect
            );
          }
        }
      },
      pagination: {
        mode: 'simple',
        showPageSizeOptions: false,
        winTablePagination: true
      }
    };
  }

  getProjects(recordsTemp?: any[], startPage?: number) {
    const params = {
      pageNo: startPage || CommonConsts.PAGE_START,
      pageSize: CommonConsts.PAGE_SIZE,
      conditions: JSON.stringify({
        type: DataMap.Resource_Type.Project.value,
        path: [['=~'], this.authorizeParams.path + '/']
      })
    };
    this.protectedResourceApiService.ListResources(params).subscribe(res => {
      if (!recordsTemp) {
        recordsTemp = [];
      }
      if (!isNumber(startPage)) {
        startPage = CommonConsts.PAGE_START;
      }
      startPage++;
      recordsTemp = [...recordsTemp, ...res.records];
      if (
        startPage === Math.ceil(res.totalCount / CommonConsts.PAGE_SIZE) ||
        res.totalCount === 0
      ) {
        if (this.authorizeParams.type) {
          recordsTemp = recordsTemp.filter(item => !item.authorizedUser);
        } else {
          recordsTemp = recordsTemp.filter(item => !!item.authorizedUser);
        }
        each(recordsTemp, item => {
          const project = JSON.parse(item.extendInfo?.project);
          assign(item, { owner: project?.vdcInfo?.name });
        });
        this.projectTableData = {
          data: recordsTemp,
          total: size(recordsTemp)
        };

        return;
      }
      this.getProjects(recordsTemp, startPage);
    });
  }

  getUsers(recordsTemp?: any[], startPage?: number) {
    if (!isNumber(startPage)) {
      startPage = CommonConsts.PAGE_START_EXTRA;
    }
    const params = {
      startIndex: startPage,
      pageSize: CommonConsts.PAGE_SIZE_MAX,
      filter: JSON.stringify({ roleName: ['Role_DP_Admin'] })
    };
    this.usersApiService.getAllUserUsingGET(params).subscribe(res => {
      if (!recordsTemp) {
        recordsTemp = [];
      }
      recordsTemp = [...recordsTemp, ...res.userList];
      if (
        res.total === 0 ||
        startPage === Math.ceil(res.total / CommonConsts.PAGE_SIZE_MAX)
      ) {
        each(recordsTemp, (item: any) => {
          item.login = toString(item.login);
        });
        this.userTableData = {
          data: recordsTemp,
          total: res.total
        };
        return;
      }
      startPage++;
      this.getUsers(recordsTemp, startPage);
    });
  }
}
