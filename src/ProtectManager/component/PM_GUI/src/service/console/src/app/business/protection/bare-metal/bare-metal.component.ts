import { Component, OnInit } from '@angular/core';
import { I18NService } from 'app/shared';
import { AppUtilsService } from 'app/shared/services/app-utils.service';

@Component({
  selector: 'aui-bare-metal',
  templateUrl: './bare-metal.component.html',
  styleUrls: ['./bare-metal.component.less']
})
export class BareMetalComponent implements OnInit {
  subApp = [...this.appUtilsService.getApplicationConfig().bareMetal];
  typeTitle = this.i18n.get('common_bare_metal_other_label');

  constructor(
    private i18n: I18NService,
    private appUtilsService: AppUtilsService
  ) {}

  ngOnInit(): void {}
}
