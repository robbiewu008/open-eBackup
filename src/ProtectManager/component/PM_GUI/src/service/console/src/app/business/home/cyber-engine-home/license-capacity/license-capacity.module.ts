import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { LicenseCapacityComponent } from './license-capacity.component';
import { BaseModule } from 'app/shared';

@NgModule({
  imports: [CommonModule, BaseModule],
  declarations: [LicenseCapacityComponent],
  exports: [LicenseCapacityComponent]
})
export class LicenseCapacityModule {}
