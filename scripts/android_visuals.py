"""Reject blank Android captures; this does not certify visual correctness."""
import io
from PIL import Image


def has_rendered_content(png):
    with Image.open(io.BytesIO(png)) as image:
        image.load()
        width, height = image.size
        if width < 100 or height < 100:
            return False
        # Exclude system bars: their clock/icons can disguise a black app surface.
        content = image.convert('RGB').crop((0, height // 10, width, height * 9 // 10))
        if max(hi - lo for lo, hi in content.getextrema()) < 12:
            return False
        return content.getcolors(maxcolors=16) is None
