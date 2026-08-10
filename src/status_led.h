#pragma once

enum class SystemStatus
{
    Startup,
    Ready,
    Error
};

class StatusLed
{
public:
    void begin();
    void setStatus(SystemStatus status);

private:
    void setColor(
        unsigned char red,
        unsigned char green,
        unsigned char blue
    );
};