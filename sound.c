#include <windows.h>
#include "main.h"
#include "sound.h"
#include "pref.h"

INT FInitTunes(void)
{
    return fsoundOn;
}

void KillTune(void)
{
}

void EndTunes(void)
{
}

void PlayTune(INT tune)
{
    UNREFERENCED_PARAMETER(tune);
}
