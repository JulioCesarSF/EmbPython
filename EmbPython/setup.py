from distutils.core import setup
from distutils.core import Extension
MOD = "myapi"
module = Extension(MOD, sources =['python_context.cpp'])
setup(name = MOD, ext_modules = [module])
