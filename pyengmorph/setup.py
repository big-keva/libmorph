from setuptools import setup, Extension
import pybind11
import os

libmorph_root = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))

ext_module = Extension(
    'pyengmorph',
    sources=['eng.cpp'],
    include_dirs=[
        pybind11.get_include(),
        libmorph_root,
    ],
    library_dirs=['/usr/local/lib'],
    libraries=['morpheng', 'fuzzyeng', 'moonycode'],
    language='c++',
    extra_compile_args=['-std=c++17'],
    extra_link_args=['-static-libstdc++'],
)

setup(
    ext_modules=[ext_module],
)
