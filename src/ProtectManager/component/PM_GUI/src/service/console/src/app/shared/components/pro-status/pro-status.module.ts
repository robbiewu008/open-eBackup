import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { ProStatusComponent } from './pro-status.component';
import { IconModule, OverflowModule } from '@iux/live';

@NgModule({
  declarations: [ProStatusComponent],
  imports: [CommonModule, IconModule, OverflowModule],
  exports: [ProStatusComponent]
})
export class ProStatusModule {}
