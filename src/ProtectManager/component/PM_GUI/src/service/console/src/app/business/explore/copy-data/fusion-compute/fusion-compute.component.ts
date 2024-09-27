import { Component, OnInit } from '@angular/core';
import { ResourceType, DataMap, I18NService } from 'app/shared';

@Component({
  selector: 'aui-fusion-compute',
  templateUrl: './fusion-compute.component.html',
  styleUrls: ['./fusion-compute.component.less']
})
export class FusionComputeComponent implements OnInit {
  header = this.i18n.get('common_fusion_compute_label');
  resourceType = ResourceType.FUSION_COMPUTE;
  childResourceType = [DataMap.Resource_Type.FusionCompute.value];

  constructor(private i18n: I18NService) {}

  ngOnInit(): void {}
}
