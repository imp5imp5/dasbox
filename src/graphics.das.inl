R"X(
options indenting = 4
require graphics
require math
require strings

def create_image(width, height: int; pixels: string; palette: table<int; uint>&): Image
    var p <- [{for c in pixels; palette?[c] ?? 0u}]
    return <- create_image(width, height, p)

def make_color(brightness: float): uint
    let ib = uint(saturate(brightness) * 255.0 + 0.5)
    return 0xFF000000 | ib | (ib << 8u) | (ib << 16u)

def make_color(brightness, alpha: float): uint
    let ib = uint(saturate(brightness) * 255.0 + 0.5)
    let ia = uint(saturate(alpha) * 255.0 + 0.5)
    return ib | (ib << 8u) | (ib << 16u) | (ia << 24u)

def make_color(r, g, b: float): uint
    let ir = uint(saturate(r) * 255.0 + 0.5)
    let ig = uint(saturate(g) * 255.0 + 0.5)
    let ib = uint(saturate(b) * 255.0 + 0.5)
    return 0xFF000000 | ib | (ig << 8u) | (ir << 16u)

def make_color(r, g, b, a: float): uint
    let ir = uint(saturate(r) * 255.0 + 0.5)
    let ig = uint(saturate(g) * 255.0 + 0.5)
    let ib = uint(saturate(b) * 255.0 + 0.5)
    let ia = uint(saturate(a) * 255.0 + 0.5)
    return ib | (ig << 8u) | (ir << 16u) | (ia << 24u)

def make_color(c: float3): uint
    let sc = saturate(c)
    let ir = uint(sc.x * 255.0 + 0.5)
    let ig = uint(sc.y * 255.0 + 0.5)
    let ib = uint(sc.z * 255.0 + 0.5)
    return 0xFF000000 | ib | (ig << 8u) | (ir << 16u)

def make_color(c: float4): uint
    let sc = saturate(c)
    let ir = uint(sc.x * 255.0 + 0.5)
    let ig = uint(sc.y * 255.0 + 0.5)
    let ib = uint(sc.z * 255.0 + 0.5)
    let ia = uint(sc.w * 255.0 + 0.5)
    return ib | (ig << 8u) | (ir << 16u) | (ia << 24u)

def make_color32(r, g, b: int): uint
    return 0xFF000000 | uint(b) | (uint(g) << 8u) | (uint(r) << 16u)

def make_color32(r, g, b, a: int): uint
    return uint(b) | (uint(g) << 8u) | (uint(r) << 16u) | (uint(a) << 24u)

def make_color32(r, g, b: uint): uint
    return 0xFF000000 | b | (g << 8u) | (r << 16u)

def make_color32(r, g, b, a: uint): uint
    return b | (g << 8u) | (r << 16u) | (a << 24u)

def premultiply_color(c1: uint): uint
    let a = (c1 >> 24u) & 0xFF
    let r = (c1 & 0xFF) * a / 255u
    let g = ((c1 >> 8u) & 0xFF) * a / 255u
    let b = ((c1 >> 16u) & 0xFF) * a / 255u
    return b | (g << 8u) | (r << 16u) | (a << 24u)

def desaturate_color(c1: uint): uint
    let a = (c1 >> 24u) & 0xFF
    let r = c1 & 0xFF
    let g = (c1 >> 8u) & 0xFF
    let b = (c1 >> 16u) & 0xFF
    let v = (r + g * 2u + b + 2u) >> 2u
    return v | (v << 8u) | (v << 16u) | (a << 24u)

def multiply_colors(c1, c2: uint): uint
    let b1 = c1 & 0xFF
    let g1 = (c1 >> 8u) & 0xFF
    let r1 = (c1 >> 16u) & 0xFF
    let a1 = (c1 >> 24u) & 0xFF
    let b2 = c2 & 0xFF
    let g2 = (c2 >> 8u) & 0xFF
    let r2 = (c2 >> 16u) & 0xFF
    let a2 = (c2 >> 24u) & 0xFF
    let r = r1 * r2 / 255u
    let g = g1 * g2 / 255u
    let b = b1 * b2 / 255u
    let a = a1 * a2 / 255u
    return b | (g << 8u) | (r << 16u) | (a << 24u)

def add_colors(c1, c2: uint): uint
    let b1 = c1 & 0xFF
    let g1 = (c1 >> 8u) & 0xFF
    let r1 = (c1 >> 16u) & 0xFF
    let a1 = (c1 >> 24u) & 0xFF
    let b2 = c2 & 0xFF
    let g2 = (c2 >> 8u) & 0xFF
    let r2 = (c2 >> 16u) & 0xFF
    let a2 = (c2 >> 24u) & 0xFF
    let r = min(r1 + r2, 255u)
    let g = min(g1 + g2, 255u)
    let b = min(b1 + b2, 255u)
    let a = min(a1 + a2, 255u)
    return b | (g << 8u) | (r << 16u) | (a << 24u)

)X"