import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared/base.module';
import { ProTableModule } from 'app/shared/components/pro-table';
import { TargetLocationComponent } from './target-location.component';

@NgModule({
  declarations: [TargetLocationComponent],
  imports: [BaseModule, CommonModule, ProTableModule]
})
export class TargetLocationModule {}
