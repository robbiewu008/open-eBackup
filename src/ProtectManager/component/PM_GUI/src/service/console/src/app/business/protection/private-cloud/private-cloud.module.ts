import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';

import { PrivateCloudRoutingModule } from './private-cloud-routing.module';
import { PrivateCloudComponent } from './private-cloud.component';
import { CloudServerModule } from '../cloud/huawei-stack/cloud-server/cloud-server.module';

@NgModule({
  declarations: [PrivateCloudComponent],
  imports: [CommonModule, PrivateCloudRoutingModule, CloudServerModule]
})
export class PrivateCloudModule {}
