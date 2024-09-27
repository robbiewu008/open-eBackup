import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { BaseModule } from 'app/shared';
import { ProTableModule } from 'app/shared/components/pro-table';
import { ProtectionObjectComponent } from './protection-object.component';
import { SelectDiskModule } from './select-disk/select-disk.module';
import { SelectDiskModule as HyperVSelectDiskModule } from 'app/business/protection/virtualization/hyper-v/select-disk/select-disk.module';

@NgModule({
  declarations: [ProtectionObjectComponent],
  imports: [
    CommonModule,
    BaseModule,
    ProTableModule,
    SelectDiskModule,
    HyperVSelectDiskModule
  ],
  exports: [ProtectionObjectComponent]
})
export class ProtectionObjectModule {}
