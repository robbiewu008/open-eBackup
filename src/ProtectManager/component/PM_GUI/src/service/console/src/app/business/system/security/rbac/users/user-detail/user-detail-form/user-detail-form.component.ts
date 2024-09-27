/*
 * This file is a part of the open-eBackup project.
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
 * If a copy of the MPL was not distributed with this file, You can obtain one at
 * http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) [2024] Huawei Technologies Co.,Ltd.
 *
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 */
import {
  AfterViewInit,
  Component,
  Input,
  OnInit,
  TemplateRef,
  ViewChild
} from '@angular/core';
import {
  CommonConsts,
  DataMap,
  DataMapService,
  I18NService,
  ResourceSetApiService,
  RoleApiService
} from 'app/shared';
import {
  Filters,
  ProTableComponent,
  TableCols,
  TableConfig,
  TableData
} from 'app/shared/components/pro-table';
import { assign, each, find, includes, trim } from 'lodash';

@Component({
  selector: 'aui-user-detail-form',
  templateUrl: './user-detail-form.component.html',
  styleUrls: ['./user-detail-form.component.less']
})
export class UserDetailFormComponent implements OnInit, AfterViewInit {
  // 从详情进入时，输入data
  @Input() data;
  // 从创建/修改用户进入时，输入以下三项
  @Input() roleList;
  @Input() resourceSetMap;
  @Input() formGroup;

  isOceanProtect = !includes(
    [
      DataMap.Deploy_Type.cloudbackup2.value,
      DataMap.Deploy_Type.cloudbackup.value,
      DataMap.Deploy_Type.hyperdetect.value,
      DataMap.Deploy_Type.cyberengine.value
    ],
    this.i18n.get('deploy_type')
  );
  dataMap = DataMap;

  roleData = {
    sourceData: [],
    tableData: {
      data: [],
      total: 0
    }
  };
  resourceData = {
    sourceData: [],
    tableData: {
      data: [],
      total: 0
    }
  };
  roleTableConfig: TableConfig;
  resourceTableConfig: TableConfig;

  @ViewChild('roleDataTable', { static: false })
  roleDataTable: ProTableComponent;
  @ViewChild('treeTableTpl', { static: true }) treeTableTpl: TemplateRef<any>;
  @ViewChild('resourceDataTable', { static: false })
  resourceDataTable: ProTableComponent;
  @ViewChild('roleNameTpl', { static: true }) roleNameTpl: TemplateRef<any>;

  constructor(
    public i18n: I18NService,
    public roleApiService: RoleApiService,
    public dataMapService: DataMapService,
    private resourceSetService: ResourceSetApiService
  ) {}

  ngOnInit() {
    this.initData();
    this.initRoleTable();
    this.initResourceTable();
  }

  ngAfterViewInit() {
    this.roleDataTable.fetchData();
    this.resourceDataTable.fetchData();
  }

  initRoleTable() {
    const cols: TableCols[] = [
      {
        key: 'roleName',
        name: this.i18n.get('common_name_label'),
        cellRender: this.roleNameTpl,
        filter: {
          type: 'search'
        }
      },
      {
        key: 'userNum',
        name: this.i18n.get('system_associated_users_num_label')
      },
      {
        key: 'roleDescription',
        name: this.i18n.get('common_desc_label')
      }
    ];

    this.roleTableConfig = {
      table: {
        compareWith: 'roleId',
        columns: cols,
        rows: {
          expandable: true,
          expandContent: this.treeTableTpl
        },
        fetchData: this.fetchTableData(this.roleData, 'roleName')
      }
    };
  }

  initResourceTable() {
    const cols: TableCols[] = [
      {
        key: 'name',
        name: this.i18n.get('common_resource_set_label'),
        filter: {
          type: 'search'
        }
      },
      {
        key: 'roleName',
        name: this.i18n.get('common_roles_label')
      },
      {
        key: 'description',
        name: this.i18n.get('common_desc_label')
      }
    ];
    this.resourceTableConfig = {
      table: {
        compareWith: 'uuid',
        columns: cols,
        fetchData: this.fetchTableData(this.resourceData, 'name')
      }
    };
  }

