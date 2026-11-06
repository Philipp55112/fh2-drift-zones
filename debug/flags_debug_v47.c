#ifndef RELEASE
#define RELEASE 0
#endif
#if RELEASE
#define LOGNAME "DriftzonenFlaggen.log"
#define BANNER "DriftzonenFlaggen 0.9.9 bereit"
#define STATSEC 30.0f
#else
#define LOGNAME "PropTest.log"
#define BANNER "PropTest 47 (Daten v21: F123 nochmal 0,5 m aussen; sonst wie 45): F6 Manager, F7 Flagge vor Auto, F8/F11/F12 Definition, F9 Zone-1-Start, F10 Objekte, Ende Dump"
#define STATSEC 5.0f
#endif
// PropTest: Flagge ueber den Welt-Objekt-Manager (CWorldCollObjMan) spawnen, wie WreckingBall seine Trümmer.
// F6 = Manager suchen, F7 = Flagge vor dem Auto, F8 = naechste Definition (121..124), F9 = Flagge am Start von Zone 1
typedef unsigned char u8; typedef unsigned int u32; typedef unsigned long long u64; typedef int BOOL; typedef unsigned long DWORD; typedef void *HANDLE;
#define WINAPI __stdcall
int _fltused;
typedef struct { void *Base, *AllocBase; DWORD AllocProt; DWORD pad; u64 Size; DWORD State, Protect, Type, pad2; } MBI;
__declspec(dllimport) u64 WINAPI VirtualQuery(const void *, MBI *, u64);
__declspec(dllimport) HANDLE WINAPI CreateFileA(const char *, DWORD, DWORD, void *, DWORD, DWORD, HANDLE);
__declspec(dllimport) BOOL WINAPI WriteFile(HANDLE, const void *, DWORD, DWORD *, void *);
__declspec(dllimport) BOOL WINAPI CloseHandle(HANDLE);
__declspec(dllimport) DWORD WINAPI SetFilePointer(HANDLE, long, long *, DWORD);
__declspec(dllimport) short WINAPI GetAsyncKeyState(int);

static u8 *g;
static char lb[16384]; static int ln;
static int quiet;
static void lch(char c) { if (quiet) return; if (ln < (int)sizeof lb - 2) lb[ln++] = c; }
static void lstr(const char *s) { while (*s) lch(*s++); }
static void ldec(int v) { char t[12]; int k = 0; if (v < 0) { lch('-'); v = -v; } if (!v) t[k++] = '0'; while (v) { t[k++] = (char)('0' + v % 10); v /= 10; } while (k) lch(t[--k]); }
static void lhex(u64 v, int d) { int i; for (i = d - 1; i >= 0; i--) lch("0123456789abcdef"[(v >> (i * 4)) & 15]); }
static void lfloat(float f) { int neg = f < 0; long long m; if (neg) { lch('-'); f = -f; } m = (long long)(f * 1000.0f + 0.5f); ldec((int)(m / 1000)); lch('.'); { int r = (int)(m % 1000); lch('0' + r / 100); lch('0' + (r / 10) % 10); lch('0' + r % 10); } }
static float gtime;
static void ltime(void) { int a = (int)gtime; lstr("[t="); ldec(a); lch('.'); ldec((int)((gtime - (float)a) * 10.0f)); lstr("] "); }
static void lflush(void)
{
    DWORD w; HANDLE h = CreateFileA(LOGNAME, 4, 3, 0, 4, 0x80, 0);
    if (h != (HANDLE)-1 && h) { SetFilePointer(h, 0, 0, 2); WriteFile(h, lb, (DWORD)ln, &w, 0); CloseHandle(h); }
    ln = 0;
}
static int readable(const void *p, u64 n)
{
    MBI m; if (!VirtualQuery(p, &m, sizeof m)) return 0;
    if (m.State != 0x1000 || !(m.Protect & 0xEE) || (m.Protect & 0x101)) return 0;
    return (u64)p + n <= (u64)m.Base + m.Size;
}
static float fsqrt(float x) { float r; __asm__("sqrtss %1, %0" : "=x"(r) : "x"(x)); return r; }
static int same(const u8 *a, const u8 *b, int n) { int i; for (i = 0; i < n; i++) if (a[i] != b[i]) return 0; return 1; }

typedef int (*SpawnFn)(u8 *mgr, u8 flag, u32 id, int slot, const float *pos, float *rot, int a7, void *key);
typedef void (*BindFn)(u8 *mgr, int slot, u8 *obj, int model, const float *m);
typedef void (*PhysFn)(u8 *mgr, u8 *obj);
typedef u8 *(*LookupFn)(u8 *table, u32 id);
typedef int (*ModelFn)(u8 *table, u32 id);
typedef int (*RecycleFn)(u8 *mgr);
typedef void (*BodyIntFn)(void *body, int v);
typedef void (*BodyWakeFn)(void *body, int a, int b);
typedef void (*ObjVecFn)(u8 *obj, const float *v);

static int codeok;
static u8 *mgr;
static int defid = 121;
static float cx, cy, cz, gx = 1.0f, gz;


static void checkcode(void)
{
    static const struct { u32 rva; u8 b[6]; } c[] = {
        {0x369690, {0x48, 0x8B, 0xC4, 0x44, 0x89, 0x40}}, {0x377340, {0x48, 0x89, 0x6C, 0x24, 0x18, 0x48}},
        {0x369E80, {0x48, 0x89, 0x5C, 0x24, 0x08, 0x57}}, {0x12CC010, {0x48, 0x8B, 0x41, 0x08, 0x44, 0x0F}},
        {0x12CD300, {0x48, 0x8B, 0x41, 0x08, 0x44, 0x0F}}, {0x37A7D0, {0x48, 0x89, 0x5C, 0x24, 0x08, 0x48}} };
    int i; codeok = 1;
    for (i = 0; i < 6; i++) if (!same(g + c[i].rva, c[i].b, 6)) codeok = 0;
}

