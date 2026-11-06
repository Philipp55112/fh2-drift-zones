import pandas as pd, numpy as np, math, re, pickle
U='input/'
OUT=0.4            # m NACH AUSSEN vom Fahrbahnrand (negativ = zur Strasse)
TURN=math.radians(0); nflip=0   # Phil: Flaggen exakt 90 Grad zur Strasse (Tuchebene senkrecht zur Fahrtrichtung)
def zaxis(dx,dz,ux,uz):
    global nflip
    Zx,Zz=dx*math.cos(TURN)+ux*math.sin(TURN),dz*math.cos(TURN)+uz*math.sin(TURN)
    n=math.hypot(Zx,Zz);Zx,Zz=Zx/n,Zz/n
    if -Zz*ux+Zx*uz>0: Zx,Zz=-Zx,-Zz; nflip+=1
    return Zx,Zz
t=open('input/gate_flags_old.h').read()
old=[tuple(map(float,m)) for m in re.findall(r'\{\s*(-?[\d.]+)f,\s*(-?[\d.]+)f,\s*(-?[\d.]+)f,\s*(-?[\d.]+)f,\s*(-?[\d.]+)f\s*\}',t)]
class FL(list):
    def append(self,v):
        super().append(v+(CURK[0],))
CURK=[0]
F=FL()
TORS=[]
paths=pickle.load(open('input/paths_dense.pkl','rb'))
def arcs(p):
    c=[0.0]
    for a,b in zip(p,p[1:]): c.append(c[-1]+math.hypot(b[0]-a[0],b[2]-a[2]))
    return c
def nearest(p,x,z): return min(range(len(p)),key=lambda i:(p[i][0]-x)**2+(p[i][2]-z)**2)
def at(p,c,s):
    s=min(max(s,0),c[-1]); i=max(1,next((k for k,v in enumerate(c) if v>=s),len(c)-1))
    a,b=p[i-1],p[i];dd=max(c[i]-c[i-1],1e-9);f=(s-c[i-1])/dd
    return tuple(a[k]+(b[k]-a[k])*f for k in range(3)),(b[0]-a[0])/dd,(b[2]-a[2])/dd
v5=pd.read_csv(U+'v5/zone_fahrbahn_1m.csv',encoding='utf-8-sig')
v7=pd.read_csv(U+'v7/zone_fahrbahn_1m.csv',encoding='utf-8-sig')
ld=pd.read_csv(U+'v7/zone_laengen_dicht.csv',encoding='utf-8-sig').set_index('zone').laenge_linie_m
import numpy as np
hh=pd.read_csv(U+'v5/zone_hindernisse.csv',encoding='utf-8-sig')
def eff(r):
    m=re.search(r'dx=([\d.]+);dy=([\d.]+);dz=([\d.]+)',str(r.bbox))
    if r.typ=='busch': return 0
    if r.typ=='baum': return min(r.radius_m,0.7)
    if m: dx,dy,dz=(float(x) for x in m.groups()); return max(dx,dz)/2
    return min(r.radius_m,3.0)   # statische AABBs: nur Mittelpunkt/Radius, grob
def aabb(r):
    m=re.search(r'world AABB x=(-?[\d.]+)\.\.(-?[\d.]+) y=(-?[\d.]+)\.\.(-?[\d.]+) z=(-?[\d.]+)\.\.(-?[\d.]+)',str(r.bbox))
    return tuple(float(v) for v in m.groups()) if m else None
hh['ab']=hh.apply(aabb,axis=1)
hh['eff']=hh.apply(eff,axis=1); hh=hh[(hh.eff>0)&(hh.hoehe_m>=0.5)].reset_index(drop=True)
hh['x0']=hh.ab.map(lambda a:a[0] if a else np.nan); hh['x1']=hh.ab.map(lambda a:a[1] if a else np.nan)
hh['z0']=hh.ab.map(lambda a:a[4] if a else np.nan); hh['z1']=hh.ab.map(lambda a:a[5] if a else np.nan)
print('AABB-Hindernisse',int(hh.x0.notna().sum()),'von',len(hh))
HZ={z:g.reset_index(drop=True) for z,g in hh.groupby('zone')}