  /**
   * 使用闭包初始化并返回fetchData函数，适用于需要手动为proTable筛选、分页的情况
   * @param datas 绑定表格数据，包含源数据sourceData和表格数据tableData
   * @param filterName 筛选项名称 TODO：把此项改为对象数组，适配多种筛选、排序组合
   */
  fetchTableData(datas, filterName) {
    let cacheFilterVal = '';
    let displayData = [...datas.sourceData];
    return (filter: Filters, reInit?: boolean) => {
      const currentFilterVal =
        trim(JSON.parse(filter.conditions || '{}')[filterName]) || '';
      if (cacheFilterVal !== currentFilterVal || reInit) {
        cacheFilterVal = currentFilterVal;
        displayData = datas.sourceData.filter(item =>
          item[filterName].includes(cacheFilterVal)
        );
      }
      const { pageIndex, pageSize } = filter.paginator;
      datas.tableData = {
        data: displayData.slice(
          pageIndex * pageSize,
          (pageIndex + 1) * pageSize
        ),
        total: displayData.length
      };
    };
  }

  initData() {
    if (this.data) {
      assign(this.data, {
        roleId: this.data?.rolesSet[0]?.roleId
      });
      this.getRoleData();
      this.getResourceData();
    } else {
      this.data = this.formGroup.value;
      this.roleData.sourceData = this.roleList;
      this.resourceData.sourceData = this.getResourceSetList();
    }
  }

  /**
   * 由角色->资源集的映射建立资源集->角色的映射
   * @private
   */
  private getResourceSetList() {
    const resourceSetList = [];
    const roleNameMap = {};
    this.roleList.forEach(item => {
      roleNameMap[item.roleId] = item.roleName;
    });
    this.resourceSetMap.forEach((rsList, roleId) => {
      rsList.forEach(rsData => {
        const findData = find(resourceSetList, { uuid: rsData.uuid });
        if (findData) {
          findData.roleName.push(roleNameMap[roleId]);
        } else {
          const tmpData = { ...rsData };
          tmpData.roleName = [roleNameMap[roleId]];
          resourceSetList.push(tmpData);
        }
      });
    });
    return resourceSetList;
  }

  private getRoleData() {
    let pageNo = 0;
    const pageSize = CommonConsts.PAGE_SIZE_MAX;
    this.roleData.sourceData = [];
    let total = 1;
    while (pageSize * pageNo < total) {
      this.roleApiService
        .getUsingGET({
          pageNo: pageNo,
          pageSize: pageSize,
          conditions: JSON.stringify({
            userId: this.data.userId
          })
        })
        .subscribe(res => {
          this.roleDataProcess(res);
          total = res.totalCount;
          this.roleData.sourceData = this.roleData.sourceData.concat(
            res.records
          );
          this.roleDataTable.fetchData(true);
        });
      pageNo++;
    }
  }

  private roleDataProcess(res) {
    each(res.records, item => {
      if (item['is_default']) {
        item.roleName = this.dataMapService.getLabel(
          'defaultRoleName',
          item.roleName
        );
        item['roleDescription'] = this.dataMapService.getLabel(
          'defaultRoleDescription',
          item['roleDescription']
        );
      }
    });
  }

  private getResourceData() {
    this.resourceSetService
      .QueryResourceSetByUserId({
        userId: this.data.userId
      })
      .subscribe(res => {
        this.resourceData.sourceData = res.map(item => {
          const roleNameTransformed = this.dataMapService.getLabel(
            'defaultRoleName',
            item.roleName
          );
          return {
            uuid: item.resourceSetId,
            name: item.resourceSetName,
            description: item.resourceSetDescription,
            roleName:
              roleNameTransformed === '--' ? item.roleName : roleNameTransformed
          };
        });
        this.resourceDataTable.fetchData(true);
      });
  }
}
