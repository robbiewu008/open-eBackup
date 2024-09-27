import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared';
import { SpecialBaseInfoModule } from 'app/shared/components/special-base-info/special-base-info.module';
import { ProButtonModule } from 'app/shared/components/pro-button';
import { ProStatusModule } from 'app/shared/components/pro-status';
import { ProTableModule } from 'app/shared/components/pro-table';
import { ProjectSummaryComponent } from './project-summary.component';

@NgModule({
  declarations: [ProjectSummaryComponent],
  imports: [
    BaseModule,
    SpecialBaseInfoModule,
    ProTableModule,
    ProButtonModule,
    ProStatusModule
  ]
})
export class ProjectSummaryModule {}
