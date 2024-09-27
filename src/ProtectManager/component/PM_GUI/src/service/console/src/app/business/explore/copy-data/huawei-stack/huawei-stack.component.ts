import { Component, OnInit } from '@angular/core';
import {
  ResourceType,
  DataMap,
  I18NService,
  CookieService,
  CommonConsts
} from 'app/shared';

@Component({
  selector: 'aui-huawei-stack',
  templateUrl: './huawei-stack.component.html',
  styleUrls: ['./huawei-stack.component.less']
})
export class HuaweiStackComponent implements OnInit {
  header =
    this.cookieService.get('userType') === CommonConsts.HCS_USER_TYPE
      ? this.i18n.get('common_cloud_server_label')
      : this.i18n.get('common_cloud_label');
  resourceType = ResourceType.HCS;
  childResourceType = [DataMap.Resource_Type.HCSCloudHost.value];

  constructor(
    private i18n: I18NService,
    private cookieService: CookieService
  ) {}

  ngOnInit(): void {}
}
