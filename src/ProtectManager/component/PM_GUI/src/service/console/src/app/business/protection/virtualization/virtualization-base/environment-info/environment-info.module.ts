import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { EnvironmentInfoComponent } from './environment-info.component';
import { BaseModule } from 'app/shared';
import { ProTableModule } from 'app/shared/components/pro-table';

@NgModule({
  declarations: [EnvironmentInfoComponent],
  imports: [CommonModule, BaseModule, ProTableModule],
  exports: [EnvironmentInfoComponent]
})
export class EnvironmentInfoModule {}
