# Russian for a phone typed with two thumbs — research only

Open [the self-contained proposal](proposal.html) in a browser. It recovers the Android 2Thumbs v24 source geometry, distinguishes the unfinished draft, and compares the lateral-discomfort revision, the superseded first candidate, and the shipped Russian layout on held-out public text. “Lateral” is interpreted as the two outer screen-edge columns (0 and 9); that interpretation and the numerical discomfort penalty are explicitly not confirmed measurements. No application or firmware change is included.

Reproduce (Python 3 standard library, network needed for public corpus downloads):

```sh
free -h  # coordinate shared resources; do not run below 4 GiB available
nice -n 15 python3 -B study.py
nice -n 15 python3 -B evaluate.py
nice -n 15 python3 -B lateral_study.py
python3 -B render_report.py
nice -n 15 python3 -B -m unittest discover -s . -p 'test_*.py' -v
```

Run from this directory. The offline tests verify saved metrics, completeness, provenance totals and report limits; they do not establish phone ergonomics. The report renderer uses the bundled unchanged house-style CSS, not a machine-local path. Source references are pinned to the inspected pixel-tools commit.

## Public corpus attribution and license

[UD Russian-GSD r2.15](https://github.com/UniversalDependencies/UD_Russian-GSD/tree/r2.15), contributors Ryan McDonald, Vitaly Nikolaev and Olga Lyashevskaya, wiki genre. Its [README](https://github.com/UniversalDependencies/UD_Russian-GSD/blob/r2.15/README.md) states [CC BY-SA 4.0](https://creativecommons.org/licenses/by-sa/4.0/); Google's annotation permission does not claim ownership of underlying text. The derived frequency tables in [counts.json](counts.json) are distributed under CC BY-SA 4.0 with this attribution. Changes: lowercasing, retaining Russian letters, collapsing all other runs to single spaces, trimming sentence edges and aggregating unigram/bigram counts. Original prose is not redistributed. URLs and original-byte SHA-256 values are retained in the counts. This is not a personal typing corpus and contains no private prompt text.
