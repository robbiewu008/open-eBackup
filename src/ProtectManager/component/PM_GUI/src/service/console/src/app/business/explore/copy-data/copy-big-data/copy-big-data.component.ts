import { Component, OnInit } from '@angular/core';
import { I18NService } from 'app/shared';
import { AppUtilsService } from 'app/shared/services/app-utils.service';

@Component({
  selector: 'aui-copy-big-data',
  templateUrl: './copy-big-data.component.html',
  styleUrls: ['./copy-big-data.component.less']
})
export class CopyBigDataComponent implements OnInit {
  subApp = [...this.appUtilsService.getApplicationConfig().bigData];
  typeTitle = this.i18n.get('common_bigdata_label');
  routerType = 'copy';

  constructor(
    private i18n: I18NService,
    private appUtilsService: AppUtilsService
  ) {}

  ngOnInit(): void {}
}
