// DriftzonenRegeln: Strassencheck und Abbruchregeln fuer die Driftzonen (FH2, Extra Options Plugin)
// Arbeitet mit den Zonen-Slots von Driftzonen.dll (RVA 0x9050, 8 Slots a 0x18: +0 Komponente, +8 aktiv, +0xc Score, +0x10 Tick)
typedef unsigned char u8; typedef unsigned int u32; typedef unsigned long long u64; typedef int BOOL; typedef unsigned long DWORD; typedef void *HANDLE;
#define WINAPI __stdcall
int _fltused;
#include "zonepaths.h"
typedef struct { void *Base, *AllocBase; DWORD AllocProt; DWORD pad; u64 Size; DWORD State, Protect, Type, pad2; } MBI;
__declspec(dllimport) u64 WINAPI VirtualQuery(const void *, MBI *, u64);
__declspec(dllimport) HANDLE WINAPI CreateFileA(const char *, DWORD, DWORD, void *, DWORD, DWORD, HANDLE);
__declspec(dllimport) BOOL WINAPI WriteFile(HANDLE, const void *, DWORD, DWORD *, void *);
__declspec(dllimport) BOOL WINAPI CloseHandle(HANDLE);
__declspec(dllimport) DWORD WINAPI SetFilePointer(HANDLE, long, long *, DWORD);
__declspec(dllimport) u64 WINAPI GetTickCount64(void);
__declspec(dllimport) void *WINAPI GetModuleHandleA(const char *);

// ---- Regeln (Werte zum Abstimmen) ----
#define STILL_KMH     5.0f   // darunter gilt das Auto als stehend
#define STILL_SEC     2.0f   // so lange stehen -> Abbruch
#define WRONG_M      30.0f   // so weit gegen die Fahrtrichtung zurueck -> Abbruch
#define CORRIDOR_M   35.0f   // weiter weg von der Zonenlinie -> gilt als neben der Strasse
#define OFF_SEC       3.0f   // so lange neben der Strasse -> Abbruch
#define JUMP_M       25.0f   // Sprung pro Frame -> Zurueckspulen/Teleport, kein Abbruch

// ---- Log ----
static char lb[8192]; static int ln; static u64 t0;
static void lch(char c) { if (ln < (int)sizeof lb - 2) lb[ln++] = c; }
static void lstr(const char *s) { while (*s) lch(*s++); }
static void ldec(int v) { char t[12]; int k = 0; if (v < 0) { lch('-'); v = -v; } if (!v) t[k++] = '0'; while (v) { t[k++] = (char)('0' + v % 10); v /= 10; } while (k) lch(t[--k]); }
static void lhex(u64 v, int d) { int i; for (i = d - 1; i >= 0; i--) lch("0123456789abcdef"[(v >> (i * 4)) & 15]); }
static void lflush(void)
{
    DWORD w; HANDLE h = CreateFileA("DriftzonenRegeln.log", 4, 3, 0, 4, 0x80, 0);
    if (h != (HANDLE)-1 && h) { SetFilePointer(h, 0, 0, 2); WriteFile(h, lb, (DWORD)ln, &w, 0); CloseHandle(h); }
    ln = 0;
}
static void lt(void) { ldec((int)((GetTickCount64() - t0) / 100)); lstr("  "); }

static float fsqrt(float x) { float r; __asm__("sqrtss %1, %0" : "=x"(r) : "x"(x)); return r; }
static float dist2(float ax, float az, float bx, float bz) { float dx = ax - bx, dz = az - bz; return dx * dx + dz * dz; }

static int readable(const void *p, u64 n)
{
    MBI m; if (!VirtualQuery(p, &m, sizeof m)) return 0;
    if (m.State != 0x1000 || !(m.Protect & 0xEE) || (m.Protect & 0x101)) return 0;
    return (u64)p + n <= (u64)m.Base + m.Size;
}

static u8 *g, *dz;          // Spiel, Driftzonen.dll
static int ok;              // 1 = Driftzonen.dll erkannt
#define SLOT(k) (dz + 0x9050 + 0x18 * (k))

// ---- Zustand der laufenden Zone ----
static int slot = -1, zone, rev, frozen, aborted, idx;
static u8 *car;
static float lx, lz, maxS, stillT, offT, segS[NP];
static int seen[16];

static float pathS(int z, int i) { return segS[POFF[z] + i]; }
static float pathLen(int z) { return segS[POFF[z] + PCNT[z] - 1]; }

// naechster Punkt der Zonenlinie, lokal um idx (lokal=1) oder global; gibt Abstand^2 zurueck
static float nearest(int z, float x, float zz, int local, int *outI)
{
    int a = 0, b = PCNT[z] - 1, i, bi = 0; float bd = 1e30f;
    if (local) { a = idx - 6; b = idx + 12; if (a < 0) a = 0; if (b > PCNT[z] - 1) b = PCNT[z] - 1; }
    for (i = a; i <= b; i++) { const float *p = PP[POFF[z] + i]; float d = dist2(p[0], p[1], x, zz); if (d < bd) { bd = d; bi = i; } }
    *outI = bi; return bd;
}
static float progress(int z, int i) { float s = pathS(z, i); return rev ? pathLen(z) - s : s; }

