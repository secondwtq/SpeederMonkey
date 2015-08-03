/**
 * Created by secondwtq on 8/3/15.
 */

print('Test started.');
print('Test name: test_value_wrap');
print('Createing TestClassValueWrap variable a');
var a = new TestClassValueWrap();
print('Setting a.data to string \'Hello PersistentRooted\' and print');
a.data = 'Hello PersistentRooted';
a.print_data();
print('Setting a.data to object \'{ a : 1 }\' and print');
a.data = { a: 1 };
a.print_data();
print('Print the \'a\' property of a.data');
a.print_property('a');
print('Print the \'a\' property of a');
print(a.a);
print('Set a.b to \'Foundation\'');
a.b = 'Foundation';
print('Print a.data.b and a.b');
a.print_property('b');
print(a.b);
print('Setting a.data to another TestClassValueWrap, print and GC');
a.data = new TestClassValueWrap();
a.print_data();
print('Setting a.data to undefined, print and GC');
a.data = undefined;
a.print_data();
collectgarbage();
print('Test end.');
