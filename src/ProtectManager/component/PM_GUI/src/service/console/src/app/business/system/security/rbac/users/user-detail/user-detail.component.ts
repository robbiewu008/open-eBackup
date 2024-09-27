import { Component, Input, OnInit } from '@angular/core';
import { DataMap, I18NService } from 'app/shared';

@Component({
  selector: 'aui-user-detail',
  templateUrl: './user-detail.component.html',
  styleUrls: ['./user-detail.component.less']
})
export class UserDetailComponent {
  @Input() openPage;
  @Input() data;

  constructor(public i18n: I18NService) {}

  back() {
    this.openPage.emit();
  }
}
