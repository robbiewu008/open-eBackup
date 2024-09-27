import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { BaseModule } from 'app/shared';
import { DatabaseTemplateModule } from '../../host-app/database-template/database-template.module';
import { CommonShareRoutingModule } from './commonshare-routing.module';
import { CommonShareComponent } from './commonshare.component';
import { ProtectModule } from 'app/shared/components/protect/protect.module';
import { SummaryCommonShareModule } from './summary-commonshare/summary-commonshare.module';
import { RegisterCommonShareModule } from './create-commonshare/create-commonshare.module';
import { LinkModule } from './link/link.module';

@NgModule({
  declarations: [CommonShareComponent],
  imports: [
    CommonModule,
    CommonShareRoutingModule,
    BaseModule,
    DatabaseTemplateModule,
    ProtectModule,
    SummaryCommonShareModule,
    RegisterCommonShareModule,
    LinkModule
  ],
  exports: [CommonShareComponent]
})
export class CommonShareModule {}
