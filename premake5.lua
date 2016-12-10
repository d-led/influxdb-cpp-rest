include 'premake'

make_solution 'influxdb-cpp-rest'

includedirs {
	'deps/fmt'
}

filter 'system:macosx'
	includedirs {
		'/usr/local/include', -- brew install boost cpprestsdk openssl
		'/usr/local/opt/openssl/include',
	}
	libdirs {
		'/usr/local/lib',
		'/usr/local/opt/openssl/lib',
	}
filter {}

--------------------------------------------------------------------
make_static_lib('influxdb-cpp-rest', {
	'src/influxdb-cpp-rest/**.*',
})

use_standard('c++11')

--------------------------------------------------------------------
make_console_app('demo', {
	'src/demo/**.*'
})

use_standard('c++11')

links { 'influxdb-cpp-rest' }

--------------------------------------------------------------------
make_console_app('test-influxdb-cpp-rest', {
	'src/test/**.*'
})

includedirs {
	'deps/catch/single_include'
}

use_standard('c++11')

--osx: influxd -config /usr/local/etc/influxdb.conf