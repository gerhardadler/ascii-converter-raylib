#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "font_metrics.h"
#include "grow_string.h"
#include "parse_args.h"
#include "raylib.h"

typedef struct CommandLineArguments {
    char* inImagePath;
    char* fontPath;
    char* outImagePath;
    char* outSvgPath;
    int fontSize;
    float perPixelWeight;
    int colCount;
    bool wiggleGlyph;
    float wiggleGlyphCost;
} CommandLineArguments;

typedef struct FontMember {
    char fontChar;
    Rectangle rec;
    GlyphInfo glyph;
    float value;
    Rectangle fullRec;
} FontMember;

typedef struct FontInformation {
    int fontSize;
    float cssFontSize;
    int glyphWidth;
    int glyphHeight;
    Color* pixels;
    Image atlas;
    unsigned int memberCount;
    FontMember* members;
} FontInformation;

Color BlendColors(Color color1, Color color2, float color1Weight) {
    float color2Weight = 1.0f - color1Weight;
    Color out = {
        .r = (unsigned char)(color1.r * color1Weight + color2.r * color2Weight),
        .g = (unsigned char)(color1.g * color1Weight + color2.g * color2Weight),
        .b = (unsigned char)(color1.b * color1Weight + color2.b * color2Weight),
        .a = 255};
    return out;
}

int parseArguments(CommandLineArguments* commandLineArguments, int argc,
                   char* argv[]) {
    // default arguments
    commandLineArguments->inImagePath = NULL;
    commandLineArguments->fontPath = NULL;
    commandLineArguments->outImagePath = NULL;
    commandLineArguments->outSvgPath = NULL;
    commandLineArguments->fontSize = 16;
    commandLineArguments->perPixelWeight = 5.0f;
    commandLineArguments->colCount = 50;
    commandLineArguments->wiggleGlyph = false;
    commandLineArguments->wiggleGlyphCost = 0.04f;

    args_option_t options[] = {
        ARGS_OPTION("-i", "--input-path", ARGTYPE_STRING,
                    &commandLineArguments->inImagePath),
        ARGS_OPTION("-fp", "--font-path", ARGTYPE_STRING,
                    &commandLineArguments->fontPath),
        ARGS_OPTION("-oi", "--out-image-path", ARGTYPE_STRING,
                    &commandLineArguments->outImagePath),
        ARGS_OPTION("-osvg", "--out-svg-path", ARGTYPE_STRING,
                    &commandLineArguments->outSvgPath),
        ARGS_OPTION("-fs", "--font-size", ARGTYPE_INT,
                    &commandLineArguments->fontSize),
        ARGS_OPTION("-ppw", "--per-pixel-weight", ARGTYPE_FLOAT,
                    &commandLineArguments->perPixelWeight),
        ARGS_OPTION("-c", "--columns", ARGTYPE_INT,
                    &commandLineArguments->colCount),
        ARGS_OPTION("-wc", "--wiggle-cost", ARGTYPE_FLOAT,
                    &commandLineArguments->wiggleGlyphCost),
        ARGS_FLAG("-w", "--wiggle", &commandLineArguments->wiggleGlyph),
        ARGS_END_OF_OPTIONS};

    // Parse all arguments and convert to target types
    if (parse_arguments(argc, argv, options) < 0) {
        printf("Error parsing arguments:\n%s\n",
               parse_arguments_error_string());
        return -1;
    }

    return 0;
}

Color GetAverageColorInSection(Color* imagePixels, int imageWidth,
                               Rectangle* rect) {
    long sumR = 0;
    long sumG = 0;
    long sumB = 0;
    int pixelCount = (int)(rect->height * rect->width);
    for (float y = 0; y < rect->height; y++) {
        for (float x = 0; x < rect->width; x++) {
            int yCoordinate = (int)(y + rect->y);
            int xCoordinate = (int)(x + rect->x);

            Color color = imagePixels[yCoordinate * imageWidth + xCoordinate];
            sumR += color.r;
            sumG += color.g;
            sumB += color.b;
        }
    }
    Color out = {.r = (unsigned char)(sumR / pixelCount),
                 .g = (unsigned char)(sumG / pixelCount),
                 .b = (unsigned char)(sumB / pixelCount),
                 .a = 255};
    return out;
}

float GetFontAverageValue(Color* fontPixels, int imageWidth, Rectangle* rect) {
    long sumA = 0;
    int pixelCount = (int)(rect->height * rect->width);
    for (float y = 0; y < rect->height; y++) {
        for (float x = 0; x < rect->width; x++) {
            int yCoordinate = (int)(y + rect->y);
            int xCoordinate = (int)(x + rect->x);

            Color color = fontPixels[yCoordinate * imageWidth + xCoordinate];
            sumA += color.a;
        }
    }

    long averageA = sumA / pixelCount;
    float averageValue = (float)averageA / 255.0f;
    return averageValue;
}

