import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared';
import { MultiClusterSwitchModule } from 'app/shared/components/multi-cluster-switch/multi-cluster-switch.module';
import { ProButtonModule } from 'app/shared/components/pro-button';
import { ProTableModule } from 'app/shared/components/pro-table';
import { SamlSsoConfigRoutingModule } from './saml-sso-config-routing.module';
import { SamlSsoConfigComponent } from './saml-sso-config.component';
import { SamlSsoDetailModule } from './saml-sso-detail/saml-sso-detail.module';
import { CreateSamlSsoModule } from './create-saml-sso/create-saml-sso.module';

@NgModule({
  declarations: [SamlSsoConfigComponent],
  imports: [
    CommonModule,
    SamlSsoConfigRoutingModule,
    SamlSsoDetailModule,
    CreateSamlSsoModule,
    BaseModule,
    ProTableModule,
    ProButtonModule,
    MultiClusterSwitchModule
  ]
})
export class SamlSsoConfigModule {}
