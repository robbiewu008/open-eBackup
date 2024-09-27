import { BaseModule } from 'app/shared/base.module';
import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { TargetLocationComponent } from './target-location.component';
import { ProTableModule } from 'app/shared/components/pro-table';

@NgModule({
  declarations: [TargetLocationComponent],
  imports: [BaseModule, CommonModule, ProTableModule]
})
export class TargetLocationModule {}
