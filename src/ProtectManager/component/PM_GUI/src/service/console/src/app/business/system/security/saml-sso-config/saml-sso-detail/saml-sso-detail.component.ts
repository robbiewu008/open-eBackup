import {
  ChangeDetectionStrategy,
  ChangeDetectorRef,
  Component,
  OnInit
} from '@angular/core';
import { FormBuilder, FormGroup } from '@angular/forms';
import {
  BaseUtilService,
  DataMapService,
  I18NService,
  KerberosAPIService,
  WarningMessageService
} from 'app/shared';
import { DrawModalService } from 'app/shared/services/draw-modal.service';
import { VirtualScrollService } from 'app/shared/services/virtual-scroll.service';
import { filter as _filter } from 'lodash';

@Component({
  selector: 'aui-saml-sso-detail',
  templateUrl: './saml-sso-detail.component.html',
  styleUrls: ['./saml-sso-detail.component.less'],
  changeDetection: ChangeDetectionStrategy.OnPush
})
export class SamlSsoDetailComponent implements OnInit {
  rowData;
  detailItems = [];
  formGroup: FormGroup;

  constructor(
    public i18n: I18NService,
    public drawModalService: DrawModalService,
    public kerberosApi: KerberosAPIService,
    public cdr: ChangeDetectorRef,
    public virtualScroll: VirtualScrollService,
    public warningMessageService: WarningMessageService,
    public dataMapService: DataMapService,
    private fb: FormBuilder,
    public baseUtilService: BaseUtilService
  ) {}

  ngOnInit() {
    this.initForm();
  }

  initForm() {
    this.formGroup = this.fb.group({});
    this.detailItems = [
      {
        key: 'configName',
        value: this.rowData?.configName,
        label: 'common_name_label'
      },
      {
        key: 'status',
        value: this.rowData?.status,
        label: 'common_status_label'
      },
      {
        key: 'protocol',
        value: this.rowData?.protocol,
        label: 'common_protocol_label'
      },
      {
        key: 'description',
        value: this.rowData?.description,
        label: 'common_desc_label'
      }
    ];
  }
}