static void endrun(const char *why)
{
    lt(); lstr("Zone "); ldec(zone + 1); lstr(" "); lstr(why); lstr("\r\n"); lflush();
}
static float lastdd; static int lastcls[4];
static void doabort(const char *why)
{
    u8 *s = SLOT(slot);
    u8 *comp = (u8 *)*(u64 *)s;
    int called = 0; u64 vt = 0;
    // Score NICHT auf 0 setzen: der Zaehler soll einfach stehen bleiben und ausfaden.
    *(u32 *)(s + 8) = 0;          // zaehlt nicht weiter
    aborted = 1; frozen = 0;
    // Abbruch wie bei einer Tempozone: Ereignis "SpeedZoneAborted" auf der Zonen-Entitaet
    // (Spiel-Funktion g+0x6a2ae0(this); sie nutzt nur die Komponenten-Basis, this+0x60 -> Entitaet).
    if (comp && readable(comp, 0x100))
    {
        vt = *(u64 *)comp;
        if (vt > (u64)g && vt < (u64)g + 0x3a18000 && readable((void *)vt, 8))
        {
            ((void (*)(void *))(g + 0x6a2ae0))(comp);
            called = 1;
        }
    }
    lt(); lstr("ABBRUCH Zone "); ldec(zone + 1); lstr(": "); lstr(why);
    lstr(" (Abstand zur Linie "); ldec((int)lastdd); lstr(" m, Radklassen "); ldec(lastcls[0]); ldec(lastcls[1]); ldec(lastcls[2]); ldec(lastcls[3]); lstr(")"); lstr(" - Komponente vtable game+"); lhex(vt - (u64)g, 8);
    lstr(called ? " - Spiel-Abbruch ausgeloest\r\n" : " - Spiel-Abbruch NICHT ausgeloest\r\n"); lflush();
}

static void startrun(int k, float x, float z)
{
    int i, best = -1, r = 0; float bd = 80.0f * 80.0f;
    for (i = 0; i < NZ; i++)
    {
        float da = dist2(ZA[i][0], ZA[i][1], x, z), db = dist2(ZB[i][0], ZB[i][1], x, z);
        if (da < bd) { bd = da; best = i; r = 0; }
        if (db < bd) { bd = db; best = i; r = 1; }
    }
    slot = k; frozen = aborted = 0; stillT = offT = 0; lx = x; lz = z;
    if (best < 0) { zone = -1; lt(); lstr("Zone gestartet, aber keine Zone in der Naehe erkannt\r\n"); lflush(); return; }
    zone = best; rev = r;
    nearest(zone, x, z, 0, &idx); maxS = progress(zone, idx);
    lt(); lstr("Start Zone "); ldec(zone + 1); lstr(rev ? " (Rueckrichtung)" : ""); lstr(" Linie "); ldec((int)pathLen(zone)); lstr(" m\r\n"); lflush();
}

__declspec(dllexport) int FH2PluginInitialize(void *ctx)
{
    int z, i;
    if (!ctx || *(u32 *)ctx < 0x18 || *(u32 *)((u8 *)ctx + 0xc) != 0x3a18000) return 0;
    g = *(u8 **)((u8 *)ctx + 0x10); t0 = GetTickCount64();
    for (z = 0; z < NZ; z++)
    {
        float s = 0; segS[POFF[z]] = 0;
        for (i = 1; i < PCNT[z]; i++) { const float *a = PP[POFF[z] + i - 1], *b = PP[POFF[z] + i]; s += fsqrt(dist2(a[0], a[1], b[0], b[1])); segS[POFF[z] + i] = s; }
    }
    lstr("DriftzonenRegeln 1.6 init\r\n"); lflush();
    return 1;
}

static int checkdz(void)
{
    // erkennt Driftzonen.dll an ihrem Code: Startroutine setzt Slot (mov dword [rdi+0xc],0 / [rdi+8],1)
    static const u8 sig1[] = {0xc7, 0x47, 0x0c, 0, 0, 0, 0};
    static const u8 sig2[] = {0xc7, 0x47, 0x08, 1, 0, 0, 0};
    int i;
    dz = (u8 *)GetModuleHandleA("Driftzonen.dll");
    if (!dz || !readable(dz + 0x6a82, 0x20)) return 0;
    for (i = 0; i < 7; i++) if (dz[0x6a82 + i] != sig1[i] || dz[0x6a93 + i] != sig2[i]) return 0;
    return 1;
}

