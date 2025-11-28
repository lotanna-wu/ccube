# CCube

A 3D rotating cube rendered in the terminal.

## Compile

```bash
gcc ccube.c -o ccube -lm
```

Or use the Makefile:

```bash
make
```

## Usage

```bash
./ccube [options]
```

### Options

- `-h` - Display help message
- `-b` - Use bold characters
- `-n` - Enable nested mode (smaller cube inside the larger one)
- `-R` - Enable rainbow mode (color cycling)
- `-s` - Enable screensaver mode (terminates on any keypress)
- `-c [color]` - Set cube color
- `-C [character]` - Set cube character
- `-d [0-10]` - Set animation delay (default: 5, lower = faster)

### Examples

```bash
# Default cube
./ccube

# Rainbow cube with bold characters
./ccube -R -b

# Fast red cube
./ccube -c red -d 1

# Slow cyan cube
./ccube -c cyan -d 8

# Rainbow cube with * characters
./ccube -R -C *
```

## Controls

- `Ctrl+C` - Exit the program

## Compatibility

- **Linux/macOS**: Full support (macOS colors might be weird)
- **Windows**:
