import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { RegisterComponent } from './register.component';
import { BaseModule } from 'app/shared';
import { ProTableModule } from 'app/shared/components/pro-table';
import { StoreResourceNodeModule } from 'app/business/protection/cloud/huawei-stack/register-huawei-stack/store-resource-node/store-resource-node-redis.module';
import { ProButtonModule } from 'app/shared/components/pro-button';

@NgModule({
  declarations: [RegisterComponent],
  imports: [
    CommonModule,
    BaseModule,
    ProTableModule,
    StoreResourceNodeModule,
    ProButtonModule
  ],
  exports: [RegisterComponent]
})
export class RegisterModule {}
