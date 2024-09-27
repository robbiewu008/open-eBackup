import { Component, Input, OnInit } from '@angular/core';
import {
  ApiService,
  I18NService,
  PermissionTable
} from '../../../../../../shared';
import { cloneDeep } from 'lodash';

@Component({
  selector: 'aui-role-detail',
  templateUrl: './role-detail.component.html',
  styleUrls: ['./role-detail.component.less']
})
export class RoleDetailComponent implements OnInit {
  @Input() openPage;
  @Input() data;

  constructor(public i18n: I18NService) {}

  ngOnInit() {}

  back() {
    this.openPage.emit();
  }
}
