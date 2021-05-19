from setuptools import setup


setup(
    name="albert-python-api",
    version="0.4",
    description="module interface for Albert launcher",
    url="https://albertlauncher.github.io/",
    license="Free To Use With Restrictions",
    classifiers=[
        "License :: Free To Use But Restricted",
        "Development Status :: 3 - Alpha",
    ],
    packages=["albert"],
    package_data={
        "albert": ["__init__.pyi"],
    },
)