static void findmgr(void)
{
    u64 vt = (u64)(g + 0x2c6a308), a = 0x10000; int found = 0; MBI m;
    mgr = 0;
    lstr("Suche Manager (vtable game+2c6a308)...\r\n");
    while (a < 0x7ffe00000000ULL && VirtualQuery((void *)a, &m, sizeof m))
    {
        u64 base = (u64)m.Base, end = base + m.Size;
        if (m.State == 0x1000 && m.Protect == 0x04 && !(base >= (u64)g && base < (u64)g + 0x3a18000))
        {
            u64 p;
            for (p = base; p + 8 <= end; p += 8)
                if (*(u64 *)p == vt)
                {
                    u8 *c = (u8 *)p; float *pp = (float *)(c + 0x4f0); int ok = 0;
                    if (readable(c, 0x520) && readable(*(u8 **)(c + 0x4c8), 16))
                    {
                        float dx = pp[0] - cx, dz = pp[2] - cz; ok = (dx * dx + dz * dz < 250000.0f);
                    }
                    found++;
                    lstr(" Kandidat "); lhex(p, 16); lstr(ok ? " passt zur Autoposition" : " passt NICHT"); lstr("\r\n");
                    if (ok && !mgr) mgr = c;
                }
        }
        a = end;
    }
    lstr("Kandidaten: "); ldec(found); lstr(mgr ? "  -> Manager gewaehlt\r\n" : "  -> kein Manager\r\n"); lflush();
    if (mgr)
    {   // gibt es einen globalen Zeiger im Spiel-Image auf den Manager? (.data)
        u64 r; for (r = 0x2c4b000; r < 0x37b0000 && r + 8 <= 0x3a18000; r += 8)
            if (*(u64 *)(g + r) == (u64)mgr) { lstr(" globaler Zeiger auf Manager bei game+"); lhex(r, 8); lstr("\r\n"); }
        lflush();
    }
}

static int lastslot; static float rzx, rzz = 1.0f;
static u8 *spawn(float x, float y, float z)
{
    static float pos[4] __attribute__((aligned(16))), rot[12] __attribute__((aligned(16))), m[16] __attribute__((aligned(16))), zero[4] __attribute__((aligned(16)));
    u8 *table, *whole, *head, *first, *obj; int slot, model, i; u32 id = 0xFFFF0000u | (u32)defid;
    if (!codeok) { lstr("Code-Pruefung fehlgeschlagen, kein Spawn\r\n"); lflush(); return 0; }
    if (!mgr || !readable(mgr, 0x520) || *(u64 *)mgr != (u64)(g + 0x2c6a308)) { lstr("kein gueltiger Manager (F6 druecken)\r\n"); lflush(); return 0; }
    table = *(u8 **)(mgr + 0x4c8);
    whole = ((LookupFn)(g + 0x12CC010))(table, id);
    if (!quiet) { lstr("Definition "); ldec(defid); lstr(": "); lhex((u64)whole, 16); }
    if (!whole || !readable(whole, 0x98) || *(u64 *)whole != (u64)(g + 0x2EB5BD8)) { lstr(" ist keine Definition\r\n"); lflush(); return 0; }
    model = ((ModelFn)(g + 0x12CD300))(table, id);
    lstr(" Modell "); ldec(model);
    head = *(u8 **)(mgr + 0x508);
    if (!head || !readable(head, 8)) { lstr(" Slotliste unlesbar\r\n"); lflush(); return 0; }
    first = *(u8 **)head;
    slot = (first != head) ? *(int *)(first + 0x1C) : ((RecycleFn)(g + 0x37A7D0))(mgr);
    lstr(" Slot "); ldec(slot);
    if (slot < 0 || slot >= 0x1000) { lstr(" kein freier Slot\r\n"); lflush(); return 0; }
    for (i = 0; i < 12; i++) rot[i] = 0;
    rot[0] = rzz; rot[2] = -rzx; rot[5] = 1; rot[8] = rzx; rot[10] = rzz;
    pos[0] = x; pos[1] = y; pos[2] = z; pos[3] = 1;
    slot = ((SpawnFn)(g + 0x369690))(mgr, 0, id, slot, pos, rot, 0, 0);
    lstr(" -> SpawnObject "); ldec(slot);
    if (slot < 0) { lstr(" abgelehnt\r\n"); lflush(); return 0; }
    {
        u8 **vec = *(u8 ***)(mgr + 0x290);
        obj = vec ? vec[slot] : 0;
    }
    if (!obj || !readable(obj, 0x150)) { lstr(" Objekt leer\r\n"); lflush(); return 0; }
    {   // Schluessel wie bei platzierten Spiel-Objekten (Standard-Schluessel), damit das Spiel sie nicht als losen Schutt recycelt
        int dg = *(int *)(g + 0x2C6881C); int usekey = 0; u8 **fv = *(u8 ***)(mgr + 0x290), **lv = *(u8 ***)(mgr + 0x298); int n2 = (fv && lv && lv > fv) ? (int)(lv - fv) : 0, j, done = 0;
        if (n2 > 0x1000) n2 = 0x1000;
        for (j = 0; j < n2 && !done && usekey; j++)
        {
            u8 *t = fv[j];
            if (!t || t == obj || !readable(t, 0x150)) continue;
            if (*(int *)(t + 0xC8) == -1 && *(int *)(t + 0xCC) == dg && *(int *)(t + 0xD0) == -1) { for (i = 0; i < 16; i++) obj[0xC8 + i] = t[0xC8 + i]; done = 1; }
        }
        if (!done) { for (i = 0; i < 16; i++) obj[0xC8 + i] = 0; if (usekey) { *(int *)(obj + 0xC8) = -1; *(int *)(obj + 0xCC) = dg; *(int *)(obj + 0xD0) = -1; } }
        if (!quiet) { lstr(done ? " Schluessel kopiert" : " Schluessel gesetzt"); }
    }
    *(int *)(obj + 0x148) = 5; *(float *)(obj + 0xB0) = (x + cx) * 0.5f; *(float *)(obj + 0xB4) = (z + cz) * 0.5f; *(unsigned short *)(obj + 0x120) = 0;
    for (i = 0; i < 16; i++) m[i] = 0;
    m[0] = rzz; m[2] = -rzx; m[5] = 1; m[8] = rzx; m[10] = rzz; m[12] = x; m[13] = y; m[14] = z; m[15] = 1;
    *(u32 *)(obj + 0x124) = id;
    ((BindFn)(g + 0x377340))(mgr, slot, obj, model, m);
    ((PhysFn)(g + 0x369E80))(mgr, obj);
    {
        void *body = *(void **)(obj + 8);
        if (body) { void **bvt = *(void ***)body; ((BodyIntFn)bvt[0x70 / 8])(body, 7); ((BodyWakeFn)bvt[0x128 / 8])(body, 0, 0); }
        { void **ovt = *(void ***)obj; ((ObjVecFn)ovt[0x88 / 8])(obj, zero); ((ObjVecFn)ovt[0x90 / 8])(obj, zero); }
    }
    lstr(" fertig, Objekt "); lhex((u64)obj, 16); lstr("\r\n"); lflush();
    lastslot = slot; return obj;
}

