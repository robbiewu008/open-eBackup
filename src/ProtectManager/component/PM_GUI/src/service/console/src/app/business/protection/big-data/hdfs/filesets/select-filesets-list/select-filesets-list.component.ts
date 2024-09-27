import { Component, OnInit, ChangeDetectorRef } from '@angular/core';
import { Subject } from 'rxjs';
import { size, isEmpty, assign, map } from 'lodash';
import { I18NService, DataMap, ProtectedResourceApiService } from 'app/shared';

@Component({
  selector: 'aui-select-filesets-list',
  templateUrl: './select-filesets-list.component.html',
  styleUrls: ['./select-filesets-list.component.less']
})
export class SelectFilesetsListComponent implements OnInit {
  resourceData = [];
  selectionData = [];
  valid$ = new Subject<boolean>();
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
      key: 'environment_name',
      name: this.i18n.get('protection_cluster_label'),
      filter: {
        type: 'search',
        filterMode: 'contains'
      }
    },
    {
      key: 'sla_name',
      name: this.i18n.get('common_sla_label'),
      filter: {
        type: 'search',
        filterMode: 'contains'
      }
    }
  ];

  title = this.i18n.get('protection_selected_fileset_label');

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

    if (!isEmpty(filters.conditions_v2)) {
      const conditions_v2 = JSON.parse(filters.conditions_v2);
      if (conditions_v2.environment_name) {
        conditions_v2['environment'] = {
          name: conditions_v2.environment_name
        };
        delete conditions_v2.environment_name;
      }
      assign(params, {
        conditions: JSON.stringify({
          ...conditions_v2,
          subType: DataMap.Resource_Type.HDFSFileset.value
        })
      });
    } else {
      assign(params, {
        conditions: JSON.stringify({
          subType: DataMap.Resource_Type.HDFSFileset.value
        })
      });
    }

    this.protectedResourceApiService.ListResources(params).subscribe(res => {
      this.allTableData = {
        total: res.totalCount,
        data: map(res.records, (item: any) => {
          item['environment_uuid'] = item.environment?.uuid;
          item['environment_name'] = item.environment?.name;
          item['environment_endpoint'] = item.environment?.endpoint;
          return item;
        })
      };
      this.cdr.detectChanges();
    });
  }

  dataChange(selection) {
    this.selectionData = selection;
    this.valid$.next(!!size(this.selectionData));
  }

  initData(data: any, resourceType: string) {
    this.resourceData = data;
    this.selectionData = data;
  }

  onOK() {
    return { selectedList: this.selectionData };
  }
}
