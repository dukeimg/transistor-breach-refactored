# transistor-breach-refactored
А small utility that will help you to listen and record .bank of Transistor or any other game. Finally refactored!

Forget about a crazy setup process — now everything works out-of-box.

All glitches and error of original transistor-breach are carefully saved.

*For educational purposes only*

## Usage

Steps to set up:

1. Get yourself a licensed copy of Transistor (http://store.steampowered.com/app/237930/Transistor) 
2. Copy contents of `<Transistror Path>/Content/Audio/AudenFMOD/Build/Desktop` to `<Project Path>/assets/sounds`
3. Copy `libFModPlugins.dylib` to `<Project Path>/assets/plugins`
    - If you work under Linux/Windows `libFModPlugins.*` will have different extension. In this case you'll have to 
    change filename in `main.cpp:197`
  
4. Compile!

## Credits

I want to give a big thanks to Steven La (@[stevenla](https://github.com/stevenla "Steven La")) for inspiration and Matthew Ready and his [blog](https://craxic.com/transistor-sound-ripper/ "Transistor Sound Ripper") for giving me the actual working example. 

No files of Transistor are used in this repository. Respect other people's rights. If you're looking just for the music, buy an [official soundtrack](https://supergiantgames.bandcamp.com/album/transistor-original-soundtrack "Transistor Oficial Soundtrack") 