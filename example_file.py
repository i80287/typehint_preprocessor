from __future__ import annotations
from typing import TYPE_CHECKING
if TYPE_CHECKING:
    from typing import Literal

s: Literal[''] = ''
s1 = 'a'
l2: list[str] = [s]
l1: list[str] = ['a', 'b']
st1: set[str] = {'a', "b"}
d1:dict[int, dict[int, int]]=\
    {
    0: {
        0: 0
    },
    1: {1:1}
}
d2:dict[int, dict[int, int]]={
    0: {
        0: 0
    },
    1: {1:1}
}

def \
    foo(a: int, b: str)\
        :
    return 1

def \
    g(a: int, b: str) -> dict[int, \
                              dict[int, \
                                   int]]\
        :
    return {0: {}}


def h(x = {0: 0}, w = {1: 1}, y: dict[int, int] = {2: 2}):
    return x

s:str="a : a = a"

def string(s: Literal['''"a" : a = a'''] = "a : a = a"):
    return "a : a = a"

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

a: list[int] = [1] [:]