#!/usr/bin/env python3
"""Research only: public UD Russian-GSD counts, normalized touchscreen model."""
import collections, hashlib, json, math, pathlib, random, re, urllib.request
ROOT=pathlib.Path(__file__).resolve().parent
ALPHA='абвгдеёжзийклмнопрстуфхцчшщъыьэюя'
BASE='йцукенгшщзфывапролджячсмитьбюх'
POP={'ё':'е','ъ':'ь','э':'ж'}
URL='https://raw.githubusercontent.com/UniversalDependencies/UD_Russian-GSD/r2.15/'
def corpus(split):
    url=URL+f'ru_gsd-ud-{split}.conllu'
    data=urllib.request.urlopen(url, timeout=60).read()
    uni=collections.Counter(); bi=collections.Counter(); sentences=0
    for line in data.decode().splitlines():
        if not line.startswith('# text = '): continue
        text=re.sub('[^'+ALPHA+']+', ' ',line[9:].lower()).strip()
        if not text: continue
        uni.update(text); bi.update(a+b for a,b in zip(text,text[1:])); sentences+=1
    return {'url':url,'sha256':hashlib.sha256(data).hexdigest(),'sentences':sentences,'uni':dict(uni),'bi':dict(bi)}
def coord(i, aspect):
    if i==30: return (6+6/9.4,3*aspect) # Space: second right cell
    r,c=divmod(i,10)
    return (c+(6/9.4 if c>=5 else 0),r*aspect)
def model(uni,bi,aspect=1,hold=3):
    total=sum(uni.values()); bt=sum(bi.values())
    chars=list(ALPHA+' '); uf=[uni.get(c,0)/total for c in chars]
    pairs=[(chars.index(g[0]),chars.index(g[1]),n/bt) for g,n in bi.items()]
    xy=[coord(i,aspect) for i in range(31)]
    hands=[int(i==30 or i%10>=5) for i in range(31)]
    reach=[math.dist(p,((2 if not hands[i] else 7+6/9.4),2*aspect)) for i,p in enumerate(xy)]
    distance=[[math.dist(a,b) for b in xy] for a in xy]
    def metrics(layout):
        pos={c:i for i,c in enumerate(layout)}; pos[' ']=30
        for c,b in POP.items(): pos[c]=pos[b]
        slots=[pos[c] for c in chars]
        right=sum(f for f,s in zip(uf,slots) if hands[s])
        travel=same=repeat=0
        for a,b,f in pairs:
            x,y=slots[a],slots[b]
            if hands[x]==hands[y]:
                same+=f
                if x==y: repeat+=f
                else: travel+=f*distance[x][y]
        reach_mean=sum(f*reach[s] for f,s in zip(uf,slots))
        popup=sum(uni.get(c,0) for c in POP)/total
        score=travel+.25*reach_mean+.35*(same-repeat)+3*(right-.5)**2+hold*popup
        return {'score':score,'same_thumb_travel_u_per_pair':travel,'mean_anchor_reach_u':reach_mean,'same_thumb_pct':same*100,'same_target_pct':repeat*100,'right_load_including_space_pct':right*100,'popup_pct':popup*100,'space_pct':uni.get(' ',0)/total*100,'representable_mass_pct':100.0}
    return metrics

def run():
    train=corpus('train'); test=corpus('test')
    (ROOT/'counts.json').write_text(json.dumps({'train':train,'test':test},ensure_ascii=False,indent=2)+'\n')
    score=model(train['uni'],train['bi']); rng=random.Random(20261102)
    best=BASE; bestscore=score(best)['score']
    for restart in range(3):
        lay=list(best if restart==0 else BASE); current=score(lay)['score']
        for step in range(12000):
            a,b=rng.sample(range(30),2); lay[a],lay[b]=lay[b],lay[a]
            value=score(lay)['score']; temp=.10*(.0005/.10)**(step/12000)
            if value<current or rng.random()<math.exp(min(0,(current-value)/temp)):
                current=value
                if current<bestscore: best=''.join(lay); bestscore=current
            else: lay[a],lay[b]=lay[b],lay[a]
    report={'candidate':best,'baseline':BASE,'seed':20261102,'restarts':3,'steps_per_restart':12000,'train':{'sentences':train['sentences'],'characters':sum(train['uni'].values()),'pairs':sum(train['bi'].values())},'test':{'sentences':test['sentences'],'characters':sum(test['uni'].values()),'pairs':sum(test['bi'].values())},'sensitivity':{}}
    for aspect in (.7,1,1.3):
        for hold in (1,3,6):
            metric=model(test['uni'],test['bi'],aspect,hold)
            report['sensitivity'][f'vertical_pitch_{aspect}_popup_{hold}']={'baseline':metric(BASE),'candidate':metric(best)}
    assert len(best)==30 and set(best)==set(BASE)
    assert set(best)|set(POP)==set(ALPHA)
    (ROOT/'results.json').write_text(json.dumps(report,ensure_ascii=False,indent=2)+'\n')
    print(json.dumps(report,ensure_ascii=False,indent=2))
if __name__=='__main__':run()
