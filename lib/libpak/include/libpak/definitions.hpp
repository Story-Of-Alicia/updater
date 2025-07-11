//
// Created by maros on 15.3.2023.
//

#ifndef LIBPAK_DEFINITIONS_HPP
#define LIBPAK_DEFINITIONS_HPP

#include <cstdint>
#include <cstring>
#include <memory>
#include <vector>
#include <string>

namespace libpak
{

#pragma pack(push, 1)

static constexpr size_t PAK_CONTENT_SECTOR = 0x7D000;
static constexpr size_t PAK_DATA_SECTOR = 0xF00000;

/**
 * Represents pak header.
 */
struct pak_header
{
  uint32_t header_magic{};
  uint32_t unknown0{};
  uint32_t paklib_version{};
  uint32_t locale{};

  uint32_t assets_count{};
  uint32_t used_assets_count{};
  uint32_t deleted_assets_count{};

  uint32_t file_size{};
  uint32_t team_version{};
  uint32_t header_sign{};
};

/**
 * Represents content header.
 */
struct content_header
{
  uint32_t first_magic{};  // FILS
  uint32_t second_magic{}; // FILZ

  uint32_t assets_count{};
};

/**
 * Represents asset header.
 */
struct asset_header
{
  uint32_t asset_prefix{};
  uint32_t asset_magic{};

  uint32_t embedded_data_offset{};
  uint32_t embedded_data_length{};

  uint32_t data_decompressed_length{};
  uint32_t is_data_compressed{};
  uint32_t data_decompressed_length0{};
  uint32_t unknown0{};
  uint32_t data_decompressed_length1{};

  uint32_t unknown1{};
  uint32_t unknown2{};
  uint32_t unknown3{};
  uint32_t unknown4{};
  uint32_t unknown5{};
  uint32_t is_asset_deleted{};
  uint32_t asset_offset{};
  uint32_t is_asset_embedded{};

  uint32_t unknown_type_l{};
  uint32_t unknown_type_h{};
  uint32_t unknown_value_l{};
  uint32_t unknown_value_h{};

  uint32_t crc_decompressed{};
  uint32_t crc_embedded{};
  uint32_t crc_identity{};
  uint32_t checksum_decompressed{};
  uint32_t checksum_embedded{};
  uint32_t unknown6{};
  char16_t path[256]{};
};

/**
 * Represents data header.
 */
struct data_header
{
  uint32_t magic{0x454C4946}; // ASCII: FILE
};

/**
 * Represents asset data.
 */
struct asset_data
{
  std::vector<std::byte> buffer;
};

/**
 * Represents asset.
 */
struct asset
{
  /**
   * Asset header
   */
  asset_header header{};

  /**
   * Asset data
   */
  asset_data data{};

  /**
   * @return String view of the asset path.
   */
  [[nodiscard]] std::string path() const {
    char path[512] {};
    for (unsigned index = 0, ptr = 0; index < std::size(header.path); ++index) {
      if (header.path[index] == '\0')
        break;
      int const size = wctomb(&path[ptr], header.path[index]);
      ptr += size;
    }
    return {path};
  }

  /**
   * Mark the asset as patched.
   */
  void markAsPatched() { this->patched = true; }

private:
  bool patched = false;
};

#pragma pack(pop)

} // namespace libpak

#endif // LIBPAK_DEFINITIONS_HPP
