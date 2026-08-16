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