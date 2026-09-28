"""Assemble unretouched Qt application captures into a stepped demonstration.

Requires Pillow 12.0.0 for documentation generation only. This does not invoke
the simulator or synthesize UI state: frames come from NAND_DEMO_CAPTURE.
"""
import argparse
from pathlib import Path
from PIL import Image

parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument('captures', type=Path)
parser.add_argument('output', type=Path)
args = parser.parse_args()
frames = []
for index in range(4):
    with Image.open(args.captures / f'eval-frame-{index}.png') as source:
        frames.append(source.convert('RGB'))
if len({frame.size for frame in frames}) != 1:
    raise ValueError('All real application frames must have the same dimensions')
args.output.parent.mkdir(parents=True, exist_ok=True)
frames[0].save(args.output, save_all=True, append_images=frames[1:],
               duration=1800, loop=0, disposal=2, optimize=False)
with Image.open(args.output) as result:
    assert result.n_frames == 4
print(f'Created {args.output}: four real evaluated states, 1.8 seconds per state')
