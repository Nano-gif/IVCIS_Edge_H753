# STM32H753_Ethernet

*Written by @dabecart, 2026*

This repo is an example project designed to test the Ethernet of the [NUCLEO-H753ZI](https://www.st.com/en/evaluation-tools/nucleo-h753zi.html) board. It was developed using STM32CubeIDE v1.19.0.

Setting up Ethernet in STM32 is rather cumbersome as everything is not automatically handled by the IDE. Here is a list with all the tweaks I've had to add to make it work:

- Set ETH in RMII mode. Set the descriptor addresses well in this section. Enable the Ethernet global interrupt. Modify the default GPIO pinout to match the one used by the board.
- Enable LWIP:
  - General Settings: Disable DHCP and set an static IP, in this case `192.168.1.10`.
  - Key Options: MEM_SIZE and LWIP_RAM_HEAP_POINTER.
  - Platform Settings: LAN8742.
- In the CORTEX_M7 section:
  - Add a Cortex Memory Protection Unit.
  - Enable ICache and DCache.
- Add to the [linker](STM32H753ZITX_FLASH.ld) the arrays used by LWIP (`RxDecripSection`, `TxDecripSection`, `Rx_PoolSection`). They must match the addresses in the `.ioc` configuration.
- Enable the `RAM_D2` clock signals in software before the call to `MX_LWIP_Init`.

## LICENSE

This project is licensed under MIT License. Read the [LICENSE](LICENSE) file.