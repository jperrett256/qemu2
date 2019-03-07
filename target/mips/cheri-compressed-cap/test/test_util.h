#include <ostream>
#include <iostream>
#include <string>

std::ostream& operator<<( std::ostream& os, unsigned __int128 value);

std::ostream& operator<<( std::ostream& os, unsigned __int128 value) {
    os << "{" << (uint64_t)(value >> 64) << "," << (uint64_t)(value) << "}";
    return os;
}

#include "catch.hpp"

static bool failed = false;

template <typename T> static bool check(T expected, T actual, const std::string& msg) {
    if (expected == actual)
        return true;
    std::cerr << "ERROR: " << msg << ": expected 0x" << std::hex << expected << " != 0x" << actual << "\n";
    failed = true;
    return false;
}

template <class T, std::size_t N>
constexpr inline size_t array_lengthof(T (&)[N]) {
  return N;
}

static const char* otype_suffix(uint32_t otype) {
    // Two separate switches since if the number of otype bits is the same
    // we cannot have both of the case statements in one switch
    switch(otype) {
    case CC128_OTYPE_UNSEALED: return " (CC128_OTYPE_UNSEALED)";
    case CC128_OTYPE_SENTRY: return " (CC128_OTYPE_SENTRY)";
    case CC128_OTYPE_RESERVED2: return " (CC128_OTYPE_RESERVED2)";
    case CC128_OTYPE_RESERVED3: return " (CC128_OTYPE_RESERVED3)";
    default: break;
    }
    switch(otype) {
    case CC256_OTYPE_UNSEALED: return " (CC256_OTYPE_UNSEALED)";
    case CC256_OTYPE_SENTRY: return " (CC256_OTYPE_SENTRY)";
    case CC256_OTYPE_RESERVED2: return " (CC256_OTYPE_RESERVED2)";
    case CC256_OTYPE_RESERVED3: return " (CC256_OTYPE_RESERVED3)";
    default: return "";
    }
}

static void dump_cap_fields(const cap_register_t& result) {
    fprintf(stderr, "Permissions: 0x%" PRIx32 "\n", result.cr_perms); // TODO: decode perms
    fprintf(stderr, "User Perms:  0x%" PRIx32 "\n", result.cr_uperms);
    fprintf(stderr, "Base:        0x%016" PRIx64 "\n", result.cr_base);
    fprintf(stderr, "Offset:      0x%016" PRIx64 "\n", result.cr_offset);
    fprintf(stderr, "Length:      0x%" PRIx64 "%016" PRIx64 " %s\n",
            (uint64_t)(result._cr_length >> 64), (uint64_t)result._cr_length,
            result._cr_length > UINT64_MAX ? " (greater than UINT64_MAX)": "");
    unsigned __int128 top_full = result.cr_base + result._cr_length;
    fprintf(stderr, "Top:         0x%" PRIx64 "%016" PRIx64 " %s\n",
            (uint64_t)(top_full >> 64), (uint64_t)top_full,
            top_full > UINT64_MAX ? " (greater than UINT64_MAX)": "");
    fprintf(stderr, "Sealed:      %d\n", (int)cc128_is_cap_sealed(&result));
    fprintf(stderr, "OType:       0x%" PRIx32 "%s\n", result.cr_otype, otype_suffix(result.cr_otype));
    fprintf(stderr, "\n");
}

__attribute__((used)) static cap_register_t decompress_representable(uint64_t pesbt, uint64_t cursor) {
    cap_register_t result;
    printf("Decompressing pesbt = %016" PRIx64 ", cursor = %016" PRIx64 "\n", pesbt, cursor);
    decompress_128cap(pesbt, cursor, &result);
    dump_cap_fields(result);
    // Check that the result is the same again when compressed
    uint64_t new_pesbt = compress_128cap(&result);
    check(pesbt, new_pesbt, "recompressing resulted in different pesbt");
    check(cursor, result.cr_base + result.cr_offset, "recompressing resulted in different cursor");
    return result;
}

#define DO_STRINGIFY2(x) #x
#define DO_STRINGIFY1(x) DO_STRINGIFY2(x)
#define STRINGIFY(x) DO_STRINGIFY1(x)

#define CHECK_FIELD_RAW(value, expected) CHECK(value == expected)
#define CHECK_FIELD(cap, field, expected) CHECK((uint64_t)expected == (uint64_t)cap.cr_##field)
