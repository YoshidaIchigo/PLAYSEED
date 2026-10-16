"""Generate our own small repeatable patterns; no external image input."""
import json,math,random,struct
from pathlib import Path
stage=Path(__file__).parent
folder=stage/'ReferenceTextures';folder.mkdir(exist_ok=True)
rng=random.Random(20261016)
palette={'Grass':(80,170,18),'GrassEdge':(115,78,35),'Dirt':(150,92,38),'Stone':(155,150,135),'Wood':(147,96,44),'Cave':(96,92,84),'Fence':(164,119,59),'Rock':(137,91,43),'Shadow':(50,65,38)}
def tga(name,w,h,pixels):
    header=struct.pack('<BBBHHBHHHHBB',0,0,2,0,0,0,0,0,w,h,32,0x28)
    (folder/(name+'.tga')).write_bytes(header+b''.join(bytes((b,g,r,a)) for r,g,b,a in pixels))
for name,base in palette.items():
    pixels=[]
    for y in range(32):
        for x in range(32):
            delta=rng.choice((-15,-8,-3,0,3,8,13));rgb=[min(255,max(0,c+delta)) for c in base];alpha=255
            if name=='Grass' and (x*13+y*7)%17<2:rgb=[55,132,12]
            if name=='GrassEdge' and y<6+((x*7)%5):rgb=[64,151,13]
            if name in ('Stone','Cave') and (y%8==0 or (x+(y//8%2)*8)%16==0):rgb=[65,62,52]
            if name=='Wood' and (x%8==0 or (y+(x//8)*4)%27==0):rgb=[75,44,17]
            if name=='Rock' and (y%13<2 or (x+(y//13%2)*7)%17<2):rgb=[86,52,25]
            if name=='Fence':alpha=255 if x%16<3 or y<4 or 13<y<17 or y>28 else 0
            if name=='Shadow':alpha=255 if (x-15.5)**2+(y-15.5)**2<180 else 0
            pixels.append((*rgb,alpha))
    tga(name,32,32,pixels)
pixels=[];w,h=512,256
for y in range(h):
    for x in range(w):
        t=y/h;rgb=[int(72+80*t),int(151+55*t),int(239+10*t)]
        # 周回できる雲と遠景。元画像の切り貼りではなく数式で描きます。
        cloud=False
        for cx,cy,rx,ry in [(55,57,33,12),(160,83,45,16),(310,50,48,14),(440,94,34,12)]:
            dx=min(abs(x-cx),w-abs(x-cx))
            if (dx/rx)**2+((y-cy)/ry)**2<1:cloud=True
        if cloud:rgb=[237,246,255]
        horizon=182+int(8*math.sin(x*math.tau/w*5)+6*math.cos(x*math.tau/w*9))
        if y>horizon:rgb=[53,121,119] if y<horizon+9 else [67,156,194]
        pixels.append((*rgb,255))
tga('Sky',w,h,pixels)
if (stage/'manifest.json').exists():
    manifest=json.loads((stage/'manifest.json').read_text())
    manifest['copy']+=[dict(source='create_reference_assets.py',target='Tools/create_reference_assets.py')]
    manifest['copy']+=[dict(source='ReferenceTextures/'+p.name,target='Tools/ReferenceTextures/'+p.name) for p in sorted(folder.glob('*.tga'))]
    (stage/'manifest.json').write_text(json.dumps(manifest,indent=2),encoding='utf8')
print('Generated ten original procedural patterns')