# Chattie v10: exakte Welt-AABBs von 40 Gebaeuden/Waenden (Zone 2)
import re as _re
sm=pd.read_csv(U+'v10/zone2_3_wand_instanz_summary.csv',encoding='utf-8-sig')
rows=[]
for _,r in sm.iterrows():
    m=_re.search(r'x=(-?[\d.]+)\.\.(-?[\d.]+);y=(-?[\d.]+)\.\.(-?[\d.]+);z=(-?[\d.]+)\.\.(-?[\d.]+)',str(r.bbox_world))
    if not m: continue
    a=[float(v) for v in m.groups()]
    rows.append(dict(zone=int(r.zone),x=(a[0]+a[1])/2,z=(a[4]+a[5])/2,eff=1.0,hoehe_m=a[3]-a[2],x0=a[0],x1=a[1],z0=a[4],z1=a[5],typ='gebaeude'))
hh=pd.concat([hh,pd.DataFrame(rows)],ignore_index=True)
print('Gebaeude-AABBs ergaenzt',len(rows))
HZ={z:g.reset_index(drop=True) for z,g in hh.groupby('zone')}

# ---- Einzelkorrekturen nach Phils End-Dumps (13:40-13:50): Ziel = Abstand zum Fahrbahnrand der Chattie-Daten (+ aussen, - zur Strasse)
REMOVE={(3,16)}
OV={(2,18):-0.6,(2,16):-0.9,(2,14):-0.9,(2,12):-0.9,(2,10):-0.45,(2,6):-0.9,(3,19):3.0,(3,17):('rel',-1.5),(3,2):('rel',1.5),
    (4,9):-0.7,(4,12):6.75,(4,15):-0.9,(4,17):-0.9,(4,21):-1.0,(4,22):-1.0,(4,23):-1.3,(4,24):-1.3,(4,25):-1.2,
    (8,6):-0.9,(8,10):-0.9,
    (9,16):0.4,(9,21):0.4,(10,15):-1.4,(10,52):-1.1}
TOROV={26:-1.0,27:-1.2,14:-1.6,15:-1.4,34:0.0,35:0.0,38:-1.1,39:-1.1}
_rd={}
def _road(zn):
    if zn not in _rd: _rd[zn]=(v7 if zn in (1,3) else v5)
    d=_rd[zn]; return d[d.zone==zn].reset_index(drop=True)
override_log=[]
def override(zn,x,z_,target):
    g=_road(zn); med=False; rel=False
    if isinstance(target,tuple):
        if target[0]=='rel': rel=True
        else: med=True
        target=target[1]
    j=int(((g.mitte_x-x)**2+(g.mitte_z-z_)**2).idxmin()); r=g.iloc[j]
    dx,dz=r.links_x-r.rechts_x,r.links_z-r.rechts_z; w=math.hypot(dx,dz); ux,uz=dx/w,dz/w
    if med:
        w=float(g.breite_m.iloc[max(0,j-20):j+21].median())
    lat=(x-r.mitte_x)*ux+(z_-r.mitte_z)*uz; sg=1 if lat>0 else -1
    if rel: target=abs(lat)-w/2-target   # rel: +d = d Meter zur Strasse hin, -d = nach aussen
    nx,nz=r.mitte_x+sg*ux*(w/2+target),r.mitte_z+sg*uz*(w/2+target)
    # Querversatz beibehalten? nein: nur quer verschieben, Laengsposition bleibt
    lon=(x-r.mitte_x)*(-uz)+(z_-r.mitte_z)*ux
    nx+=(-uz)*lon; nz+=ux*lon
    override_log.append((zn,round(x,1),round(z_,1),round(nx,1),round(nz,1),round(math.hypot(nx-x,nz-z_),2)))
    return nx,nz
MARG=0.55; MARGB=0.9
OFFS=[OUT+0.25*k for k in range(0,11)]+[OUT-0.25*k for k in range(1,4)]   # 0,4 ... +2,9 nach aussen, dann bis -0,35 (nie weiter auf die Strasse)
shifted=[0,0,0]; walls=[0]
T=pd.read_csv(U+'v5/zone_rand_gelaende.csv',encoding='utf-8-sig')
T=T[T.y.notna()]
TW={}
for (z,sd),g in T.groupby(['zone','seite']):
    TW[(z,sd)]=g
