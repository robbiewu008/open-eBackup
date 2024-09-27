import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { LanFreeComponent, SelectionPipe } from './lan-free.component';
import { BaseModule } from 'app/shared';
import { ProTableModule } from 'app/shared/components/pro-table';

@NgModule({
  declarations: [LanFreeComponent, SelectionPipe],
  imports: [CommonModule, BaseModule, ProTableModule],
  exports: [LanFreeComponent]
})
export class LanFreeModule {}
