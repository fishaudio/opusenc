from pybind11.setup_helpers import Pybind11Extension, build_ext
from setuptools import setup

__version__ = "0.0.1"


ext_modules = [
    Pybind11Extension(
        "opusenc",
        ["src/main.cpp"],
        define_macros=[("VERSION_INFO", __version__)],
        include_dirs=[
            "/usr/include/opus",
            "/usr/include/opusenc",
        ],
        library_dirs=["/usr/lib/x86_64-linux-gnu"],
        libraries=["opus", "opusenc"],
    ),
]

setup(
    name="opusenc",
    version=__version__,
    author="Fish Audio",
    author_email="lengyue@fish.audio",
    url="https://github.com/fishaudio/opusenc",
    description="OPUS wrapper that supports streaming",
    long_description="",
    ext_modules=ext_modules,
    extras_require={"test": "pytest"},
    # Currently, build_ext only provides an optional "highest supported C++
    # level" feature, but in the future it may provide more features.
    cmdclass={"build_ext": build_ext},
    zip_safe=False,
    python_requires=">=3.7",
)
