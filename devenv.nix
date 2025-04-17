{
  pkgs,
  ...
}:

{
  packages = [
    pkgs.platformio
    pkgs.go-task
  ];

  languages.cplusplus.enable = true;
  languages.go.enable = true;
  languages.javascript = {
    enable = true;

    npm.enable = true;
  };
}
