/etc/systemd/system/sound.service


[Unit]
Description=Set usb sond card as default and increase mic volume

[Service]
ExecStart=pactl list short sources
ExecStart=pactl set-default-source alsa_input.usb-C-Media_Electronics_Inc._USB_Audio_Device-00.analog-mono
ExecStart=pactl set-source-volume 1 150%
Type=oneshot
RemainAfterExit=yes

[Install]
WantedBy=multi-user.target


sudo systemctl daemon-reload
sudo systemctl enable sound.service