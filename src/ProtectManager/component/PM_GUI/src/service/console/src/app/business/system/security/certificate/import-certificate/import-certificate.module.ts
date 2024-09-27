import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { AlertModule, UploadModule } from '@iux/live';
import { BaseModule } from 'app/shared';
import { ImportCertificateComponent } from './import-certificate.component';

@NgModule({
  declarations: [ImportCertificateComponent],
  imports: [CommonModule, BaseModule, UploadModule, AlertModule],
  exports: [ImportCertificateComponent]
})
export class ImportCertificateModule {}
