import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { ClustreDetailComponent } from './clustre-detail.component';
import { BaseModule } from 'app/shared';
import { ProTableModule } from 'app/shared/components/pro-table';

@NgModule({
  declarations: [ClustreDetailComponent],
  imports: [CommonModule, BaseModule, ProTableModule]
})
export class ClustreDetailModule {}
