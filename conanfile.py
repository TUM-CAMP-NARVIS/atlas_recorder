from conans import ConanFile, CMake


class ExportMKVConan(ConanFile):
    name = "export_mkv_k4a"
    version = "0.1.0"

    description = "export_mkv_k4a"
    url = "https://github.com/TUM-CAMP-NARVIS/export_mkv_k4a"
    license = "GPL"

    short_paths = True
    settings = "os", "compiler", "build_type", "arch"
    generators = "cmake", "virtualrunenv"

    requires = (
        "kinect-azure-sensor-sdk/1.4.1@camposs/stable",
        "bzip2/1.0.8@conan/stable",
        "Boost/1.75.0@camposs/stable"
         )

    # all sources are deployed with the package
    exports_sources = "cmake/*", "include/*", "src/*", "CMakeLists.txt"

    def configure(self):
        if self.settings.os == "Windows":
            raise Exception("Windows not currently supported.")

    def imports(self):
        self.copy(src="bin", pattern="*.dll", dst="./bin") # Copies all dll files from packages bin folder to my "bin" folder
        self.copy(src="lib", pattern="*.dll", dst="./bin") # Copies all dll files from packages bin folder to my "bin" folder
        self.copy(src="lib", pattern="*.dylib*", dst="./lib") # Copies all dylib files from packages lib folder to my "lib" folder
        self.copy(src="lib", pattern="*.so*", dst="./lib") # Copies all so files from packages lib folder to my "lib" folder
        self.copy(src="lib", pattern="*.a", dst="./lib") # Copies all static libraries from packages lib folder to my "lib" folder
        self.copy(src="bin", pattern="*", dst="./bin") # Copies all applications

    def _cmake_configure(self):
        cmake = CMake(self)
        cmake.verbose = True
        cmake.configure()
        return cmake

    def build(self):
        cmake = self._cmake_configure()
        cmake.build()

    def package(self):
        cmake = self._cmake_configure()
        cmake.install()

    def package_info(self):
        self.cpp_info.libs = ['atlas_recorder']
