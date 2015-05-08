a = 1 + 1;
print("a = ", a);

// print(vx_test);

var obj = new vx_test(1);
print("obj.test: ", obj.test);

obj.test_func(2);
print("obj.test: ", obj.test);

var obj2 = new vx_test(3);

obj2.test_func(4);
obj.test = 5;
print("obj.test: ", obj.test);

obj.test_func(6);
print("obj2.test: ", obj2.test);

obj.objref = obj2;
print(obj.objref.test);