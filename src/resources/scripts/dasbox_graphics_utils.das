require graphics
require dasbox
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


/////////////////////////////////////////////////////////////////////////////////////////////

var private images: array<Image> <- [{ Image[] create_image(1, 1, [{ uint[] 0xFFFFFFFF }]) }]
var private deleted_images: array<int>


struct ImageHandle
    index: int
    width: int
    height: int


def private setup_image(index: int; smooth: bool = false; use_mips: bool = false): ImageHandle
    if smooth
        images[index] |> set_image_smooth(true)
    if use_mips
        images[index] |> set_image_use_mipmap()
    var h: ImageHandle
    h.index = index
    h.width = images[index].width
    h.height = images[index].height
    return h

def private allocate_image(): int
    if empty(deleted_images)
        var img: Image
        images |> emplace(img)
        return length(images) - 1
    let res = deleted_images |> back()
    deleted_images |> pop()
    return res

def delete_managed_image(image_handle: ImageHandle)
    if (image_handle.index > 0 && images[image_handle.index].valid)
        deleted_images |> push(image_handle.index)
        delete images[image_handle.index]

def clone_managed_collection(image_handle: ImageHandle): ImageHandle
    var i = allocate_image()
    images[i] := images[image_handle.index]
    return setup_image(i)

def create_managed_image(file_name: string; smooth: bool = false; use_mips: bool = false): ImageHandle
    var i = allocate_image()
    images[i] <- create_image(file_name)
    return setup_image(i, smooth, use_mips)

def create_managed_image(width, height: int; smooth: bool = false; use_mips: bool = false): ImageHandle
    var i = allocate_image()
    images[i] <- create_image(width, height)
    return setup_image(i, smooth, use_mips)

def create_managed_image(width, height: int; pixels: array<uint> &; smooth: bool = false; use_mips: bool = false): ImageHandle
    var i = allocate_image()
    images[i] <- create_image(width, height, pixels)
    return setup_image(i, smooth, use_mips)

def create_managed_image(width, height: int; pixels: string; palette): ImageHandle
    var i = allocate_image()
    images[i] <- create_image(width, height, pixels, palette)
    return setup_image(i, smooth, use_mips)

def draw_image_transformed(image_handle: ImageHandle; x, y, texture_rect, color, size, angle, relative_pivot_x, relative_pivot_y)
    images[image_handle.index] |> draw_image_transformed(x, y, texture_rect, color, size, angle, relative_pivot_x, relative_pivot_y)

def draw_image_transformed(image_handle: ImageHandle; x, y, texture_rect, color, size, angle)
    images[image_handle.index] |> draw_image_transformed(x, y, texture_rect, color, size, angle)

def draw_image(image_handle: ImageHandle; x, y)
    images[image_handle.index] |> draw_image(x, y)

def draw_image(image_handle: ImageHandle; x, y, color)
    images[image_handle.index] |> draw_image(x, y, color)

def draw_image(image_handle: ImageHandle; x, y, color, size)
    images[image_handle.index] |> draw_image(x, y, color, size)

def draw_image_region(image_handle: ImageHandle; x, y, texture_rect)
    images[image_handle.index] |> draw_image_region(x, y, texture_rect)

def draw_image_region(image_handle: ImageHandle; x, y, texture_rect, color)
    images[image_handle.index] |> draw_image_region(x, y, texture_rect)

def draw_image_region(image_handle: ImageHandle; x, y, texture_rect, color, size)
    images[image_handle.index] |> draw_image_region(x, y, texture_rect)

def draw_quad(image_handle: ImageHandle; p0, p1, p2, p3)
    images[image_handle.index] |> draw_quad(p0, p1, p2, p3)

def draw_quad(image_handle: ImageHandle; p0, p1, p2, p3, color)
    images[image_handle.index] |> draw_quad(p0, p1, p2, p3, color)

def draw_triangle_strip(image_handle: ImageHandle; coords, texture_coords)
    images[image_handle.index] |> draw_triangle_strip(coords, texture_coords)

def draw_triangle_strip(image_handle: ImageHandle; coords, texture_coords, colors)
    images[image_handle.index] |> draw_triangle_strip(coords, texture_coords, colors)

def set_pixel(image_handle: ImageHandle; x, y, color)
    images[image_handle.index] |> set_pixel(x, y, color)

def get_pixel(image_handle: ImageHandle; x, y)
    return images[image_handle.index] |> get_pixel(x, y)

def set_image_data(image_handle: ImageHandle; data)
    return images[image_handle.index] |> set_image_data(data)

def get_image_data(image_handle: ImageHandle; data)
    return images[image_handle.index] |> get_image_data(data)

def premultiply_alpha(image_handle: ImageHandle)
    images[image_handle.index] |> premultiply_alpha()

def make_image_color_transparent(image_handle: ImageHandle; color)
    images[image_handle.index] |> make_image_color_transparent(color)

def set_image_smooth(image_handle: ImageHandle; is_smooth)
    images[image_handle.index] |> set_image_smooth(is_smooth)

def set_image_clamp(image_handle: ImageHandle; is_clamp)
    images[image_handle.index] |> set_image_clamp(is_clamp)

def set_image_use_mipmap(image_handle: ImageHandle)
    images[image_handle.index] |> set_image_use_mipmap()

def flip_image_x(image_handle: ImageHandle)
    images[image_handle.index] |> flip_image_x()

def flip_image_y(image_handle: ImageHandle)
    images[image_handle.index] |> flip_image_y()

