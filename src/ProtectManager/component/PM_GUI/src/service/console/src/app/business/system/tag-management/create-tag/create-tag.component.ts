import { Component, Input } from '@angular/core';
import { FormBuilder, FormControl, FormGroup } from '@angular/forms';
import { BaseUtilService, I18NService, LabelApiService } from 'app/shared';
import { Observable, Observer } from 'rxjs';

@Component({
  selector: 'aui-create-tag',
  templateUrl: './create-tag.component.html',
  styleUrls: ['./create-tag.component.less']
})
export class CreateTagComponent {
  formGroup: FormGroup;
  @Input() data;

  constructor(
    private i18n: I18NService,
    public fb: FormBuilder,
    private labelApiService: LabelApiService,
    public baseUtilService: BaseUtilService
  ) {}

  ngOnInit(): void {
    this.initForm();
  }

  initForm() {
    this.formGroup = this.fb.group({
      tagName: new FormControl(this.data ? this.data.name : '', {
        validators: [
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.name(),
          this.baseUtilService.VALID.maxLength(64)
        ]
      })
    });
  }

  onOk(): Observable<void> {
    return new Observable<void>((observer: Observer<void>) => {
      if (this.data) {
        this.labelApiService
          .modifyLabelUsingPUT({
            CreateOrUpdateLabelRequest: {
              uuid: this.data.uuid,
              name: this.formGroup.get('tagName').value
            }
          })
          .subscribe(
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
        this.labelApiService
          .createLabelUsingPOST({
            CreateOrUpdateLabelRequest: {
              name: this.formGroup.get('tagName').value
            }
          })
          .subscribe(
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