def wallat(z,s,side,edgey):
    # True, wenn direkt hinter dem Rand eine Steilwand/Boeschung steht (Terrain steigt >0,6 m oder >40 Grad innerhalb 2 m)
    g=TW.get((z,'links' if side==1 else 'rechts'))
    if g is None: return False
    h=g[(g.s_m-s).abs()<=3.0]
    h=h[h.abstand_m<=2]
    if not len(h): return False
    return bool(((h.y-edgey)>0.6).any() or (h.neigung_grad>40).any())
def clear(z,x,y):
    g=HZ.get(z)
    if g is None: return 9.0
    d=np.hypot(g.x.values-x,g.z.values-y)-(g.eff.values+MARG)
    hasab=g.x0.notna().values
    if hasab.any():
        gx=np.maximum(np.maximum(g.x0.values-x,x-g.x1.values),0); gz=np.maximum(np.maximum(g.z0.values-y,y-g.z1.values),0)
        da=np.hypot(gx,gz)-MARGB
        d=np.where(hasab,da,d)
    return float(d.min()) if len(d) else 9.0
def place(z,Px,Pz,ox,oz,off0=0.0,maxoff=None,base=None):
    b0=OUT if base is None else base
    OFFS=[b0+0.25*k for k in range(0,11)]+[b0-0.25*k for k in range(1,12) if b0-0.25*k>=-0.35]
    # Basis = Rand + off, off in OFFS; erster freier Platz; sonst bester
    best=None
    for k,o in enumerate(OFFS):
        if maxoff is not None and o>maxoff: continue
        if maxoff is not None and o<maxoff: continue
        x,y=Px+ox*o,Pz+oz*o; c=clear(z,x,y)
        if c>=0: 
            if k>0: shifted[0]+=1
            return x,y,o
        if best is None or (c>best[0] and o<=1.4): best=(c,x,y,o)
    shifted[1]+=1
    if best is None: best=(0,Px+ox*OUT,Pz+oz*OUT,OUT)
    return best[1],best[2],best[3]
for z in range(10):
    g_=old[z*4:z*4+4]
    for a,b in ((0,1),(1,0),(2,3),(3,2)):
        p,q=g_[a],g_[b]; dx,dz=p[0]-q[0],p[2]-q[2]; d=math.hypot(dx,dz)
        x_,z_,o_=place(z+1,p[0],p[2],dx/d,dz/d,base=0.4)
        if len(F) in TOROV: x_,z_=override(z+1,x_,z_,TOROV[len(F)])
        _g=_road(z+1); _j=int(((_g.mitte_x-x_)**2+(_g.mitte_z-z_)**2).idxmin()); _r=_g.iloc[_j]
        Zx_,Zz_=zaxis(_r.richtung_x,_r.richtung_z,dx/d,dz/d)
        F.append(((x_,p[1],z_,Zx_,Zz_),'tor',z+1))
