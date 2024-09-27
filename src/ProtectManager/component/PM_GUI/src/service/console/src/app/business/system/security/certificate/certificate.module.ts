import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { CertificateRoutingModule } from './certificate-routing.module';
import { CertificateComponent } from './certificate.component';
import { BaseModule } from 'app/shared';
import { ExportRequestFileModule } from './export-request-file/export-request-file.module';
import { ModalService } from '@iux/live';
import { AddComponentsModule } from './add-components/add-components.module';
import { CertificateDetailModule } from './certificate-detail/certificate-detail.module';
import { ImportCertificateModule } from './import-certificate/import-certificate.module';
import { ImportRevocationListModule } from './import-revocation-list/import-revocation-list.module';
import { ModifyModule } from './modify/modify.module';
import { MultiClusterSwitchModule } from 'app/shared/components/multi-cluster-switch/multi-cluster-switch.module';
@NgModule({
  declarations: [CertificateComponent],
  imports: [
    CommonModule,
    CertificateRoutingModule,
    BaseModule,
    ExportRequestFileModule,
    ImportCertificateModule,
    ImportRevocationListModule,
    ModifyModule,
    CertificateDetailModule,
    AddComponentsModule,
    MultiClusterSwitchModule
  ],
  providers: [ModalService]
})
export class CertificateModule {}
