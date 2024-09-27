import { Component, OnInit } from '@angular/core';
import { I18NService } from 'app/shared';
import { AppUtilsService } from 'app/shared/services/app-utils.service';

@Component({
  selector: 'aui-copy-bare-metal',
  templateUrl: './copy-bare-metal.component.html',
  styleUrls: ['./copy-bare-metal.component.less']
})
export class CopyBareMetalComponent implements OnInit {
  subApp = [...this.appUtilsService.getApplicationConfig().bareMetal];
  typeTitle = this.i18n.get('common_bare_metal_other_label');
  routerType = 'copy';

  constructor(
    private i18n: I18NService,
    private appUtilsService: AppUtilsService
  ) {}

  ngOnInit(): void {}
}
