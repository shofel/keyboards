#!/usr/bin/env python3
"""Extra held-out checks; no optimization, no private corpus, no device access."""
import collections, hashlib, json, math, re, urllib.request
from study import ROOT, ALPHA, POP, BASE, coord, model
r=json.loads((ROOT/'results.json').read_text()); counts=json.loads((ROOT/'counts.json').read_text())
test=counts['test']; data=urllib.request.urlopen(test['url'],timeout=60).read()
assert hashlib.sha256(data).hexdigest()==test['sha256']
texts=[re.sub('[^'+ALPHA+']+',' ',line[9:].lower()).strip() for line in data.decode().splitlines() if line.startswith('# text = ')]
texts=[t for t in texts if t]
assert sum(map(len,texts))==r['test']['characters']
def evaluate(layout,aspect=1,anchor=2):
    pos={c:i for i,c in enumerate(layout)}; pos[' ']=30
    for c,b in POP.items(): pos[c]=pos[b]
    def hand(i):return int(i==30 or i%10>=5)
    total=sum(map(len,texts)); travel=reach=longmass=space_travel=0; space_pairs=0; maxjump=0
    for text in texts:
        previous=[None,None]
        for c in text:
            p=pos[c]; h=hand(p); xy=coord(p,aspect)
            reach+=math.dist(xy,(anchor if not h else 9-anchor+6/9.4,2*aspect))
            if previous[h] is not None:
                d=math.dist(xy,coord(previous[h],aspect));travel+=d;maxjump=max(maxjump,d)
                longmass+=d>3
            previous[h]=p
        for a,b in zip(text,text[1:]):
            if ' ' in (a,b):
                space_pairs+=1
                if hand(pos[a])==hand(pos[b]):space_travel+=math.dist(coord(pos[a],aspect),coord(pos[b],aspect))
    return {'each_thumb_path_u_per_character':travel/total,'long_thumb_moves_over_3u_per_100_chars':longmass/total*100,'max_thumb_jump_u':maxjump,'anchor_reach_u':reach/total,'space_adjacent_pairs':space_pairs,'space_adjacent_same_thumb_travel_u_per_space_pair':space_travel/space_pairs}
r['extra_test_checks']={}
for aspect in (.7,1,1.3):
    for anchor in (1,2,3):
        r['extra_test_checks'][f'vertical_{aspect}_anchor_L{anchor}_R{9-anchor}']={name:evaluate(lay,aspect,anchor) for name,lay in [('baseline',BASE),('candidate',r['candidate'])]}
for lay in (BASE,r['candidate']):
    assert len(lay)==30 and len(set(lay))==30
    assert set(lay)|set(POP)==set(ALPHA)
    m=model(test['uni'],test['bi'])(lay)
    assert all(math.isfinite(v) for v in m.values())
    assert m['same_thumb_pct']>=m['same_target_pct']
(ROOT/'results.json').write_text(json.dumps(r,ensure_ascii=False,indent=2)+'\n')
print(json.dumps(r['extra_test_checks']['vertical_1_anchor_L2_R7'],indent=2))
print('Coverage, source hash, counts, finite-value and metric consistency assertions passed.')
