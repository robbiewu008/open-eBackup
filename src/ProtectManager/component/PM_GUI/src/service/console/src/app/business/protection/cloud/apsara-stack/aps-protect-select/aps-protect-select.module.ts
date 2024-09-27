import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared';
import { ProTableModule } from 'app/shared/components/pro-table';
import { AddApsDiskModule } from '../add-aps-disk/add-aps-disk.module';
import { ApsProtectSelectComponent } from './aps-protect-select.component';

@NgModule({
  declarations: [ApsProtectSelectComponent],
  imports: [CommonModule, BaseModule, ProTableModule, AddApsDiskModule],
  exports: [ApsProtectSelectComponent]
})
export class ApsProtectSelectModule {}
