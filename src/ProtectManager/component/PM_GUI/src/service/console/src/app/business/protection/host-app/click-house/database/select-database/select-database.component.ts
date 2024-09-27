import { ChangeDetectorRef, Component, OnInit } from '@angular/core';
import {
  DataMap,
  extendSlaInfo,
  I18NService,
  ProtectedResourceApiService,
  ResourceType
} from 'app/shared';
import { assign, each, get, isEmpty, size } from 'lodash';
import { Subject } from 'rxjs';
import { map } from 'rxjs/operators';

@Component({
  selector: 'app-select-database',
  templateUrl: './select-database.component.html',
  styles: []
})
export class SelectDatabaseComponent implements OnInit {
  title = this.i18n.get('common_selected_info_label', ['']);
  allTableData = {};
  columns = [
    {
      key: 'name',
      name: this.i18n.get('common_name_label'),
      filter: {
        type: 'search',
        filterMode: 'contains'
      }
    },
    {
      key: 'sla_name',
      name: this.i18n.get('common_sla_label')
    }
  ];
  isNfsShareMode = false;
  resourceData = [];
  selectionData = [];
  valid$ = new Subject<boolean>();

  constructor(
    private i18n: I18NService,
    private cdr: ChangeDetectorRef,
    private protectedResourceApiService: ProtectedResourceApiService
  ) {}

  ngOnInit() {}

  updateTable(filters) {
    const params = {
      pageNo: filters.paginator.pageIndex,
      pageSize: filters.paginator.pageSize
    };

    const defaultConditions = {
      subType: [DataMap.Resource_Type.ClickHouse.value],
      type: ResourceType.DATABASE
    };

    if (!isEmpty(filters.conditions_v2)) {
      assign(defaultConditions, JSON.parse(filters.conditions_v2));
    }

    assign(params, { conditions: JSON.stringify(defaultConditions) });

    if (!!size(filters.sort)) {
      assign(params, { orders: filters.orders });
    }

    this.protectedResourceApiService
      .ListResources(params)
      .pipe(
        map(res => {
          each(res.records, item => {
            extendSlaInfo(item);
            assign(item, {
              sub_type: item.subType,
              disabled: !!get(item, 'sla_id')
            });
          });
          return res;
        })
      )
      .subscribe(res => {
        this.allTableData = {
          total: res.totalCount,
          data: res.records
        };
        this.cdr.detectChanges();
      });
  }

  dataChange(selection) {
    this.selectionData = selection;
    this.valid$.next(!!size(this.selectionData));
  }

  initData(data: any) {
    this.resourceData = data;
    this.selectionData = data;
  }

  onOK() {
    return { selectedList: this.selectionData };
  }
}
