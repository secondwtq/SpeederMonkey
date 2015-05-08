
print("Test Print function.");

var a = new vx_test(65535);
print("a.test ", a.test);
test_funbind_void();
a.test = 5;
print("a.test ", a.test);

print(test_funbind_objptr(6, a));

var b = new vx_test(7);
print(a.test_func_objptr(8, b));