static void nearby(void)
{
    u8 **first, **last; int n, i, shown = 0;
    if (!mgr || !readable(mgr, 0x520)) { lstr("kein Manager (F6)\r\n"); lflush(); return; }
    first = *(u8 ***)(mgr + 0x290); last = *(u8 ***)(mgr + 0x298);
    if (!first || !last || last < first) { lstr("Slotliste leer\r\n"); lflush(); return; }
    n = (int)(last - first); if (n > 0x1000) n = 0x1000;
    lstr("Objekte im Umkreis von 30 m (von "); ldec(n); lstr(" Slots):\r\n");
    for (i = 0; i < n && shown < 40; i++)
    {
        u8 *o = first[i]; float *q; float dx, dz;
        if (!o || !readable(o, 0x150)) continue;
        q = (float *)(o + 0xA0); dx = q[0] - cx; dz = q[2] - cz;
        if (dx * dx + dz * dz > 900.0f) continue;
        lstr(" Slot "); ldec(i); lstr(" def "); lhex(*(u32 *)(o + 0x124), 8); lstr(" ("); ldec((int)(*(u32 *)(o + 0x124) & 0xFFFF)); lstr(") state "); ldec(*(int *)(o + 0x148));
        lstr(" pos "); lfloat(q[0]); lch(' '); lfloat(q[1]); lch(' '); lfloat(q[2]);
        lstr(" abstand "); lfloat(fsqrt(dx * dx + dz * dz)); lstr("\r\n"); shown++;
    }
    lstr("Ende ("); ldec(shown); lstr(" Objekte)\r\n"); lflush();
}

static u8 *findtp(void); static u64 cntvec(u8 *tp, u32 off);
#include "flags18.h"
typedef struct { u8 *obj; int slot; u32 id; float x, y, z; int state; int armed; float t; float cool; int manual; float fx, fz, py; float fade; float sy, age; int chk; float hx, hz, nomiss; int settled; float fdur; } Fl;
static Fl fs[NF + 8];
static int fails, off, nspawn, nhit, nweg; static float stat_t;
static float scanwait, spawn_cool;

static float dist2(float x, float z) { float dx = x - cx, dz = z - cz; return dx * dx + dz * dz; }
static int nearflag(void) { int i; for (i = 0; i < NF; i++) if (dist2(fs[i].x, fs[i].z) < 230.0f * 230.0f) return 1; return 0; }
static unsigned char freebits[0x1000 / 8]; static int freeok = 1, freeinit, nfreewarn, nfreehit;
static int isfree(int slot) { return freeok && slot >= 0 && slot < 0x1000 && (freebits[slot >> 3] >> (slot & 7) & 1); }
static void buildfree(void)
{
    u8 *head, *n; int k;
    for (k = 0; k < (int)sizeof freebits; k++) freebits[k] = 0;
    if (!mgr || !readable(mgr + 0x508, 8)) return;
    head = *(u8 **)(mgr + 0x508);
    if (!head || !readable(head, 8)) return;
    n = *(u8 **)head;
    for (k = 0; k < 6000 && n && n != head && readable(n, 0x20); k++)
    {   int s = *(int *)(n + 0x1C); if (s >= 0 && s < 0x1000) freebits[s >> 3] |= (unsigned char)(1 << (s & 7)); n = *(u8 **)n; }
}
static int alive(Fl *f)
{
    u8 **vec;
    if (!f->obj || !mgr) return 0;
    if (isfree(f->slot)) { nfreehit++; return 0; }
    vec = *(u8 ***)(mgr + 0x290);
    if (!vec || !readable(vec + f->slot, 8) || vec[f->slot] != f->obj || !readable(f->obj, 0x150) || *(u32 *)(f->obj + 0x124) != f->id) return 0;
    return 1;
}
static int mgrok(void) { return mgr && readable(mgr, 0x520) && *(u64 *)mgr == (u64)(g + 0x2c6a308); }


