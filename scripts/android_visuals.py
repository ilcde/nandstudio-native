"""Reject blank Android captures; this does not certify visual correctness."""
import io
from PIL import Image, ImageChops


def pin_pixels_changed(before, after, old_rect, new_rect, scale):
    """Compare the text interior of one pin, excluding its border and system UI."""
    if any(abs(old_rect[key]-new_rect[key])*scale>0.5 for key in ('x','y','width','height')):
        raise ValueError('Pin moved during evaluation; cannot certify its changed pixels')
    def crop(png, rect):
        with Image.open(io.BytesIO(png)) as image:
            box = tuple(round(value * scale) for value in (
                rect['x'] + 8, rect['y'] + 8,
                rect['x'] + rect['width'] - 8, rect['y'] + rect['height'] - 8))
            if box[0] < 0 or box[1] < 0 or box[2] > image.width or box[3] > image.height or box[2] <= box[0] or box[3] <= box[1]:
                raise ValueError('Pin text region is outside the capture')
            return image.convert('RGB').crop(box)
    old, new = crop(before, old_rect), crop(after, new_rect)
    if old.size != new.size:
        raise ValueError('Pin text region changed size during evaluation')
    # Ignore small rasterization noise, but require a real changed glyph region.
    diff = ImageChops.difference(old, new).convert('L')
    return sum(value > 40 for value in diff.getdata()) >= 8


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
