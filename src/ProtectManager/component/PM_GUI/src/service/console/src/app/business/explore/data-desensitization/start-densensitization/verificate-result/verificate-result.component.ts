import { Component, OnInit } from '@angular/core';
import { each, isEmpty, isArray, assign, range } from 'lodash';
import { I18NService } from 'app/shared';

@Component({
  selector: 'aui-verificate-result',
  templateUrl: './verificate-result.component.html',
  styleUrls: ['./verificate-result.component.less']
})
export class VerificateResultComponent implements OnInit {
  rowItem;
  tableData = [];
  columns = [
    {
      key: 'column_name',
      label: this.i18n.get('explore_column_name_label')
    },
    {
      key: 'line_content',
      label: this.i18n.get('explore_line_content_label')
    }
  ];
  constructor(private i18n: I18NService) {}

  getTableData() {
    if (isArray(this.rowItem.result) && !isEmpty(this.rowItem.result)) {
      const arr = [];
      const keys = this.rowItem.result.shift();
      each(range(keys.length), index => {
        each(this.rowItem.result, item => {
          const obj = {};
          assign(obj, {
            column_name: keys[index],
            line_content: item[index]
          });
          arr.push(obj);
        });
      });
      this.tableData = arr;
    } else {
      this.tableData = [];
    }
  }

  ngOnInit() {
    this.getTableData();
  }
}
