# -*- coding: gbk -*-
from collections import deque
from pathlib import Path

from PIL import Image


root = Path(__file__).resolve().parents[2]


maps = [
    {
        "id": 1,
        "name": "arms_factory",
        "file": "6007118D32747325657A7319694256D1.jpg",
        "crop": (55, 270, 1288, 1372),
        "long_edge_m": 160.0,
    },
    {
        "id": 2,
        "name": "sacred_heart_hospital",
        "file": "53FDD64D9D62B7045A5A2D1DCC9C36BA.jpg",
        "crop": (50, 245, 1282, 1435),
        "long_edge_m": 150.0,
    },
    {
        "id": 3,
        "name": "red_church",
        "file": "088A0EF1C3E93DB67DDF4BCFBF2B9452.jpg",
        "crop": (100, 220, 1262, 1430),
        "long_edge_m": 145.0,
    },
    {
        "id": 4,
        "name": "eversleeping_town",
        "file": "0C2DBB8832B12683A04CFA399F630268.jpg",
        "crop": (130, 270, 1210, 1630),
        "long_edge_m": 170.0,
    },
    {
        "id": 5,
        "name": "china_town",
        "file": "48A8206C540DD491F364E78C5E6BD285.jpg",
        "crop": (60, 310, 1265, 1410),
        "long_edge_m": 155.0,
    },
    {
        "id": 6,
        "name": "darkwoods",
        "file": "4AAF0BC0C2522B0E9313ABE763EAED26.jpg",
        "crop": (80, 290, 1260, 1425),
        "long_edge_m": 155.0,
    },
    {
        "id": 7,
        "name": "lakeside_village",
        "file": "27F8C4A00A2B7D7D5136CB06367DEA92.jpg",
        "crop": (85, 285, 1288, 1438),
        "long_edge_m": 190.0,
    },
    {
        "id": 8,
        "name": "moon_river_park",
        "file": "352ECF962A74C2B1709046B9B2062795.jpg",
        "crop": (60, 210, 1265, 928),
        "long_edge_m": 210.0,
    },
    {
        "id": 9,
        "name": "leo_memory",
        "file": "88D08FB9E3CFA9B9F0AA396B5EB12094.jpg",
        "crop": (60, 325, 1265, 1405),
        "long_edge_m": 170.0,
    },
]


grid_width = 90
grid_height = 90


def is_green(r, g, b):
    return g >= 110 and g > r + 35 and g > b + 8 and r < 170


def is_blue(r, g, b):
    return b >= 145 and g >= 120 and r <= 190 and b > r + 25 and g > r + 10


def is_colored_marker(r, g, b):
    if is_green(r, g, b) or is_blue(r, g, b):
        return True
    if r > 170 and b > 130 and g < 130:
        return True
    if r > 185 and g > 130 and b < 120:
        return True
    return False


def is_wall_pixel(r, g, b):
    gray = (r + g + b) / 3.0
    spread = max(r, g, b) - min(r, g, b)
    if is_colored_marker(r, g, b):
        return False
    return gray < 215 and spread < 55


def components(mask, min_area, max_area):
    height = len(mask)
    width = len(mask[0])
    seen = [[False] * width for _ in range(height)]
    result = []

    for y in range(height):
        for x in range(width):
            if not mask[y][x] or seen[y][x]:
                continue

            queue = deque([(x, y)])
            seen[y][x] = True
            area = 0
            left = right = x
            top = bottom = y

            while queue:
                cx, cy = queue.popleft()
                area += 1
                left = min(left, cx)
                right = max(right, cx)
                top = min(top, cy)
                bottom = max(bottom, cy)

                for nx, ny in ((cx + 1, cy), (cx - 1, cy), (cx, cy + 1), (cx, cy - 1)):
                    if 0 <= nx < width and 0 <= ny < height and mask[ny][nx] and not seen[ny][nx]:
                        seen[ny][nx] = True
                        queue.append((nx, ny))

            if min_area <= area <= max_area:
                result.append((left, top, right, bottom, area))
    return result


def normalized_segment(component, crop_width, crop_height):
    left, top, right, bottom, _ = component
    x1 = round(left * 1000 / crop_width)
    y1 = round(top * 1000 / crop_height)
    x2 = round((right + 1) * 1000 / crop_width)
    y2 = round((bottom + 1) * 1000 / crop_height)
    cx = (x1 + x2) // 2
    cy = (y1 + y2) // 2

    if (right - left) >= (bottom - top):
        return (x1, cy, x2, cy)
    return (cx, y1, cx, y2)


