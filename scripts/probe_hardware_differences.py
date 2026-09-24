"""Byte-for-byte signed-pin regression against the uploaded desktop engines."""
import json, shutil, sys
import differential as d

d.WORK.mkdir(parents=True, exist_ok=True)
shutil.copytree(d.BASE/'tools', d.TOOLS, dirs_exist_ok=True)

def probe(name, chip, pins, commands, files=None):
    files = dict(files or {})
    files['probe.tst'] = (f'load {chip}.hdl, output-file probe.out, output-list ' +
        ' '.join(p+'%D1.7.1' for p in pins.split()) + ';\n' + commands)
    d.test('hardware-signed-'+name, 'HardwareSimulator', 'HardwareSimulatorMain',
           files, ['probe.tst'], ['probe.out'])

for chip, pins, commands in [
    ('Not','in out','set in -1, eval, output; set in -32768, eval, output;'),
    ('Nand','a b out','set a -1, set b -2, eval, output;'),
    ('And','a b out','set a -1, set b -2, eval, output;'),
    ('Or','a b out','set a -1, set b -2, eval, output;'),
    ('Xor','a b out','set a -1, set b -2, eval, output;'),
    ('Mux','a b sel out','set a -1, set b -2, set sel -1, eval, output;'),
    ('DMux','in sel a b','set in -2, set sel -1, eval, output;'),
    ('DMux4Way','in sel a b c d','set in -2, set sel -1, eval, output;'),
    ('DMux8Way','in sel a b c d e f g h','set in -2, set sel -1, eval, output;'),
    ('Mux4Way16','a b c d sel out','set a -1, set sel -1, eval, output;'),
    ('Mux8Way16','a b c d e f g h sel out','set a -1, set sel -1, eval, output;'),
    ('HalfAdder','a b sum carry','set a -1, set b 1, eval, output;'),
    ('FullAdder','a b c sum carry','set a -1, set b -1, set c -1, eval, output; set a -32768, set b -32768, eval, output;'),
    ('Or8Way','in out','set in -256, eval, output;'),
    ('ALU','x y zx nx zy ny f no out zr ng','set x 3, set y 5, set zx -1, set nx -1, set zy -1, set ny -1, set f -1, set no -1, eval, output;'),
    ('DFF','in out','set in -32768, eval, output; tick, output; tock, output;'),
    ('Bit','in load out','set in -2, set load 1, eval, output; tick, output; tock, output;'),
]:
    probe(chip, chip, pins, commands)

files = {
    'SignedWire.hdl': 'CHIP SignedWire { IN in; OUT out; PARTS: Not(in=in,out=n); Not(in=n,out=out); }',
    'SignedHierarchy.hdl': 'CHIP SignedHierarchy { IN in; OUT out; PARTS: SignedWire(in=in,out=n); SignedWire(in=n,out=out); }',
    'SourceSlice.hdl': 'CHIP SourceSlice { IN in; OUT out; PARTS: Not(in=in[0],out=out); }',
    'TargetSlice.hdl': 'CHIP TargetSlice { IN in; OUT out; PARTS: Not(in[0]=in,out=out); }',
    'MergeSlice.hdl': 'CHIP MergeSlice { IN in; OUT out[2]; PARTS: Not(in=in[0],out[0]=out[0],out[0]=out[1]); }',
    'ConstantSlice.hdl': 'CHIP ConstantSlice { OUT out[2]; PARTS: Not(in=false,out[0]=out[0],out[0]=out[1]); }',
}
for chip in ('SignedWire','SignedHierarchy','SourceSlice','TargetSlice','MergeSlice'):
    probe(chip, chip, 'in out', 'set in -2, eval, output; set in -1, eval, output; set in -32768, eval, output;', files)
probe('ConstantSlice','ConstantSlice','out','eval, output;',files)

for row in d.report:
    row['parity_feature'] = 'hardware.narrow-negative-values'
    row['normalization'] = 'none; output bytes compared exactly'
(d.ROOT/'docs/evidence/hardware-known-differences.json').write_text(json.dumps(d.report, indent=2), encoding='utf-8')
print(f'{sum(r["passed"] for r in d.report)}/{len(d.report)} signed-pin differential cases passed')
sys.exit(0 if all(r['passed'] for r in d.report) else 1)
