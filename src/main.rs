use std::fs;
use std::io::{self, Write};
use std::env;
extern crate colored;
use colored::*;

// a buffer which expands/reduces as needed
struct AdaptiveBuffer {
    buffer: Vec<String>,
    buffer_size: usize,
    expansion_factor: f64,
    shrinkage_threshold: f64,
}

impl AdaptiveBuffer {
    fn new(initial_size: usize, expansion_factor: f64, shrinkage_threshold: f64) -> Self {
        Self {
            buffer: Vec::with_capacity(initial_size),
            buffer_size: initial_size,
            expansion_factor,
            shrinkage_threshold,
        }
    }

    fn expand_buffer(&mut self) {
        let new_size = (self.buffer_size as f64 * self.expansion_factor) as usize;
        self.buffer.reserve_exact(new_size - self.buffer_size);
        self.buffer_size = new_size;
    }

    fn shrink_buffer(&mut self) {
        let new_size = (self.buffer_size as f64 * self.shrinkage_threshold) as usize;
        self.buffer.shrink_to_fit();
        self.buffer_size = new_size;
    }

    fn insert_line(&mut self, index: usize, content: String) {
        if self.buffer.len() + 1 > self.buffer_size {
            self.expand_buffer();
        }
        self.buffer.insert(index, content);
    }

    fn delete_line(&mut self, index: usize) {
        self.buffer.remove(index);
        if self.buffer_size > (self.shrinkage_threshold * self.buffer.len() as f64) as usize {
            self.shrink_buffer();
        }
    }

    fn display_buffer(&self) {
        for (line_num, line) in self.buffer.iter().enumerate() {
            println!("{} {}", line_num.to_string().cyan().italic(), line);
        }
    }
}

fn main() {
    let args: Vec<String> = env::args().collect();
    if args.len() < 2 {
        eprintln!("Enter a filename as a cli argument.");
        std::process::exit(100);
    }

    let filename = &args[1];

    let mut buffer = AdaptiveBuffer::new(10, 500.0, 0.5);

    // Read file into the buffer
    if let Ok(contents) = fs::read_to_string(filename) {
        buffer.buffer = contents.lines().map(|s| s.to_owned()).collect();
    }

    loop {
        print!("ksi> ");
        io::stdout().flush().unwrap();
        let mut input = String::new();
        io::stdin().read_line(&mut input).unwrap();

        let command: Vec<&str> = input.trim().splitn(2, ' ').collect();
        match command[0] {
            "q" => break,
            "n" => {
                buffer.display_buffer();
            }
            "a" => {
                let mut new_lines = Vec::new();
                loop {
                    let mut line = String::new();
                    // add option to show number you are in
                    io::stdin().read_line(&mut line).unwrap();
                    if line.trim() == "." {
                        break;
                    }
                    new_lines.push(line.trim().to_owned());
                }
                buffer.insert_line(buffer.buffer.len(), new_lines.join("\n"));
            }
            "d" => {
                if let Some(index) = command.get(1).and_then(|s| s.parse::<usize>().ok()) {
                    buffer.delete_line(index - 1);
                } else {
                    println!("Invalid command. Usage: d <line number>");
                }
            }
            _ => println!("Invalid command"),
        }
    }

    // Save changes back to the file
    if let Err(err) = fs::write(filename, buffer.buffer.join("\n")) {
        eprintln!("Failed to save changes: {}", err);
    }
}

