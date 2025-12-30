{ pkgs, lib, config, inputs, ... }:

{
  # https://devenv.sh/packages/
  packages = [ pkgs.qmk ];

  # To install qmk udev rules, add to your nixos configuration:
  # hardware.keyboard.qmk.enable = true;

  env = {
    S_QMK_TAG = "0.31.1";
    S_QMK_FIRMWARE = "/tmp/qmk_firmware";
  };

  enterShell = /* sh */ ''
    qmk config user.overlay_dir=$(pwd)
    qmk config user.qmk_home="$S_QMK_FIRMWARE"

    echo QMK config:
    qmk config -ro user
    echo

    source <(qmk env)
    if test ! -d "$S_QMK_FIRMWARE"; then
      gh repo clone qmk/qmk_firmware "$S_QMK_FIRMWARE" -- \
        --branch "$S_QMK_TAG" \
        --filter=blob:none --depth=1
      qmk setup -y
    fi
  '';

  # Keymap drawer # https://github.com/caksoylar/keymap-drawer
  languages.python.enable = true;
  languages.python.poetry.enable = true;
  languages.python.poetry.activate.enable = true;
  languages.python.poetry.install.enable = true;
}