static u8 *findtp(void);
static int modelloaded(int mid)
{
    u8 *tp = findtp(); u8 **mb, **me, *md;
    if (!tp) return 0;
    mb = *(u8 ***)(tp + 0xF618); me = *(u8 ***)(tp + 0xF620);
    md = (mb && me > mb && mid < (me - mb)) ? mb[mid] : 0;
    return md && readable(md, 0x28) && *(u64 *)(md + 8) != 0;
}
// Flaggen-Definitionen in Reihenfolge der Vorliebe (Definition, Modell-ID): 121 Horizon Navy, 122 Orange, 123 Pink, 124 Teardrop, 118 Hub-Segel, 119 Segel, 117 gruenes Segel, 120 gelbes Segel
static int pickdef(float x, float z)
{
    // Region: Frankreich (x < 6000) = orange (122/397), Italien = blau (121/395); sonst Rueckfall auf das, was gerade geladen ist
    static const int cdI[8] = {121, 122, 123, 124, 118, 119, 117, 120}, cdF[8] = {122, 121, 123, 124, 118, 119, 117, 120};
    static const int cmI[8] = {395, 397, 399, 393, 386, 376, 381, 388}, cmF[8] = {397, 395, 399, 393, 386, 376, 381, 388};
    static int last = -1;
    const int *cd = (x < 6000.0f) ? cdF : cdI, *cm = (x < 6000.0f) ? cmF : cmI;
    int k;
    for (k = 0; k < 8; k++) if (modelloaded(cm[k])) { if (cd[k] != last) { lstr("Flaggenmodell gewaehlt: Definition "); ldec(cd[k]); lstr(" Modell "); ldec(cm[k]); lstr(x < 6000.0f ? " (Region Frankreich)" : " (Region Italien)"); lstr("\r\n"); lflush(); last = cd[k]; } return cd[k]; }
    if (last != 0) { lstr("KEIN Flaggenmodell geladen (117-124) - warte\r\n"); lflush(); last = 0; }
    return 0;
}

