import { Component, OnInit, Input } from '@angular/core';
import { DataMap, DataMapService, I18NService } from 'app/shared';

@Component({
  selector: 'aui-basic-info',
  templateUrl: './basic-info.component.html',
  styleUrls: ['./basic-info.component.less']
})
export class BasicInfoComponent implements OnInit {
  formItems;

  @Input() data;
  constructor(private i18n: I18NService, private dataMap: DataMapService) {}

  ngOnInit(): void {
    this.initFormItems();
  }

  initFormItems() {
    this.formItems = [
      [
        {
          key: 'name',
          value: this.data.name,
          label: this.i18n.get('common_name_label')
        }
      ],
      [
        {
          key: 'clusterType',
          value: this.dataMap.getLabel(
            'Mysql_Cluster_Type',
            this.data?.clusterType
          ),
          label: this.i18n.get('common_type_label')
        }
      ]
    ];
  }
}
