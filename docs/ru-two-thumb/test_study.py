"""Scoped offline checks of the saved research, not Android/device tests."""
import json, math, unittest
from html.parser import HTMLParser
from study import ALPHA, BASE, POP, ROOT, coord, model

class ResearchChecks(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.r=json.loads((ROOT/'results.json').read_text())
        cls.counts=json.loads((ROOT/'counts.json').read_text())
    def test_full_alphabet_and_fixed_space(self):
        for lay in (BASE,self.r['candidate']):
            self.assertEqual(len(lay),30)
            self.assertEqual(len(set(lay)),30)
            self.assertEqual(set(lay)|set(POP),set(ALPHA))
        self.assertEqual(coord(30,1),(6+6/9.4,3))
    def test_saved_totals_and_public_provenance(self):
        for split in ('train','test'):
            c=self.counts[split]; totals=self.r[split]
            self.assertEqual(sum(c['uni'].values()),totals['characters'])
            self.assertEqual(sum(c['bi'].values()),totals['pairs'])
            self.assertEqual(totals['characters']-totals['pairs'],totals['sentences'])
            self.assertTrue(c['url'].startswith('https://raw.githubusercontent.com/UniversalDependencies/UD_Russian-GSD/r2.15/'))
            self.assertEqual(len(c['sha256']),64)
    def test_all_saved_model_metrics_reproduce(self):
        c=self.counts['test']
        for aspect in (.7,1,1.3):
            for hold in (1,3,6):
                metric=model(c['uni'],c['bi'],aspect,hold)
                saved=self.r['sensitivity'][f'vertical_pitch_{aspect}_popup_{hold}']
                for name,lay in (('baseline',BASE),('candidate',self.r['candidate'])):
                    values=metric(lay)
                    for key,value in values.items():
                        self.assertTrue(math.isfinite(value))
                        self.assertAlmostEqual(value,saved[name][key],places=10)
                    self.assertGreaterEqual(values['same_thumb_pct'],values['same_target_pct'])
    def test_document_retains_limits_and_local_style(self):
        report=(ROOT/'proposal.html').read_text(); HTMLParser().feed(report)
        self.assertEqual(report.split('<style>')[1].split('</style>')[0],(ROOT/'house-style.css').read_text())
        self.assertEqual(report.count('<li>'),6)
        self.assertIn(':root{font-family:system-ui',report)
        self.assertEqual(report.count('<style>'),1)
        self.assertEqual(report.count('</style>'),1)
        self.assertNotIn('/home/',report)
        self.assertIn('2.6% worse',report)
        self.assertIn('not a speed, accuracy or comfort claim',report)
        self.assertNotIn('<script',report)

if __name__=='__main__':unittest.main()
