var path = require('path');

if (process.argv.length <= 2) {
    console.error('not provide swagger json file');
    process.exit(1)
}
if (process.argv.length <= 3) {
    console.error('not provide output file');
    process.exit(1)
}
var file = process.argv[2];
// file = path.resolve(__dirname, file);
file = path.join(process.cwd(), file);
if (!/\.json$/.test(file)) {
    console.error('provide file path  json file');
    process.exit(1);
}
var fs = require('fs')
if (!fs.existsSync(file)) {
    console.error('provide file path is not exists: ' + file);
//   console.error('provide file path is not exists: ' + path.normalize(path.join(process.cwd(), file)));
  process.exit(1);
}

// 获取自定义参数配置
var customParameters = require('./custom-http-params.json');

var json = require(file);
var paths = json.paths;
for (var url in paths) {
    var methods = paths[url];
    for (var method in methods) {
        var parameters = methods[method].parameters;
        if (!parameters) {
            continue;
        }
        parameters.filter(function(parameter, key) {
            // TODO: remove Date https://github.com/google/shaka-player/issues/159
            if (parameter.name === 'Date') {
                parameters.splice(key, 1);
                console.log("Remove Date");
                return false;
            }
            if (parameter.name === 'X-Request-ID') {
                parameters.splice(key, 1);
                console.log("Remove X-Request-ID");
                return false;
            }

            // if (parameter.name === 'X-Auth-Token') {
            //   parameters.splice(key, 1);
            //   console.log('Remove X-Auth-Token');
            //   return false;
            // }

            return parameter.in == 'header';
        }).forEach(function(parameter) {
            console.log("Modify %s as optional. url: %s, method: %s", parameter.name, url, method);
            parameter.required = false;
        });
        // 每个接口支持注入器自定义参数
        parameters.push(...customParameters);
    }
}

var path = require('path');

fs.writeFile(process.argv[3], JSON.stringify(json, null, 2), 'utf8', function (err) {
    if (err) {
        console.error(err);
        return;
    }
    console.log("prepare done.")
});
