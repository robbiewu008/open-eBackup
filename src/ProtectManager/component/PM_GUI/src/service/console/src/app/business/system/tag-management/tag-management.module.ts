import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';

import { TagManagementRoutingModule } from './tag-management-routing.module';
import { BaseModule } from 'app/shared/base.module';
import { TagManagementComponent } from './tag-management.component';
import { ProButtonModule } from 'app/shared/components/pro-button';
import { ProTableModule } from 'app/shared/components/pro-table';
import { CreateTagModule } from './create-tag/create-tag.module';

@NgModule({
  declarations: [TagManagementComponent],
  imports: [
    BaseModule,
    CommonModule,
    ProButtonModule,
    ProTableModule,
    TagManagementRoutingModule,
    CreateTagModule
  ]
})
export class TagManagementModule {}
