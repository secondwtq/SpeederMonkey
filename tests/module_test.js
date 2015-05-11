var path = require('path');

print(process.execPath);
var mt = require('mt');
print('Let\'s say we have global a = 10.');
a = 10;
var os = require('os');
var mt2 = require('mt2');
var fs = require('fs');
print();

print('module mt: ', mt);
mt.export_func_1();
mt.export_func_2();
print();

print('my platform is mt.export_func_3() -> ', mt.export_func_3());
print('mt modifies a to 15, now a = ', a);

var localexp = require('test_mod_localexp');
print(localexp.resolve);

print(fs.readdirSync('.'));