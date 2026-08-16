#include "crsf_decoder.h"


void CrsfDecoder::reset()
{
    parser.reset();

    dispatcher.reset();
}


void CrsfDecoder::pushByte(
    uint8_t byte
)
{
    CrsfFrame frame;


    if (
        parser.pushByte(
            byte,
            frame
        )
    )
    {
        // CrsfFrame contains a transient view into the parser's
        // internal receive buffer.
        //
        // Dispatch therefore remains synchronous.

        dispatcher.dispatch(
            frame
        );
    }
}


bool CrsfDecoder::hasNewChannels() const
{
    return
        dispatcher.hasNewChannels();
}


const RawChannels&
CrsfDecoder::getChannels() const
{
    return
        dispatcher.getChannels();
}


void CrsfDecoder::clearNewChannels()
{
    dispatcher.clearNewChannels();
}


bool CrsfDecoder::hasNewLinkStatistics() const
{
    return
        dispatcher.hasNewLinkStatistics();
}


const LinkStatistics&
CrsfDecoder::getLinkStatistics() const
{
    return
        dispatcher.getLinkStatistics();
}


void CrsfDecoder::clearNewLinkStatistics()
{
    dispatcher.clearNewLinkStatistics();
}


bool CrsfDecoder::hasDevicePing() const
{
    return
        dispatcher.hasDevicePing();
}


const CrsfDevicePing&
CrsfDecoder::getDevicePing() const
{
    return
        dispatcher.getDevicePing();
}


void CrsfDecoder::clearDevicePing()
{
    dispatcher.clearDevicePing();
}


bool CrsfDecoder::hasParameterRead() const
{
    return
        dispatcher.hasParameterRead();
}


const CrsfParameterRead&
CrsfDecoder::getParameterRead() const
{
    return
        dispatcher.getParameterRead();
}


void CrsfDecoder::clearParameterRead()
{
    dispatcher.clearParameterRead();
}


bool CrsfDecoder::hasParameterWrite() const
{
    return
        dispatcher.hasParameterWrite();
}


const CrsfParameterWrite&
CrsfDecoder::getParameterWrite() const
{
    return
        dispatcher.getParameterWrite();
}


void CrsfDecoder::clearParameterWrite()
{
    dispatcher.clearParameterWrite();
}
