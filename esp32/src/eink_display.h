#ifndef EINK_DISPLAY_H
#define EINK_DISPLAY_H

#include "config.h"

class EinkDisplay {
    public:
        void init();
        void hibernate();
        void refreshScreen();
};

#endif
