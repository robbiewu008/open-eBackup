import { Component, OnInit } from '@angular/core';
import { I18NService } from 'app/shared';
import { AppUtilsService } from 'app/shared/services/app-utils.service';

@Component({
  selector: 'aui-big-data-summary',
  templateUrl: './big-data.component.html',
  styleUrls: ['./big-data.component.less']
})
export class BigDataComponent implements OnInit {
  subApp = [...this.appUtilsService.getApplicationConfig().bigData];
  typeTitle = this.i18n.get('common_bigdata_label');

  constructor(
    private i18n: I18NService,
    private appUtilsService: AppUtilsService
  ) {}

  ngOnInit(): void {}
}
