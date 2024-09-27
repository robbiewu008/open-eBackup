import { Component, OnInit } from '@angular/core';
import { DataMap, I18NService } from 'app/shared';
import { TableCols } from 'app/shared/components/pro-table';

@Component({
  selector: 'aui-database',
  templateUrl: './database.component.html',
  styleUrls: ['./database.component.less']
})
export class DatabaseComponent implements OnInit {
  columns: TableCols[] = [];
  resourceSubType = DataMap.Resource_Type.OpenGauss_database.value;

  constructor(private i18n: I18NService) {}

  ngOnInit(): void {
    this.initConfig();
  }

  initConfig(): void {
    const cols: TableCols[] = [
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
            iconPos: 'flow-text',
            click: data => {}
          }
        }
      },
      {
        key: 'owned_instance',
        name: this.i18n.get('commom_owned_instance_label'),
        filter: {
          type: 'search',
          filterMode: 'contains'
        }
      }
    ];
    this.columns = cols;
  }
}
