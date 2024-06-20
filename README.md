# Omnicept FaceCam

Access the IR Camera video stream on the HP Reverb G2 Omnicept Edition.

## Requirements

- [HP Reverb G2 Omnicept Edition](https://store.steampowered.com/app/1613900/HP_Reverb_G2_Omnicept_Edition/)
- [HP Omnicept Runtime](https://developers.hp.com/omnicept/downloads/hp-omnicept-runtime)

## Usage

### Starting Capture

```sh
omnicept-facecam
```

### Viewing the stream using `ffmpeg`

```sh
ffplay -protocol_whitelist file,udp,rtp omnicept-facecam.sdp
```

## Building

### Prerequisites

- [CMake](https://cmake.org/) (`winget install --id Kitware.CMake`)
- [Build Tools for Visual Studio 2022](https://visualstudio.microsoft.com/downloads/?q=build+tools#build-tools-for-visual-studio-2022) (`winget install --id Microsoft.VisualStudio.2022.BuildTools`)
- [Ninja](https://ninja-build.org/) (`winget install --id Ninja-build.Ninja`)
- [HP Omnicept SDK](https://developers.hp.com/omnicept/downloads)

### Building via the Command-line

```sh
# Configure `build` directory
cmake --preset default

# Build using Ninja
cmake --build --preset default
```

## Licence

Licenced under the [MIT License](https://choosealicense.com/licenses/mit/). See [`LICENSE.txt`](LICENSE.txt) for details.
