from __future__ import annotations
from typing import TYPE_CHECKING
if TYPE_CHECKING:
    from typing import Literal, Callable, Dict

lit = ''
lit1                  = """":" """
s1 = 'a'
l2 = [lit]
l1 = ['a', 'b']
st1 = {'a', "b"}
d1 =\
    {
    0: {
        0: 0
    },
    1: {1:1}
}
d2 ={
    0: {
        0: 0
    },
    1: {1:1}
}

def \
    foo(a, b)\
        :
    return 1

def \
    g(a, b)   \
                               \
                                   \
        :
    return {0: {}}


def h(x = {0: 0}, w = {1: 1}, y= {2: 2}):
    return x

""""a" : a = a"""

"""":" : a = a"""

s="""":" : a = a"""
s=""": : a = a"""
s=""": : "a" = a"""

def string(s= """":" : a = a""") \
         :
    return "a : a = a"

def function_not_to_ignore(a, b, c) \
         \
    :
        return None or {(1, 2): {1, 2}}

def function_not_to_ignore_2() \
     \
        :
    return """":" : : """

def function_to_ignore(a: int, b: set[str], c: memoryview) \
    -> None | dict[tuple, set[int]] \
    :
        return None or {(1, 2): {1, 2}}

def function_to_ignore_2() \
    -> \
        Literal["""":" : : """]:
    return """":" : : """

try\
    :
    raise Exception
except:
    pass
else:
    memoryview(Exception())

for\
i in range(10)\
:
    pass

a = [1] [:]

f1 = lambda m: m
f2=lambda x:x*x

num = 10
match num:
    case 10:
        pass
    case 20:
        pass
    case 30:
        pass

with open("example_file.py") as f:
    print(f.newlines)

def fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff()  :
    return

def ignored_ignored_ignored_ignored_ignored_ignored_ignored_ignored_ignored_ignored_ignored_ignored_function() -> Callable[..., Literal[1234567890]]:
    return lambda x: 1234567890

def true_func\
                \
                    \
                        \
                            \
                                \
    (a)  :
    return True

if true_func({1: 1}):
    print("Indeed")

foo_clone  = foo

class Foo:
    def __init__(self, arg1, arg2, arg3,arg4,arg5,arg6)  :
        pass

f3 = Foo(arg1=10, arg2=11, arg3=12, arg4={}, arg5="abc", arg6="def")

def complex_func(
    a= Foo(
        arg1=38,
        arg2=42, ###
        arg3=308,
        arg4={"""":" """: """":" """}, # some comment
        arg5="abc",#other comment " """ '' "" #''"': ::;" :';: :#;; ":;L""; ; ""\##
        \
        \
        arg6="""":" """
    )
)  :
    return a

lst1 = [2 for i in range(10)]
st2 = {2 for i in range(10) if i % 50 == 0}
lst2 = [2 for i in range(10)]
st3 = {2 for i in range(10) if i % 40 == 0 and (b := i % 3)}
st4 = {2 for _ in range(10)}

uninit_var1: int

uninit_var2: \
    dict[
    int, int \
        \
] | dict[int, int]