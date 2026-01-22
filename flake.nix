{
  description = "chisel";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/master";
    flake-utils.url = "github:numtide/flake-utils";
  };

  outputs = { self, nixpkgs, flake-utils }:
    let
      overlay = import ./overlay.nix;
    in
    flake-utils.lib.eachDefaultSystem
      (system:
        let
          pkgs = import nixpkgs { inherit system; overlays = [ overlay ]; };
          deps = with pkgs; [
            jdk21
            mill
            # jextract-21
            lit
            scala-cli
            # llvm
            verilator
          ];
        in
        {
          legacyPackages = pkgs;
          devShells.default = pkgs.mkShell {
            buildInputs = deps;
            env = {
            #   JEXTRACT_INSTALL_PATH = pkgs.jextract-21;
            };
          };
        }
      ) // { 
        overlays.default = overlay; 
      };
}
