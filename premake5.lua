include 'premake'

make_solution 'influxdb-cpp-rest'

includedirs {
	'deps/fmt',
	'deps/rxcpp/Rx/v2/src/rxcpp',
	'src/influxdb-cpp-rest'
}

--linuxbrew
local cpprestsdk_root = '~/.linuxbrew/Cellar/cpprestsdk/2.9.1'

filter 'system:macosx'
	includedirs {
		'/usr/local/include', -- brew install boost cpprestsdk openssl
		'/usr/local/opt/openssl/include',
	}
	libdirs {
		'/usr/local/lib',
		'/usr/local/opt/icu4c/lib',
		'/usr/local/opt/openssl/lib',
	}
filter 'system:linux' -- conan install .
	includedirs {
		cpprestsdk_root..'/include' --same as macosx, via linuxbrew: brew install gcc cmake cpprestsdk
	}
	libdirs {
		cpprestsdk_root..'/lib',
		'~/.linuxbrew/lib64',
	}
filter {}

function default_links()
	filter 'system:macosx'
		links {
			'ssl',
			'crypto',
			'cpprest',
			'fmt',
			'boost_thread-mt',
			'boost_system-mt',
			'boost_chrono',
		}
	filter 'system:linux'
		links {
			'fmt',
			'ssl',
			'crypto',
			'boost_random',
			'boost_chrono',
			'boost_thread-mt',
			'boost_system-mt',
			'boost_regex',
			'boost_filesystem',
			'cpprest',
			'pthread',
		}
	filter {}
end

--------------------------------------------------------------------
make_static_lib('fmt', {
	'deps/fmt/fmt/**.*',
})

use_standard('c++14')

--------------------------------------------------------------------
make_static_lib('influxdb-cpp-rest', {
	'src/influxdb-cpp-rest/**.*',
})

use_standard('c++14')

--------------------------------------------------------------------
make_console_app('demo', {
	'src/demo/**.*'
})

use_standard('c++14')

links { 'influxdb-cpp-rest' }

default_links()

--------------------------------------------------------------------
make_console_app('test-influxdb-cpp-rest', {
	'src/test/**.*'
})

includedirs {
	'deps/catch/single_include'
}

use_standard('c++14')

links { 'influxdb-cpp-rest' }

default_links()


--osx: influxd -config /usr/local/etc/influxdb.conf