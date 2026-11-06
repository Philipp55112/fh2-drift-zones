// DriftZones - "Logic" plugin: merges the abort rules and the track flags into ONE Extra Options plugin.
// (The two components are compiled as separate translation units and only share the two exported entry points.)
typedef unsigned char u8; typedef unsigned int u32; typedef unsigned long DWORD; typedef void *HANDLE; typedef int BOOL;
#define WINAPI __stdcall
__declspec(dllimport) HANDLE WINAPI CreateFileA(const char *, DWORD, DWORD, void *, DWORD, DWORD, HANDLE);
__declspec(dllimport) BOOL WINAPI CloseHandle(HANDLE);
int _fltused;

int Rules_Init(void *ctx);  void Rules_Update(void *frame);
int Flags_Init(void *ctx);  void Flags_Update(void *frame);

static int rules_ok, flags_ok;

__declspec(dllexport) int FH2PluginInitialize(void *ctx)
{
    // start every game session with a fresh, empty log (CREATE_ALWAYS), so it can never accumulate
    HANDLE h = CreateFileA("DriftZonesLogic.log", 0x40000000, 3, 0, 2, 0x80, 0);
    if (h != (HANDLE)-1 && h) CloseHandle(h);
    rules_ok = Rules_Init(ctx);
    flags_ok = Flags_Init(ctx);
    return rules_ok || flags_ok;
}

__declspec(dllexport) void FH2PluginVehicleUpdate(void *frame)
{
    if (rules_ok) Rules_Update(frame);
    if (flags_ok) Flags_Update(frame);
}
