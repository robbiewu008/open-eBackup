import { Component, OnInit } from '@angular/core';
import { DataMap, I18NService, ResourceType } from 'app/shared';

@Component({
  selector: 'aui-object',
  templateUrl: './object.component.html',
  styleUrls: ['./object.component.less']
})
export class ObjectComponent implements OnInit {
  header = this.i18n.get('common_object_storage_label');
  resourceType = ResourceType.Storage;
  childResourceType = [
    DataMap.Resource_Type.ObjectSet.value,
    DataMap.Resource_Type.ObjectStorage.value
  ];

  constructor(private i18n: I18NService) {}

  ngOnInit(): void {}
}
