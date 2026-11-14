#include "putty.h"
#ifdef _WIN32_WCE
#include <aygshell.h>
#endif
//include "StdAfx.h"
//include "porting.h"

void logwevent(void *frontend, const wchar_t *string) {
    char *astring = unicode2ansi(string);
    logevent(frontend, astring);
    sfree(astring);
}

BOOL TextOut(HDC hdc, int nXStart, int nYStart, LPCWSTR lpwString, int wcbString) {
    RECT *rect = snew(RECT);
    int result;
    rect->left = nXStart;
    rect->top = nYStart;
    // Aleq: ooops! Nasty temporary workaroung
    rect->right = 0;
    rect->bottom = 0;

    // Aleq: Possibly DT_NOCLIP usage?
    rect->bottom = rect->top + DrawText(hdc, lpwString, wcbString, rect, DT_CALCRECT);
    result = DrawText(hdc, lpwString, wcbString, rect, DT_LEFT);
    sfree(rect);

    return (result);
}

HFONT CreateFont(int nHeight, int nWidth, int nEscapement, int nOrientation, int fnWeight,
  DWORD fdwItalic, DWORD fdwUnderline, DWORD fdwStrikeOut, DWORD fdwCharSet, 
  DWORD fdwOutputPrecision, DWORD fdwClipPrecision, DWORD fdwQuality, DWORD fdwPitchAndFamily,
  LPCTSTR lpszFace) {
    LOGFONTW lfw;

    lfw.lfHeight = nHeight;
    lfw.lfWidth = nWidth;
    lfw.lfEscapement = nEscapement;
    lfw.lfOrientation = nOrientation;
    lfw.lfWeight = fnWeight;
    lfw.lfItalic = (BYTE) fdwItalic;
    lfw.lfUnderline = (BYTE) fdwUnderline;
    lfw.lfStrikeOut = (BYTE) fdwStrikeOut;
    lfw.lfCharSet = (BYTE) fdwCharSet;
    lfw.lfOutPrecision = (BYTE) fdwOutputPrecision;
    lfw.lfClipPrecision = (BYTE) fdwClipPrecision;
    lfw.lfQuality = (BYTE) fdwQuality;
    lfw.lfPitchAndFamily = (BYTE) fdwPitchAndFamily;
    _tcsncpy(lfw.lfFaceName, lpszFace, lenof(lfw.lfFaceName));

    return (CreateFontIndirect(&lfw));
}

BOOL GetKeyboardState(PBYTE lpKeyState) {
    int vk;
    for (vk = 0; vk <= 0xFF; vk++)
	lpKeyState[vk] = (BYTE) GetKeyState(vk);
    return TRUE;
}

int ToAsciiEx(UINT uVirtKey, UINT uScanCode, PBYTE lpKeyState, LPWORD lpChar, UINT uFlags, HKL dwhkl) {

    WORD chvalue;
    chvalue = MapVirtualKey(uVirtKey, 2);

    if (!chvalue)
	return (0);
    lpChar[0] = chvalue;
    if (chvalue & 0x80000000)
	return (2);
    else
	return (1);
}

LONG RegCreateKey(HKEY hKey, LPCTSTR lpSubKey, PHKEY phkResult) {
    DWORD dwDisp;
    return (RegCreateKeyEx(hKey, lpSubKey, 0, NULL, 0, 0, NULL, phkResult, &dwDisp));
}

LONG RegOpenKey(HKEY hKey, LPCTSTR lpSubKey, PHKEY phkResult) {
    return (RegOpenKeyEx(hKey, lpSubKey, 0, 0, phkResult));
}

LONG RegEnumKey(HKEY hKey, DWORD dwIndex, LPTSTR lpName, DWORD cchName) {
    return (RegEnumKeyEx(hKey, dwIndex, lpName, &cchName, 0, NULL, NULL, NULL));
}


char *unicode2ansi(const wchar_t *source) {
    char *result = snewn(wcslen(source) + 1, char);
    result[wcslen(source)] = '\0';
    wcstombs(result, source, wcslen(source));
    return (result);
}

wchar_t *ansi2unicode(const char *source) {
    wchar_t *result = snewn(strlen(source) + 1, wchar_t);
    result[strlen(source)] = L'\0';
    mbstowcs(result, source, strlen(source));
    return (result);
}

BOOL remove(TCHAR *filename) {
    //Aleq
    // TODO: Implementovat
    // ...
    // ...
    // ...
    // ...
    // ...
    // ...
    // ...
    // ...
    // ...
    // ...
    return (TRUE);
}

