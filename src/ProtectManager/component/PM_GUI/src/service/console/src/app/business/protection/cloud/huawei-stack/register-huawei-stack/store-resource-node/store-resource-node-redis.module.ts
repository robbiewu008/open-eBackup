import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { BaseModule } from 'app/shared';
import { NasSharedModule } from 'app/business/protection/storage/nas-shared/nas-shared.module';
import { ProTableModule } from 'app/shared/components/pro-table';
import { StorResourceNodeComponent } from './store-resource-node.component';

@NgModule({
  declarations: [StorResourceNodeComponent],
  imports: [CommonModule, BaseModule, NasSharedModule, ProTableModule]
})
export class StoreResourceNodeModule {}
