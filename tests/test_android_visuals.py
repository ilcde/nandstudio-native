import io
import pathlib
import sys
import unittest
from PIL import Image, ImageDraw
sys.path.insert(0, str(pathlib.Path(__file__).resolve().parents[1] / 'scripts'))
from android_visuals import has_rendered_content, pin_pixels_changed


def png(image):
    output = io.BytesIO()
    image.save(output, format='PNG')
    return output.getvalue()


class AndroidVisuals(unittest.TestCase):
    def test_stale_output_and_unrelated_pixels_fail(self):
        rect=dict(x=80,y=180,width=80,height=44)
        image=Image.new('RGB',(300,600),'#182330')
        draw=ImageDraw.Draw(image);draw.text((95,192),'0',fill='white')
        before=png(image)
        self.assertFalse(pin_pixels_changed(before,before,rect,rect,1))
        draw.rectangle((0,0,200,30),fill='white')
        draw.rectangle((0,70,100,110),fill='blue')
        self.assertFalse(pin_pixels_changed(before,png(image),rect,rect,1))
        draw.rectangle((90,188,120,208),fill='#182330')
        draw.text((95,192),'1',fill='white')
        self.assertTrue(pin_pixels_changed(before,png(image),rect,rect,1))
        with self.assertRaises(ValueError):
            pin_pixels_changed(before,png(image),rect,dict(rect,y=190),1)

    def test_black_and_flat_surfaces_fail(self):
        for color in ('black', 'white', '#182330'):
            self.assertFalse(has_rendered_content(png(Image.new('RGB', (300, 600), color))))

    def test_system_bars_cannot_mask_black_app(self):
        image = Image.new('RGB', (300, 600), 'black')
        draw = ImageDraw.Draw(image)
        draw.rectangle((0, 0, 299, 30), fill='white')
        draw.rectangle((0, 570, 299, 599), fill='white')
        self.assertFalse(has_rendered_content(png(image)))

    def test_content_has_dynamic_range(self):
        image = Image.new('RGB', (300, 600), '#182330')
        draw = ImageDraw.Draw(image)
        for n in range(32):
            draw.line((40+n, 120, 40+n, 320), fill=(n*7, n*5, n*3))
        self.assertTrue(has_rendered_content(png(image)))


if __name__ == '__main__':
    unittest.main()
