#ifndef WINMINE_SOUND_H
#define WINMINE_SOUND_H

#define TUNE_TICK      1
#define TUNE_WINGAME   2
#define TUNE_LOSEGAME  3

#define fsoundOn  3
#define fsoundOff 2

#define FSoundSwitchable() (Preferences.fSound > 1)
#define FSoundOn()         (Preferences.fSound == fsoundOn)

INT FInitTunes(void);
void PlayTune(INT tune);
void KillTune(void);
void EndTunes(void);

#endif