Color GetFontAverageColor(Color* fontPixels, int imageWidth, Rectangle* rect,
                          Color foregroundColor, Color backgroundColor) {
    float averageValue = GetFontAverageValue(fontPixels, imageWidth, rect);
    return BlendColors(foregroundColor, backgroundColor, averageValue);
}

float PerPixelDifference(Color* image1Pixels, Color* image2Pixels,
                         int image1Width, int image2Width, Rectangle* rect1,
                         Rectangle* rect2) {
    double sumDifference = 0;
    double pixelCount = rect1->width * rect1->height;
    for (int y = 0; y < (int)rect1->height; y++) {
        for (int x = 0; x < (int)rect1->width; x++) {
            int y1Coordinate = y + (int)rect1->y;
            int x1Coordinate = x + (int)rect1->x;
            int y2Coordinate = y + (int)rect2->y;
            int x2Coordinate = x + (int)rect2->x;

            Color image1Color =
                image1Pixels[y1Coordinate * image1Width + x1Coordinate];
            Color image2Color =
                image2Pixels[y2Coordinate * image2Width + x2Coordinate];

            float image1Value = ColorToHSV(image1Color).z;
            float image2Value = (float)image2Color.a / 255.0f;
            sumDifference += fabs(image1Value - image2Value);
        }
    }
    float out = (float)(sumDifference / pixelCount);
    assert(out >= 0 && out <= 1);
    return out;
}

float PerPixelDifferenceOffset(Color* image1Pixels, Color* image2Pixels,
                               int image1Width, int image2Width,
                               Rectangle* rect1, Rectangle* rect2,
                               float offsetCost) {
    float leastDifference = 999.0f;
    for (int y = -1; y <= 1; y++) {
        for (int x = -1; x <= 1; x++) {
            Rectangle offsetRect = *rect2;
            offsetRect.x += (float)x;
            offsetRect.y += (float)y;
            float diff =
                PerPixelDifference(image1Pixels, image2Pixels, image1Width,
                                   image2Width, rect1, &offsetRect);
            diff += (float)(abs(y) + abs(x)) * offsetCost;
            if (diff < leastDifference) {
                leastDifference = diff;
            }
        }
    }
    assert(leastDifference >= 0 && leastDifference <= 1);
    return leastDifference;
}

void ExportSvg(char* path, char* selectedChars, int colCount, int rowCount,
               FontInformation fontInformation) {
    String svg;
    string_init(&svg, 1024 * 5);  // 5kB
    string_append_fmt(&svg,
                      "<svg xmlns=\"http://www.w3.org/2000/svg\" "
                      "width=\"%d\" height=\"%d\" "
                      "style=\"font-family: 'Fira Code'; font-weight: "
                      "bold; font-size: %f; "
                      "background-color: black; "
                      "font-variant-ligatures: none;\">",
                      fontInformation.glyphWidth * colCount,
                      fontInformation.glyphHeight * rowCount,
                      fontInformation.cssFontSize);
    string_append(
        &svg, "<text x=\"0\" y=\"0\" xml:space=\"preserve\" fill=\"white\">");
    for (int y = 0; y < rowCount; y++) {
        string_append_fmt(&svg,
                          "<tspan x=\"0\" dy=\"%d\" textLength=\"%d\" "
                          "lengthAdjust=\"spacingAndGlyphs\">",
                          fontInformation.glyphHeight,
                          colCount * fontInformation.glyphWidth);
        for (int x = 0; x < colCount; x++) {
            char selectedChar = (char)selectedChars[y * colCount + x];
            if (selectedChar == '<') {
                string_append(&svg, "&lt;");
            } else if (selectedChar == '>') {
                string_append(&svg, "&gt;");
            } else if (selectedChar == '&') {
                string_append(&svg, "&amp;");
            } else if (selectedChar == '"') {
                string_append(&svg, "&quot;");
            } else if (selectedChar == '\'') {
                string_append(&svg, "&#39;");
            } else {
                string_append_char(&svg, selectedChar);
            }
        }
        string_append(&svg, "</tspan>");
    }
    string_append(&svg, "</text></svg>");
    if (string_write_to_file(&svg, path) != 0) {
        perror("svg write failed");
    }
    string_free(&svg);
}

