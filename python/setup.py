from distutils.core import setup, Extension
import numpy.distutils.misc_util

setup(
    ext_modules=[Extension("_noise", ["_noise.c"], include_dirs=["../"], library_dirs=["../"], libraries=["noise"])],
    include_dirs=numpy.distutils.misc_util.get_numpy_include_dirs(),
)
