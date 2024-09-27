import { Component, OnInit } from '@angular/core';
import { DataMap } from 'app/shared';
import { assign, get, includes } from 'lodash';

@Component({
  selector: 'aui-mongodb-copy-data',
  templateUrl: './copy-data.component.html',
  styleUrls: ['./copy-data.component.less']
})
export class CopyDataComponent implements OnInit {
  data;
  resourceType;

  constructor() {}

  ngOnInit() {}

  initDetailData(data: any) {
    this.resourceType = data.subType || data.sub_type;
    if (
      includes(
        [
          DataMap.Resource_Type.ExchangeSingle.value,
          DataMap.Resource_Type.ExchangeGroup.value,
          DataMap.Resource_Type.ExchangeDataBase.value
        ],
        this.resourceType
      )
    ) {
      if (!get(data, 'environment')) {
        assign(data, {
          environment: {
            name: get(data, 'name'),
            endpoint: get(data, 'endpoint'),
            uuid: get(data, 'rootUuid')
          }
        });
      }
    }
    this.data = data;
  }
}
