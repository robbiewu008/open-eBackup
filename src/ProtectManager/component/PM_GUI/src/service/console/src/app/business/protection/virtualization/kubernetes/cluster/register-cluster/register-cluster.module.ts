import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { RegisterClusterComponent } from './register-cluster.component';
import { BaseModule } from 'app/shared';
import { ProTableModule } from 'app/shared/components/pro-table';
import { ProButtonModule } from 'app/shared/components/pro-button';
import { AddStorageComponent } from './add-storage/add-storage.component';

@NgModule({
  declarations: [RegisterClusterComponent, AddStorageComponent],
  imports: [CommonModule, BaseModule, ProTableModule, ProButtonModule],
  exports: [RegisterClusterComponent]
})
export class RegisterClusterModule {}