stats={}; moved_junc=[]; omitted=[]; big_exact=[]
for z in range(1,11):
    if z in (1,3): g=v7[v7.zone==z].reset_index(drop=True); sv=g.s_linie_m.values; L=float(ld[z]); src='Chattie v7 (dichte Linie)'
    else: g=v5[v5.zone==z].reset_index(drop=True); sv=g.s_m.values; L=float(g.s_m.max()); src='Chattie v5'
    ok0=((g.status=='gemessen')&(g.breite_m.between(5,20)))
    bw=g.breite_m.where(ok0); med=bw.rolling(41,center=True,min_periods=5).median()
    ok=(ok0&((g.breite_m-med).abs()<=1.5).fillna(True)).values   # Breiten-Ausreisser (Kreuzungen/ueberlagerte Meshes) nicht als Rand verwenden
    stats_out=int(ok0.sum()-ok.sum())
    # Stabil = Station und 8 Nachbarn je Seite gemessen, Breite +-0,6 m konstant, Randpunkte ohne Sprung (>2 m zwischen Nachbarn) -> sonst Kreuzung/Mehrdeutigkeit
    gl=g[['links_x','links_z']].values; gr=g[['rechts_x','rechts_z']].values; bwv=g.breite_m.values
    jl=np.r_[0,np.hypot(*(gl[1:]-gl[:-1]).T)]; jr=np.r_[0,np.hypot(*(gr[1:]-gr[:-1]).T)]
    stab=np.zeros(len(g),bool)
    for j in range(len(g)):
        a,b=max(0,j-6),min(len(g),j+7)
        if ok[a:b].all() and abs(bwv[a:b]-bwv[j]).max()<=1.2 and max(jl[a+1:b].max(initial=0),jr[a+1:b].max(initial=0))<=3.0: stab[j]=True
    n=round(L/30); d=L/n; stats[z]=dict(L=L,n=n,d=d,flags=n-1,interp=0,none=0,src=src)
    
    stats[z]['ausreisser']=stats_out
    p=paths[z]; cp=arcs(p)
    def info(j):
        r=g.iloc[j]; ip=nearest(p,r.mitte_x,r.mitte_z); (q,hx,hz)=at(p,cp,cp[ip]); nx,nz=-hz,hx
        lat=(r.mitte_x-q[0])*nx+(r.mitte_z-q[2])*nz
        sl=1 if ((r.links_x-r.mitte_x)*nx+(r.links_z-r.mitte_z)*nz)>0 else -1
        return cp[ip],lat,sl,r.breite_m/2,r.mitte_y-q[1],sv[j]
    used=[]
    for k in range(1,n):
        CURK[0]=k; s=k*d; i=int(np.argmin(np.abs(sv-s))); side=1 if k%2==1 else -1
        gi=i; slide=0.0
        if (z,k) in REMOVE: omitted.append((z,k,int(sv[i]),'entfernt (Kreuzung, von Phil gemeldet)')); continue
        if not stab[i]:
            cand=[j for j in range(max(0,i-45),min(len(g),i+46)) if stab[j]]
            if cand:
                j=min(cand,key=lambda j:abs(j-i))
                if abs(sv[j]-sv[i])>8:
                    big_exact.append((z,k,int(sv[i])))   # gemessen, nur 'unruhig' -> exakte Station mit eigener Geometrie
                else:
                    moved_junc.append((z,k,int(sv[i]),int(sv[j]))); gi=j; slide=float(sv[i]-sv[j])
            elif not ok[i]:
                omitted.append((z,k,int(sv[i]),'unklar')); continue
        if any(abs(sv[i]-u)<8 for u in used): omitted.append((z,k,int(sv[i]),'doppelt')); continue
        used.append(sv[i])
        if ok[gi]:
            r=g.iloc[gi]; sx,sz=r.richtung_x*slide,r.richtung_z*slide; Lp=(r.links_x+sx,r.links_y,r.links_z+sz); Rp=(r.rechts_x+sx,r.rechts_y,r.rechts_z+sz)
            dx,dz=Lp[0]-Rp[0],Lp[2]-Rp[2];dd=math.hypot(dx,dz);ux,uz=dx/dd,dz/dd
            P=Lp if side==1 else Rp
            Zx,Zz=zaxis(r.richtung_x,r.richtung_z,ux*side,uz*side)
            wl=wallat(z,s,side,P[1]); walls[0]+=wl
            x_,z_,o_=place(z,P[0],P[2],ux*side,uz*side,maxoff=(-0.3 if wl else None))
            if (z,k) in OV: x_,z_=override(z,x_,z_,OV[(z,k)])
            F.append(((x_,P[1]+1.15,z_,Zx,Zz),'z%d'%z,z)); continue
        A=next((j for j in range(i,-1,-1) if ok[j] and s-sv[j]<150),None); B=next((j for j in range(i,len(g)) if ok[j] and sv[j]-s<150),None)
        if A is None and B is None: stats[z]['none']+=1; continue
        if A is not None and B is not None and sv[B]-sv[A]<=60 and sv[B]>sv[A]:
            ra,rb=g.iloc[A],g.iloc[B]; t_=(s-sv[A])/(sv[B]-sv[A])
            Lp=[ra[c]+(rb[c]-ra[c])*t_ for c in ('links_x','links_y','links_z')]; Rp=[ra[c]+(rb[c]-ra[c])*t_ for c in ('rechts_x','rechts_y','rechts_z')]
            dx,dz=Lp[0]-Rp[0],Lp[2]-Rp[2];dd=math.hypot(dx,dz);ux,uz=dx/dd,dz/dd
            rdx,rdz=ra.richtung_x+(rb.richtung_x-ra.richtung_x)*t_,ra.richtung_z+(rb.richtung_z-ra.richtung_z)*t_
            nn=math.hypot(rdx,rdz); rdx,rdz=rdx/nn,rdz/nn
            P=Lp if side==1 else Rp
            Zx,Zz=zaxis(rdx,rdz,ux*side,uz*side)
            x_,z_,o_=place(z,P[0],P[2],ux*side,uz*side)
            if (z,k) in OV: x_,z_=override(z,x_,z_,OV[(z,k)])
            F.append(((x_,P[1]+1.15,z_,Zx,Zz),'z%d'%z,z)); stats[z]['interp']+=1; stats[z]['edgeint']=stats[z].get('edgeint',0)+1; continue
        ia=info(A) if A is not None else None; ib=info(B) if B is not None else None
        base=ia or ib; oth=ib or ia
        t_=(s-ia[5])/(ib[5]-ia[5]) if (ia and ib and ib[5]>ia[5]) else 0.0
        sarc=(ia[0]+(ib[0]-ia[0])*t_) if (ia and ib) else base[0]+(s-base[5])
        half=max(base[3]+(oth[3]-base[3])*t_,5.5); sl=base[2]
        (q,hx,hz)=at(p,cp,sarc); nx,nz=-hz,hx; sd=sl*side; off=sd*(half+0.8)   # Luecke gross: Linie als Mitte, Zusatzabstand 0,8 m
        dy=base[4]+(oth[4]-base[4])*t_
        Zx,Zz=zaxis(hx,hz,nx*sd,nz*sd)
        x_,z_,o_=place(z,q[0]+nx*off,q[2]+nz*off,nx*sd,nz*sd)
        if (z,k) in OV: x_,z_=override(z,x_,z_,OV[(z,k)])
        F.append(((x_,q[1]+dy+1.15,z_,Zx,Zz),'z%d'%z,z)); stats[z]['interp']+=1; stats[z]['pathint']=stats[z].get('pathint',0)+1
