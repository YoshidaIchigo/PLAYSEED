"""Generate our own small repeatable patterns; no external image input."""
import json,math,random,struct
from pathlib import Path
stage=Path(__file__).parent
folder=stage/'ReferenceTextures';folder.mkdir(exist_ok=True)
rng=random.Random(20261016)
palette={'Grass':(30,150,20),'GrassEdge':(202,178,132),'Dirt':(150,106,58),'Stone':(155,150,135),'Wood':(147,96,44),'Cave':(170,170,164),'Fence':(180,143,84),'Rock':(164,116,64),'Shadow':(50,65,38),'WireFence':(116,132,104)}
def tga(name,w,h,pixels):
    header=struct.pack('<BBBHHBHHHHBB',0,0,2,0,0,0,0,0,w,h,32,0x28)
    (folder/(name+'.tga')).write_bytes(header+b''.join(bytes((b,g,r,a)) for r,g,b,a in pixels))
for name,base in palette.items():
    pixels=[]
    for y in range(32):
        for x in range(32):
            delta=rng.choice((-15,-8,-3,0,3,8,13));rgb=[min(255,max(0,c+delta)) for c in base];alpha=255
            if name=='Grass' and (x*13+y*7)%17<2:rgb=[24,118,14]
            if name=='GrassEdge' and (x*5+y*11)%19<3:rgb=[151,129,94]
            if name=='Stone' and (y%8==0 or (x+(y//8%2)*8)%16==0):rgb=[65,62,52]
            if name=='Cave' and (x*3+y*7)%23<5:rgb=[126,129,124]
            if name=='Wood':
                grain=int(8*math.sin(x*.9+math.sin(y*.32)*1.2)+3*math.sin(x*2.3+y*.2))
                rgb=[min(255,max(0,c+grain)) for c in base]
                if x%16==0:rgb=[75,44,17]
            if name=='Rock' and (x*3+y*7)%29<5:rgb=[116,80,43]
            if name=='Fence':alpha=255 if x%16<3 or y<4 or 13<y<17 or y>28 else 0
            if name=='WireFence':alpha=255 if x%8<2 or y<2 or y>29 else 0
            if name=='Shadow':alpha=255 if (x-15.5)**2+(y-15.5)**2<180 else 0
            pixels.append((*rgb,alpha))
    tga(name,32,32,pixels)
pixels=[];w,h=512,256
# 周回できる遠景と小さい雲を数式だけで自作します。外部画像を読み込みません。
clouds=[(24,87,9,3),(71,106,15,4),(115,91,8,3),(164,113,13,4),(207,80,10,3),(255,102,14,4),(304,118,12,3),(355,89,9,3),(399,108,15,4),(453,95,10,3),(490,121,9,3)]
for y in range(h):
    for x in range(w):
        t=y/h;rgb=[int(64+82*t),int(139+64*t),int(222+30*t)]
        for cx,cy,rx,ry in clouds:
            dx=min(abs(x-cx),w-abs(x-cx))
            if (dx/rx)**2+((y-cy)/ry)**2<1:rgb=[230,240,246] if y>cy else [246,250,252]
        ridge=177+int(5*math.sin(x*math.tau/w*5)+3*math.cos(x*math.tau/w*9))
        if y>ridge:rgb=[117,137,132] if y<ridge+5 else [82,143,171]
        if y>199:rgb=[75+int((y-199)*.15),145+int((y-199)*.12),181+int((y-199)*.1)]
        pixels.append((*rgb,255))
tga('Sky',w,h,pixels)
print('Generated eleven original procedural patterns; selected four are imported by refresh_reference_surfaces.py')
