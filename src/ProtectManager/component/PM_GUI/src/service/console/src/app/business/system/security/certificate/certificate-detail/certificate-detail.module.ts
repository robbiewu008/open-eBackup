import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared';
import { CertificateDetailComponent } from './certificate-detail.component';

@NgModule({
  declarations: [CertificateDetailComponent],
  imports: [CommonModule, BaseModule],

  exports: [CertificateDetailComponent]
})
export class CertificateDetailModule {}
