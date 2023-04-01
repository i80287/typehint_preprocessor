from __future__ import annotations
from typing import TYPE_CHECKING
if TYPE_CHECKING:
    from typing import Literal

s= ''
s1 = 'a'
l2= [s]
l1= ['a', 'b']
st1= {'a', "b"}
d1=\
    {
    0: {
        0: 0
    },
    1: {1:1}
}
d2={
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

a= [1] [:]

f = lambda m: m

num = 10
match num:
    case 10:
        pass
    case 20:
        pass
    case 30:
        pass