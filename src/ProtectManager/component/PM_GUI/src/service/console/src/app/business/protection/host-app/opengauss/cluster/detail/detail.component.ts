import { Component, OnInit } from '@angular/core';
import { I18NService } from 'app/shared';
import {
  Filters,
  TableCols,
  TableConfig,
  TableData
} from 'app/shared/components/pro-table';

@Component({
  selector: 'aui-detail',
  templateUrl: './detail.component.html',
  styleUrls: ['./detail.component.less']
})
export class DetailComponent implements OnInit {
  data;
  activeIndex = 0;

  constructor() {}

  ngOnInit(): void {}
}
