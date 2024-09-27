import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared';
import { BaseInfoModule } from 'app/shared/components/base-info/base-info.module';
import { ProTableModule } from 'app/shared/components/pro-table';
import { NameDetailComponent } from './name-detail.component';

@NgModule({
  declarations: [NameDetailComponent],
  imports: [CommonModule, BaseInfoModule, BaseModule, ProTableModule]
})
export class NameDetailModule {}
