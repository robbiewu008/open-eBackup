import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { SaphanaRestoreComponent } from './saphana-restore.component';
import { BaseModule } from 'app/shared';
import { ResourceFilterModule } from 'app/shared/components/resource-filter/resource-filter.module';

@NgModule({
  declarations: [SaphanaRestoreComponent],
  imports: [CommonModule, BaseModule, ResourceFilterModule],
  exports: [SaphanaRestoreComponent]
})
export class SaphanaRestoreModule {}
