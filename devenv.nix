{
  pkgs,
  ...
}:

{
  packages = [
    pkgs.platformio
    pkgs.clang-tools
  ];

  languages.cplusplus.enable = true;
  languages.go.enable = true;
  languages.javascript = {
    enable = true;

    npm.enable = true;
  };

  scripts = {
    build-esp = {
      description = "Build the esp32 firmware";
      exec = ''
        cd "$DEVENV_ROOT/esp32"
        pio run
      '';
    };

    upload-esp = {
      description = "Build and upload the esp32 firmware";
      exec = ''
        cd "$DEVENV_ROOT/esp32"
        pio run --target upload
      '';
    };

    calibrate-esp = {
      description = "Upload firmware that draws rulers for sizing DISPLAY_PADDING_*";
      exec = ''
        cd "$DEVENV_ROOT/esp32"
        pio run -e calibrate --target upload
      '';
    };

    monitor-esp = {
      description = "Attach to the esp32 serial monitor";
      exec = ''
        cd "$DEVENV_ROOT/esp32"
        pio device monitor -e lolin_d32_pro -b 115200
      '';
    };

    preview-esp = {
      description = "Render the display to a PNG without flashing";
      exec = ''
        cd "$DEVENV_ROOT/esp32/preview"
        make run OUTPUT="''${1:-preview.png}"
      '';
    };

    build-backend = {
      description = "Build the backend binary";
      exec = ''
        cd "$DEVENV_ROOT/backend"
        GO111MODULE=on CGO_ENABLED=0 \
          go build -a -ldflags '-s -w' -trimpath -installsuffix cgo -o server cmd/server/main.go
      '';
    };

    build-frontend = {
      description = "Build the frontend";
      exec = ''
        cd "$DEVENV_ROOT/frontend"
        npm run build
      '';
    };
  };

  processes = {
    backend.exec = ''
      cd "$DEVENV_ROOT/backend"
      go run -race ./cmd/server
    '';

    frontend.exec = ''
      cd "$DEVENV_ROOT/frontend"
      npm run dev
    '';
  };
}
