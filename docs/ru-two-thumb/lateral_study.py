#!/usr/bin/env python3
"""Bounded revision for uncomfortable lateral columns; no device interaction."""
import hashlib,json,math,random,re,urllib.request
from study import ROOT,ALPHA,BASE,POP,coord,model
EDGE_COLUMNS={0,9} # interpretation: the two OUTER screen edges; not yet confirmed
PENALTY=2.0 # abstract cost per outer-edge tap, not a fitted biomechanical constant

def edge_mass(layout,uni):
    pos={c:i for i,c in enumerate(layout)};pos[' ']=30
    for c,b in POP.items():pos[c]=pos[b]
    return sum(n for c,n in uni.items() if pos[c]!=30 and pos[c]%10 in EDGE_COLUMNS)/sum(uni.values())

def metrics(layout,counts,penalty=2,aspect=1):
    out=model(counts['uni'],counts['bi'],aspect)(layout)
    out['outer_edge_tap_pct']=edge_mass(layout,counts['uni'])*100
    out['score_with_lateral_penalty']=out['score']+penalty*out['outer_edge_tap_pct']/100
    return out

def path_metrics(layout,texts,aspect=1,anchor=2):
    pos={c:i for i,c in enumerate(layout)};pos[' ']=30
    for c,b in POP.items():pos[c]=pos[b]
    total=sum(map(len,texts));travel=reach=longmass=space_travel=0;space_pairs=0
    hand=lambda i:int(i==30 or i%10>=5)
    for text in texts:
        previous=[None,None]
        for c in text:
            p=pos[c];h=hand(p);xy=coord(p,aspect)
            reach+=math.dist(xy,(anchor if not h else 9-anchor+6/9.4,2*aspect))
            if previous[h] is not None:
                d=math.dist(xy,coord(previous[h],aspect));travel+=d;longmass+=d>3
            previous[h]=p
        for a,b in zip(text,text[1:]):
            if ' ' in (a,b):
                space_pairs+=1
                if hand(pos[a])==hand(pos[b]):space_travel+=math.dist(coord(pos[a],aspect),coord(pos[b],aspect))
    return {'each_thumb_path_u_per_character':travel/total,'anchor_reach_u':reach/total,'long_moves_over_3u_per_100_chars':longmass/total*100,'space_adjacent_travel_u_per_pair':space_travel/space_pairs}

def run():
    counts=json.loads((ROOT/'counts.json').read_text());old=json.loads((ROOT/'results.json').read_text())['candidate']
    train=counts['train'];test=counts['test'];raw_model=model(train['uni'],train['bi'])
    score=lambda lay:raw_model(lay)['score']+PENALTY*edge_mass(lay,train['uni'])
    rng=random.Random(20261103);best=old;bestscore=score(best)
    for start in (old,BASE,old):
        lay=list(start);current=score(lay)
        for step in range(12000):
            a,b=rng.sample(range(30),2);lay[a],lay[b]=lay[b],lay[a]
            value=score(lay);temp=.10*(.0005/.10)**(step/12000)
            if value<current or rng.random()<math.exp(min(0,(current-value)/temp)):
                current=value
                if current<bestscore:best=''.join(lay);bestscore=value
            else:lay[a],lay[b]=lay[b],lay[a]
    layouts={'baseline':BASE,'previous':old,'revised':best}
    data=urllib.request.urlopen(test['url'],timeout=60).read()
    assert hashlib.sha256(data).hexdigest()==test['sha256']
    texts=[re.sub('[^'+ALPHA+']+',' ',line[9:].lower()).strip() for line in data.decode().splitlines() if line.startswith('# text = ')]
    texts=[t for t in texts if t];assert sum(map(len,texts))==sum(test['uni'].values())
    report={'interpretation':'outer screen-edge columns 0 and 9; unconfirmed meaning of lateral','penalty':PENALTY,'seed':20261103,'restarts':3,'steps_per_restart':12000,'layouts':layouts,'nominal':{n:metrics(l,test) for n,l in layouts.items()},'penalty_sensitivity':{},'posture_sensitivity':{}}
    for p in (0,.5,1,2,4):report['penalty_sensitivity'][str(p)]={n:metrics(l,test,p) for n,l in layouts.items()}
    for aspect in (.7,1,1.3):
        for anchor in (1,2,3):report['posture_sensitivity'][f'vertical_{aspect}_anchor_{anchor}']={n:path_metrics(l,texts,aspect,anchor) for n,l in layouts.items()}
    for l in layouts.values():assert len(l)==30 and set(l)|set(POP)==set(ALPHA)
    (ROOT/'lateral_results.json').write_text(json.dumps(report,ensure_ascii=False,indent=2)+'\n')
    print(json.dumps({'layouts':layouts,'nominal':report['nominal'],'path':report['posture_sensitivity']['vertical_1_anchor_2']},ensure_ascii=False,indent=2))
if __name__=='__main__':run()
