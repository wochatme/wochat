#ifndef __WT_DUI_RENDER_H__
#define __WT_DUI_RENDER_H__

#include "wochatypes.h"

#ifdef __cplusplus
extern "C" {
#endif

/* fill the whole screen with one color */
int DUI_ScreenClear(uint32_t* dst, uint32_t size, uint32_t color);

int DUI_ScreenDrawRect(uint32_t* dst, int w, int h, uint32_t* src, int sw, int sh, int dx, int dy);

int DUI_ScreenDrawRectRound(uint32_t* dst, int w, int h, uint32_t* src, int sw, int sh, int dx, int dy, uint32_t color0, uint32_t color1);

int DUI_ScreenFillRect(uint32_t* dst, int w, int h, uint32_t color, int sw, int sh, int dx, int dy);

int DUI_ScreenFillRectRound(uint32_t* dst, int w, int h, uint32_t color, int sw, int sh, int dx, int dy, uint32_t c1, uint32_t c2);

#ifdef __cplusplus
}
#endif

#endif // __WT_DUI_RENDER_H__