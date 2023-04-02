from __future__ import annotations
from typing import TYPE_CHECKING
if TYPE_CHECKING:
    from typing import Literal, Callable

lit: Literal[''] = ''
lit1 : Literal["""":" """ # comment
               ] = """":" """
s1 = 'a'
l2: list[str] = [lit]
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

""""a" : a = a"""

"""":" : a = a"""

s:str="""":" : a = a"""
s:str=""": : a = a"""
s:str=""": : "a" = a"""

def string(s: Literal["""":" : a = a"""] = """":" : a = a""") \
        -> Literal['a : a = a']:
    return "a : a = a"

def function_not_to_ignore(a: int, b: set[str], c: memoryview) \
    -> None | dict[tuple, set[int]] \
    :
        return None or {(1, 2): {1, 2}}

def function_not_to_ignore_2() \
    -> \
        Literal["""":" : : """]:
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

a: list[int] = [1] [:]

f1 = lambda m: m
f2=lambda x:x*x

num: int = 10
match num:
    case 10:
        pass
    case 20:
        pass
    case 30:
        pass

with open("example_file.py") as f:
    print(f.newlines)

def fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff() -> None:
    return

def ignored_ignored_ignored_ignored_ignored_ignored_ignored_ignored_ignored_ignored_ignored_ignored_function() -> Callable[..., Literal[1234567890]]:
    return lambda x: 1234567890

def true_func\
                \
                    \
                        \
                            \
                                \
    (a: dict) -> Literal[True]:
    return True

if true_func({1: 1}):
    print("Indeed")

foo_clone: Callable[..., Literal[1]] = foo

class Foo:
    def __init__(self, arg1: int, arg2: int, arg3: int,arg4: dict[str, str],arg5:str,arg6:str) -> None:
        pass

def complex_func(
    a: Foo = Foo(
        arg1=38,
        arg2=42, ###
        arg3=308,
        arg4={"""":" """: """":" """}, # some comment
        arg5="abc",#other comment " """ '' "" #''"': ::;" :';: :#;; ":;L""; ; ""\##
        \
        \
        arg6="""":" """
    )
) -> Foo:
    return a