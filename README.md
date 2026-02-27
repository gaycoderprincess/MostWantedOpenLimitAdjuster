# Most Wanted Open Limit Adjuster

Experiment for Need for Speed: Most Wanted that removes or increases certain memory limits

## Installation

- Make sure you have v1.3 of the game, as this is the only version this plugin is compatible with. (exe size of 6029312 bytes)
- Patch your game executable with the [4GB patch](https://ntcore.com/4gb-patch/).
- Plop the files into your game folder, edit `NFSMWOpenLimitAdjuster_gcp.toml` to change the options to your liking.
- Enjoy, nya~ :3

Note that this mod does not do anything without other mods that make use of the increased limits!  
Use [Extra Options](https://github.com/ExOptsTeam/NFSMWExOpts/releases/latest) to increase the amount of opponents in quick races.  
Use [Bartender](https://github.com/rng-guy/NFSMWBartender/releases/latest) and/or edit the pursuit levels VLT to increase the maximum amount of cops.  

My mods [Cwoee Chaos](https://github.com/gaycoderprincess/MostWantedChaos/releases/latest) and [Time Trial Ghosts](https://github.com/gaycoderprincess/MostWantedTimeTrialGhosts/releases/latest) can also take advantage of this mod's increased limits.

## Features

- 16 opponents in one race (requires up to `DUMMY_SKIN16` to be present in `CARS/TEXTURES.BIN`, all of them being the same resolution as `DUMMY_SKIN1`)
- Increased memory for the frame buffer, fixing issues with flickering and disappearing cop cars
- The RigidBody type has been changed to use 16-bit indices, allowing up to 32767 physics objects on the map at once
- Virtually unlimited amounts of traffic and police cars, with memory allocated on the fly (confirmed working with over 500 vehicles on the map at once)

## Building

Building is done on an Arch Linux system with CLion and vcpkg being used for the build process. 

Before you begin, clone [nya-common](https://github.com/gaycoderprincess/nya-common), [nya-common-nfsmw](https://github.com/gaycoderprincess/nya-common-nfsmw) and [CwoeeMenuLib](https://github.com/gaycoderprincess/CwoeeMenuLib) to folders next to this one, so they can be found.

Required packages: `mingw-w64-gcc`

You should be able to build the project now in CLion.
