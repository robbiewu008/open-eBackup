import { Component, Input, OnInit } from '@angular/core';
import {
  AbstractControl,
  FormControl,
  FormGroup,
  ValidatorFn
} from '@angular/forms';
import { OptionItem } from '@iux/live';
import { DataMap } from 'app/shared/consts';
import {
  BaseUtilService,
  DataMapService,
  I18NService
} from 'app/shared/services';
import {
  each,
  filter,
  find,
  first,
  includes,
  isEmpty,
  map,
  trim,
  union,
  uniq
} from 'lodash';
import { Subject } from 'rxjs';

@Component({
  selector: 'aui-protect-filter',
  templateUrl: './protect-filter.component.html',
  styleUrls: ['./protect-filter.component.less']
})
export class ProtectFilterComponent implements OnInit {
  @Input() formGroup: FormGroup;
  @Input() valid$: Subject<boolean>;
  @Input() resType: string;

  _includes = includes;
  filterLabel: string;
  filterType: string;
  typeOptions: OptionItem[] = this.dataMapService
    .toArray('resourceFilterType')
    .filter(item => (item.isLeaf = true));
  dataMap = DataMap;

  filterErrorTip = {
    ...this.baseUtilService.requiredErrorTip,
    maxLengthError: this.i18n.get('common_invalid_input_label'),
    filterError: this.i18n.get('protection_stateful_name_invalid_label'),
    sameFilterError: this.i18n.get('common_same_filter_error_label')
  };

  constructor(
    private i18n: I18NService,
    private dataMapService: DataMapService,
    private baseUtilService: BaseUtilService
  ) {}

  ngOnInit(): void {
    this.initForm();
  }

  initForm() {
    if (this.resType === DataMap.Resource_Type.KubernetesNamespace.value) {
      this.filterLabel = this.i18n.get('protection_statefulset_filter_label');
      this.filterType = this.i18n.get('protection_statefulset_label');
    } else if (
      [
        DataMap.Resource_Type.fusionComputeCNA.value,
        DataMap.Resource_Type.fusionComputeCluster.value,
        DataMap.Resource_Type.hostSystem.value,
        DataMap.Resource_Type.clusterComputeResource.value
      ].includes(this.resType)
    ) {
      this.filterLabel = this.i18n.get('protection_vm_filter_label');
      this.filterType = this.i18n.get('common_virtual_machine_label');
    } else if (
      includes(
        [
          DataMap.Resource_Type.APSResourceSet.value,
          DataMap.Resource_Type.APSZone.value,
          DataMap.Resource_Type.Project.value,
          DataMap.Resource_Type.openStackProject.value
        ],
        this.resType
      )
    ) {
      this.filterLabel = this.i18n.get(
        'protection_hcs_cloud_host_filter_label'
      );
      this.filterType = this.i18n.get('common_cloud_server_label');
    } else if (
      [
        DataMap.Resource_Type.tidbCluster.value,
        DataMap.Resource_Type.tidbDatabase.value
      ].includes(this.resType)
    ) {
      this.filterLabel = this.i18n.get('protection_tidb_filter_table_label');
    } else {
      this.filterLabel = this.i18n.get('protection_vm_filter_label');
      this.filterType = this.i18n.get('common_virtual_machine_label');
    }
    if (this.formGroup) {
      this.formGroup.addControl('enableFilter', new FormControl(false));
      this.formGroup.addControl(
        'type',
        new FormControl(DataMap.resourceFilterType.mix.value)
      );
      this.formGroup.addControl('include', new FormControl(''));
      this.formGroup.addControl('andExclude', new FormControl(''));
      this.formGroup.addControl('exclude', new FormControl(''));
      this.watchForm();
    }
  }

  typeVaildChange(res) {
    if (res === DataMap.resourceFilterType.include.value) {
      this.formGroup
        .get('include')
        .setValidators([
          this.baseUtilService.VALID.required(),
          this.validFilters()
        ]);
      this.formGroup.get('exclude').clearValidators();
      this.formGroup.get('andExclude').clearValidators();
    } else if (res === DataMap.resourceFilterType.exclude.value) {
      this.formGroup
        .get('exclude')
        .setValidators([
          this.baseUtilService.VALID.required(),
          this.validFilters()
        ]);
      this.formGroup.get('include').clearValidators();
      this.formGroup.get('andExclude').clearValidators();
    } else {
      this.formGroup
        .get('include')
        .setValidators([
          this.baseUtilService.VALID.required(),
          this.validFilters()
        ]);
      this.formGroup
        .get('andExclude')
        .setValidators([
          this.baseUtilService.VALID.required(),
          this.validFilters()
        ]);
      this.formGroup.get('exclude').clearValidators();
    }
    this.formGroup.get('include').updateValueAndValidity();
    this.formGroup.get('exclude').updateValueAndValidity();
    this.formGroup.get('andExclude').updateValueAndValidity();
  }

