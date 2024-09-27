import { Component, OnInit } from '@angular/core';
import { ResourceType, DataMap } from 'app/shared';

@Component({
  selector: 'aui-elasticsearch',
  templateUrl: './elasticsearch.component.html',
  styleUrls: ['./elasticsearch.component.less']
})
export class ElasticSearchComponent implements OnInit {
  header = DataMap.Resource_Type.Elasticsearch.label;
  resourceType = ResourceType.DATABASE;
  childResourceType = [DataMap.Resource_Type.ElasticsearchBackupSet.value];

  constructor() {}

  ngOnInit(): void {}
}
