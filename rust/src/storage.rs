use crate::array::DynamicArray;
use crate::doubly_list::DoublyList;
use crate::forward_list::ForwardList;
use crate::stack_queue::{Queue, Stack};
use std::collections::BTreeMap;
use std::fs::{self, File};
use std::io::{BufRead, BufReader, Write};
use std::path::Path;

pub enum Container {
    Array(DynamicArray),
    Forward(ForwardList),
    Doubly(DoublyList),
    Stack(Stack),
    Queue(Queue),
}
impl Container {
    pub fn new(kind: char) -> Result<Self, String> {
        match kind {
            'M' => Ok(Self::Array(DynamicArray::default())),
            'F' => Ok(Self::Forward(ForwardList::default())),
            'L' => Ok(Self::Doubly(DoublyList::default())),
            'S' => Ok(Self::Stack(Stack::default())),
            'Q' => Ok(Self::Queue(Queue::default())),
            'T' => Err("AVL tree is implemented in C++ only".into()),
            _ => Err("unknown structure type".into()),
        }
    }
    pub fn kind(&self) -> char {
        match self { Self::Array(_) => 'M', Self::Forward(_) => 'F',
            Self::Doubly(_) => 'L', Self::Stack(_) => 'S', Self::Queue(_) => 'Q' }
    }
    pub fn values(&self) -> Vec<String> {
        match self { Self::Array(x) => x.values(), Self::Forward(x) => x.values(false),
            Self::Doubly(x) => x.values(false), Self::Stack(x) => x.values(),
            Self::Queue(x) => x.values() }
    }
}
pub type Database = BTreeMap<String, Container>;

pub fn load(path: &Path) -> Result<Database, String> {
    let file = match File::open(path) {
        Ok(f) => f,
        Err(e) if e.kind() == std::io::ErrorKind::NotFound => return Ok(Database::new()),
        Err(e) => return Err(format!("cannot open {}: {e}", path.display())),
    };
    let mut db = Database::new();
    for (line_number, line) in BufReader::new(file).lines().enumerate() {
        let line = line.map_err(|e| e.to_string())?;
        let parts: Vec<&str> = line.split_whitespace().collect();
        if parts.is_empty() { continue; }
        if parts.len() < 3 { return Err(format!("invalid data at line {}", line_number + 1)); }
        let kind = parts[0].chars().next().ok_or("missing kind")?;
        if parts[0].len() != 1 { return Err("invalid structure kind".into()); }
        let name = parts[1];
        let count: usize = parts[2].parse().map_err(|_| "invalid count")?;
        if count > 1_000_000 || parts.len() != count + 3 { return Err("incorrect element count".into()); }
        if db.contains_key(name) { return Err("duplicate structure name".into()); }
        let mut c = Container::new(kind)?;
        for field in &parts[3..] {
            let value = field.to_string();
            match &mut c {
                Container::Array(x) => x.push(value),
                Container::Forward(x) => x.push_back(value),
                Container::Doubly(x) => x.push_back(value),
                Container::Stack(x) => x.push(value),
                Container::Queue(x) => x.push(value),
            }
        }
        db.insert(name.to_string(), c);
    }
    Ok(db)
}

pub fn save(path: &Path, db: &Database) -> Result<(), String> {
    let temp = path.with_extension(format!("{}tmp", path.extension()
        .and_then(|s| s.to_str()).map(|s| format!("{s}.")).unwrap_or_default()));
    let mut file = File::create(&temp).map_err(|e| e.to_string())?;
    for (name, c) in db {
        let mut vals = c.values();
        if c.kind() == 'S' { vals.reverse(); } // Снизу вверх: восстановление push() сохраняет вершину.
        write!(file, "{} {} {}", c.kind(), name, vals.len()).map_err(|e| e.to_string())?;
        for value in vals { write!(file, " {value}").map_err(|e| e.to_string())?; }
        writeln!(file).map_err(|e| e.to_string())?;
    }
    file.flush().map_err(|e| e.to_string())?;
    file.sync_all().map_err(|e| e.to_string())?;
    drop(file);
    fs::rename(&temp, path).map_err(|e| format!("cannot replace data file: {e}"))
}
