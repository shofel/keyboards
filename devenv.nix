{ pkgs, lib, config, inputs, ... }:

{
  # https://devenv.sh/packages/
  packages = [ pkgs.qmk pkgs.dos2unix ];

  # To install qmk udev rules, add to your nixos configuration:
  # hardware.keyboard.qmk.enable = true;

  env = {
    # Stock upstream QMK — no fork. The keymap lives at the standard userspace
    # layout path (layouts/split_3x6_3/shofel/), which pristine qmk resolves.
    # Keep this tag in sync with .github/workflows/ci.yml's qmk_ref.
    S_QMK_TAG = "0.33.13";
    # S_QMK_FIRMWARE (the upstream qmk_firmware checkout) is resolved at shell
    # entry — it defaults to $HOME/qmk_firmware, so it can't be a static path here.
  };

  scripts.setup-qmk.exec = /* sh */ ''
    : "''${S_QMK_FIRMWARE:=$HOME/qmk_firmware}"
    source <(qmk env)
    if test ! -d "$S_QMK_FIRMWARE"; then
      gh repo clone qmk/qmk_firmware "$S_QMK_FIRMWARE" -- \
        --branch "$S_QMK_TAG" \
        --filter=blob:none --depth=1
      qmk setup -y
    fi
  '';

  enterShell = /* sh */ ''
    # Upstream (stock) qmk_firmware checkout. Defaults to ~/qmk_firmware (qmk's
    # own default); override by exporting S_QMK_FIRMWARE before entering the shell
    # (e.g. in your login shell, or a gitignored devenv.local.nix / .env).
    : "''${S_QMK_FIRMWARE:=$HOME/qmk_firmware}"
    export S_QMK_FIRMWARE

    qmk config user.overlay_dir=$(pwd)
    qmk config user.qmk_home="$S_QMK_FIRMWARE"

    echo QMK config:
    qmk config -ro user
    echo

    setup-qmk
  '';

  # python3 for the tools/ scripts (compose-table generator + its test). They
  # import stdlib only, so there is nothing to resolve and no dep manager here.
  languages.python.enable = true;
}
