import { Component, OnInit } from '@angular/core';
import { I18NService } from 'app/shared';
import { AppUtilsService } from 'app/shared/services/app-utils.service';

@Component({
  selector: 'aui-virtualization',
  templateUrl: './virtualization.component.html',
  styleUrls: ['./virtualization.component.less']
})
export class VirtualizationComponent implements OnInit {
  subApp = [...this.appUtilsService.getApplicationConfig().virtualization];
  typeTitle = this.i18n.get('common_virtualization_label');

  constructor(
    private i18n: I18NService,
    private appUtilsService: AppUtilsService
  ) {}

  ngOnInit() {}
}
