import { Component, OnInit, ViewChild } from '@angular/core';
import { DataMap } from 'app/shared';
import { TableTemplateComponent } from './table-template/table-template.component';

@Component({
  selector: 'aui-general-database',
  templateUrl: './general-database.component.html',
  styleUrls: ['./general-database.component.less']
})
export class GeneralDatabaseComponent implements OnInit {
  activeIndex = 0;
  dataMap = DataMap;

  @ViewChild(TableTemplateComponent, { static: false })
  tableTemplateComponent: TableTemplateComponent;

  constructor() {}

  ngOnInit() {}

  onChange() {
    this.tableTemplateComponent.ngOnInit();
  }
}
