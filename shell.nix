let pkgs = import <nixpkgs> {
      config.allowUnfree = true;
    };

    osm-python = pkgs.python3.withPackages (python-packages: with python-packages; [
      pyyaml
    ]);

in pkgs.mkShell rec {
  name = "osm";
  buildInputs = with pkgs; [
    osm-python
    osmium-tool
    pkgconfig
    libosmium
    spdlog
    gflags

    # Compression
    bzip2
    zlib

    protozero  # Protocol Buffer
    expat      # XML Parser
  ];
  shellHook = ''
    export PS1="$(echo -e '\uf277') {\[$(tput sgr0)\]\[\033[38;5;228m\]\w\[$(tput sgr0)\]\[\033[38;5;15m\]} (${name}) \\$ \[$(tput sgr0)\]"
  '';
}
