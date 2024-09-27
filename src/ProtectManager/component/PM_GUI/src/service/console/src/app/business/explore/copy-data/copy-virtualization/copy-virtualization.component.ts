import { Component, OnInit } from '@angular/core';
import { I18NService } from 'app/shared';
import { AppUtilsService } from 'app/shared/services/app-utils.service';

@Component({
  selector: 'aui-copy-virtualization',
  templateUrl: './copy-virtualization.component.html',
  styleUrls: ['./copy-virtualization.component.less']
})
export class CopyVirtualizationComponent implements OnInit {
  subApp = [...this.appUtilsService.getApplicationConfig().virtualization];
  typeTitle = this.i18n.get('common_virtualization_label');
  routerType = 'copy';

  constructor(
    private i18n: I18NService,
    private appUtilsService: AppUtilsService
  ) {}

  ngOnInit(): void {}
}
