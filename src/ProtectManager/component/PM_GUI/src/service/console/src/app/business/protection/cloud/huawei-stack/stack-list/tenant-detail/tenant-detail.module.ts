import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { TenantDetailComponent } from './tenant-detail.component';
import { BaseModule } from 'app/shared';
import { ProTableModule } from 'app/shared/components/pro-table';
import { CustomModalOperateModule } from 'app/shared/components';

@NgModule({
  declarations: [TenantDetailComponent],
  imports: [CommonModule, BaseModule, ProTableModule, CustomModalOperateModule]
})
export class TenantDetailModule {}
