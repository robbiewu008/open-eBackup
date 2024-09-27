import { Component, OnInit } from '@angular/core';
import { ResourceType, DataMap } from 'app/shared';

@Component({
  selector: 'aui-redis',
  templateUrl: './redis.component.html',
  styleUrls: ['./redis.component.less']
})
export class RedisComponent implements OnInit {
  header = 'Redis';
  resourceType = ResourceType.DATABASE;
  childResourceType = [DataMap.Resource_Type.Redis.value];

  constructor() {}

  ngOnInit(): void {}
}
