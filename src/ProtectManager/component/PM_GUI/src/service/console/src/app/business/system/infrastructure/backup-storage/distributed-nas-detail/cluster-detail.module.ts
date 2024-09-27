import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared';
import { ProTableModule } from 'app/shared/components/pro-table/pro-table.module';
import { UserAuthModule } from '../user-auth/user-auth.module';
import { ClusterDetailComponent } from './cluster-detail.component';

@NgModule({
  declarations: [ClusterDetailComponent],
  imports: [CommonModule, BaseModule, ProTableModule, UserAuthModule],
  exports: [ClusterDetailComponent]
})
export class ClusterDetailModule {}
