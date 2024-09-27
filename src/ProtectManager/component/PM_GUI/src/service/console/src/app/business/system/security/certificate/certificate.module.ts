/*
 * This file is a part of the open-eBackup project.
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
 * If a copy of the MPL was not distributed with this file, You can obtain one at
 * http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) [2024] Huawei Technologies Co.,Ltd.
 *
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 */
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
