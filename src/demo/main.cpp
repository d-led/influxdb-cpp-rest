#include <influxdb_raw_db_utf8.h>
#include <influxdb_simple_api.h>
#include <influxdb_line.h>
#include <influxdb_simple_async_api.h>

#include <iostream>
#include <thread>
#include <chrono>
#include <time.h>
#include <unistd.h>
#include <sys/time.h>

using namespace influxdb::api;
using namespace std;

int main(int argc, char *argv[]) {
	try {
		int keyNum = 0;

		influxdb::async_api::simple_db async_api("http://localhost:8086", "bluewhale");

		struct timeval begin;
		gettimeofday(&begin, NULL);

		while (1) {
			struct timeval tv,td;
			gettimeofday(&tv, NULL);
			long time = (long long) (tv.tv_sec) * 1000000000 + tv.tv_usec * 1000;

			influxdb::api::key_value_pairs tags;
			influxdb::api::key_value_pairs fields;
			string g_app_name = "test";
			string rankID = "1";
			std::cout << "key num: " << keyNum << std::endl;
			tags.add("appName", g_app_name);
			tags.add("svrID", rankID);
			fields.add("value", keyNum);
			async_api.insert(line("t_lr_key", tags, fields));

			gettimeofday(&td, NULL);
			std::cout << "cost time " << (td.tv_sec - tv.tv_sec) * 1000000 + (td.tv_usec - tv.tv_usec) << std::endl;

			std::cout << "current time " << (td.tv_sec - begin.tv_sec) / 60 << std::endl << std::endl;
			keyNum++;
			sleep(1);
		}

	}
	catch (std::exception const &e) {
		std::cerr << e.what() << std::endl;
	}

	return 0;
}
