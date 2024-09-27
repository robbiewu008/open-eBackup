import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared';
import { ProButtonModule } from 'app/shared/components/pro-button';
import { ProStatusModule } from 'app/shared/components/pro-status';
import { ProTableModule } from 'app/shared/components/pro-table';
import { KingBaseDetailModule } from './detail/king-base-detail.module';
import { KingBaseClusterComponent } from './king-base-cluster.component';
import { KingBaseRegisterModule } from './register/king-base-register.module';

@NgModule({
  declarations: [KingBaseClusterComponent],
  imports: [
    CommonModule,
    BaseModule,
    ProTableModule,
    ProButtonModule,
    ProStatusModule,
    KingBaseRegisterModule,
    KingBaseDetailModule
  ],
  exports: [KingBaseClusterComponent]
})
export class KingBaseClusterModule {}
