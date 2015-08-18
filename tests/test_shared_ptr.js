/**
 * Created by secondwtq on 8/2/15.
 */

print('Test started.');
print('Test name: shared_ptr_test');
print('Creating shared 1 with number 16');
var shared_1 = vx_test.createShared(16);
print('Creating shared 2 with number 32 and GC');
var shared_2 = vx_test.createShared(32);
collectgarbage();
print('Setting global shared to shared 2');
vx_test.setShared(shared_2);
collectgarbage();
print('Set shared 1 to undefined and GC');
shared_1 = undefined;
collectgarbage();
print('Test end.');