static int nsaved;
static void setalpha(u8 *o, float a)
{
    u8 *pres; void **vt; void *fn;
    if (!mgr || !readable(mgr + 0x10, 8)) return;
    pres = *(u8 **)(mgr + 0x10);
    if (!pres || !readable(pres, 8)) return;
    vt = *(void ***)pres;
    if (!vt || !readable(vt + 0x170 / 8, 8)) return;
    fn = vt[0x170 / 8];
    if ((u8 *)fn < g || (u8 *)fn >= g + 0x3a18000) return;
    ((void (*)(void *, int, float))fn)(pres, *(int *)(o + 0x128), a);
}
static Fl gh[32]; static int ghn;
static void ghost(Fl *f, float t)
{   // Objekt gehoert uns nicht mehr (wird vom Spiel ausgeblendet), aber noch sichtbar: merken, damit dort nicht gleich eine zweite Flagge entsteht
    float *q;
    if (!f->obj || !readable(f->obj, 0x150)) return;
    q = (float *)(f->obj + 0xA0);
    gh[ghn & 31] = *f; gh[ghn & 31].x = q[0]; gh[ghn & 31].z = q[2]; gh[ghn & 31].t = t; ghn++;
}
static int ghostnear(float x, float z)
{   int k;
    for (k = 0; k < 32; k++) { Fl *h = &gh[k]; float dx, dz; if (!h->obj || h->t <= 0.0f) continue; if (!alive(h)) { h->obj = 0; continue; } dx = h->x - x; dz = h->z - z; if (dx * dx + dz * dz < 9.0f) return 1; }
    return 0;
}
static u8 *nodeat(u8 *tp, u64 np, u64 ns, int slot)
{   u8 **nb = *(u8 ***)(tp + 0xF630), **ne = *(u8 ***)(tp + 0xF638); u64 idx = np + ns + (u64)slot;
    return (nb && ne > nb && idx < (u64)(ne - nb)) ? nb[idx] : 0;
}
static int nverschl, nsettle, nconf, nzomb, nfar, wcool, wghost, wcap, wlate, nspd; static float spdsum, spdmin = 1e9f;
static u8 *tpc; static u64 npc, nsc;
static void maintain(float dt)
{
    static float zero[4] __attribute__((aligned(16)));
    int i; int nearany = 0; int spawned_now = 0;
    if (off) return;
    gtime += dt; spawn_cool -= dt;
    for (i = 0; i < NF; i++) if (dist2(fs[i].x, fs[i].z) < 230.0f * 230.0f) nearany = 1;
    scanwait -= dt;
    if (!mgrok()) { mgr = 0; if (nearany && scanwait <= 0.0f) { scanwait = 4.0f; findmgr(); } if (!mgr) return; }
    buildfree();
    { int k; for (k = 0; k < 32; k++) if (gh[k].t > 0.0f) gh[k].t -= dt; }
    tpc = findtp(); npc = tpc ? cntvec(tpc, 0xFB78) : 0; nsc = tpc ? cntvec(tpc, 0xFBC0) : 0;
    for (i = 0; i < NF + 8; i++)
    {
        Fl *f = &fs[i]; float d2 = dist2(f->x, f->z); float ex2 = (f->x - cx) * (f->x - cx) + 2.0f * (f->z - cz) * (f->z - cz);
        f->cool -= dt;
        if (f->state)
        {
            u8 *o = f->obj; float *q;
            if (!alive(f))
            {
                ltime(); lstr("Flagge "); ldec(i); lstr(isfree(f->slot) ? " weg (Slot frei)\r\n" : " weg\r\n"); lflush(); f->cool = (f->state == 2) ? 5.0f : 1.5f; f->state = 0; f->obj = 0; if (!f->manual) f->armed = 1; nweg++; if (f->manual) f->x = 1e9f;
                continue;
            }
            q = (float *)(o + 0xA0);
            if (f->state == 1)
            {
                float ex = q[0] - f->hx, ez = q[2] - f->hz;
                if (!f->manual)
                {   float tx = q[0] - f->x, tz = q[2] - f->z;
                    if (tx * tx + tz * tz > 9.0f && d2 >= 100.0f)
                    {   // Objekt steht ploetzlich weit von seinem Platz (verschleppt, z. B. an den Platz einer anderen Flagge)
                        int j, bj = -1; float bd = 1e9f; u8 *bo = *(u8 **)(o + 8);
                        for (j = 0; j < NF; j++) if (j != i) { float ax = q[0] - fs[j].x, az = q[2] - fs[j].z, dd = ax * ax + az * az; if (dd < bd) { bd = dd; bj = j; } }
                        nverschl++;
                        lstr("Verschleppt: Flagge "); ldec(i); lstr(" (slot "); ldec(f->slot); lstr(" obj "); lhex((u64)o, 12); lstr(" koerper "); lhex((u64)bo, 12); lstr(" alter "); lfloat(f->age); lstr(" s) steht "); lfloat(fsqrt(tx * tx + tz * tz)); lstr(" m weg, bei Flagge "); ldec(bj); lstr(" (dist "); lfloat(fsqrt(bd));
                        if (bj >= 0) { Fl *h = &fs[bj]; u8 *hb = (h->state && h->obj && readable(h->obj, 0x150)) ? *(u8 **)(h->obj + 8) : 0; lstr(", slot "); ldec(h->slot); lstr(" obj "); lhex((u64)h->obj, 12); lstr(" koerper "); lhex((u64)hb, 12); lstr(" zustand "); ldec(h->state); }
                        lstr(")\r\n"); lflush();
                        *(unsigned short *)(o + 0x120) = 0x4000; ghost(f, 8.0f); f->state = 0; f->obj = 0; f->armed = 1; f->cool = 2.0f; continue;
                    }
                }
                if (!f->settled)
                {   // Einpendeln: die Physik darf die frisch gesetzte Flagge aus Waenden/Objekten herausschieben
                    if (ex * ex + ez * ez > 0.0004f) { f->hx = q[0]; f->hz = q[2]; }
                    if (f->age >= 1.5f)
                    {   float bx = q[0] - f->x, bz = q[2] - f->z; f->settled = 1;
                        if (bx * bx + bz * bz > 0.09f) { nsettle++; lstr("Einpendeln Flagge "); ldec(i); lstr(": verschoben um "); lfloat(fsqrt(bx * bx + bz * bz)); lstr(" m (dx "); lfloat(bx); lstr(" dz "); lfloat(bz); lstr(")\r\n"); lflush(); }
                    }
                }
                else if (ex * ex + ez * ez > 0.04f)
                {
                    if (d2 < 10.0f * 10.0f) { f->state = 2; f->t = 0; nhit++; }      // Auto in der Naehe: seitlich weggeschoben = umgefahren
                    else { f->hx = q[0]; f->hz = q[2]; }                             // nur zurechtgerueckt (Tabellenposition x/z bleibt unveraendert)
                }
                f->y = q[1];
                f->age += dt;
                if (!f->chk && f->age >= 2.5f && !f->manual)
                {   float drop = f->sy - q[1]; f->chk = 1;
                    if (drop > 2.5f || drop < -0.4f) { lstr("Fallpruefung Flagge "); ldec(i); lstr(": Fall "); lfloat(drop); lstr(" m, Spawn-y "); lfloat(f->sy); lstr(" jetzt "); lfloat(q[1]); lstr(" bei "); lfloat(q[0]); lch(' '); lfloat(q[2]); lstr(drop < 0.25f ? " (haengt/steht auf etwas oder schwebt)" : " (faellt tief)"); lstr("\r\n"); lflush(); }
                }
                if (f->state == 1)
                {
                    void **ovt = *(void ***)o;
                    // Herkunftspunkt (B0/B4) auf die Mitte zwischen Flagge und Spieler: das Spiel blendet lose Teile aus,
                    // wenn Spieler ODER Teil mehr als ~106-150 m davon entfernt sind (dx^2 + 2 dz^2 > 22500).
                    // Den Ausblend-Timer (+0x120) NICHT mehr anfassen: sonst bleibt die Transparenz haengen (Blinken/unsichtbar).
                    *(float *)(o + 0xB0) = (q[0] + cx) * 0.5f; *(float *)(o + 0xB4) = (q[2] + cz) * 0.5f;
                    if (f->fade < 1.0f && !f->manual) { f->fade += dt / (f->fdur > 0.1f ? f->fdur : 0.5f); if (f->fade > 1.0f) f->fade = 1.0f; setalpha(o, f->fade); }
                    else if (*(unsigned short *)(o + 0x120) != 0 && d2 < 262.0f * 262.0f) { *(unsigned short *)(o + 0x120) = 0; setalpha(o, 1.0f); nsaved++; }
                    if (f->settled)
                    {   // nur waagerecht festhalten: senkrecht darf die Schwerkraft die Flagge auf den Boden setzen (kein Schweben)
                        static float lin[4] __attribute__((aligned(16)));
                        float vy = dt > 0.0f ? (q[1] - f->py) / dt : 0.0f;
                        if (vy > 3.0f) vy = 3.0f; if (vy < -15.0f) vy = -15.0f;
                        lin[0] = 0; lin[1] = vy; lin[2] = 0; lin[3] = 0;
                        ((ObjVecFn)ovt[0x88 / 8])(o, lin); ((ObjVecFn)ovt[0x90 / 8])(o, zero);
                    }
                    else ((ObjVecFn)ovt[0x90 / 8])(o, zero);
                    f->py = q[1];
                    // Zombie: Objekt lebt, hat aber keinen Render-Knoten (unsichtbar) -> aufgeben, neu setzen
                    if (tpc && f->age > 0.5f && !f->manual)
                    {   u8 *nd = nodeat(tpc, npc, nsc, f->slot);
                        if (!nd) f->nomiss += dt; else f->nomiss = 0.0f;
                        if (f->nomiss > 0.6f)
                        {   ltime(); lstr("Flagge "); ldec(i); lstr(" ohne Knoten (unsichtbar) - neu setzen, Slot "); ldec(f->slot); lstr("\r\n"); lflush();
                            *(unsigned short *)(o + 0x120) = 0x4000; f->state = 0; f->obj = 0; f->armed = 1; f->cool = 0.3f; nzomb++; continue;
                        }
                    }
                }
            }
            if (f->state == 2)
            {
                f->t += dt;
                *(float *)(o + 0xB0) = (q[0] + cx) * 0.5f; *(float *)(o + 0xB4) = (q[2] + cz) * 0.5f;
                if (f->t > 10.0f && *(unsigned short *)(o + 0x120) == 0) *(unsigned short *)(o + 0x120) = 0x4000;
            }
            if ((d2 > 270.0f * 270.0f || ex2 > 86000.0f) && !f->manual)
            {   // weit weg: Objekt dem Spiel zum Ausblenden ueberlassen und die Flagge komplett freigeben (kein halb unsichtbares Objekt behalten)
                *(unsigned short *)(o + 0x120) = 0x4000; ghost(f, 8.0f); f->state = 0; f->obj = 0; f->armed = 1; f->cool = 1.5f; nfar++; continue;
            }
        }
        else if (!f->manual)
        {
            if (d2 > 270.0f * 270.0f || ex2 > 86000.0f) f->armed = 1;
            if (f->armed && d2 < 220.0f * 220.0f) { if (f->cool > 0.0f) wcool++; else if (spawned_now >= 24) wcap++; }
            if (f->armed && f->cool <= 0.0f && d2 < 240.0f * 240.0f && ex2 < 78000.0f && spawned_now < 24)
            {
                u8 *o = 0; int dd = pickdef(f->x, f->z);
                if (!dd) continue;
                if (ghostnear(f->x, f->z)) { f->cool = 0.5f; wghost++; continue; }
                { int sd = defid; defid = dd; quiet = 1; rzx = f->fx; rzz = f->fz; o = spawn(f->x, f->y, f->z); quiet = 0; defid = sd; }
                if (o)
                {   int j;
                    *(float *)(o + 0xB0) = (f->x + cx) * 0.5f; *(float *)(o + 0xB4) = (f->z + cz) * 0.5f; *(unsigned short *)(o + 0x120) = 0;
                    for (j = 0; j < NF + 8; j++) if (j != i && fs[j].state && (fs[j].obj == o || fs[j].slot == lastslot))
                    {   lstr("Slot-Konflikt: Flagge "); ldec(j); lstr(" verliert Objekt (Slot "); ldec(lastslot); lstr(") an Flagge "); ldec(i); lstr("\r\n"); lflush();
                        fs[j].state = 0; fs[j].obj = 0; fs[j].armed = 1; fs[j].cool = 1.0f; nconf++;
                    }
                    f->py = f->y; f->obj = o; f->slot = lastslot; f->id = 0xFFFF0000u | (u32)dd; f->state = 1; f->armed = 0; f->fade = 0.05f; f->fdur = (d2 < 160.0f * 160.0f) ? 1.4f : 0.5f; f->sy = f->y; f->age = 0.0f; f->chk = 0; f->hx = f->x; f->hz = f->z; f->settled = 0; f->nomiss = 0.0f; setalpha(o, 0.05f); nspawn++; spawned_now++;
                    { u8 *bo2 = *(u8 **)(o + 8); ltime(); lstr("Spawn Flagge "); ldec(i); lstr(" slot "); ldec(lastslot); lstr(" obj "); lhex((u64)o, 12); lstr(" koerper "); lhex((u64)bo2, 12); lstr("\r\n"); }
                    { float sd = fsqrt(d2); nspd++; spdsum += sd; if (sd < spdmin) spdmin = sd; if (sd < 150.0f) { wlate++; lstr("Spaet-Spawn Flagge "); ldec(i); lstr(" bei "); ldec((int)sd); lstr(" m\r\n"); lflush(); } }
                    if (freeok) { buildfree(); if (isfree(lastslot)) { freeok = 0; lstr("Freiliste unzuverlaessig (belegter Slot steht darin) - abgeschaltet\r\n"); lflush(); } }
                }
                else { f->cool = 3.0f; lstr("Auto-Spawn fehlgeschlagen\r\n"); lflush(); if (++fails >= 8) { off = 1; lstr("zu viele Spawn-Fehler, PropTest schaltet sich ab\r\n"); lflush(); return; } }
            }
        }
    }
}

