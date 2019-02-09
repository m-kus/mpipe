import sys
from setuptools import setup, Distribution


class BinaryDistribution(Distribution):
    def has_ext_modules(foo):
        return True
        
        
if "win" in sys.platform:
    module_name = 'mpipe.pyd'
else:
    module_name = 'mpipe.so'

setup(
    name='mpipe',
    version='1.0.0',
    description='Pipeline trading library',
    author='Mikhail Zaikin, Artem Poltorzhicky',
    author_email='mz@darth.vc, ap@darth.vc',
    url='git@ec2-52-57-189-23.eu-central-1.compute.amazonaws.com:mpipe',
    packages=[''],
    package_data={'': [module_name]},
    distclass=BinaryDistribution
)
