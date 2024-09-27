import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';

import { HcsStorageRoutingModule } from './hcs-storage-routing.module';
import { HcsStorageComponent } from './hcs-storage.component';
import { BaseModule } from 'app/shared';
import { ProTableModule } from 'app/shared/components/pro-table';
import { ProButtonModule } from 'app/shared/components/pro-button';
import { StoreResourceNodeModule } from 'app/business/protection/cloud/huawei-stack/register-huawei-stack/store-resource-node/store-resource-node-redis.module';

@NgModule({
  declarations: [HcsStorageComponent],
  imports: [
    CommonModule,
    HcsStorageRoutingModule,
    BaseModule,
    ProTableModule,
    ProButtonModule,
    StoreResourceNodeModule
  ]
})
export class HcsStorageModule {}
