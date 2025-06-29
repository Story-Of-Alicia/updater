#include <cassert>
#include <format>
#include <iostream>
#include <cuchar>

#include "libpak/libpak.hpp"

int main() {
    libpak::resource resource("res.pak");
    resource.read(false);

    FILE *f = fopen("res.pak.manifest", "w");

    for(auto& [path, asset] : resource.assets) {
        char buff[MB_LEN_MAX] = {};
        mbstate_t mbstate = {};
        // iterate over asset.header.path print it to file converting to ucode
        for (int i = 0; ;i++) {
            if (path[i] == 0)
                break;
            size_t const mb_size = c16rtomb(buff, asset.header.path[i], &mbstate);
            fwrite(buff, mb_size, 1, f);
        }

        fprintf(f, ":%8x\n", asset.header.crc_embedded);
        fflush(f);
    }
    fclose(f);
}