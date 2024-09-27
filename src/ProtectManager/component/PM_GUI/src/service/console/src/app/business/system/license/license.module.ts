import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared/base.module';
import { LicenseComponent } from './license.component';
import { ImportLicenseModule } from './import-license/import-license.module';
import { MultiClusterSwitchModule } from 'app/shared/components/multi-cluster-switch/multi-cluster-switch.module';
import { LicenseRoutingModule } from './license-routing.module';

@NgModule({
  imports: [
    BaseModule,
    CommonModule,
    LicenseRoutingModule,
    ImportLicenseModule,
    MultiClusterSwitchModule
  ],
  declarations: [LicenseComponent]
})
export class LicenseModule {}
