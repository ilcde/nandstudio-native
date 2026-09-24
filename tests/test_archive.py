"""Archive rejection regression tests; entirely in memory."""
import io,pathlib,sys,unittest,zipfile
sys.path.insert(0,str(pathlib.Path(__file__).resolve().parents[1]/'scripts'))
from audit_baseline import checked_entries
class ArchiveTests(unittest.TestCase):
    def archive(self,names):
        data=io.BytesIO()
        with zipfile.ZipFile(data,'w') as z:
            for name in names:z.writestr(name,'test')
        data.seek(0);return zipfile.ZipFile(data)
    def test_traversal(self):
        for name in ['../outside','a/../../outside','/absolute','C:/outside','a\\..\\..\\outside']:
            with self.subTest(name=name),self.archive([name]) as z,self.assertRaises(ValueError):checked_entries(z,pathlib.Path('safe'))
    def test_duplicate_case(self):
        with self.archive(['A.hdl','a.hdl']) as z,self.assertRaises(ValueError):checked_entries(z,pathlib.Path('safe'))
    def test_valid(self):
        with self.archive(['projects/a b/Chip.hdl']) as z:self.assertEqual(len(checked_entries(z,pathlib.Path('safe'))),1)
    def test_symlink(self):
        data=io.BytesIO()
        with zipfile.ZipFile(data,'w') as z:
            info=zipfile.ZipInfo('link');info.external_attr=0o120777<<16;z.writestr(info,'../outside')
        data.seek(0)
        with zipfile.ZipFile(data) as z,self.assertRaises(ValueError):checked_entries(z,pathlib.Path('safe'))
if __name__=='__main__':unittest.main()
