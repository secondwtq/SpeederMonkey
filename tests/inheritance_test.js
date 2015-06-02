
var p = new parent();
print("p.a ", p.a);
print("p.b ", p.b);
print(p.func());
print(p.func_nonv());
print(p.func_parent());
// print(p.func_child());

a = { a: 1 };

print(p.data);
p.data = a;
print(p.data);
print(p.data.a);
print(p.data.b);
a.b = 2;
print(p.data.b);

call_data(p);

var c = new child();
print("c.a ", c.a);
print("c.b ", c.b);
print(c.func());
print(c.func_nonv());
print(c.func_parent());
print(c.func_child());

print('\n------------\n');

parent.prototype.func_js = function () {
    return 'I\'m a JS function , my value is ' + this.a + '.'; };

c.a = 2;

print(p.func());
print(c.func());
print(p.func_js());
print(c.func_js());

print();

print(c.itsthis().a);
c.data = 3;
print(c.data);
print(c.itsthis().data);
c.data = function () { print('haha ', this.a); };
c.data();
print(c.itsthis().func_js());

print('\n------------\n');
