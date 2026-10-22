{
  description = "A very basic flake";

  inputs = {
    nixpkgs.url = "https://channels.nixos.org/nixpkgs-unstable/nixexprs.tar.zst";
  };

  outputs = inputs: {
    devShells = builtins.mapAttrs (system: pkgs: {
      default = pkgs.mkShell {
        packages = with pkgs; [
          gcc
          gnumake
          qemu
          tree
          sdl3
          OVMFFull
          compiledb
          gdb
        ];

        OVMF_ROOT = pkgs.OVMFFull.fd.outPath;
        UEFI_SHELL = "${pkgs.edk2-uefi-shell}/shell.efi";
      };
    }) inputs.nixpkgs.legacyPackages;

    packages = builtins.mapAttrs (system: pkgs: rec {
      default = qemu-runner;

      qemu-runner = pkgs.writeShellApplication {
        name = "qemu-runner.sh";

        text = ''
        '';
      };
    }) inputs.nixpkgs.legacyPackages;
  };
}
