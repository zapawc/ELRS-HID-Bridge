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
        // CrsfFrame contains a transient view into CrsfParser's
        // internal receive buffer.
        //
        // Dispatch therefore remains synchronous: the frame is
        // consumed before another byte is supplied to the parser.

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