/**
 * Created by secondwtq on 8/3/15.
 */

print('Test started.');
print('Test name: test_value_wrap');

print('Creating TestClassValueWrap variable a');
var a = TestClassValueWrap.prototype.createShared();

print('Setting a.data to string \'Hello PersistentRooted\', print and GC');
a.data = 'Hello PersistentRooted';
a.print_data();
collectgarbage();

print('Setting a.data to object \'{ a : 1 }\' and print');
a.data = { a: 1 };
a.print_data();
print('Print the \'a\' property of a.data');
a.print_property('a');
print('Print the \'a\' property of a');
print(a.a);
print();

print('Set a.b to \'Foundation\'');
a.b = 'Foundation';
print('Print a.data.b and a.b');
a.print_property('b');
print(a.b);
print();

print('Setting a.c to another TestClassValueWrap, print and GC');
a.c = TestClassValueWrap.prototype.createShared();
a.print_property('c');
print(a.data.c);
print('Setting a.c to undefined, print and GC');
a.c = undefined;
print(a.c);
print(a.data.c);

print();
collectgarbage();
print('Test end.');
