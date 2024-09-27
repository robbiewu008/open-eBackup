import {
  ChangeDetectorRef,
  Component,
  OnInit,
  TemplateRef,
  ViewChild
} from '@angular/core';
import {
  DataMap,
  DataMapService,
  I18NService,
  ProtectedResourceApiService
} from 'app/shared';

@Component({
  selector: 'aui-environment-info',
  templateUrl: './environment-info.component.html',
  styleUrls: ['./environment-info.component.less']
})
export class EnvironmentInfoComponent implements OnInit {
  data;
  treeSelection;
  online = true;
  formItems;
  interval;

  @ViewChild('timeTpl', { static: false })
  timeTpl: TemplateRef<any>;

  constructor(
    private i18n: I18NService,
    public dataMapService: DataMapService,
    private protectedResourceApiService: ProtectedResourceApiService
  ) {}

  ngOnInit() {
    this.data = this.treeSelection;
    this.getResource();
  }

  getResource() {
    this.protectedResourceApiService
      .ShowResource({
        resourceId: this.treeSelection.uuid
      })
      .subscribe((res: any) => {
        this.online =
          res.linkStatus === DataMap.resource_LinkStatus_Special.normal.value;
        this.interval = res.scanInterval / 3600;
        this.formItems = [
          [
            {
              label: this.i18n.get('common_name_label'),
              content: this.data.name
            },
            {
              label: this.i18n.get('common_ip_label'),
              content: this.data.endpoint
            },
            {
              label: this.i18n.get('common_username_label'),
              content: this.data.userName
            }
          ],
          [
            {
              label: this.i18n.get('common_port_label'),
              content: this.data.port
            },
            {
              label: this.i18n.get('protection_register_vm_rescan_label'),
              content: `${this.interval}${this.i18n.get('common_hour_label')}`
            }
          ]
        ];
      });
  }
}
