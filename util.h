#ifndef WINMINE_UTIL_H
#define WINMINE_UTIL_H

void InitConst(void);
void LoadSz(WORD id, CHAR *sz);
void ReportErr(WORD idErr);
INT Rnd(INT rndMax);
INT GetDlgInt(HWND hDlg, INT dlgID, INT numLo, INT numHi);
void DoHelp(WORD wCommand, LPARAM lParam);
void DoAbout(void);
void CheckEm(WORD idm, BOOL fCheck);
void SetMenuBar(INT fActive);

#endif
