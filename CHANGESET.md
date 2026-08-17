# M3 Research Checkpoint — Receiver Wi-Fi

## Result

M3 does **not** add a receiver Wi-Fi transmission command.

Authoritative ExpressLRS 3.4.3 source review found that receiver Wi-Fi mode is entered through the ExpressLRS-specific RF/MSP control path (`MSP_ELRS_SET_RX_WIFI_MODE = 0x0E`). The FC-side CRSF UART parser does not expose an equivalent supported Wi-Fi command.

Per the project's standing rule, no command is fabricated or spoofed.

## Files

### `README.md`

Updates the BOOT-button documentation to the validated repeating 2-second menu, records M2 Bind as implemented, and adds the ExpressLRS 3.4+ Bind prerequisite with hardware validation on 3.4.3.

### `docs/Receiver-Maintenance-Protocol.md`

Records the protocol research and current disposition for Bind, Wi-Fi, and the still-unresolved Reset/Recovery slot.

## Runtime impact

None. No source files are modified in this checkpoint.

The white Wi-Fi slot remains a non-transmitting reserved selection. Bind behavior remains unchanged from validated M2.

## Recommended validation

1. Overlay the ZIP at repository root.
2. Confirm only documentation files changed.
3. Review the compatibility wording.
4. No firmware rebuild is required because runtime source is unchanged.
5. Commit/sync this documentation checkpoint if desired.

## Next decision

Return to architecture review for the Wi-Fi slot, or proceed to authoritative research on the Receiver Reset/Recovery slot before assigning another physical maintenance action.
