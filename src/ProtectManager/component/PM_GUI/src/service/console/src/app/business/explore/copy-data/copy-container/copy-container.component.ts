import { Component, OnInit } from '@angular/core';
import { I18NService } from 'app/shared';
import { AppUtilsService } from 'app/shared/services/app-utils.service';

@Component({
  selector: 'aui-copy-container',
  templateUrl: './copy-container.component.html',
  styleUrls: ['./copy-container.component.less']
})
export class CopyContainerComponent implements OnInit {
  subApp = [...this.appUtilsService.getApplicationConfig().container];
  typeTitle = this.i18n.get('common_container_label');
  routerType = 'copy';

  constructor(
    private i18n: I18NService,
    private appUtilsService: AppUtilsService
  ) {}

  ngOnInit(): void {}
}
