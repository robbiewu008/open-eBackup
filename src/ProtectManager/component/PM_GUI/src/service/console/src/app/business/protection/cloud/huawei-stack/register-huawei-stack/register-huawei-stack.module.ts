import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared';
import { ProButtonModule } from 'app/shared/components/pro-button';
import { ProStatusModule } from 'app/shared/components/pro-status';
import { ProTableModule } from 'app/shared/components/pro-table';
import { RegisterHuaWeiStackComponent } from './register-huawei-stack.component';
import { StoreResourceNodeModule } from './store-resource-node/store-resource-node-redis.module';

@NgModule({
  declarations: [RegisterHuaWeiStackComponent],
  imports: [
    BaseModule,
    ProTableModule,
    ProButtonModule,
    ProStatusModule,
    StoreResourceNodeModule
  ]
})
export class RegisterHuaWeiStackModule {}
