import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { AlertModule, IconModule } from '@iux/live';
import { BaseModule } from 'app/shared';
import { ProTableModule } from 'app/shared/components/pro-table';
import { LinkComponent } from './link.component';

@NgModule({
  declarations: [LinkComponent],
  imports: [CommonModule, BaseModule, IconModule, ProTableModule, AlertModule],
  exports: [LinkComponent]
})
export class LinkModule {}