__declspec(dllexport) void FH2PluginVehicleUpdate(void *frame)
{
    static int tried; u8 *c; float dt, x, z, d, sp; int k, act = -1, w, off, i;
    if (!g || !frame) return;
    if (!tried) { tried = 1; ok = checkdz(); lstr(ok ? "Driftzonen.dll erkannt\r\n" : "Driftzonen.dll NICHT erkannt - Regeln aus\r\n"); lflush(); }
    if (!ok) return;
    c = *(u8 **)((u8 *)frame + 8);
    if (!c || !readable(c, 0x5d40)) return;
    { u64 vt = *(u64 *)(c + 0x20); if (vt != (u64)(g + 0x2c5ce28) && vt != (u64)(g + 0x2e6b348)) return; }
    dt = *(float *)((u8 *)frame + 0x14);
    if (!(dt > 0.0f && dt < 0.25f)) return;
    x = *(float *)(c + 0x4f50); z = *(float *)(c + 0x4f58);
    if (!(x > -20000.0f && x < 20000.0f && z > -20000.0f && z < 20000.0f)) return;

    for (k = 0; k < 8; k++) if (*(u32 *)(SLOT(k) + 8) && *(u64 *)SLOT(k)) { act = k; break; }

    // Zustand abgleichen
    if (slot >= 0)
    {
        u32 a = *(u32 *)(SLOT(slot) + 8);
        if (aborted || frozen)
        {
            // Slot steht auf 0 (von uns). Live-Anzeige des Spiels erneuert den Tick, solange die Zone laeuft.
            u64 tick = *(u64 *)(SLOT(slot) + 0x10);
            if (a) { startrun(slot, x, z); car = c; }                 // Startlinie erneut -> neuer Lauf
            else if (act >= 0 && act != slot) { startrun(act, x, z); car = c; }
            else if (GetTickCount64() - tick > 1500) { endrun(aborted ? "Ende (abgebrochen)" : "Ende (neben der Strasse)"); slot = -1; }
        }
        else if (!a)
        {
            if (act >= 0 && act != slot) { startrun(act, x, z); car = c; }
            else { endrun("Ende"); slot = -1; }
        }
    }
    else if (act >= 0) { startrun(act, x, z); car = c; }
    if (slot < 0 || zone < 0 || aborted) { lx = x; lz = z; return; }
    if (c != car) return;   // nur das Auto, mit dem die Zone gestartet wurde

    // Zurueckspulen / Teleport: Bezug neu setzen, kein Abbruch
    d = fsqrt(dist2(x, z, lx, lz));
    if (d > JUMP_M)
    {
        nearest(zone, x, z, 0, &idx); maxS = progress(zone, idx); stillT = offT = 0;
        lt(); lstr("Sprung "); ldec((int)d); lstr(" m (Zurueckspulen?) - Bezug neu\r\n"); lflush();
        lx = x; lz = z; return;
    }
    sp = d / dt * 3.6f; lx = x; lz = z;

    // 1) Stillstand
    if (sp < STILL_KMH) stillT += dt; else stillT = 0;
    if (stillT > STILL_SEC) { doabort("Stillstand"); return; }

    // 2) Fortschritt entlang der Zonenlinie / falsche Richtung
    {
        int ni; float dd = nearest(zone, x, z, 1, &ni), s;
        if (dd > CORRIDOR_M * CORRIDOR_M) { int gi; float gd = nearest(zone, x, z, 0, &gi); if (gd < dd) { dd = gd; ni = gi; } }
        idx = ni; s = progress(zone, idx);
        if (s > maxS) maxS = s;
        if (s < maxS - WRONG_M) { doabort("falsche Richtung"); return; }
        off = dd > CORRIDOR_M * CORRIDOR_M; lastdd = fsqrt(dd);
    }

    // 3) Strassencheck pro Rad: Klasse 0 Asphalt, 1 Feldweg/Schotter, 2 Gras
    for (w = 0; w < 4; w++)
    {
        u8 *r = c + 0x5d08 + 0xc * w; int cls = r[1]; lastcls[w] = r[0] == 1 ? cls : 9;
        if (r[0] != 1) continue;
        if (cls < 16 && !seen[cls]) { seen[cls] = 1; lt(); lstr("neue Untergrund-Klasse "); ldec(cls); lstr(" (Material "); lhex(*(unsigned short *)(r + 2), 4); lstr(")\r\n"); lflush(); }
        if (cls == 2) off = 1;
    }
    i = *(u32 *)(SLOT(slot) + 8);
    if (off)
    {
        offT += dt;
        if (offT > OFF_SEC) { doabort("zu lange neben der Strasse"); return; }
        if (!frozen) { frozen = 1; *(u32 *)(SLOT(slot) + 8) = 0; }   // Punkte pausieren
    }
    else
    {
        offT = 0;
        if (frozen) { frozen = 0; *(u32 *)(SLOT(slot) + 8) = 1; }   // weiter zaehlen
    }
    (void)i;
}
