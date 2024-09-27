import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared';
import { ImportLicenseComponent } from './import-license.component';

@NgModule({
  declarations: [ImportLicenseComponent],
  imports: [CommonModule, BaseModule]
})
export class ImportLicenseModule {}
