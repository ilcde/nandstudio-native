"""Reproduce known release-blocking differences without hiding them in passing suites."""
import json, shutil
import differential as d

d.WORK.mkdir(parents=True,exist_ok=True)
shutil.copytree(d.BASE/'tools',d.TOOLS,dirs_exist_ok=True)
d.test('hardware-known-narrow-negative','HardwareSimulator','HardwareSimulatorMain',{
    'probe.tst':'load Not.hdl, output-file probe.out, output-list in%D1.6.1 out%D1.6.1; set in -1, eval, output;'
},['probe.tst'],['probe.out'])
record=d.report[0]
record['release_blocker']='hardware.narrow-negative-values'
record['explanation']='Legacy nodes retain signed short values even on one-bit pins. Native bit nets mask to declared width. This is an unresolved behavioral difference, not approved normalization.'
(d.ROOT/'docs/evidence/hardware-known-differences.json').write_text(json.dumps(d.report,indent=2),encoding='utf-8')
print('KNOWN RELEASE BLOCKER reproduced' if not record['passed'] else 'Behavior now matches; review and update parity register')
