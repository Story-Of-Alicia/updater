#include <map>
#include <libupdate/libupdate.hpp>

typedef struct {
   std::map<std::string_view, uint32_t> crc_map;
} manifest;


int main() {
   libupdate::update u{};
   u.initiate();
}