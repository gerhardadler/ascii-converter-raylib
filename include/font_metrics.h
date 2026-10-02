typedef struct {
    int unitsPerEm;
    int ascender;
    int descender;
} FontMetrics;

FontMetrics GetFontMetrics(unsigned char* fontData);
