{
  pkgs,
  ...
}:

{
  packages = [
    pkgs.platformio
  ];

  languages.cplusplus.enable = true;
  languages.go.enable = true;
  languages.javascript = {
    enable = true;

    npm.enable = true;
  };
}
