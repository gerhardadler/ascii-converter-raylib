#define _POSIX_C_SOURCE 200809L

typedef struct {
    int unitsPerEm;
    int ascender;
    int descender;
} FontMetrics;

FontMetrics GetFontMetrics(unsigned char* fontData);
