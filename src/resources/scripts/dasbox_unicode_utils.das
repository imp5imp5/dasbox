options indenting = 4

def convert_utf8_code_to_chars(ch: uint): array<uint8>
    var res: array<uint8>

    if ch == 0u
        return <- res

    if ch <= 0x7F
        res |> push(uint8(ch))
        return <- res

    if ch <= 0x7FF
        res |> push(uint8((ch >> 6u) | 0xC0))
        res |> push(uint8((ch & 0x3F) | 0x80))
        return <- res

    if ch <= 0xFFFF
        res |> push(uint8((ch >> 12u) | 0xE0))
        res |> push(uint8(((ch >> 6u) & 0x3F) | 0x80))
        res |> push(uint8(((ch >> 0u) & 0x3F) | 0x80))
        return <- res

    if ch <= 0x1FFFFF
        res |> push(uint8((ch >> 18u) | 0xF0))
        res |> push(uint8(((ch >> 12u) & 0x3F) | 0x80))
        res |> push(uint8(((ch >>  6u) & 0x3F) | 0x80))
        res |> push(uint8(((ch >>  0u) & 0x3F) | 0x80))
        return <- res

    if ch <= 0x3FFFFFF
        res |> push(uint8((ch >> 24u) | 0xF8))
        res |> push(uint8(((ch >> 18u) & 0x3F) | 0x80))
        res |> push(uint8(((ch >> 12u) & 0x3F) | 0x80))
        res |> push(uint8(((ch >>  6u) & 0x3F) | 0x80))
        res |> push(uint8(((ch >>  0u) & 0x3F) | 0x80))
        return <- res;

    if ch <= 0x7FFFFFFF
        res |> push(uint8((ch >> 30u) | 0xFC))
        res |> push(uint8(((ch >> 24u) & 0x3F) | 0x80))
        res |> push(uint8(((ch >> 18u) & 0x3F) | 0x80))
        res |> push(uint8(((ch >> 12u) & 0x3F) | 0x80))
        res |> push(uint8(((ch >>  6u) & 0x3F) | 0x80))
        res |> push(uint8(((ch >>  0u) & 0x3F) | 0x80))
        return <- res

    return <- res


def is_first_byte_of_utf8_char(c: uint8): bool
    let x = uint(c)
    return (x > 0u) && (x < 128u || (x & 0xC0) == 0xC0)
