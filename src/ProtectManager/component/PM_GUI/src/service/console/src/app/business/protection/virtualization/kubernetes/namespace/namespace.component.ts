import { Component, OnInit } from '@angular/core';
import { DataMap, I18NService } from 'app/shared';
import { TableCols } from 'app/shared/components/pro-table';

@Component({
  selector: 'aui-kubernetes-namespace',
  templateUrl: './namespace.component.html',
  styleUrls: ['./namespace.component.less']
})
export class NamespaceComponent implements OnInit {
  subType: string = DataMap.Resource_Type.KubernetesNamespace.value;
  extraConfig: any;
  columns: TableCols[] = [];
  data;

  constructor(private i18n: I18NService) {}

  ngOnInit() {
    this.initConfig();
  }

  initConfig() {
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
            iconPos: 'flow-text',
            click: data => {}
          }
        }
      },
      {
        key: 'cluster',
        name: this.i18n.get('insight_report_belong_cluster_label'),
        filter: {
          type: 'search',
          filterMode: 'contains'
        }
      },
      {
        key: 'endpoint',
        name: this.i18n.get('protection_cluster_ip_label'),
        filter: {
          type: 'search',
          filterMode: 'contains'
        }
      }
    ];
    this.extraConfig = {};
  }
}
