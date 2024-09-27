import { NgModule } from '@angular/core';
import { EnvironmentInfoComponent } from './environment-info.component';
import { BaseModule } from 'app/shared';
import { TenantBasicInfoModule } from './basic-info/tenant-basic-info.module';
import { HCSStoreResourceModule } from './basic-info/store-resource/hcs-store-resource.module';
import { CustomModalOperateModule } from 'app/shared/components';

@NgModule({
  declarations: [EnvironmentInfoComponent],
  imports: [
    BaseModule,
    TenantBasicInfoModule,
    HCSStoreResourceModule,
    CustomModalOperateModule
  ]
})
export class EnvironmentInfoModule {}
