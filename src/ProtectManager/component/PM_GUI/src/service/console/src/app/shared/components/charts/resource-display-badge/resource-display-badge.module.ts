import { NgModule } from '@angular/core';
import { ResourceDisplayBadgeComponent } from './resource-display-badge.component';
import { BaseModule } from 'app/shared/base.module';
import {
  DropdownModule,
  IconModule,
  ButtonModule,
  GroupModule
} from '@iux/live';
import { RouterModule } from '@angular/router';

@NgModule({
  imports: [
    BaseModule,
    DropdownModule,
    IconModule,
    ButtonModule,
    GroupModule,
    RouterModule
  ],
  declarations: [ResourceDisplayBadgeComponent],
  exports: [ResourceDisplayBadgeComponent]
})
export class ResourceDisplayBadgeModule {}
