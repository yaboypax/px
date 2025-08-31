# px_Audio
px_Audio is a header only DSP library based on the STB header-only library structure. It's meant to be dropped into a C or C++ project and just work. Individual DSP objects are written in respective header files within the Source/ folder, and the CompileToSingleHeader.sh bash script "compiles" (really just copy and pastes) each header into the single px_Audio.h header. This makes dropping the entire library into a project easy without sacrificing the organizaton required to develop or peruse objects individually.

## Helper Objects

- px_globals
- px_memory
- px_vector
- px_converter

## DSP Objects

- px_buffer
- px_biquad
- px_equalizer
- px_saturator
- px_compressor
- px_delay
- px_clip
  

>[!WARNING]
>This library is a work in progress, and mostly used for internal development.
