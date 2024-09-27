import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { ConfigLogLevelComponent } from './config-log-level.component';
import { BaseModule } from 'app/shared';

@NgModule({
  declarations: [ConfigLogLevelComponent],
  imports: [CommonModule, BaseModule],
  exports: [ConfigLogLevelComponent]
})
export class ConfigLogLevelModule {}
