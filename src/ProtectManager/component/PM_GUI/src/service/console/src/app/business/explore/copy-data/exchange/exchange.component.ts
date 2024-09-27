import { Component, OnInit } from '@angular/core';
import { DataMap, I18NService, ResourceType } from 'app/shared';

@Component({
  selector: 'aui-exchange',
  templateUrl: './exchange.component.html',
  styleUrls: ['./exchange.component.less']
})
export class ExchangeComponent implements OnInit {
  header = this.i18n.get('common_exchange_label');
  resourceType = ResourceType.DATABASE;
  childResourceType = [
    DataMap.Resource_Type.Exchange.value,
    DataMap.Resource_Type.ExchangeDataBase.value,
    DataMap.Resource_Type.ExchangeEmail.value,
    DataMap.Resource_Type.ExchangeSingle.value,
    DataMap.Resource_Type.ExchangeGroup.value
  ];

  constructor(private i18n: I18NService) {}
  ngOnInit(): void {}
}
