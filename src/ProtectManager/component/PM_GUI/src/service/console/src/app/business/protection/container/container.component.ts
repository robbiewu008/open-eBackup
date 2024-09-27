import { Component, OnInit } from '@angular/core';
import { I18NService } from 'app/shared';
import { AppUtilsService } from 'app/shared/services/app-utils.service';

@Component({
  selector: 'aui-container',
  templateUrl: './container.component.html',
  styleUrls: ['./container.component.less']
})
export class ContainerComponent implements OnInit {
  subApp = [...this.appUtilsService.getApplicationConfig().container];
  typeTitle = this.i18n.get('common_container_label');

  constructor(
    private i18n: I18NService,
    private appUtilsService: AppUtilsService
  ) {}

  ngOnInit(): void {}
}
