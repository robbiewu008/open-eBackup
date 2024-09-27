import { Component, OnInit } from '@angular/core';
import { FormBuilder, FormControl, FormGroup } from '@angular/forms';
import { BaseUtilService, I18NService } from 'app/shared';

@Component({
  selector: 'aui-configure-dataplane-ip',
  templateUrl: './configure-dataplane-ip.component.html'
})
export class ConfigureDataplaneIpComponent implements OnInit {
  drawData;
  formGroup: FormGroup;
  requiredLabel = this.i18n.get('common_required_label');
  dataplaneIpLabel = this.i18n.get('common_dataplane_ip_label');

  constructor(
    public i18n: I18NService,
    public fb: FormBuilder,
    public baseUtilService: BaseUtilService
  ) {}

  initForm() {
    this.formGroup = this.fb.group({
      dataplaneIp: new FormControl(this.drawData.dataplaneIp, {
        validators: [
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.ip()
        ],
        updateOn: 'change'
      })
    });
  }

  ngOnInit() {
    this.initForm();
  }
}
