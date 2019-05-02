include 'premake'

make_solution 'influxdb-cpp-rest'

pic "On"

includedirs {
	'deps/fmt/include',
	'deps/rxcpp/Rx/v2/src/rxcpp',
	'src/influxdb-cpp-rest',
	'src/influxdb-c-rest',
}

filter 'system:linux'
		defines {
			-- '_GLIBCXX_USE_CXX11_ABI=0',
		}
filter {}

--linuxbrew
local cpprestsdk_root_mac = '~/.linuxbrew/Cellar/cpprestsdk/2.9.1'

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
	--
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
		defines {
			-- '_GLIBCXX_USE_CXX11_ABI=0',
		}
		links {
			'fmt',
			'ssl',
			'crypto',
			'boost_random',
			'boost_chrono',
			'boost_thread',
			'boost_system',
			'boost_regex',
			'boost_filesystem',
			'cpprest',
			'pthread',
		}
	filter {}
end

--------------------------------------------------------------------
make_static_lib('fmt', {
	'deps/fmt/src/**.cc',
})

use_standard('c++14')

--------------------------------------------------------------------
make_static_lib('influxdb-cpp-rest', {
	'src/influxdb-cpp-rest/**.*',
})

use_standard('c++14')


--------------------------------------------------------------------
make_shared_lib('influx-c-rest', {
	'src/influx-c-rest/**.*'
})

defines {
	'BUILDING_INFLUX_C_REST'
}

use_standard('c++14')

links { 'influxdb-cpp-rest' }

default_links()


--------------------------------------------------------------------
make_console_app('demo', {
	'src/demo/**.*'
})

use_standard('c++14')

links { 'influxdb-cpp-rest' }

default_links()


--------------------------------------------------------------------
make_console_app('test-influxdb-c-rest', {
	'src/test-shared/**.*'
})

includedirs {
	'deps/catch/single_include'
}

use_standard('c++14')

links { 'influx-c-rest' }

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

--osx: influxd -config ./src/auth_test/influxdb.conf

--------------------------------------------------------------------
make_console_app('test-influxdb-cpp-auth', {
	'src/auth_test/**.*',
	'src/test/fixtures.*',
})

includedirs {
	'deps/catch/single_include',
	'src/test',
}

use_standard('c++14')

links {
	'influxdb-cpp-rest',
	'influx-c-rest'
}

default_links()



--osx: influxd -config /usr/local/etc/influxdb.conf