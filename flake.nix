{
  description = "A flake for building LibreVNA-GUI";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
  };

  outputs = { self, nixpkgs }:
    let
      inherit (nixpkgs) lib;
      systems = [ "x86_64-linux" "aarch64-linux" ];
      forAllSystems = lib.genAttrs systems;
      nixpkgsFor = nixpkgs.legacyPackages;
    in
    {
      packages = forAllSystems (system: {
        default = with nixpkgsFor.${system}; stdenv.mkDerivation rec {

          pname = "LibreVNA-GUI";
          version = "1.6.0";
          locale = "UTF-8";
          allLocales = "UTF-8";

          outputs = [ "out" "udev" ];

          src = fetchFromGitHub {
            owner = "jankae";
            repo = "LibreVNA";
            rev = "v${version}";
            hash = "sha256-BzSFRvAvh8mBxtfpxvKW93PIaXG0XMg0wpWFO75N6zo=";
          };

          nativeBuildInputs = [ qt6.wrapQtAppsHook ];

          buildInputs = [ qt6.qtbase pkgs.libusb1 pkgs.git pkgs.udev ];

          preConfigure = ''
            export LOCALE_ARCHIVE="${glibcLocales}/lib/locale/locale-archive"
            export LANG="en_US.UTF-8"
            export LC_ALL="en_US.UTF-8"
            export QT_SELECT=qt6
          '';

          buildPhase = ''
            runHook preBuild
            cd Software/PC_Application/LibreVNA-GUI
            qmake LibreVNA-GUI.pro
            make -j $NIX_BUILD_CORES
            runHook postBuild
          '';

          installPhase = ''
            runHook preInstall
            mkdir -p $out/bin
            cp LibreVNA-GUI $out/bin/
            mkdir -p $udev/lib/udev/rules.d
            cp ../51-vna.rules $udev/lib/udev/rules.d/
            runHook postInstall
          '';

          meta = with lib; {
            homepage = "https://github.com/jankae/LibreVNA/";
            description = "100kHz to 6GHz 2 port USB based VNA";
            mainProgram = "LibreVNA-GUI";
            license = licenses.gpl3;
            platforms = platforms.linux;
          };
        };
      });
      checks = forAllSystems
        (system:
          let
            packages = lib.mapAttrs' (n: lib.nameValuePair "package-${n}") self.packages.${system};
          in
          packages
        );

      apps =
        forAllSystems
          (system: {
            LibreVNA-GUI = {
              type = "app";
              program = "${self.packages.${system}.default}/bin/LibreVNA-GUI";
            };
            default = self.apps.${system}.LibreVNA-GUI;
          });
    };
}






