
#!/bin/bash

output_file="../px_audio.h"
> "$output_file" 

header_files=("px_globals.h" "px_memory.h" "px_vector.h" "px_buffer.h" "px_converter.h" "px_delay.h" "px_biquad.h" "px_saturator.h" "px_clip.h" "px_equalizer.h" "px_compressor.h")

cat "${header_files[0]}" >> "$output_file"

for file in "${header_files[@]:1}"; do
    # Extract lines between the first #ifndef and the corresponding #endif
    # Exclude lines containing #include
    sed -n '/#ifndef/,${
            /#include/d
            p
    }' "$file" >> "$output_file"   
done

git -C ../ add px_audio.h
git -C ../ commit -m "update px_audio.h"

