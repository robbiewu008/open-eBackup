import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared';
import { CopyResourceListModule } from '../copy-resource-list/copy-resource-list.module';
import { ElasticSearchRoutingModule } from './elasticsearch-routing.module';
import { ElasticSearchComponent } from './elasticsearch.component';

@NgModule({
  declarations: [ElasticSearchComponent],
  imports: [
    CommonModule,
    ElasticSearchRoutingModule,
    BaseModule,
    CopyResourceListModule
  ]
})
export class ElasticSearchModule {}
