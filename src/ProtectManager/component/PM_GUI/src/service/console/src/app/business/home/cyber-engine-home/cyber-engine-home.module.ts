import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { CyberEngineHomeComponent } from './cyber-engine-home.component';
import { BigScreenModule } from './big-screen/big-screen.module';
import { ExceptionsFileModule } from './exceptions-file/exceptions-file.module';
import { AlarmsCountModule } from './alarms-count/alarms-count.module';
import { FileInterceptionModule } from './file-interception/file-interception.module';
import { TaskCountModule } from './task-count/task-count.module';
import { AirGapModule } from './air-gap/air-gap.module';
import { LicenseCapacityModule } from './license-capacity/license-capacity.module';
import { BaseModule } from 'app/shared';

@NgModule({
  imports: [
    CommonModule,
    BaseModule,
    BigScreenModule,
    ExceptionsFileModule,
    AlarmsCountModule,
    FileInterceptionModule,
    LicenseCapacityModule,
    TaskCountModule,
    AirGapModule
  ],
  declarations: [CyberEngineHomeComponent],
  exports: [CyberEngineHomeComponent]
})
export class CyberEngineHomeModule {}
