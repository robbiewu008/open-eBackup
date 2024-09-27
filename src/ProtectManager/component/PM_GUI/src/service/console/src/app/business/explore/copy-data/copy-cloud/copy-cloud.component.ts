import { Component, OnInit } from '@angular/core';
import { I18NService } from 'app/shared';
import { AppUtilsService } from 'app/shared/services/app-utils.service';

@Component({
  selector: 'aui-copy-cloud',
  templateUrl: './copy-cloud.component.html',
  styleUrls: ['./copy-cloud.component.less']
})
export class CopyCloudComponent implements OnInit {
  subApp = [...this.appUtilsService.getApplicationConfig().cloud];
  typeTitle = this.i18n.get('common_huawei_clouds_label');
  routerType = 'copy';

  constructor(
    private i18n: I18NService,
    private appUtilsService: AppUtilsService
  ) {}

  ngOnInit(): void {}
}
