import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared/base.module';
import { ProtectionRouterModule } from './protection-router.module';
import { ProtectionComponent } from './protection.component';
import { DatabaseTemplateModule } from './host-app/database-template/database-template.module';
import { LinkStatusModule } from 'app/shared/components/link-status/link-status.module';

@NgModule({
  imports: [
    BaseModule,
    ProtectionRouterModule,
    DatabaseTemplateModule,
    LinkStatusModule
  ],
  declarations: [ProtectionComponent]
})
export class ProtectionModule {}
