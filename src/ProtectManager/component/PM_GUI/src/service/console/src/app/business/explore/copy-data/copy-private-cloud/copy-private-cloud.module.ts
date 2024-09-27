import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';

import { CopyResourceListModule } from '../copy-resource-list/copy-resource-list.module';
import { CopyPrivateCloudRoutingModule } from './copy-private-cloud-routing.module';
import { CopyPrivateCloudComponent } from './copy-private-cloud.component';

@NgModule({
  declarations: [CopyPrivateCloudComponent],
  imports: [CommonModule, CopyPrivateCloudRoutingModule, CopyResourceListModule]
})
export class CopyPrivateCloudModule {}
