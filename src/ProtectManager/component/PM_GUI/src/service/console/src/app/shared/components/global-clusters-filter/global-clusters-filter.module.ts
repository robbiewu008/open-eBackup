import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { GlobalClustersFilterComponent } from './global-clusters-filter.component';
import { ProTableModule } from '../pro-table/pro-table.module';
import { BaseModule } from 'app/shared/base.module';

@NgModule({
  declarations: [GlobalClustersFilterComponent],
  imports: [CommonModule, BaseModule, ProTableModule],
  exports: [GlobalClustersFilterComponent]
})
export class GlobalClustersFilterModule {}
