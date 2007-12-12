/* wutil.h */
 
#define kNormal		00000
#define kStandout	00001
#define kUnderline	00002
#define kReverse	00004
#define kBlink		00010
#define kDim		00020
#define kBold		00040

void EndWin(void);
void Exit(int exitStatus);
void SaveScreen(void);
void TTYWaitForReturn(void);
void RestoreScreen(int pressKey);
void Beep(int on);
void WAttr(WINDOW *w, int attr, int on);
void swclrtoeol(WINDOW *w);
void DrawStrAt(WINDOW *const win, int y, int x, const char *const str);
void WAddCenteredStr(WINDOW *const w, int y, const char *const str);
int PrintDimensions(int);
int InitWindows(void);
