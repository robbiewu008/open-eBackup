import { Component, OnInit } from '@angular/core';
import { DataMap } from 'app/shared';

@Component({
  selector: 'aui-redis',
  templateUrl: './redis.component.html',
  styleUrls: ['./redis.component.less']
})
export class RedisComponent implements OnInit {
  header = 'Redis';
  subType = DataMap.Resource_Type.Redis.value;
  constructor() {}
  ngOnInit() {}
}