  watchForm() {
    this.formGroup.get('enableFilter').valueChanges.subscribe(res => {
      if (res) {
        this.typeVaildChange(this.formGroup.value.type);
      } else {
        this.formGroup.get('include').clearValidators();
        this.formGroup.get('exclude').clearValidators();
        this.formGroup.get('andExclude').clearValidators();
        this.formGroup.get('include').updateValueAndValidity();
        this.formGroup.get('exclude').updateValueAndValidity();
        this.formGroup.get('andExclude').updateValueAndValidity();
      }
    });
    this.formGroup
      .get('type')
      .valueChanges.subscribe(res => this.typeVaildChange(res));
    this.formGroup.statusChanges.subscribe(() => {
      this.valid$.next(this.formGroup.valid);
    });
  }

  validFilters(): ValidatorFn {
    return (control: AbstractControl): { [key: string]: any } | null => {
      if (!trim(control.value)) {
        return null;
      }
      const paths = control.value.split(',')?.filter(item => {
        return !isEmpty(item);
      });
      if (paths.length !== uniq(paths).length) {
        return { sameFilterError: { value: control.value } };
      }
      // K8S才有下面的限制条件
      if (this.resType !== DataMap.Resource_Type.KubernetesNamespace.value) {
        return null;
      }
      if (
        find(paths, path => {
          return path.length > 128;
        })
      ) {
        return { maxLengthError: { value: control.value } };
      }
      if (
        find(paths, path => {
          return /[\u4e00-\u9fa5]+/.test(path);
        })
      ) {
        return { filterError: { value: control.value } };
      }

      return null;
    };
  }

  fixValueByRule(item, value) {
    return {
      START_WITH: value + '*',
      END_WITH: '*' + value,
      FUZZY: '*' + value + '*',
      ALL: value
    }[item.rule];
  }

  parseFilters(vmFilters, type, tags) {
    const filterList = [
      { rule: 'START_WITH', values: [] },
      { rule: 'END_WITH', values: [] },
      { rule: 'FUZZY', values: [] },
      { rule: 'ALL', values: [] }
    ];
    if (tags.length) {
      each(tags, item => {
        const fuzzyCacheLabel = item.slice(1, -1);
        const endCacheLabel = item.slice(0, -1);
        const startCacheLabel = item.slice(1);
        if (fuzzyCacheLabel && item.startsWith('*') && item.endsWith('*')) {
          const filter = filterList.find(item => item.rule === 'FUZZY');
          filter.values.push(fuzzyCacheLabel);
        } else if (startCacheLabel && item.startsWith('*')) {
          const filter = filterList.find(item => item.rule === 'END_WITH');
          filter.values.push(startCacheLabel);
        } else if (endCacheLabel && item.endsWith('*')) {
          const filter = filterList.find(item => item.rule === 'START_WITH');
          filter.values.push(endCacheLabel);
        } else {
          const filter = filterList.find(item => item.rule === 'ALL');
          filter.values.push(item);
        }
      });
      filterList.forEach(item => {
        if (item.values.length) {
          vmFilters.push({
            filter_by: 'NAME',
            type: 'VM',
            rule: item.rule,
            mode: type,
            values: item.values
          });
        }
      });
    }
  }

  getAllFilters() {
    const vmFilters = [];
    if (!this.formGroup.value.enableFilter) {
      return vmFilters;
    }
    if (
      this.formGroup.value.type === DataMap.resourceFilterType.include.value
    ) {
      this.parseFilters(
        vmFilters,
        DataMap.resourceFilterType.include.value,
        this.formGroup.value.include.split(',')
      );
    } else if (
      this.formGroup.value.type === DataMap.resourceFilterType.exclude.value
    ) {
      this.parseFilters(
        vmFilters,
        DataMap.resourceFilterType.exclude.value,
        this.formGroup.value.exclude.split(',')
      );
    } else {
      this.parseFilters(
        vmFilters,
        DataMap.resourceFilterType.include.value,
        this.formGroup.value.include.split(',')
      );
      this.parseFilters(
        vmFilters,
        DataMap.resourceFilterType.exclude.value,
        this.formGroup.value.andExclude.split(',')
      );
    }

    return vmFilters;
  }

  toFilterString(filters): string {
    let unionFilter = [];
    each(filters, item => {
      unionFilter = union(
        unionFilter,
        map(item.values, value => this.fixValueByRule(item, value))
      );
    });
    return unionFilter.join(',');
  }

  setFilter(filters) {
    if (!this.formGroup) {
      return;
    }
    this.formGroup.get('enableFilter').setValue(true, { emitEvent: false });
    if (
      find(filters, { mode: DataMap.resourceFilterType.include.value }) &&
      find(filters, { mode: DataMap.resourceFilterType.exclude.value })
    ) {
      this.formGroup.get('type').setValue(DataMap.resourceFilterType.mix.value);
      this.formGroup
        .get('include')
        .setValue(
          this.toFilterString(
            filter(
              filters,
              item => item.mode === DataMap.resourceFilterType.include.value
            )
          )
        );
      this.formGroup
        .get('andExclude')
        .setValue(
          this.toFilterString(
            filter(
              filters,
              item => item.mode === DataMap.resourceFilterType.exclude.value
            )
          )
        );
    } else {
      this.formGroup.get('type').setValue(first(filters)['mode']);
      if (first(filters)['mode'] === DataMap.resourceFilterType.include.value) {
        this.formGroup.get('include').setValue(this.toFilterString(filters));
      } else {
        this.formGroup.get('exclude').setValue(this.toFilterString(filters));
      }
    }
  }
}
