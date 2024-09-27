const fs = require('fs');
const path = require('path');
const enObject = {};
const zhObject = {};
const enExcludedList = {};
const zhExcludedList = {};
const enErrorCodeList = {};
const zhErrorCodeList = {};

function readFiles(_path_, object, excludedObject, errorCodeObject) {
  fs.readdirSync(_path_).map(file => {
    let filePath = `${_path_}/${file}`;

    if (fs.lstatSync(filePath).isDirectory()) {
      readFiles(filePath, object, excludedObject, errorCodeObject);
    } else {
      const suffixName = path.extname(file).substring(1);
      if (suffixName === 'json') {
        // 排除错误码告警等非界面词条
        if (
          !filePath.includes('error-code') &&
          !filePath.includes('alarm') &&
          !filePath.includes('operation') &&
          !filePath.includes('task') &&
          !filePath.includes('params')
        ) {
          Object.assign(object, JSON.parse(fs.readFileSync(filePath)));
        }
      }
    }
  });
}

readFiles('./src/assets/i18n/en-us', enObject, enExcludedList, enErrorCodeList);
readFiles('./src/assets/i18n/zh-cn', zhObject, zhExcludedList, zhErrorCodeList);

// 1. 检查值相同，Key不同的情况？去掉该检查项
const result1 = {};
const result2 = {};

// 2. 中文和英文对比，两边的国际化key的数量必须相同
const result3 = {};
(enKeys = Object.keys(enObject)), (zhKeys = Object.keys(zhObject));
for (const key in enObject) {
  if (!zhKeys.includes(key)) {
    result3[key] = enObject[key];
  }
}

for (const key in zhObject) {
  if (!enKeys.includes(key)) {
    result3[key] = zhObject[key];
  }
}

// 3. 国际化key定义不能包含.并且必须以_label结尾的小写字符串
const result4 = {},
  keySet = new Set(),
  reg = /^[^\.A-Z]+_label$/,
  excludeSet = [
    'OM_Admin',
    'OM_Admin_dsc',
    'multi_cluster_start_sync_data',
    'multi_cluster_end_sync_data',
    'cluster_sync_success',
    'cluster_sync_fail',
    'multi_cluster_sync_result'
  ];
for (const key in enObject) {
  if (
    !reg.test(key) &&
    !excludeSet.includes(key) &&
    !Object.keys(enExcludedList).includes(key)
  ) {
    keySet.add(key);
  }
}

for (const key in zhObject) {
  if (
    !reg.test(key) &&
    !excludeSet.includes(key) &&
    !Object.keys(zhExcludedList).includes(key)
  ) {
    keySet.add(key);
  }
}

for (const k of keySet) {
  result4[k] = '--';
}

const result5 = {};
trimExcludeList = {
  protection_total_physical_capacity_label: ' ',
  protection_every_label: ' ',
  protection_backup_one_label: ' ',
  protection_archive_one_label: ' ',
  protection_replicate_one_label: ' ',
  protection_and_manual_backup_label: ' and manually backs up the resource'
};
for (const key in enObject) {
  const val1 = enObject[key];
  const val2 = enObject[key].trim();
  if (val1 !== val2 && !Object.keys(trimExcludeList).includes(key.trim())) {
    result5[key] = enObject[key];
  }
}

if (!!Object.keys(result1).length || !!Object.keys(result2).length) {
  console.log(
    '错误1: 存在值相同，Key不同的数据，具体详情请查看"repeat-i18n.json"文件内容。'
  );
}

if (!!Object.keys(result3).length) {
  console.log(
    '错误2: 中文和英文对比发现，两者的国际化信息中存在key不一致的情况，具体详情请查看"repeat-i18n.json"文件内容。'
  );
}

if (!!Object.keys(result4).length) {
  console.log(
    '错误3: 国际化key定义不能包含.并且必须以_label结尾的小写字符串，具体详情请查看"repeat-i18n.json"文件内容。'
  );
}

if (!!Object.keys(result5).length) {
  console.log(
    '错误4: 英文翻译开始和结尾不能够包含空格，具体详情请查看"repeat-i18n.json"文件内容。'
  );
}

fs.writeFileSync(
  './repeat-i18n.json',
  JSON.stringify(Object.assign(result1, result2, result3, result4, result5))
);
