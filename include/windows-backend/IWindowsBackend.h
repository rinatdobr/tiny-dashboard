#pragma once

namespace TDWindows
{

class IWindowsBackend
{
public:
    virtual ~IWindowsBackend() { };
    virtual bool start() = 0;
    virtual void stop() = 0;
};

}