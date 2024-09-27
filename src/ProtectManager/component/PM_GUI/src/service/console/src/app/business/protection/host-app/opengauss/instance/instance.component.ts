import { Component, OnInit } from '@angular/core';
import { DataMap, DataMapService, I18NService } from 'app/shared';
import { TableCols } from 'app/shared/components/pro-table';

@Component({
  selector: 'aui-openGauss-instance',
  templateUrl: './instance.component.html',
  styleUrls: ['./instance.component.less']
})
export class InstanceComponent implements OnInit {
  resourceSubType = DataMap.Resource_Type.OpenGauss_instance.value;
  columns: TableCols[] = [];
  constructor(
    private i18n: I18NService,
    private dataMapService: DataMapService
  ) {}

  ngOnInit(): void {
    this.columns = [
      {
        key: 'uuid',
        name: this.i18n.get('protection_resource_id_label'),
        hidden: true,
        filter: {
          type: 'search',
          filterMode: 'contains'
        }
      },
      {
        key: 'name',
        name: this.i18n.get('common_name_label'),
        sort: true,
        cellRender: {
          type: 'text',
          config: {
            id: 'outerClosable',
            iconPos: 'flow-text'
          }
        }
      },
      {
        key: 'instanceStatus',
        name: this.i18n.get('common_status_label'),
        filter: {
          type: 'select',
          isMultiple: true,
          options: this.dataMapService.toArray('openGauss_InstanceStatus')
        },
        cellRender: {
          type: 'status',
          config: this.dataMapService.toArray('openGauss_InstanceStatus')
        }
      }
    ];
  }
}
