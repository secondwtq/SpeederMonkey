/**
 * Created by secondwtq on 15-8-7.
 */

print('Test started.');
print('Test name: test_enum');
print();

print('We have an enum declaration as the following:');
print('enum EnumTest { \n\
    Foundation, \n\
    Pressure, \n\
    Reliable \n\
};');
print();

print('Then we go through its corresponding object inside JS:');
print('EnumTest: ', EnumTest);
print('EnumTest.prototype: ', EnumTest.prototype);
var keys = [ 'Foundation', 'Pressure', 'Reliable', 0, 1, 2 ];
for (var i = 0; i < keys.length; i++) {
    print('EnumTest.', keys[i], '\t\t= ', EnumTest[keys[i]]); }

print();
print('Test ended.');
