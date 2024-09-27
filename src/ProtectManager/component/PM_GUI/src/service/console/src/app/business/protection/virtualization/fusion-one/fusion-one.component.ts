import { Component } from '@angular/core';
import { DataMap } from 'app/shared';

@Component({
  selector: 'aui-fusion-one',
  templateUrl: './fusion-one.component.html',
  styleUrls: ['./fusion-one.component.less']
})
export class FusionOneComponent {
  resType = DataMap.Resource_Type.fusionOne.value;
}
