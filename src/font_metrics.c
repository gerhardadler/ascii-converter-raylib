#include <string.h>

typedef struct {
    int unitsPerEm;
    int ascender;
    int descender;
} FontMetrics;

static unsigned short ReadU16(unsigned char* d, long o) {
    return (unsigned short)((d[o] << 8) | d[o + 1]);
}
static short ReadI16(unsigned char* d, long o) { return (short)ReadU16(d, o); }
static unsigned int ReadU32(unsigned char* d, long o) {
    return ((unsigned int)d[o] << 24) | ((unsigned int)d[o + 1] << 16) |
           ((unsigned int)d[o + 2] << 8) | (unsigned int)d[o + 3];
}

FontMetrics GetFontMetrics(unsigned char* fontData) {
    unsigned short numTables = ReadU16(fontData, 4);
    long headOffset = -1, hheaOffset = -1;
    for (int i = 0; i < numTables; i++) {
        long rec = 12 + i * 16;
        if (memcmp(fontData + rec, "head", 4) == 0)
            headOffset = ReadU32(fontData, rec + 8);
        if (memcmp(fontData + rec, "hhea", 4) == 0)
            hheaOffset = ReadU32(fontData, rec + 8);
    }
    FontMetrics m = {0};
    m.unitsPerEm = ReadU16(fontData, headOffset + 18);
    m.ascender = ReadI16(fontData, hheaOffset + 4);
    m.descender = ReadI16(fontData, hheaOffset + 6);
    return m;
}
