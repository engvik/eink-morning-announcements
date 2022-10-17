#ifndef EINK_DISPLAY_H
#define EINK_DISPLAY_H

#include <GxEPD2_BW.h>

#include "config.h"

class EinkDisplay {
    public:
        void init();
        void hibernate();
        void refreshScreen();
};

#endif
