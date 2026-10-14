# Building 
On **Linux**, use Clang.

```sh
./code/build.sh
```

On **Windows**, use MSVC.

```bat
.\code\build.bat
```

# Demo 

A "text editor".
![Watch the demo](https://github.com/lucaraymaekers/editor/blob/luca/data/showcase_editor_260404.gif)

## Key bindings
- `[ctrl]+[b]`: trigger debug break

- `[ctrl]+[p]`: split the selected panel vertically  
- `[ctrl]+[shift]+[p]`: close the selected panel

- `[ctrl]+[n]`: makes the selected panel a text panel

- `[ctrl]+[-]`: split the selected panel horizontally

- `[ctrl]+[,]`: select the next panel  
- `[ctrl]+[shift]+[,]`: select the previous panel

- `[escape]`: quit the application

### Text panel key bindings

- `[text input]`: insert a character at the cursor (replaces the selection)
- `[ctrl]+[a]`: select all text  
- `[ctrl]+[c]`: copy selection  
- `[ctrl]+[x]`: cut selection  
- `[ctrl]+[v]`: paste from the clipboard  
- `[ctrl]+[s]`: save to `./hello.c`  
- `[ctrl]+[o]`: load from `./hello.c`

- `[left]`: move cursor left  
- `[right]`: move cursor right  
- `[shift]+[left/right]`: extend the selection
- `[ctrl]+[left]`: move cursor left by word  
- `[ctrl]+[right]`: move cursor right by word
- `[up]`: move cursor up one line  
- `[down]`: move cursor down one line
- `[home]`: move to the start of the line  
- `[end]`: move to the end of the line
- `[ctrl]+[home]`: move to the start of the text  
- `[ctrl]+[end]`: move to the end of the text
- `[page up]`: move up by one page  
- `[page down]`: move down by one page

- `[enter]`: insert a newline
- `[backspace]`: delete the character before the cursor  
- `[ctrl]+[backspace]`: delete the previous word
- `[delete]`: delete the character after the cursor  
- `[ctrl]+[delete]`: delete the next word
