from conan import ConanFile
from conan.tools.cmake import CMake, cmake_layout, CMakeDeps, CMakeToolchain


class InfluxdbCppRestConan(ConanFile):
    name = "influxdb-cpp-rest"
    version = "1.0.1"
    license = "MPL-2.0"
    author = "Dmitry Ledentsov"
    url = "https://github.com/d-led/influxdb-cpp-rest"
    description = "A C++ client library for InfluxDB using C++ REST SDK"
    topics = ("influxdb", "cpprest", "http", "client")
    settings = "os", "compiler", "build_type", "arch"
    options = {"shared": [True, False], "fPIC": [True, False]}
    default_options = {"shared": False, "fPIC": True}
    
    # Export source files needed for building
    exports_sources = "CMakeLists.txt", "src/*"

    def config_options(self):
        if self.settings.os == "Windows":
            self.options.rm_safe("fPIC")

    def configure(self):
        if self.options.shared:
            self.options["cpprestsdk"].shared = True

    def requirements(self):
        self.requires("cpprestsdk/2.10.19")
        self.requires("rxcpp/4.1.1")
    
    def build_requirements(self):
        # Only for tests - not linked to the library
        self.test_requires("catch2/3.11.0")

    def layout(self):
        cmake_layout(self)

    def generate(self):
        deps = CMakeDeps(self)
        deps.generate()
        tc = CMakeToolchain(self)
        tc.generate()

    def build(self):
        cmake = CMake(self)
        # Disable tests and demo for packaging
        cmake.configure(variables={"BUILD_TESTING": "OFF", "BUILD_DEMO": "OFF"})
        cmake.build()

    def package(self):
        cmake = CMake(self)
        cmake.install()

    def package_info(self):
        self.cpp_info.set_property("cmake_file_name", "influxdb-cpp-rest")
        self.cpp_info.set_property("cmake_target_name", "influxdb-cpp-rest::influxdb-cpp-rest")
        
        # Libraries to link
        self.cpp_info.libs = ["influxdb-cpp-rest"]
        
        # Include directories
        self.cpp_info.includedirs = ["include"]
        
        # System dependencies (if any)
        if self.settings.os == "Linux":
            self.cpp_info.system_libs = ["pthread"]

