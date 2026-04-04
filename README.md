# Building 
On **Linux**, use clang.

```sh
./source/build.sh
```

On **Windows**, use msvc.

```bat
.\source\build.bat
```

# Demo 

A "text editor".
![Watch the demo](https://github.com/lucaraymaekers/editor/blob/luca/data/showcase_editor_260404.gif)

## Key binds
- `[ctrl]+[b]`: trigger debug break

- `[ctrl]+[p]`: split selected panel vertically  
- `[ctrl]+[shift]+[p]`: close selected panel

- `[ctrl]+[n]`: makes the selected panel a text panel

- `[ctrl]+[-]`: split selected panel horizontally

- `[ctrl]+[,]`: select next panel  
- `[ctrl]+[shift]+[,]`: select previous panel

- `[escape]`: quit application

- `[control]+[n]`: make selected panel a text panel 

### Text panel key binds

- `[text input]`: insert character at cursor (replaces selection)
- `[ctrl]+[a]`: select all text  
- `[ctrl]+[c]`: copy selection  
- `[ctrl]+[x]`: cut selection  
- `[ctrl]+[v]`: paste clipboard  
- `[ctrl]+[s]`: save to `./hello.c`  
- `[ctrl]+[o]`: load from `./hello.c`

- `[left]`: move cursor left  
- `[right]`: move cursor right  
- `[shift]+[left/right]`: extend selection
- `[ctrl]+[left]`: move cursor left by word  
- `[ctrl]+[right]`: move cursor right by word
- `[up]`: move cursor up one line  
- `[down]`: move cursor down one line
- `[home]`: move to start of line  
- `[end]`: move to end of line
- `[ctrl]+[home]`: move to start of text  
- `[ctrl]+[end]`: move to end of text
- `[page up]`: move up by one page  
- `[page down]`: move down by one page

- `[enter]`: insert newline
- `[backspace]`: delete character before cursor  
- `[ctrl]+[backspace]`: delete previous word
- `[delete]`: delete character after cursor  
- `[ctrl]+[delete]`: delete next word
