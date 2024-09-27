import {
  ChangeDetectionStrategy,
  ChangeDetectorRef,
  Component,
  OnInit
} from '@angular/core';
import {
  DataMap,
  I18NService,
  ProtectedResourceApiService,
  extendSlaInfo
} from 'app/shared';
import { assign, each, isEmpty, size, isArray, find, isObject } from 'lodash';
import { Subject } from 'rxjs';
import { map } from 'rxjs/operators';

@Component({
  selector: 'aui-select-guassdb-t-list',
  templateUrl: './select-guassdb-t-list.component.html',
  styleUrls: ['./select-guassdb-t-list.component.less'],
  changeDetection: ChangeDetectionStrategy.OnPush
})
export class SelectGaussdbTListComponent implements OnInit {
  title = this.i18n.get('common_selected_label', ['GaussDB T']);
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
      key: 'version',
      name: this.i18n.get('common_version_label'),
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
  resourceData: any;
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
      subType: [
        DataMap.Resource_Type.GaussDB_T.value,
        DataMap.Resource_Type.gaussdbTSingle.value
      ]
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
            assign(item, {
              sub_type: item.subType,
              ip: item.extendInfo?.ip
            });
            extendSlaInfo(item);
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
    if (isArray(data)) {
      this.selectionData = data;
      const type = this.resourceData[0]?.subType;
      this.title = this.i18n.get('common_selected_label', [type]);
    } else if (isObject(this.resourceData)) {
      const type = (this.resourceData as any).subType;
      this.title = this.i18n.get('common_selected_label', [type]);
    }
    this.isNfsShareMode = isArray(data)
      ? find(data, item => item.extendInfo?.shareMode === '1')
      : data.extendInfo?.shareMode === '1';
  }

  onOK() {
    return { selectedList: this.selectionData };
  }
}
