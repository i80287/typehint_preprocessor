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


def h(x = {0: 0}):
    return x

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