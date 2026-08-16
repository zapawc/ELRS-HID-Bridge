#include "crsf_dispatcher.h"
#include "crsf_protocol.h"


void CrsfDispatcher::reset()
{
    newChannels = false;

    newLinkStatistics = false;


    crsfDevice.reset();
}


void CrsfDispatcher::dispatch(
    const CrsfFrame& frame
)
{
    switch (frame.type)
    {
        case Crsf::FRAME_RC_CHANNELS:
        {
            if (
                rcChannelDecoder.decode(
                    frame,
                    channels
                )
            )
            {
                newChannels = true;
            }

            break;
        }


        case Crsf::FRAME_LINK_STATISTICS:
        {
            if (
                linkStatisticsDecoder.decode(
                    frame,
                    linkStatistics
                )
            )
            {
                newLinkStatistics = true;
            }


            break;
        }


        case Crsf::FRAME_DEVICE_PING:
        {
            crsfDevice.handleDevicePing(
                frame
            );


            break;
        }


        case Crsf::FRAME_PARAMETER_READ:
        {
            crsfDevice.handleParameterRead(
                frame
            );


            break;
        }


        case Crsf::FRAME_PARAMETER_WRITE:
        {
            crsfDevice.handleParameterWrite(
                frame
            );


            break;
        }


        default:
        {
            // Unsupported frame types are intentionally ignored.
            //
            // CrsfParser has already established that the frame is
            // structurally valid. Understanding its semantic meaning
            // is a dispatcher/device concern.

            break;
        }
    }
}


bool CrsfDispatcher::hasNewChannels() const
{
    return
        newChannels;
}


const RawChannels&
CrsfDispatcher::getChannels() const
{
    return
        channels;
}


void CrsfDispatcher::clearNewChannels()
{
    newChannels = false;
}


bool CrsfDispatcher::hasNewLinkStatistics() const
{
    return
        newLinkStatistics;
}


const LinkStatistics&
CrsfDispatcher::getLinkStatistics() const
{
    return
        linkStatistics;
}


void CrsfDispatcher::clearNewLinkStatistics()
{
    newLinkStatistics = false;
}


bool CrsfDispatcher::hasDevicePing() const
{
    return
        crsfDevice.hasDevicePing();
}


const CrsfDevicePing&
CrsfDispatcher::getDevicePing() const
{
    return
        crsfDevice.devicePing();
}


void CrsfDispatcher::clearDevicePing()
{
    crsfDevice.clearDevicePing();
}


bool CrsfDispatcher::hasParameterRead() const
{
    return
        crsfDevice.hasParameterRead();
}


const CrsfParameterRead&
CrsfDispatcher::getParameterRead() const
{
    return
        crsfDevice.parameterRead();
}


void CrsfDispatcher::clearParameterRead()
{
    crsfDevice.clearParameterRead();
}


bool CrsfDispatcher::hasParameterWrite() const
{
    return
        crsfDevice.hasParameterWrite();
}


const CrsfParameterWrite&
CrsfDispatcher::getParameterWrite() const
{
    return
        crsfDevice.parameterWrite();
}


void CrsfDispatcher::clearParameterWrite()
{
    crsfDevice.clearParameterWrite();
}
