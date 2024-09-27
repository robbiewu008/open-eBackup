import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared';
import { DetectionWhitelistModule } from '../detection-whitelist/detection-whitelist.module';
import { HoneyPotModule } from './honey-pot/honey-pot.module';
import { RealTimeDetectRoutingModule } from './real-time-detect-routing.module';
import { RealTimeDetectComponent } from './real-time-detect.component';

@NgModule({
  declarations: [RealTimeDetectComponent],
  imports: [
    CommonModule,
    RealTimeDetectRoutingModule,
    BaseModule,
    RealTimeDetectRoutingModule,
    DetectionWhitelistModule,
    HoneyPotModule
  ],
  exports: [RealTimeDetectComponent]
})
export class RealTimeDetectModule {}
