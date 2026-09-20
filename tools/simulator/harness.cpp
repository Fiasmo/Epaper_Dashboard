#include "Arduino.h"
#include "GxEPD2_BW.h"

SerialShim Serial;
std::string g_outputPrefix;
int g_frameIndex = 0;

void setup();

// Aufruf: sim.exe <Ausgabeordner>. Jedes gezeichnete Bild landet als frame_NNN.txt darin.
int main(int argc, char** argv)
{
    if (argc > 1)
    {
        g_outputPrefix = std::string(argv[1]) + "/";
    }

    setup();
    return 0;
}
