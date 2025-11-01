from conan import ConanFile
from conan.tools.cmake import CMake, cmake_layout
from conan.tools.files import copy


class InfluxdbCppRestConan(ConanFile):
    name = "influxdb-cpp-rest"
    version = "1.0.0"
    license = "MPL-2.0"
    author = "Dmitry Ledentsov"
    url = "https://github.com/d-led/influxdb-cpp-rest"
    description = "A C++ client library for InfluxDB using C++ REST SDK"
    topics = ("influxdb", "cpprest", "http", "client")
    settings = "os", "compiler", "build_type", "arch"
    options = {"shared": [True, False], "fPIC": [True, False]}
    default_options = {"shared": False, "fPIC": True}
    generators = "CMakeDeps", "CMakeToolchain"

    def config_options(self):
        if self.settings.os == "Windows":
            self.options.rm_safe("fPIC")

    def configure(self):
        if self.options.shared:
            self.options["cpprestsdk"].shared = True

    def requirements(self):
        self.requires("cpprestsdk/2.10.19")  # Latest stable version
        self.requires("rxcpp/4.1.1")  # Latest stable version
    
    def build_requirements(self):
        # Only for tests - not linked to the library
        self.test_requires("catch2/3.11.0")

    # Don't use cmake_layout when consuming dependencies - it creates nested build directories
    # def layout(self):
    #     cmake_layout(self)

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()

    def package(self):
        cmake = CMake(self)
        cmake.install()

