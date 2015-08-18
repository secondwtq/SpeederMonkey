/**
 * Created by secondwtq on 8/5/15.
 */

print('Test started.');
print('Test name: test_intrusive_obj');
print();

print('Create an intrusive object and a non-intrusive object.');
var intrusive = new TestIntrusiveObject();
var non_intrusive = new TestIntrusiveObjectForControl();
print('Intrusive.externalData (with builtin function) - ', intrusive.getExternalData());

print('Set the internal/external data of the intrusive to (1, 2), and non_intrusive to (3, 4).');
intrusive.internalData = 1;
intrusive.externalData = 2;
non_intrusive.internalData = 3;
non_intrusive.externalData = 4;
print('Print it out:');
print('Intrusive - ', intrusive.internalData, ' ', intrusive.externalData, ' ', intrusive);
print('Intrusive.externalData (with builtin function) - ', intrusive.getExternalData());
print('Non-intrusive - ', non_intrusive.internalData, ' ', non_intrusive.externalData, ' ', non_intrusive);

print();

print('Pass around the to object to native code.');
var p_intrusive = passAroundTestIntrusive(intrusive);
var p_non_intrusive = passAroundTestIntrusiveControl(non_intrusive);
print('Then print it out:');
print('Intrusive - ', p_intrusive.internalData, ' ', p_intrusive.externalData, ' ', p_intrusive);
print('Intrusive.externalData (with builtin function) - ', intrusive.getExternalData());
print('Non-intrusive - ', p_non_intrusive.internalData, ' ', p_non_intrusive.externalData, ' ', p_non_intrusive);
print();

print('Test for memory issues.');
print('Create an instrusive object intr, and a non-instrusive object ntr.');
var intr = new TestIntrusiveObject();
var non_intr = new TestIntrusiveObjectForControl();
print('Set them to undefined and GC.');
intr = undefined;
non_intr = undefined;
collectgarbage();

print();
print('Test end.');
