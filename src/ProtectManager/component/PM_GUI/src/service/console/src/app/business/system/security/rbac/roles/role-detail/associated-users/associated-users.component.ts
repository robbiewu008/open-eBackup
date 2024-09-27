import {
  AfterViewInit,
  Component,
  Input,
  OnInit,
  ViewChild
} from '@angular/core';
import {
  Filters,
  ProTableComponent,
  TableCols
} from 'app/shared/components/pro-table';
import { I18NService, UsersApiService } from 'app/shared';
import { assign, isEmpty, size } from 'lodash';

@Component({
  selector: 'aui-associated-users',
  templateUrl: './associated-users.component.html',
  styleUrls: ['./associated-users.component.less']
})
export class AssociatedUsersComponent implements OnInit, AfterViewInit {
  @Input() data;
  tableConfig;
  tableData;

  @ViewChild('dataTable', { static: false }) dataTable: ProTableComponent;

  constructor(
    public i18n: I18NService,
    private usersApiService: UsersApiService
  ) {}

  ngOnInit() {
    this.initConfig();
  }

  ngAfterViewInit() {
    this.dataTable.fetchData();
  }

  initConfig() {
    const cols: TableCols[] = [
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
        key: 'description',
        name: this.i18n.get('common_desc_label')
      }
    ];
    this.tableConfig = {
      table: {
        compareWith: 'userId',
        columns: cols,
        fetchData: (filter: Filters) => {
          this.getData(filter);
        }
      }
    };
  }

  getData(filter) {
    const conditions = !isEmpty(filter.conditions)
      ? JSON.parse(filter.conditions)
      : {};
    const params = {
      filter: JSON.stringify({
        roleId: this.data.roleId,
        ...conditions
      }),
      startIndex: filter.paginator.pageIndex,
      pageSize: filter.paginator.pageSize
    };
    if (!!size(filter.sort)) {
      assign(params, {
        orderBy: filter.sort.key,
        orderType: filter.sort.direction
      });
    }
    this.usersApiService.getAllUserUsingGET(params).subscribe(res => {
      this.tableData = {
        data: res.userList,
        total: res.total
      };
    });
  }
}