print('wegen Kreuzung/Mehrdeutigkeit entlang der Strasse verschoben:',len(moved_junc),moved_junc[:40]); print('AUSGELASSEN (keine stabile Strasse in 45 m / doppelt):',omitted); print('exakte Station trotz unruhiger Daten:',big_exact); print('Steilwand-Stellen',walls[0]); print('Flaggen gesamt',len(F),'gespiegelt',nflip,'nach innen verschoben',shifted[0],'nicht frei',shifted[1])
for z,s in stats.items(): print(z,{k:(round(float(v),1) if not isinstance(v,str) else v) for k,v in s.items()})

# ---- F57 (Zone 2, k=10): Mauerecke direkt am Fahrbahnrand -> 3,5 m laengs der Strasse in Fahrtrichtung der Daten rueckwaerts (kleinere s) hinter das Mauerende
def _shift57():
    g=_road(2); idx=[i for i,f in enumerate(F) if f[1]=='z2' and f[3]==10][0]
    f=F[idx]; x,y,z_=f[0][:3]
    j=int(((g.mitte_x-x)**2+(g.mitte_z-z_)**2).idxmin()); j2=max(0,j-3)
    r=g.iloc[j2]; dx,dz=r.rechts_x-r.links_x,r.rechts_z-r.links_z; dd=math.hypot(dx,dz); ox,oz=dx/dd,dz/dd
    nx,nz=r.rechts_x+ox*0.4,r.rechts_z+oz*0.4
    F[idx]=((nx,r.rechts_y+1.15,nz,f[0][3],f[0][4]),)+tuple(f[1:])
    print('F57 verschoben',idx,round(x,1),round(z_,1),'->',round(nx,1),round(nz,1),'Strecke',round(math.hypot(nx-x,nz-z_),2))
# _shift57() entfernt: Phil 14:51 - F57 haette nur weiter nach innen gemusst, nicht laengs verschoben
pickle.dump((F,stats),open('/dev/null','wb'))
print('Overrides:'); [print(' ',o) for o in override_log]
open('flag_positions.h','w').write('#define NF %d\nstatic const float FP[NF][5]={%s};\n'%(len(F),','.join('{%.2ff,%.2ff,%.2ff,%.4ff,%.4ff}'%f[0] for f in F)))
