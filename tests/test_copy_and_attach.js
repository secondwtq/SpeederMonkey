/**
 * Created by secondwtq on 15-8-19.
 */

print('Test started.');
print('Test name: copy and attach');
print();

print('Create a CopyTestCopied object cpt.');
var cpt = new CopyTestCopied();

print('Deriving CopyTestDerived from CopyTest, add function newFunction()')
function CopyTestDerived () { }
CopyTestDerived.prototype = CopyTest.reproto(CopyTest.prototype);
CopyTestDerived.prototype.newFunction = function () {
    print("I'm the new function!"); };
CopyTestDerived.prototype.constructor = CopyTestDerived;

print("Create an object e with prototype of CopyTestDerived.");
var e = CopyTest.reproto(CopyTestDerived.prototype);
print("Attach a new CopyTest object created with cpt to e.");
CopyTest.attachNew.call(e, cpt);
print("e: ", e, ", e.constructor: ", e.constructor,
    ", e.prototype: ", e.prototype, ", e.__proto__: ", e.__proto__,
    ", e instanceof CopyTest ", e instanceof CopyTest, ", e instance of CopyTestDerived ", e instanceof CopyTestDerived);
print('Calling e.newFunction()');
e.newFunction();
print();

print('Calling e.testCopy(cpt)');
e.testCopy(cpt);
print();

print('Create a CopyTest object cp with cpt');
print('ctor prototype: CopyTest(CopyTestCopied)');
var cp = new CopyTest(cpt);
print("cp: ", cp, ", cp.constructor: ", cp.constructor,
    ", cp.prototype: ", cp.prototype, ", cp.__proto__: ", cp.__proto__,
    ", cp instanceof CopyTest ", cp instanceof CopyTest);
print();

print('Calling cp.testCopy(cpt)');
cp.testCopy(cpt);
print();

print("cp: ", cp, ", cp.constructor: ", cp.constructor,
    ", cp.prototype: ", cp.prototype, ", cp.__proto__: ", cp.__proto__,
    ", cp instanceof CopyTest ", cp instanceof CopyTest, ", cp instanceof CopyTestDerived ", cp instanceof CopyTestDerived);
print("cp.newFunction = ", cp.newFunction, ", e.newFunction = ", e.newFunction);

print();
print("Test end.");
