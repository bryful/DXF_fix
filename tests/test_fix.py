"""Integration checks; no third-party Python dependencies."""
import pathlib
import subprocess
import tempfile
import unittest

ROOT = pathlib.Path(__file__).resolve().parents[1]
EXE = ROOT / 'x64/Release/DXF_fix.exe'

def pairs(path):
    lines = path.read_text(encoding='utf-8-sig').splitlines()
    return [(int(lines[i]), lines[i+1]) for i in range(0, len(lines), 2)]

def geometry(p):
    start = p.index((2, 'ENTITIES')) + 1
    end = p.index((0, 'ENDSEC'), start)
    return [x for x in p[start:end] if x[0] not in (30, 31)]

class FixTests(unittest.TestCase):
    def run_fix(self, source, output):
        return subprocess.run([str(EXE), str(source), str(output)], capture_output=True)

    def test_supplied_geometry(self):
        for name in ('SketchTest_app.dxf', 'SketchTest.dxf'):
            with self.subTest(name=name), tempfile.TemporaryDirectory(dir=ROOT) as temp:
                source = pathlib.Path('D:/Downloads') / name
                output = pathlib.Path(temp) / '修正.dxf'
                original = source.read_bytes()
                result = self.run_fix(source, output)
                self.assertEqual(result.returncode, 0, result.stderr)
                p = pairs(output)
                self.assertEqual(geometry(pairs(source)), geometry(p))
                self.assertIn((1, 'AC1009'), p)
                self.assertEqual(p.count((30, '0')), 5)
                self.assertEqual(p.count((31, '0')), 4)
                self.assertNotIn((9, '$INSUNITS'), p)
                data = output.read_bytes()
                self.assertNotIn(b'\n', data.replace(b'\r\n', b''))
                self.assertEqual(source.read_bytes(), original)
                self.assertNotEqual(self.run_fix(source, output).returncode, 0)
                self.assertEqual(output.read_bytes(), data)

    def test_invalid_inputs(self):
        def dxf(entity):
            return '0\nSECTION\n2\nENTITIES\n' + entity + '0\nENDSEC\n0\nEOF\n'
        cases = [dxf('0\nSPLINE\n'), dxf('0\nCIRCLE\n10\n0\n20\n0\n40\n-1\n'),
                 dxf('0\nLINE\n10\nnan\n20\n0\n11\n1\n21\n1\n'),
                 dxf('0\nLINE\n10\n0\n20\n0\n'), '0\nSECTION\n2',
                 dxf('0\nCIRCLE\n10\n0\n20\n0\n40\n1\n').replace('0\nEOF\n', '')]
        with tempfile.TemporaryDirectory(dir=ROOT) as temp:
            source = pathlib.Path(temp) / 'bad.dxf'
            output = pathlib.Path(temp) / 'output.dxf'
            for data in cases:
                source.write_text(data)
                self.assertNotEqual(self.run_fix(source, output).returncode, 0)
                self.assertFalse(output.exists())

    def test_arc_bom_and_existing_z(self):
        with tempfile.TemporaryDirectory(dir=ROOT) as temp:
            source = pathlib.Path(temp) / 'arc.dxf'
            output = pathlib.Path(temp) / 'fixed.dxf'
            source.write_text('0\nSECTION\n2\nENTITIES\n0\nARC\n8\nOutline\n10\n1.25\n20\n-2\n30\n3\n40\n5\n50\n350\n51\n10\n0\nENDSEC\n0\nEOF\n', encoding='utf-8-sig')
            result = self.run_fix(source, output)
            self.assertEqual(result.returncode, 0, result.stderr)
            p = pairs(output)
            self.assertIn((30, '3'), p)
            self.assertIn((2, 'Outline'), p)
            self.assertIn((50, '350'), p)
            self.assertIn((51, '10'), p)

if __name__ == '__main__':
    unittest.main()