def real_size(crop_width, crop_height, long_edge_m):
    if crop_width >= crop_height:
        real_width = long_edge_m
        real_height = long_edge_m * crop_height / crop_width
    else:
        real_height = long_edge_m
        real_width = long_edge_m * crop_width / crop_height
    return real_width, real_height


def extract_map(data):
    image = Image.open(root / "data" / "map" / data["file"]).convert("RGB")
    crop = image.crop(data["crop"])
    pixels = crop.load()
    crop_width, crop_height = crop.size

    wall_cells = []
    green_mask = [[False] * crop_width for _ in range(crop_height)]
    blue_mask = [[False] * crop_width for _ in range(crop_height)]

    for gy in range(grid_height):
        for gx in range(grid_width):
            x_start = gx * crop_width // grid_width
            x_end = (gx + 1) * crop_width // grid_width
            y_start = gy * crop_height // grid_height
            y_end = (gy + 1) * crop_height // grid_height

            wall_count = 0
            total = max(1, (x_end - x_start) * (y_end - y_start))
            for y in range(y_start, y_end):
                for x in range(x_start, x_end):
                    r, g, b = pixels[x, y]
                    if is_wall_pixel(r, g, b):
                        wall_count += 1
                    if is_green(r, g, b):
                        green_mask[y][x] = True
                    if is_blue(r, g, b):
                        blue_mask[y][x] = True

            if wall_count / total >= 0.16:
                wall_cells.append(gy * grid_width + gx)

    pallets = [normalized_segment(item, crop_width, crop_height)
        for item in components(green_mask, 18, 2200)]
    windows = [normalized_segment(item, crop_width, crop_height)
        for item in components(blue_mask, 18, 2200)]

    real_width, real_height = real_size(crop_width, crop_height, data["long_edge_m"])
    return {
        "id": data["id"],
        "name": data["name"],
        "source": data["file"],
        "real_width": real_width,
        "real_height": real_height,
        "wall_cells": wall_cells,
        "windows": windows,
        "pallets": pallets,
    }


def write_vector(values, indent):
    if not values:
        return "{}"
    chunks = []
    line = " " * indent
    for value in values:
        text = str(value)
        if len(line) + len(text) + 2 > 110:
            chunks.append(line.rstrip())
            line = " " * indent
        line += text + ", "
    chunks.append(line.rstrip(", "))
    return "{\n" + "\n".join(chunks) + "\n" + " " * (indent - 4) + "}"


def write_segments(values, indent):
    if not values:
        return "{}"
    chunks = []
    line = " " * indent
    for x1, y1, x2, y2 in values:
        text = f"{{{x1}, {y1}, {x2}, {y2}}}"
        if len(line) + len(text) + 2 > 110:
            chunks.append(line.rstrip())
            line = " " * indent
        line += text + ", "
    chunks.append(line.rstrip(", "))
    return "{\n" + "\n".join(chunks) + "\n" + " " * (indent - 4) + "}"


def main():
    extracted = [extract_map(item) for item in maps]

    lines = [
        "#pragma once",
        "",
        '#include "game_map.h"',
        "",
        "#include <vector>",
        "",
        "// Generated by tools/image_recognition/generate_map_geometry.py.",
        "// Coordinates use 0-1000 relative map space. Walls are grid cells; windows and pallets are short segments.",
        "inline const std::vector<map_geometry_static_data> k_map_geometry_data = {",
    ]

    for item in extracted:
        lines.append("    {")
        lines.append(f"        {item['id']},")
        lines.append(f"        \"{item['source']}\",")
        lines.append(f"        {item['real_width']:.2f}, {item['real_height']:.2f},")
        lines.append(f"        {grid_width}, {grid_height},")
        lines.append("        " + write_vector(item["wall_cells"], 12) + ",")
        lines.append("        " + write_segments(item["windows"], 12) + ",")
        lines.append("        " + write_segments(item["pallets"], 12))
        lines.append("    },")

    lines.append("};")
    lines.append("")

    output = root / "include" / "map_geometry_data.h"
    output.write_text("\n".join(lines), encoding="utf-8")

    for item in extracted:
        print(item["id"], item["name"], "walls", len(item["wall_cells"]),
              "windows", len(item["windows"]), "pallets", len(item["pallets"]))


if __name__ == "__main__":
    main()


