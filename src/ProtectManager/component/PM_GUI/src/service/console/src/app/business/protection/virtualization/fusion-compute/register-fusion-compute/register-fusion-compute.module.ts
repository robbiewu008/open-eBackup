import { NgModule } from '@angular/core';
import { StoreResourceNodeModule } from 'app/business/protection/cloud/huawei-stack/register-huawei-stack/store-resource-node/store-resource-node-redis.module';
import { BaseModule } from 'app/shared';
import { ProButtonModule } from 'app/shared/components/pro-button';
import { ProTableModule } from 'app/shared/components/pro-table';
import { RegisterFusionComputeComponent } from './register-fusion-compute.component';

@NgModule({
  declarations: [RegisterFusionComputeComponent],
  imports: [
    BaseModule,
    ProTableModule,
    ProButtonModule,
    StoreResourceNodeModule
  ]
})
export class RegisterFusionComputeModule {}
