/**
 * Created by secondwtq on 7/13/15.
 */

print("vx_test: ", vx_test);

var t1 = new vx_test(233);

print("nullptr objref pointer: ", t1.objref);
t1.objref = t1;
print("non-null objref pointer: ", t1.objref);

print("Passing vx_test to ptr: ", t1.test_func_objptr(0, t1));
print("Passing undefined to ptr: ", t1.test_func_objptr(0, undefined));