static void status(float dt)
{
    int i, T = 0, A = 0, W = 0; float d2;
    stat_t += dt; if (stat_t < STATSEC || !nearflag()) return; stat_t = 0;
    for (i = 0; i < NF; i++) { d2 = dist2(fs[i].x, fs[i].z); if (d2 < 190.0f * 190.0f) { T++; if (fs[i].state) A++; else if (fs[i].armed) W++; } }
    lstr("Status: in 190 m "); ldec(T); lstr(" Flaggen, aktiv "); ldec(A); lstr(", wartend "); ldec(W); lstr(", gespawnt gesamt "); ldec(nspawn); lstr(", weg gesamt "); ldec(nweg); lstr(", Ausblenden abgefangen "); ldec(nsaved); lstr(", Spawn-Abstand mittel "); ldec(nspd ? (int)(spdsum / nspd) : 0); lstr(" min "); ldec((int)(nspd ? spdmin : 0)); lstr(" spaet "); ldec(wlate); lstr(" Wartezeit cool "); ldec(wcool); lstr(" ghost "); ldec(wghost); lstr(" limit "); ldec(wcap); lstr(", Verschleppt "); ldec(nverschl); lstr(", Konflikte "); ldec(nconf); lstr(" Einpendeln "); ldec(nsettle); lstr(" Zombies "); ldec(nzomb); lstr(" fern-frei "); ldec(nfar); lstr(" Freiliste "); lstr(freeok ? "an" : "AUS"); lstr(" Treffer "); ldec(nfreehit); lstr(", Auto "); lfloat(cx); lch(' '); lfloat(cz); lstr(mgrok() ? "" : " (KEIN MANAGER)");
    {   u8 *tp = findtp(); int nn = 0, nbad = 0, nmov = 0, nonce = 0, nclose = 0;
        if (tp)
        {   u64 np = cntvec(tp, 0xFB78), ns = cntvec(tp, 0xFBC0); u8 **nb = *(u8 ***)(tp + 0xF630), **ne = *(u8 ***)(tp + 0xF638);
            for (i = 0; i < NF; i++)
            {   Fl *f = &fs[i]; u8 *node; u64 idx; float *q;
                if (!f->state || !alive(f)) continue;
                idx = np + ns + (u64)f->slot; node = (nb && ne > nb && idx < (u64)(ne - nb)) ? nb[idx] : 0; nn++;
                if (!node || !readable(node + 0x20, 0x40)) { nbad++; continue; }
                q = (float *)(f->obj + 0xA0);
                { float *p2 = (float *)(node + 0x50); float ex = p2[0] - q[0], ey = p2[1] - q[1], ez = p2[2] - q[2]; if (ex * ex + ey * ey + ez * ez >= 36.0f) { nbad++; continue; } }
                if (((*(u32 *)(node + 0x18)) >> 16 & 0xff) == 1) nonce++; else nmov++;
                if (dist2(f->x, f->z) < 60.0f * 60.0f && ((*(u32 *)(node + 0x18)) >> 16 & 0xff) == 1) nclose++;
            }
            lstr(" | Knoten ok "); ldec(nn - nbad); lstr("/"); ldec(nn); lstr(" nie aktualisiert "); ldec(nonce); lstr(" aktualisiert "); ldec(nmov); lstr(" davon nahe nie-akt. "); ldec(nclose); lstr(" gestreamt "); ldec((int)ns);
            {   u8 **mb = *(u8 ***)(tp + 0xF618), **me = *(u8 ***)(tp + 0xF620); u8 *md = (mb && me > mb && 395 < (me - mb)) ? mb[395] : 0;
                { static const int dm[8] = {395, 397, 399, 393, 386, 376, 381, 388}; int q2; lstr(" | geladen:"); for (q2 = 0; q2 < 8; q2++) if (modelloaded(dm[q2])) { lch(' '); ldec(dm[q2]); } }
                lstr(" | Modell395 "); lstr((md && readable(md, 0x28) && *(u64 *)(md + 8)) ? "geladen" : "NICHT geladen"); lstr(" Modellvektor "); ldec(mb ? (int)(me - mb) : 0);
                lstr(" Knotenvektor "); ldec(nb ? (int)(ne - nb) : 0); lstr(" platziert "); ldec((int)np);
                if (nbad) { int sh = 0; for (i = 0; i < NF && sh < 2; i++) { Fl *f = &fs[i]; u64 idx; u8 *node; if (!f->state || !alive(f)) continue; idx = np + ns + (u64)f->slot; node = (nb && ne > nb && idx < (u64)(ne - nb)) ? nb[idx] : 0;
                    if (!node) { lstr(" [F"); ldec(i); lstr(" slot "); ldec(f->slot); lstr(" idx "); ldec((int)idx); lstr(" KEIN KNOTEN]"); sh++; }
                    else if (!readable(node + 0x20, 0x40)) { lstr(" [F"); ldec(i); lstr(" Knoten unlesbar]"); sh++; }
                    else { float *p2 = (float *)(node + 0x50), *q2 = (float *)(f->obj + 0xA0); float ex = p2[0] - q2[0], ez = p2[2] - q2[2]; if (ex * ex + ez * ez >= 36.0f) { lstr(" [F"); ldec(i); lstr(" Knoten woanders dx "); lfloat(ex); lstr(" dz "); lfloat(ez); lstr("]"); sh++; } } } }
            }
        }
        else lstr(" | Presentation fehlt");
    }
    lstr("\r\n"); lflush();
}

