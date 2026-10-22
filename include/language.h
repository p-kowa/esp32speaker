#pragma once

#define LANG_DE 1
#define LANG_EN 2
#define LANG_PL 3

#ifndef UI_LANG
#define UI_LANG LANG_DE
#endif

#if UI_LANG == LANG_DE
    #include "languages/language_de.h"
#elif UI_LANG == LANG_EN
    #include "languages/language_en.h"
#elif UI_LANG == LANG_PL
    #include "languages/language_pl.h"
#else
    #error "Unknown language"
#endif
