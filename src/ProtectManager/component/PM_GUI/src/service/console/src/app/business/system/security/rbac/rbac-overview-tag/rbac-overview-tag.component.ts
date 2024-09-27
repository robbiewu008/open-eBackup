import { Component, Input } from '@angular/core';

@Component({
  selector: 'aui-rbac-overview-tag',
  templateUrl: './rbac-overview-tag.component.html',
  styleUrls: ['./rbac-overview-tag.component.less']
})
export class RbacOverviewTagComponent {
  @Input() title: string;
  @Input() article: string;
}
