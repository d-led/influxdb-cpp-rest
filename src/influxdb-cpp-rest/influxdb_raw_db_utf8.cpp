#include "influxdb_raw_db_utf8.h"
#include "influxdb_raw_db.h"

#include <cpprest/http_client.h>

using namespace utility;

struct influxdb::raw::db_utf8::impl {
    db db_utf16;

public:
    impl(std::string const& url) :db_utf16(conversions::utf8_to_utf16(url))
    {}
};

influxdb::raw::db_utf8::db_utf8(std::string const & url) :
    pimpl(std::make_unique<impl>(url))
{
}

influxdb::raw::db_utf8::~db_utf8()
{
}

void influxdb::raw::db_utf8::post(std::string const& query) {
    pimpl->db_utf16.post(conversions::utf8_to_utf16(query));
}

std::string influxdb::raw::db_utf8::get(std::string const& query) {
    return conversions::utf16_to_utf8(
        pimpl->db_utf16.get(
            conversions::utf8_to_utf16(query)
        )
    );
}

void influxdb::raw::db_utf8::insert(std::string const & db, std::string const & lines) {
    pimpl->db_utf16.insert(conversions::utf8_to_utf16(db), conversions::utf8_to_utf16(lines));
}