int loadFont(FontInformation* fontInformation, int fontSize, char* fontPath) {
    // Load font file into memory
    unsigned char* fontData = NULL;
    fontInformation->fontSize = fontSize;

    FILE* fontFile = fopen(fontPath, "rb");
    if (!fontFile) {
        printf("Error loading font file\n");
        return 1;
    }

    // Get file size
    fseek(fontFile, 0, SEEK_END);
    long dataSize = ftell(fontFile);
    if (dataSize < 0) {
        printf("Error getting file size\n");
        return 1;
    }
    fseek(fontFile, 0, SEEK_SET);

    // Read file into memory
    fontData = (unsigned char*)malloc((size_t)dataSize);
    if (!fontData) {
        printf("Error allocating font buffer\n");
        fclose(fontFile);
        return 1;
    }

    size_t readBytes = fread(fontData, 1, (size_t)dataSize, fontFile);
    fclose(fontFile);
    if (readBytes != (size_t)dataSize) {
        printf("Error reading font file\n");
        free(fontData);
        return 1;
    }

    // Define which characters you want
    int fontChars[] = {
        32,  33,  34,  35,  36,  37,  38,  39,  40,  41,  42,  43,  44,  45,
        46,  47,  48,  49,  50,  51,  52,  53,  54,  55,  56,  57,  58,  59,
        60,  61,  62,  63,  64,  65,  66,  67,  68,  69,  70,  71,  72,  73,
        74,  75,  76,  77,  78,  79,  80,  81,  82,  83,  84,  85,  86,  87,
        88,  89,  90,  91,  92,  93,  94,  95,  96,  97,  98,  99,  100, 101,
        102, 103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 115,
        116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126};
    fontInformation->memberCount = sizeof(fontChars) / sizeof(fontChars[0]);

    // Load font data
    GlyphInfo* glyphs = LoadFontData(
        fontData, (int)dataSize, fontInformation->fontSize, fontChars,
        (int)fontInformation->memberCount, FONT_DEFAULT);

    if (!glyphs) {
        printf("Error loading font data\n");
        free(fontData);
        return 1;
    }

    // measurements
    fontInformation->glyphWidth = glyphs[0].advanceX;  // For monospace
    fontInformation->glyphHeight = fontSize;

    FontMetrics fm = GetFontMetrics(fontData);
    fontInformation->cssFontSize = (float)fontInformation->fontSize *
                                   (float)fm.unitsPerEm /
                                   (float)(fm.ascender - fm.descender);

    // Generate image atlas from glyph data
    Rectangle* recs = NULL;
    fontInformation->atlas = GenImageFontAtlas(
        glyphs, &recs, (int)fontInformation->memberCount,
        fontInformation->fontSize, fontInformation->fontSize, 0);
    if (fontInformation->atlas.data == NULL) {
        printf("Error generating font atlas\n");
        UnloadFontData(glyphs, (int)fontInformation->memberCount);
        free(fontData);
        return 1;
    }

    fontInformation->pixels = LoadImageColors(fontInformation->atlas);

    fontInformation->members =
        malloc(fontInformation->memberCount * sizeof(FontMember));
    // for normalizing
    float maxValue = 0;
    for (unsigned int i = 0; i < fontInformation->memberCount; i++) {
        FontMember* fontMember = &fontInformation->members[i];
        fontMember->fontChar = (char)fontChars[i];
        fontMember->glyph = glyphs[i];
        fontMember->rec = recs[i];
        fontMember->fullRec =
            (Rectangle){.x = recs[i].x - (float)glyphs[i].offsetX,
                        .y = recs[i].y - (float)glyphs[i].offsetY,
                        .width = (float)fontInformation->glyphWidth,
                        .height = (float)fontInformation->glyphHeight};
        fontMember->value = GetFontAverageValue(fontInformation->pixels,
                                                fontInformation->atlas.width,
                                                &fontMember->fullRec);
        if (fontMember->value > maxValue) {
            maxValue = fontMember->value;
        }
    };

    // normalize FontMember values
    for (unsigned int i = 0; i < fontInformation->memberCount; i++) {
        fontInformation->members[i].value *= 1.0f / maxValue;
    }

    UnloadFontData(glyphs, (int)fontInformation->memberCount);
    free(recs);
    free(fontData);

    return 0;
}

typedef struct {
    float averageDifference;
    int index;
} AverageDifferenceMember;

int AverageDistanceMemberCmpAsc(const void* a, const void* b) {
    const AverageDifferenceMember *x = a, *y = b;
    return (y->averageDifference < x->averageDifference) -
           (y->averageDifference > x->averageDifference);
}

AverageDifferenceMember* getAverageDifferenceSorted(
    float averageValue, FontInformation fontInformation) {
    AverageDifferenceMember* averageDifferenceMembers =
        malloc(sizeof(AverageDifferenceMember) * fontInformation.memberCount);

    // find closest value
    for (int i = 0; i < (int)fontInformation.memberCount; i++) {
        FontMember fontMember = fontInformation.members[i];
        float averageDifferance = fabsf(fontMember.value - averageValue);
        assert(averageDifferance >= 0 && averageDifferance <= 1);

        averageDifferenceMembers[i].index = i;
        averageDifferenceMembers[i].averageDifference = averageDifferance;
    }
    qsort(averageDifferenceMembers, fontInformation.memberCount,
          sizeof *averageDifferenceMembers, AverageDistanceMemberCmpAsc);
    return averageDifferenceMembers;
}

