# ELRS HID Bridge Architecture

                  UART
                    │
                    ▼
            CrsfDecoder
                    │
                    ▼
             RcChannelDecoder
                    │
                    ▼
              RawChannels
                    │
                    ▼
          ChannelNormalizer
                    │
                    ▼
         NormalizedChannels
                    │
                    ▼
            ChannelMapper
                    │
                    ▼
             ChannelState
                    │
                    ▼
                UsbHid