static int vec_ok(u8 *q, u32 at, u32 elem)
{
    u64 v[3]; int i;
    if (!readable(q + at, 24)) return 0;
    for (i = 0; i < 3; i++) v[i] = *(u64 *)(q + at + 8 * i);
    return v[0] > 0x10000 && v[1] > v[0] && v[2] >= v[1] && (v[1] - v[0]) % elem == 0 && (v[1] - v[0]) < (1ULL << 32);
}
static int valid_tp(u8 *q) { if ((u64)q < 0x10000 || ((u64)q & 7)) return 0; return vec_ok(q, 0xFB78, 0x70) && vec_ok(q, 0xF618, 8) && vec_ok(q, 0xF630, 8); }
static u8 *findtp(void)
{
    u8 *pres, *real; int o, k; static const int offs[3] = {0x38, 0x20, 0x28};
    if (!mgr || !readable(mgr, 0x20)) return 0;
    pres = *(u8 **)(mgr + 0x10);
    if (!pres || !readable(pres, 0x40)) return 0;
    for (k = 0; k < 3; k++)
    {
        real = *(u8 **)(pres + offs[k]);
        if (real && readable(real, 0x800) && (*(u64 *)real == (u64)(g + 0x2EB7BA0) || *(u64 *)real == (u64)(g + 0x2EBC148)))
        {
            for (o = 0; o < 0x800; o += 8) { u8 *q = *(u8 **)(real + o); if (valid_tp(q)) return q; }
        }
    }
    return 0;
}
static u64 cntvec(u8 *tp, u32 off) { u8 *b = *(u8 **)(tp + off), *e = *(u8 **)(tp + off + 8); return (b && e > b) ? (u64)(e - b) / 0x70 : 0; }

