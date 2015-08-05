/**
 * Created by secondwtq on 8/3/15.
 */

print('Test started.');
print('Test name: test_value_wrap');

print('Creating TestClassValueWrap variable a');
var a = TestClassValueWrap.prototype.createShared();

print('Setting a.data to object \'{ t : 1 }\'');
a.data = { t: 1 };
print('Set a.t to 2 and print');
print(a.data.t);
a.t = 2;
print(a.data.t);
print('Set a.t to 1 and print');
a.t = 1;
print(a.data.t);
print(a.t);

print();

print('Setting a.c to another TestClassValueWrap');
a.c = 1;
print('Setting a.c to undefined, print and GC');
a.c = undefined;
print(a.data.c);

print();
collectgarbage();
print('Test end.');
