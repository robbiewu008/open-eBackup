import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { AlarmsCountComponent } from './alarms-count.component';
import { BaseModule } from 'app/shared';

@NgModule({
  imports: [CommonModule, BaseModule],
  declarations: [AlarmsCountComponent],
  exports: [AlarmsCountComponent]
})
export class AlarmsCountModule {}
