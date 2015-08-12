/**
 * Created by secondwtq on 15-8-16.
 */

print('Test started.');
print('Test name: test_static');

print('Creating shared vx_test variable a with 1208 and print ...');
var a = vx_test.createShared(1208);
print(a);
print('Setting a to undefined and GC ...');
a = undefined;
collectgarbage();
print();

print('Creating shared vx_test variable b with 1996 and print ...');
var b = vx_test.createShared(1996);
print(b);
print('Setting sharedGlobal to b ...');
vx_test.sharedGlobal = b;
print('Setting a to undefined and GC ...');
b = undefined;
collectgarbage();
print();

print('Calling getShared and print ...');
print(vx_test.getShared(), ' - ', vx_test.getShared().test);

print();
collectgarbage();
print('Test end.');