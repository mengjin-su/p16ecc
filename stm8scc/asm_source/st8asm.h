#ifndef _ST8ASM_H
#define _ST8ASM_H

extern "C" {
#include "define.h"
#include "common.h"
#include "asm.h"
}

#include "seg.h"

int ST8_asm (Cseg *sp, int pass);

#endif

