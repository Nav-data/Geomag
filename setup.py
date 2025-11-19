"""Setup script for the geomag package."""

from setuptools import setup, Extension
from setuptools.command.build_ext import build_ext
import sys
import platform
import subprocess
from pathlib import Path
import shutil


class MakeExtension(build_ext):
    """Custom build extension that uses Make to compile the C library."""

    def run(self):
        """Build the C library using Make."""
        try:
            # Clean only our library build artifacts, not the entire build directory
            lib_name = self._get_library_name()
            lib_path = Path('build') / lib_name
            obj_path = Path('build') / 'geomag.o'
            static_lib = Path('build') / 'libgeomag.a'
            example = Path('build') / 'example'
            
            # Remove only our specific build artifacts
            for path in [lib_path, obj_path, static_lib, example]:
                if path.exists():
                    path.unlink()
            
            # Run make to build the shared library
            subprocess.check_call(['make'])
        except subprocess.CalledProcessError as e:
            sys.stderr.write(f"Error building C library: {e}\n")
            sys.exit(1)

        # Copy the shared library to the package directory
        lib_name = self._get_library_name()
        src_lib = Path('build') / lib_name
        dst_lib = Path('geomag_c') / lib_name

        if not src_lib.exists():
            sys.stderr.write(f"Library not found: {src_lib}\n")
            sys.exit(1)

        # Copy the library
        shutil.copy2(src_lib, dst_lib)
        print(f"Copied {src_lib} to {dst_lib}")

        # Continue with the normal build
        super().run()

    @staticmethod
    def _get_library_name():
        """Get the platform-specific library name."""
        system = platform.system()
        if system == 'Darwin':  # macOS
            return 'libgeomag.dylib'
        elif system == 'Windows':
            return 'geomag.dll'
        else:  # Linux and others
            return 'libgeomag.so'


# Read the README for the long description
with open('README.md', 'r', encoding='utf-8') as f:
    long_description = f.read()

setup(
    name='geomag-c',
    version='1.0.0',
    author='Justin',
    author_email='epa6643@gmail.com',
    description='High-performance World Magnetic Model (WMM) calculator with C backend',
    long_description=long_description,
    long_description_content_type='text/markdown',
    url='https://github.com/Nav-data/Geomag',
    packages=['geomag_c'],
    python_requires='>=3.7',
    classifiers=[
        'Development Status :: 4 - Beta',
        'Intended Audience :: Developers',
        'Intended Audience :: Science/Research',
        'Programming Language :: C',
        'Programming Language :: Python :: 3',
        'Programming Language :: Python :: 3.7',
        'Programming Language :: Python :: 3.8',
        'Programming Language :: Python :: 3.9',
        'Programming Language :: Python :: 3.10',
        'Programming Language :: Python :: 3.11',
        'Programming Language :: Python :: 3.12',
        'Topic :: Scientific/Engineering :: GIS',
        'Topic :: Scientific/Engineering :: Physics',
    ],
    keywords='magnetic declination wmm navigation compass gis',
    license='MIT',
    cmdclass={'build_ext': MakeExtension},
    # Include the data files
    package_data={
        'geomag_c': [
            '*.so',
            '*.dylib',
            '*.dll',
        ],
    },
    include_package_data=True,
    # We don't actually build a Python extension, but we need this to trigger build_ext
    ext_modules=[Extension('geomag_c._dummy', sources=['dummy.c'])],
    zip_safe=False,
)
