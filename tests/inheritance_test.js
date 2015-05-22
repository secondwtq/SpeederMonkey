
var p = parent();
print("p.a ", p.a);
print("p.b ", p.b);
print(p.func());
print(p.func_nonv());
print(p.func_parent());
// print(p.func_child());

var c = child();
print("c.a ", c.a);
print("c.b ", c.b);
print(c.func());
print(c.func_nonv());
print(c.func_parent());
print(c.func_child());