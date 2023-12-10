# ksi - cli ed-like text editor

This is a simple command line text editor I have written in Rust. It allows you to view, edit, and save text files from the command line. (just as how ed works)

## Usage

- `a` command allows you to add text to the file you are editing.
- `n` command displays all the contents of the file with line numbering.
- `d <line number>` allows you to delete a specific text using their line number.
- `q` quits the text editor.

## Installing

1. Build the binary using Cargo:
```shell
cargo install ksi
```

2. Run the `ksi` text editor, specifying a text file as a command line argument:
```shell
ksi file.TXT
```

3. Use the above commands to interact with the file.

4. After making changes, save the changes back to the original file by quitting the editor with `q` command.

## Dependencies

This project uses the `colored` crate for line number styling. It is in the Manifest file and will be installed by default when you build the binary.

## License
This project is released under the Zlib License. Please read the LICENSE file for the full license text and terms.

## Author

- John Gitahi
