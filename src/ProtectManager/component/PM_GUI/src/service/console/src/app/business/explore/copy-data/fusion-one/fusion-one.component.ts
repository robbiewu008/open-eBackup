import { Component } from '@angular/core';
import { DataMap, I18NService, ResourceType } from 'app/shared';

@Component({
  selector: 'aui-fusion-one',
  templateUrl: './fusion-one.component.html',
  styleUrls: ['./fusion-one.component.less']
})
export class FusionOneComponent {
  header = this.i18n.get('protection_fusionone_label');
  resourceType = ResourceType.FUSION_ONE;
  childResourceType = [DataMap.Resource_Type.fusionOne.value];

  constructor(private i18n: I18NService) {}
}
