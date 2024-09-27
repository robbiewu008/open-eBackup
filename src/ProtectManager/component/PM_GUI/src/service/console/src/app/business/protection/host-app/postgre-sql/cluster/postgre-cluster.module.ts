import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared';
import { ProButtonModule } from 'app/shared/components/pro-button';
import { ProStatusModule } from 'app/shared/components/pro-status';
import { ProTableModule } from 'app/shared/components/pro-table';
import { PostgreDetailModule } from './detail/postgre-detail.module';
import { PostgreClusterComponent } from './postgre-cluster.component';
import { PostgreRegisterModule } from './register/postgre-register.module';

@NgModule({
  declarations: [PostgreClusterComponent],
  imports: [
    CommonModule,
    BaseModule,
    ProTableModule,
    ProButtonModule,
    ProStatusModule,
    PostgreRegisterModule,
    PostgreDetailModule
  ],
  exports: [PostgreClusterComponent]
})
export class PostgreClusterModule {}
