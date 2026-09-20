#pragma once
#include <Adafruit_GFX.h>

#include <string>
#include <vector>

#define GxEPD_BLACK 0x0000
#define GxEPD_WHITE 0xFFFF

extern std::string g_outputPrefix;
extern int g_frameIndex;

struct GxEPD2_213_GDEY0213B74
{
    static constexpr uint16_t WIDTH = 122;
    static constexpr uint16_t HEIGHT = 250;

    GxEPD2_213_GDEY0213B74(int, int, int, int) {}
};

template <typename Panel, uint16_t PageHeight>
class GxEPD2_BW : public Adafruit_GFX
{
public:
    explicit GxEPD2_BW(Panel) : Adafruit_GFX(Panel::WIDTH, Panel::HEIGHT), pixels(250 * 250, 1) {}

    void init(uint32_t, bool, uint16_t, bool) {}
    void setFullWindow() {}
    void hibernate() {}

    void firstPage()
    {
        std::fill(pixels.begin(), pixels.end(), 1);
    }

    bool nextPage()
    {
        char name[512];
        snprintf(name, sizeof(name), "%sframe_%03d.txt", g_outputPrefix.c_str(), g_frameIndex++);
        FILE* file = fopen(name, "w");

        for (int16_t y = 0; y < height(); ++y)
        {
            for (int16_t x = 0; x < width(); ++x)
            {
                fputc(pixels[y * 250 + x] ? '.' : '#', file);
            }
            fputc('\n', file);
        }

        fclose(file);
        return false;
    }

    void drawPixel(int16_t x, int16_t y, uint16_t color) override
    {
        if (x < 0 || y < 0 || x >= width() || y >= height())
        {
            return;
        }

        pixels[y * 250 + x] = (color == GxEPD_WHITE) ? 1 : 0;
    }

private:
    std::vector<uint8_t> pixels;
};
