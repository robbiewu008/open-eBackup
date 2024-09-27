import { Component, OnInit } from '@angular/core';
import { DataMap } from 'app/shared';

@Component({
  selector: 'aui-gaussdb-t',
  templateUrl: './gaussdb-t.component.html',
  styleUrls: ['./gaussdb-t.component.less']
})
export class GaussdbTComponent implements OnInit {
  header = 'GaussDB T';
  subType = DataMap.Resource_Type.GaussDB_T.value;

  constructor() {}

  ngOnInit() {}
}
