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
parser.add_argument('--prefix', default='eval-frame', choices=('eval-frame', 'tutorial-frame'))
parser.add_argument('--duration', type=int, default=1800)
args = parser.parse_args()
if not 100 <= args.duration <= 30000: parser.error('duration must be 100..30000 milliseconds')
frames = []
for index in range(4):
    with Image.open(args.captures / f'{args.prefix}-{index}.png') as source:
        frames.append(source.convert('RGB'))
if len({frame.size for frame in frames}) != 1:
    raise ValueError('All real application frames must have the same dimensions')
args.output.parent.mkdir(parents=True, exist_ok=True)
frames[0].save(args.output, save_all=True, append_images=frames[1:],
               duration=args.duration, loop=0, disposal=2, optimize=False)
with Image.open(args.output) as result:
    assert result.n_frames == 4
print(f'Created {args.output}: four real application frames, {args.duration} ms per frame')
