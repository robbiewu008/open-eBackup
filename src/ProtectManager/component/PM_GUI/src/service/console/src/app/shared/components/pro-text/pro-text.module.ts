import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { ProTextComponent } from './pro-text.component';
import { IconModule, OverflowModule } from '@iux/live';
import { BaseModule } from 'app/shared';

@NgModule({
  declarations: [ProTextComponent],
  imports: [CommonModule, IconModule, OverflowModule, BaseModule],
  exports: [ProTextComponent]
})
export class ProTextModule {}
