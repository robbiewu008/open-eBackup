import { Component, OnInit } from '@angular/core';
import { DataMap, I18NService } from 'app/shared';

@Component({
  selector: 'aui-active-directory',
  templateUrl: './active-directory.component.html',
  styleUrls: ['./active-directory.component.less']
})
export class ActiveDirectoryComponent implements OnInit {
  header = this.i18n.get('common_application_label');
  resourceType = DataMap.Resource_Type.ActiveDirectory.value;
  childResourceType = [DataMap.Resource_Type.ActiveDirectory.value];
  constructor(private i18n: I18NService) {}

  ngOnInit(): void {}
}
