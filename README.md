# cog - cli ed-like text editor

This is a simple command line text editor I have written in the Rust language. It allows you to view, edit, and savetext files from the command line. (just as how ed works)

## Usage

- `a` command allows you to add text to the file you are editing.
- `n` command displays all the contents of the file with line numbering.
- `d <line number>` allows you to delete a specific text using their line number.
- `q` quits the text editor.

## Installing

1. Build the binary using Cargo:
```shell
cargo build --release
```

2. Run the `cog` text editor, specifying a text file as a command line argument:
```shell
./target/release/cog sample/textfile.TXT
```

3. Use the above commands to interact with the file.

4. After making changes, save the changes back to the original file by quitting the editor with `q` command.

## Dependencies

This project uses the `colored` crate for line number styling. It is in the Manifest file and will be installed by default when you build the binary.

## License
This project is released under the Unlicense. You are free to use this code for any purpose without any restrictions.

## Author

- John Gitahi
