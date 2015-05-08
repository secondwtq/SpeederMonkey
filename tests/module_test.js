
print('Let\'s say we have global a = 10.');
a = 10;
var mt = require('mt');
var os = require('os');
var mt2 = require('mt2');
print()

print('module mt: ', mt);
mt.export_func_1();
print();

print('my platform is mt.export_func_3() -> ', mt.export_func_3());
print('mt modifies a to 15, now a = ', a);