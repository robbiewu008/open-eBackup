import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { AddWhitelistRuleComponent } from './add-whitelist-rule.component';
import { BaseModule } from 'app/shared';

@NgModule({
  declarations: [AddWhitelistRuleComponent],
  imports: [CommonModule, BaseModule]
})
export class AddWhitelistRuleModule {}
