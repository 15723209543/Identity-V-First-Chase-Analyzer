# -*- coding: gbk -*-
"""兼容入口：实际图片识别脚本已经移动到 tools/image_recognition。"""

from pathlib import Path
import runpy


script = Path(__file__).resolve().parent / "image_recognition" / "generate_map_geometry.py"
runpy.run_path(str(script), run_name="__main__")


