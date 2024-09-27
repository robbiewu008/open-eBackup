import {
  BaseUtilService,
  I18NService,
  DataMapService,
  DataMap,
  copyClipboard,
  SSOConfigService
} from 'app/shared';
import { FormGroup, FormBuilder, FormControl } from '@angular/forms';
import { Component, OnInit } from '@angular/core';
import { Observable, Observer, Subject } from 'rxjs';
import { first, get, set, size } from 'lodash';
import { MessageService, UploadFile } from '@iux/live';

@Component({
  selector: 'aui-create-saml-sso',
  templateUrl: './create-saml-sso.component.html',
  styleUrls: ['./create-saml-sso.component.less']
})
export class CreateSamlSsoComponent implements OnInit {
  rowData;
  dataMap = DataMap;
  formGroup: FormGroup;
  metadataFilters = [];
  selectMetadataFile;
  validMetadata$ = new Subject<boolean>();

  protocolOptions = this.dataMapService.toArray('samlSsoProtocol').map(item => {
    item['isLeaf'] = true;
    return item;
  });
  nameErrorTip = {
    ...this.baseUtilService.nameErrorTip,
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [64])
  };
  descErrorTip = {
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [1024])
  };

  constructor(
    private fb: FormBuilder,
    private i18n: I18NService,
    private message: MessageService,
    private dataMapService: DataMapService,
    public baseUtilService: BaseUtilService,
    private ssoConfigService: SSOConfigService
  ) {}

  ngOnInit() {
    this.initForm();
    this.initFilters();
  }

  initForm() {
    this.formGroup = this.fb.group({
      name: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.name(),
          this.baseUtilService.VALID.maxLength(64)
        ]
      }),
      protocol: new FormControl(DataMap.samlSsoProtocol.version.value),
      desc: new FormControl('', {
        validators: [this.baseUtilService.VALID.maxLength(1024)]
      })
    });

    if (this.rowData) {
      this.formGroup.patchValue({
        name: this.rowData?.configName,
        desc: this.rowData?.description
      });
    }
  }

  initFilters() {
    this.metadataFilters = [
      {
        name: 'suffix',
        filterFn: (files: UploadFile[]) => {
          const supportSuffix = ['xml'];
          const validFiles = files.filter(file => {
            const suffix = file.name.split('.').pop();
            return supportSuffix.includes(suffix);
          });

          if (validFiles.length !== files.length) {
            this.message.error(
              this.i18n.get('common_format_error_label', ['xml']),
              {
                lvMessageKey: 'formatErrorKey3',
                lvShowCloseButton: true
              }
            );
            this.selectMetadataFile = '';
            this.validMetadata$.next(false);
            return validFiles;
          }
          if (files[0].size > 1024 * 1024) {
            this.message.error(
              this.i18n.get('common_max_size_file_label', ['1MB']),
              {
                lvMessageKey: 'maxSizeFileErrorKey3',
                lvShowCloseButton: true
              }
            );
            this.selectMetadataFile = '';
            this.validMetadata$.next(false);
            return [];
          }
          const reader = new FileReader();
          reader.onloadend = () => {
            this.selectMetadataFile = get(first(files), 'originFile');
            this.validMetadata$.next(true);
          };
          reader.readAsDataURL(files[0].originFile);
          return validFiles;
        }
      }
    ];
  }

  filesChange(file) {
    if (!size(file)) {
      this.selectMetadataFile = '';
      this.validMetadata$.next(false);
    }
  }

  copyLink(value) {
    copyClipboard(value);
  }

  getParams() {
    const params = {
      configName: this.formGroup.value.name,
      description: this.formGroup.value.desc
    };

    if (this.selectMetadataFile) {
      set(params, 'file', this.selectMetadataFile);
    }

    return params;
  }

  onOK(): Observable<void> {
    return new Observable<void>((observer: Observer<void>) => {
      if (this.formGroup.invalid) {
        return;
      }

      const params = this.getParams();

      if (this.rowData) {
        set(params, 'uuid', this.rowData.uuid);
        this.ssoConfigService.updateSsoConfig(params as any).subscribe(
          res => {
            observer.next();
            observer.complete();
          },
          err => {
            observer.error(err);
            observer.complete();
          }
        );
      } else {
        this.ssoConfigService.createSsoConfig(params as any).subscribe(
          res => {
            observer.next();
            observer.complete();
          },
          err => {
            observer.error(err);
            observer.complete();
          }
        );
      }
    });
  }
}
