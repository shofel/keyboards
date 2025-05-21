{ pkgs, lib, config, inputs, ... }:

{
  # https://devenv.sh/packages/
  packages = [ pkgs.qmk ];

  # To install qmk udev rules, add to your nixos configuration:
  # hardware.keyboard.qmk.enable = true;

  env = {
    QMK_TAG = "0.28.6";
  };

  enterShell = /* sh */ ''
    # assert qmk util # does this check work?
    if ! qmk >/dev/null; then
      echo "FAIL: qmk error"
      exit 2
    fi

    source <(qmk env)
    export QMK_FIRMWARE

    source <(qmk env)
    if test ! -d "$QMK_FIRMWARE"; then
      gh repo clone qmk/qmk_firmware "$QMK_FIRMWARE" -- \
        --branch "$QMK_TAG" \
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
