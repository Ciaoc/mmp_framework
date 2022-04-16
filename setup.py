import os
from setuptools import find_packages, setup


# Utility function to read the README file.
# Used for the long_description.  It's nice, because now 1) we have a top level
# README file and 2) it's easier to type in the README file than to put a raw
# string in below ...
def read(fname):
    return open(os.path.join(os.path.dirname(__file__), fname)).read()


setup(
    name='mmp_framework',
    packages=find_packages(),
    package_data={
        # If any package contains *.cpp or *.h files, include them:
        '': ['*.cpp', '*.h'],
    },
    options={"bdist_wheel": {"universal": True}},
    install_requires=['cppyy', 'numpy'],
    version='0.1.0',
    description='Simple (CL-)Interface for the FAST GLOBAL OPTIMIZATION Framework from'
                'Bho Matthiesen, Christoph Hellings, Eduard A. Jorswieck, and Wolfgang Utschick:'
                '"Mixed Monotonic Programming for Fast Global Optimization," '
                ' submitted to IEEE Transactions on Signal Processing.'
                'Github source: '
                'https://github.com/bmatthiesen/mixed-monotonic',
    author='Luca Kunz',
    license=read('LICENSE'),
    long_description=read('README.md'),
)