static void dumpflags(void)
{
    int i, n = 0;
    u8 *tp = findtp(); u64 np = 0, ns = 0;
    if (tp) { np = cntvec(tp, 0xFB78); ns = cntvec(tp, 0xFBC0); }
    lstr(tp ? "Presentation gefunden, platziert " : "Presentation NICHT gefunden, platziert "); ldec((int)np); lstr(" gestreamt "); ldec((int)ns); lstr("\r\n");
    ltime(); lstr("=== Flaggen-Dump, Auto "); lfloat(cx); lch(' '); lfloat(cy); lch(' '); lfloat(cz); lstr(" ===\r\n");
    for (i = 0; i < NF + 8; i++)
    {
        Fl *f = &fs[i]; float d2 = dist2(f->x, f->z);
        if (d2 > 160.0f * 160.0f) continue;
        n++;
        lstr("F"); ldec(i); lstr(" abstand "); ldec((int)fsqrt(d2)); { float ddx = f->x - cx, ddz = f->z - cz; float fw = ddx * gx + ddz * gz, rt = ddx * -gz + ddz * gx; lstr(" (vor "); lfloat(fw); lstr(" rechts "); lfloat(rt); lstr(")"); } lstr(" soll "); lfloat(f->x); lch(' '); lfloat(f->y); lch(' '); lfloat(f->z);
        lstr(" zustand "); ldec(f->state); lstr(" armed "); ldec(f->armed);
        if (f->state && alive(f))
        {
            float *q = (float *)(f->obj + 0xA0);
            lstr(" ist "); lfloat(q[0]); lch(' '); lfloat(q[1]); lch(' '); lfloat(q[2]);
            lstr(" fade "); lhex(*(unsigned short *)(f->obj + 0x120), 4); lstr(" st148 "); ldec(*(int *)(f->obj + 0x148)); lstr(" slot "); ldec(f->slot);
            if (tp)
            {
                u8 **nb = *(u8 ***)(tp + 0xF630), **ne = *(u8 ***)(tp + 0xF638); u64 idx = np + ns + (u64)f->slot; u8 *node = (nb && ne > nb && idx < (u64)(ne - nb)) ? nb[idx] : 0;
                int nok = 0;
                if (node && readable(node + 0x20, 0x40)) { float *np2 = (float *)(node + 0x50); float ex = np2[0] - q[0], ey = np2[1] - q[1], ez = np2[2] - q[2]; nok = (ex * ex + ey * ey + ez * ez) < 36.0f; }
                lstr(node ? (nok ? " NODE ok" : " NODE woanders") : " NODE keiner");
                if (node && readable(node, 0x100) && (i >= NF || d2 < 30.0f * 30.0f))
                {   int w; lstr(" RAW"); for (w = 0; w < 64; w++) { lch(' '); lhex(((u32 *)node)[w], 8); } }
                {   u8 **mb = *(u8 ***)(tp + 0xF618), **me = *(u8 ***)(tp + 0xF620); u8 *md = (mb && me > mb && 395 < (me - mb)) ? mb[395] : 0;
                    lstr((md && readable(md, 0x28) && *(u64 *)(md + 8)) ? " MODELL geladen" : " MODELL NICHT geladen"); }
            }
        }
        lstr("\r\n");
    }
    lstr("=== Ende ("); ldec(n); lstr(") ===\r\n"); lflush();
}

static void addmanual(float x, float y, float z)
{
    int i; u8 *o;
    if (!mgrok()) { lstr("kein Manager (F6)\r\n"); lflush(); return; }
    for (i = NF; i < NF + 8; i++) if (!fs[i].state) break;
    if (i == NF + 8) i = NF;
    rzx = gx; rzz = gz; o = spawn(x, y, z);
    if (o) { fs[i].obj = o; fs[i].slot = lastslot; fs[i].id = 0xFFFF0000u | (u32)defid; fs[i].x = x; fs[i].y = y; fs[i].z = z; fs[i].py = y; fs[i].state = 1; fs[i].manual = 1; fs[i].fade = 1.0f; fs[i].hx = x; fs[i].hz = z; fs[i].settled = 1; }
}

__declspec(dllexport) int FH2PluginInitialize(void *ctx)
{
    if (!ctx || *(u32 *)ctx < 0x18 || *(u32 *)((u8 *)ctx + 0xc) != 0x3a18000) return 0;
    g = *(u8 **)((u8 *)ctx + 0x10); checkcode();
    { int i; for (i = 0; i < NF; i++) { fs[i].x = FP[i][0]; fs[i].y = FP[i][1]; fs[i].z = FP[i][2]; fs[i].fx = FP[i][3]; fs[i].fz = FP[i][4]; fs[i].armed = 1; } for (i = NF; i < NF + 8; i++) { fs[i].manual = 1; fs[i].x = 1e9f; } }
        lstr(BANNER); lstr(" (Code "); lstr(codeok ? "ok" : "ABWEICHEND"); lstr(")\r\n"); lflush();
    return 1;
}

__declspec(dllexport) void FH2PluginVehicleUpdate(void *frame)
{
    static const int vk[8] = {0x75, 0x76, 0x77, 0x78, 0x79, 0x7a, 0x7b, 0x23};
    static int prev[8]; static float px, pz, hx = 1.0f, hz; static int have;
    u8 *c; float x, y, z, dt, dx, dz, d; int k;
    if (!g || !frame) return;
    c = *(u8 **)((u8 *)frame + 8);
    if (!c || !readable(c, 0x5d40)) return;
    { u64 vt = *(u64 *)(c + 0x20); if (vt != (u64)(g + 0x2c5ce28) && vt != (u64)(g + 0x2e6b348)) return; }
    dt = *(float *)((u8 *)frame + 0x14);
    if (!(dt > 0.0f && dt < 0.25f)) return;
    x = *(float *)(c + 0x4f50); y = *(float *)(c + 0x4f54); z = *(float *)(c + 0x4f58);
    if (!(x > -20000.0f && x < 20000.0f && z > -20000.0f && z < 20000.0f)) return;
    if (have) { dx = x - px; dz = z - pz; d = fsqrt(dx * dx + dz * dz); if (d / dt > 3.0f) { hx = dx / d; hz = dz / d; } }
    px = x; pz = z; have = 1; cx = x; cy = y; cz = z; gx = hx; gz = hz;
    maintain(dt); status(dt);
    for (k = 0; k < (RELEASE ? 0 : 8); k++)
    {
        int down = (GetAsyncKeyState(vk[k]) & 0x8000) ? 1 : 0;
        if (down && !prev[k])
        {
            if (k == 0) findmgr();
            else if (k == 1) addmanual(x + hx * 8.0f, y + 0.5f, z + hz * 8.0f);
            else if (k == 7) dumpflags();
            else if (k == 5) { defid += 10; if (defid > 260) defid = 1; lstr("Definition jetzt "); ldec(defid); lstr("\r\n"); lflush(); }
            else if (k == 6) { defid--; if (defid < 1) defid = 260; lstr("Definition jetzt "); ldec(defid); lstr("\r\n"); lflush(); }
            else if (k == 2) { defid++; if (defid > 260) defid = 1; lstr("Definition jetzt "); ldec(defid); lstr("\r\n"); lflush(); }
            else if (k == 4) nearby();
            else addmanual(10069.5f, 15.9f, -7841.0f);
        }
        prev[k] = down;
    }
}
