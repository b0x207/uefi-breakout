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
          cmake
          qemu
          tree
          sdl3
        ];

        OVMF_ROOT = pkgs.OVMFFull.fd.outPath;
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
