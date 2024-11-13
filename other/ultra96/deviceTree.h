//
// Info
//
*.dts    :: Device Tree Source
*.dtb    :: Devide Tree Blob
*.dtsi   :: Device Tree Source Include

//
// Decode DeviceTreeSource from the Blob
//
dtc -I dtb -O dts -o system.dts system.dtb
//
// Code the Blob from DeveiceTreeSource
dtc -I dts -O dtb -o system.dtb system.dts

//
// When some modification to the device tree is made in petalinux
// we need to build only device tree
//
petalinux-build -c device-tree


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// EDIT
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// /home/ice/IceNET.Platform/project-spec/meta-user/recipes-bsp/device-tree/files/system-user.dtsi
//
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/include/ "system-conf.dtsi"
/ {
};

/* SD0 only supports 3.3V, no level shifter */
&sdhci0 {
   no-1-8-v;
   disable-wp;
};

/* Enable blootooth and Wi-fi */
&sdhci1 {
   bus-width= <0x4>;
   disable-wp;
   max-frequency = <50000000>;
   /delete-property/cap-power-off-card;
   mmc-pwrseq = <&sdio_pwrseq>;
   status = "okay";
   #address-cells = <1>;
   #size-cells = <0>;
   wilc_sdio@1 {
      compatible = "microchip,wilc3000", "microchip,wilc3000";
      reg = <0>;
      bus-width= <0x4>;
      status = "okay";
   };
// Remove TI child node from U96 V1 DT
/delete-node/wifi@2;
};