int main(int argc, char* argv[]) {
    CommandLineArguments commandLineArguments;
    if (parseArguments(&commandLineArguments, argc, argv) != 0) {
        return -1;
    }

    FontInformation fontInformation;
    if (loadFont(&fontInformation, commandLineArguments.fontSize,
                 commandLineArguments.fontPath) != 0) {
        return 1;
    }

    // Save the atlas for inspection
    ExportImage(fontInformation.atlas, "font_atlas.png");
    printf("\nMonospace character size: %dx%d\n", fontInformation.glyphWidth,
           fontInformation.glyphHeight);

    Image image = LoadImage(commandLineArguments.inImagePath);
    if (image.data == NULL) {
        printf("Error loading image\n");
        return 1;
    }

    // resize image to fit colCount*charWidth
    int newWidth = commandLineArguments.colCount * fontInformation.glyphWidth;
    float newHeight =
        (float)image.height * ((float)newWidth / (float)image.width);
    ImageResize(&image, newWidth, (int)newHeight);

    Color* imagePixels = LoadImageColors(image);

    int rowCount = image.height / fontInformation.glyphHeight;

    char* selectedChars =
        malloc((size_t)rowCount * (size_t)commandLineArguments.colCount *
               sizeof(char));

    Image debugImage = GenImageColor(image.width, image.height, BLACK);

    clock_t startComputationTime = clock();

    for (int y = 0; y < rowCount; y++) {
        for (int x = 0; x < commandLineArguments.colCount; x++) {
            Rectangle imageSection = {(float)(x * fontInformation.glyphWidth),
                                      (float)(y * fontInformation.glyphHeight),
                                      (float)fontInformation.glyphWidth,
                                      (float)fontInformation.glyphHeight};

            Color averageColor = GetAverageColorInSection(
                imagePixels, image.width, &imageSection);

            Vector3 averageHSV = ColorToHSV(averageColor);

            int closestGlyph = 0;
            float closestGlyphDelta = 999.0f;

            // by sorting the average difference, we can stop calculating when
            // finding closer glyphs with perPixelDifference is impossible
            AverageDifferenceMember* averageDifferanceMembers =
                getAverageDifferenceSorted(averageHSV.z, fontInformation);

            // find closest value
            for (int i_ = 0; i_ < (int)fontInformation.memberCount; i_++) {
                int i = averageDifferanceMembers[i_].index;
                float glyphDelta =
                    averageDifferanceMembers[i_].averageDifference;

                if (glyphDelta > closestGlyphDelta) {
                    break;
                }

                FontMember fontMember = fontInformation.members[i];

                if (commandLineArguments.perPixelWeight != 0) {
                    float perPixelDifferance;
                    if (commandLineArguments.wiggleGlyph) {
                        perPixelDifferance = PerPixelDifferenceOffset(
                            imagePixels, fontInformation.pixels, image.width,
                            fontInformation.atlas.width, &imageSection,
                            &fontMember.fullRec,
                            commandLineArguments.wiggleGlyphCost);
                    } else {
                        perPixelDifferance = PerPixelDifference(
                            imagePixels, fontInformation.pixels, image.width,
                            fontInformation.atlas.width, &imageSection,
                            &fontMember.fullRec);
                    }
                    glyphDelta += perPixelDifferance *
                                  commandLineArguments.perPixelWeight;
                }

                if (glyphDelta < closestGlyphDelta) {
                    closestGlyph = i;
                    closestGlyphDelta = glyphDelta;
                }

                // export character
                // Image glyphImage = ImageFromImage(fontAtlas, fontRect);
                // char filename[11] = "test/$.png";
                // filename[5] = fontChars[i];
                // ExportImage(glyphImage, filename);
            }
            free(averageDifferanceMembers);

            FontMember closestFontMember =
                fontInformation.members[closestGlyph];
            selectedChars[y * commandLineArguments.colCount + x] =
                closestFontMember.fontChar;
            ImageDraw(&debugImage, fontInformation.atlas,
                      closestFontMember.fullRec, imageSection, WHITE);
        }
    }

    clock_t endComputationTime = clock();

    printf(
        "Calculation time: %f seconds\n",
        (double)(endComputationTime - startComputationTime) / CLOCKS_PER_SEC);

    if (commandLineArguments.outSvgPath != NULL) {
        ExportSvg(commandLineArguments.outSvgPath, selectedChars,
                  commandLineArguments.colCount, rowCount, fontInformation);
    }
    if (commandLineArguments.outImagePath != NULL) {
        ExportImage(debugImage, commandLineArguments.outImagePath);
    }

    // Cleanup TODO

    return 0;
}
