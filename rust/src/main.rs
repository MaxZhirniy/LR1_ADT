mod array;
mod commands;
mod doubly_list;
mod forward_list;
mod stack_queue;
mod storage;

use std::env;
use std::io::{self, Write};
use std::path::Path;
use storage::Database;

fn run_once(db: &mut Database, path: &Path, query: &str) -> Result<(), String> {
    let (output, modified) = commands::run(db, query)?;
    if modified { storage::save(path, db)?; }
    if !output.is_empty() { println!("{output}"); }
    Ok(())
}
fn execute() -> Result<(), String> {
    let mut path = String::from("file.data");
    let mut query: Option<String> = None;
    let mut args = env::args().skip(1);
    while let Some(arg) = args.next() {
        match arg.as_str() {
            "--file" | "-file" => path = args.next().ok_or("missing --file value")?,
            "--query" | "-query" => query = Some(args.next().ok_or("missing --query value")?),
            "--help" | "-h" => {
                println!("dbms-rust [--file file.data] [--query 'MPUSH a 1']");
                return Ok(());
            }
            _ => return Err(format!("unknown argument: {arg}")),
        }
    }
    let path = Path::new(&path);
    let mut db = storage::load(path)?;
    if let Some(q) = query { return run_once(&mut db, path, &q); }
    println!("DBMS (Rust). Type HELP for commands, EXIT to finish.");
    let stdin = io::stdin();
    loop {
        print!("> ");
        io::stdout().flush().map_err(|e| e.to_string())?;
        let mut line = String::new();
        if stdin.read_line(&mut line).map_err(|e| e.to_string())? == 0 { break; }
        if line.trim().eq_ignore_ascii_case("EXIT") { break; }
        if let Err(e) = run_once(&mut db, path, &line) { eprintln!("ERROR: {e}"); }
    }
    Ok(())
}
fn main() {
    if let Err(e) = execute() { eprintln!("ERROR: {e}"); std::process::exit(1); }
}

#[cfg(test)]
mod tests {
    use super::*;
    use crate::doubly_list::DoublyList;
    use crate::forward_list::ForwardList;
    use crate::stack_queue::{Queue, Stack};

    #[test]
    fn singly_linked_insert_delete_and_find() {
        let mut list = ForwardList::default();
        list.insert(0, 10).unwrap();
        list.insert(1, 30).unwrap();
        list.insert(1, 20).unwrap();
        assert_eq!(list.values(false), vec![10,20,30]);
        assert_eq!(list.find(20), Some(1));
        assert_eq!(list.erase(1), Ok(20));
        assert_eq!(list.values(true), vec![30,10]);
    }
    #[test]
    fn doubly_linked_previous_next_and_reuse() {
        let mut list = DoublyList::default();
        for i in 0..50 { list.insert(i as usize, i).unwrap(); }
        for _ in 0..40 { list.erase(0).unwrap(); }
        for i in 0..40 { list.insert(i, -1).unwrap(); }
        assert_eq!(list.len(), 50);
        assert_eq!(list.values(false)[40..].to_vec(), (40..50).collect::<Vec<_>>());
        assert_eq!(list.values(true)[..10].to_vec(), (40..50).rev().collect::<Vec<_>>());
    }
    #[test]
    fn stack_lifo_and_queue_fifo_with_wraparound() {
        let mut stack = Stack::default();
        let mut queue = Queue::default();
        for i in 0..100 { stack.push(i); queue.push(i); }
        for i in 0..60 { assert_eq!(queue.pop(), Ok(i)); }
        for i in 100..180 { queue.push(i); }
        for i in 60..180 { assert_eq!(queue.pop(), Ok(i)); }
        for i in (0..100).rev() { assert_eq!(stack.pop(), Ok(i)); }
        assert!(queue.pop().is_err());
        assert!(stack.pop().is_err());
    }
    #[test]
    fn command_errors_and_persistence_roundtrip() {
        let mut db = Database::new();
        assert_eq!(commands::run(&mut db, "CREATE M values").unwrap().0, "OK");
        assert!(commands::run(&mut db, "MPUSH values 10").is_ok());
        assert!(commands::run(&mut db, "MINSERT values 0 5").is_ok());
        assert_eq!(commands::run(&mut db, "MREAD values").unwrap().0, "[5 10]");
        assert!(commands::run(&mut db, "MGET values 10").is_err());
        let path = std::env::temp_dir().join(format!("nstu_lr1_{}.data", std::process::id()));
        storage::save(&path, &db).unwrap();
        let mut read = storage::load(&path).unwrap();
        assert_eq!(commands::run(&mut read, "MREAD values").unwrap().0, "[5 10]");
        std::fs::remove_file(path).unwrap();
    }
}
