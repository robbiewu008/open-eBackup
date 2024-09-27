const Converter = require('api-spec-converter');
const path = require('path');
const fs = require('fs');
convert();

function convert() {
  const array = getNeedConvertYaml();

  array.forEach(file => {
    const _path_ = '../json/' + file;
    Converter.convert({
      from: 'swagger_2',
      to: 'swagger_2',
      source: _path_
    }).then(function(converted) {
      converted.fillMissing();

      return converted.validate().then(function(result) {
        fs.writeFileSync(
          '../json/' + `${file.split('.').shift()}.json`,
          converted.stringify()
        );
      });
    });
  });
}

function getNeedConvertYaml() {
  return fs.readdirSync(path.resolve(__dirname, '../json')).filter(file => {
    return file.split('.').pop() === 'yaml';
  });
}
