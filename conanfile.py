from conans import ConanFile, CMake

class ConanPackage(ConanFile):
    name = 'network_monitor'
    version = "0.1.0"

    generators = ['cmake_find_package']

    requires = [
        'boost/1.79.0',
        'openssl/1.1.1h',
        "nlohmann_json/3.11.2",
    ]

    default_options = (
        'boost:shared=False',
    )