#if (UNDER_CE < 400 && _WIN32_WCE != 211)
//
// Pocket PC 2000, 2002 Implementations
// 

BOOL CheckDlgButton(HWND hwnd, int nIDButton, UINT nCheck) {
    SendMessage(GetDlgItem(hwnd, nIDButton), BM_SETCHECK, (WPARAM) nCheck, (LPARAM) 0);
    return TRUE;
}


UINT IsDlgButtonChecked(HWND hwnd, int nIDButton) {
    return (UINT) SendMessage(GetDlgItem(hwnd, nIDButton), BM_GETCHECK, (WPARAM) 0, (LPARAM) 0);
}

//
// Pocket PC 2000, 2002 Implementation END
//
#endif

#if defined(_WIN32_WCE) && _WIN32_WCE == 211
/* EVC 4's CE 4.x import libraries refer to CRT helpers absent from CE 2.11. */
int __cdecl _strnicmp(const char *a, const char *b, size_t n)
{
    unsigned char ca, cb;
    while (n--) {
        ca = (unsigned char)*a++;
        cb = (unsigned char)*b++;
        if (ca >= 'A' && ca <= 'Z') ca += 'a' - 'A';
        if (cb >= 'A' && cb <= 'Z') cb += 'a' - 'A';
        if (ca != cb || !ca) return (int)ca - (int)cb;
    }
    return 0;
}

size_t strspn(const char *s, const char *accept)
{
    const char *p;
    size_t n = 0;
    while (*s) {
        for (p = accept; *p && *p != *s; p++) {}
        if (!*p) break;
        s++;
        n++;
    }
    return n;
}

unsigned long strtoul(const char *s, char **end, int base)
{
    unsigned long value = 0;
    int negative = 0, digit, any = 0;
    while (*s == ' ' || (*s >= '\t' && *s <= '\r')) s++;
    if (*s == '+' || *s == '-') { negative = (*s == '-'); s++; }
    if ((base == 0 || base == 16) && s[0] == '0' &&
        (s[1] == 'x' || s[1] == 'X')) { s += 2; base = 16; }
    if (base == 0) base = (*s == '0') ? 8 : 10;
    if (base < 2 || base > 36) { if (end) *end = (char *)s; return 0; }
    for (;;) {
        unsigned char c = (unsigned char)*s;
        if (c >= '0' && c <= '9') digit = c - '0';
        else if (c >= 'a' && c <= 'z') digit = c - 'a' + 10;
        else if (c >= 'A' && c <= 'Z') digit = c - 'A' + 10;
        else break;
        if (digit >= base) break;
        value = value * (unsigned long)base + (unsigned long)digit;
        any = 1;
        s++;
    }
    if (end) *end = (char *)(any ? s : (negative ? s - 1 : s));
    return negative ? 0UL - value : value;
}

int __cdecl _isctype(int c, int mask)
{
    int alpha = (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z');
    int digit = c >= '0' && c <= '9';
    int space = c == ' ' || (c >= '\t' && c <= '\r');
    int upper = c >= 'A' && c <= 'Z';
    int lower = c >= 'a' && c <= 'z';
    int punct = c >= 33 && c <= 126 && !alpha && !digit;
    int control = c >= 0 && c < 32;
    int blank = c == ' ' || c == '\t';
    int hex = digit || (c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f');
    return ((mask & _ALPHA) && alpha) || ((mask & _DIGIT) && digit) ||
           ((mask & _SPACE) && space) || ((mask & _UPPER) && upper) ||
           ((mask & _LOWER) && lower) || ((mask & _PUNCT) && punct) ||
           ((mask & _CONTROL) && control) || ((mask & _BLANK) && blank) ||
           ((mask & _HEX) && hex);
}

int __cdecl _XcptFilter(unsigned long code, struct _EXCEPTION_POINTERS *info)
{
    (void)code;
    (void)info;
    return EXCEPTION_CONTINUE_SEARCH;
}

/* Pocket PC shell extensions are not part of the Jornada HPC ROM. */
HWND WINAPI GetDesktopWindow(void)
{
    /* The Jornada ROM does not export this CE 3.x API. Dialogs can still
     * open without desktop-relative centering. */
    return NULL;
}

BOOL WINAPI SHCreateMenuBar(SHMENUBARINFO *info)
{
    (void)info;
    return FALSE;
}

BOOL WINAPI SHInitDialog(PSHINITDLGINFO info)
{
    (void)info;
    return TRUE;
}
#endif
